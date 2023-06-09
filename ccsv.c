#include "ccsv.h"

#include <stdlib.h>

// "In" means Internal here.
typedef enum ccsv_InParseResult {
	// Found invalid quote character
	CCSV_IPR_BADQUOTE,

	// Finished a quoted cell.
	CCSV_IPR_QUOTEDCELL,

	// Found a comma.
	CCSV_IPR_COMMA,

	// End-of-line (be it a '\n', '\r\n', '\r' or EOF)
	CCSV_IPR_EOL,

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

	r->is_allocated = true;
	r->mem.alloc = mem;
	r->len = mem_i;

	return CCSV_IPR_QUOTEDCELL;
}

static ccsv_InParseResult ccsv_Cell_parse(const char *src, size_t *index, ccsv_Cell *cell) {
	size_t start = *index;
	size_t i = start;

	// skip leading whitespace
	for (; src[i] == ' '; i++);
	size_t skipped_amount = i - start;
	start = i;

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
				while (false); // dummy statement

				// Ignore trailing space, and error on
				// any other thing found
				size_t skip_trailing = i + 1;
				for (;; skip_trailing++) {
					char c = src[skip_trailing];
					if (c == '\n' || c == '\0' || c == ',') break;
					if (c == ' ') continue;
					return CCSV_IPR_BADQUOTE;
				}

				ccsv_InParseResult code =
				    ccsv_Cell_unquote_raw(&src[start], (i - *index), cell);
				*index = skip_trailing;
				return code;
			case '\0':
				// Oh no! Input finished before finishing quote!
				return CCSV_IPR_BADQUOTE;
			}

			i += 1;
			continue;
		}

		// "\r\n"
		if (src[i] == '\r' && src[i + 1] == '\n') {
			*index = i + 2;
			return CCSV_IPR_EOL;
		}

		switch (c) {
		case '\n':
		case '\r':
			start -= skipped_amount;
			cell->is_allocated = false;
			cell->mem.ref = &src[start];
			cell->len = i - start;

			*index = i + 1;
			return CCSV_IPR_EOL;
		case '\0':
			start -= skipped_amount;
			cell->is_allocated = false;
			cell->mem.ref = &src[start];
			cell->len = i - start;

			*index = i + 1;
			return CCSV_IPR_EOF;
		case ',':
			start -= skipped_amount;
			cell->is_allocated = false;
			cell->mem.ref = &src[start];
			cell->len = i - start;
			*index = i + 1;
			return CCSV_IPR_COMMA;
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
		case CCSV_IPR_COMMA:
			ccsv_Buf_push(ccsv_Cell, current_line, c);
			c.mem.ref = NULL;
			c.len = 0;
			break;
		case CCSV_IPR_EOL:
			ccsv_Buf_push(ccsv_Cell, current_line, c);
			c.mem.ref = NULL;
			c.len = 0;

			ccsv_Buf_push(ccsv_Line, (*dest), current_line);
			current_line = ccsv_Buf_new(ccsv_Line);
			break;
		case CCSV_IPR_EOF:
			ccsv_Buf_push(ccsv_Cell, current_line, c);
			c.mem.ref = NULL;
			c.len = 0;

			if (current_line.len != 0) {
				ccsv_Buf_push(ccsv_Line, (*dest), current_line);
			}
			return true;
			break;
		case CCSV_IPR_OOM:
			*err = CCSV_ERR_OOM;
			return false;
		default:
			*err = CCSV_ERR_INTERNAL;
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
			if (!cell->mem.ref || !cell->is_allocated) continue;

			free(cell->mem.alloc);
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
	case CCSV_ERR_INTERNAL:
		return "Internal error";
	default:
		return "???";
	}
}
