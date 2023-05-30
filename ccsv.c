#include "ccsv.h"

#include <stdlib.h>

// "In" means Internal here.
typedef enum ccsv_InParseResult {
	// Found invalid quote character
	CCSV_IPR_BADQUOTE,

	// Finished a quoted cell.
	CCSV_IPR_QUOTEDCELL,

	// Finished a cell.
	CCSV_IPR_NORMALCELL,

	// Finished the line (encountered newline)
	// `cell` unmodified.
	CCSV_IPR_NEWLINE,

	// End-of-file (string finished)
	CCSV_IPR_EOF,

	// Error - out of memory
	CCSV_IPR_OOM,
} ccsv_InParseResult;

static ccsv_InParseResult ccsv_Cell_unquote_raw(const char *src, size_t len, ccsv_Cell *r) {
	// the biggest size we can imagine (whole string, except surrounding
	// quotes, and with a slot for the NUL character)
	const size_t max_size = len - 2 + 1;

	char *mem = malloc(max_size);
	if (mem == NULL) return CCSV_IPR_OOM;

	size_t mem_i = 0;

	// ignore first and last chars, which is a quote
	size_t read_i = 1;
	while (read_i < len) {
		if (src[read_i] == '"') {
			mem[mem_i] = '"';
			read_i += 2;
		} else {
			mem[mem_i] = src[read_i];
			read_i += 1;
		}

		mem_i += 1;
	}

	// finish with a NUL.
	mem[mem_i] = '\0';
	mem_i += 1;

	// try to realloc to a smaller, tighter size - not an issue if it errors out
	if (mem_i < max_size) {
		char *tighter_mem = realloc(mem, mem_i);
		if (tighter_mem) {
			mem = tighter_mem;
		}
	}

	r->mem = mem;
	r->len = mem_i;
	r->is_allocated = true;

	return CCSV_IPR_QUOTEDCELL;
}

static ccsv_InParseResult ccsv_Cell_parse(const char *src, size_t *index, ccsv_Cell *cell) {
	int i = *index;

	if (src[i] == '\0') return CCSV_IPR_EOF;

	// "\r\n"
	if (src[i] == '\r' && src[i + 1] == '\n') {
		*index = i + 2;
		return CCSV_IPR_NEWLINE;
	}

	// '\n' or '\r'
	if (src[i] == '\n' || src[i] == '\r') {
		*index = i + 1;
		return CCSV_IPR_NEWLINE;
	}

	bool is_quoted = src[i] == '"';
	if (is_quoted) // skip first char already
		i += 1;

	while (true) {
		char c = src[i];

		// Found escaped quote
		if (is_quoted) {
			if (c == '"' && src[i + 1] == '"') {
				i += 2;
				continue;
			}

			switch (c) {
			case '"':
				// TODO: handle text after quote finishes, but
				// before cell text finishes.
				{
					ccsv_InParseResult code =
					    ccsv_Cell_unquote_raw(&src[*index], (i - *index), cell);
					*index = i + 1;
					return code;
				}
			case '\0':
				// Oh no! Input finished before finishing quote!
				return CCSV_IPR_BADQUOTE;
			}

			i += 1;
			continue;
		}

		switch (c) {
		case '\n':
		case '\r':
		case '\0':
			cell->mem = &src[*index];
			cell->len = i - (*index);
			cell->is_allocated = false;
			*index = i;
			return CCSV_IPR_NORMALCELL;
		case ',':
			cell->mem = &src[*index];
			cell->len = i - (*index);
			cell->is_allocated = false;
			*index = i + 1;
			return CCSV_IPR_NORMALCELL;
		case '"':
			return CCSV_IPR_BADQUOTE;
		}

		i++;
	}
}

bool ccsv_parse(const char *src, ccsv_Result *dest, ccsv_Error *err, size_t *idx) {
	*dest = ccsv_Buf_new(ccsv_Result);

	*idx = 0;
	ccsv_Line current_line = ccsv_Buf_new(ccsv_Line);

	while (true) {
		ccsv_Cell c;
		switch (ccsv_Cell_parse(src, idx, &c)) {
		case CCSV_IPR_BADQUOTE:
			*err = CCSV_ERR_INVALID;
			return false;
		case CCSV_IPR_QUOTEDCELL:
			switch (src[*idx]) {
			case ',':
				*idx += 1;
			case '\0':
			case '\r':
			case '\n':
				break;
			default:
				*err = CCSV_ERR_INVALID;
				return false;
			}
			// fallthrough
		case CCSV_IPR_NORMALCELL:
			ccsv_Buf_push(ccsv_Cell, current_line, c);
			c.mem = NULL;
			c.len = 0;
			break;
		case CCSV_IPR_NEWLINE:
			ccsv_Buf_push(ccsv_Line, (*dest), current_line);
			current_line = ccsv_Buf_new(ccsv_Line);
			break;
		case CCSV_IPR_EOF:
			if (current_line.len != 0) {
				ccsv_Buf_push(ccsv_Line, (*dest), current_line);
			}
			return true;
			break;
		case CCSV_IPR_OOM:
			*err = CCSV_ERR_OOM;
			return false;
		default:
			// FIXME: UNEXPECTED - HOW TO HANDLE THIS?
			break;
		}
	}
}

void ccsv_Result_destroy(ccsv_Result *r) {
	if (!r) return;

	for (int i = 0; i < r->len; i++) {
		const ccsv_Line *line = &r->mem[i];
		if (!line) continue;

		for (int j = 0; j < line->len; j++) {
			const ccsv_Cell *cell = &line->mem[j];
			if (!cell->mem || !cell->is_allocated) continue;

			free((void *) cell->mem);
		}

		free(line->mem);
	}

	free(r->mem);
}

const char *ccsv_Error_tostring(ccsv_Error e) {
	switch (e) {
	case CCSV_ERR_OOM:
		return "Out of memory";
	case CCSV_ERR_INVALID:
		return "Invalid input";
	default:
		return "???";
	}
}
