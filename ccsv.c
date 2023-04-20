#include "ccsv.h"

#include <stdlib.h>

// "In" means Internal here.
typedef enum ccsv_InParseResult {
	// Found invalid text.
	// `cell` unmodified.
	CCSV_IPR_INVALID,

	// Finished a cell.
	CCSV_IPR_NEXT_CELL,

	// Finished the line (encountered newline)
	// `cell` unmodified.
	CCSV_IPR_NEWLINE,

	// End-of-file (string finished)
	CCSV_IPR_EOF,

	// Error - out of memory
	CCSV_IPR_OOM,
} ccsv_InParseResult;

#define ccsv_Buf_new(T) \
	(T) { .mem = 0, .len = 0 }
#define ccsv_Buf_push(T, buf, val)                                     \
	if (buf.mem) {                                                 \
		buf.mem = realloc(buf.mem, (buf.len + 1) * sizeof(T)); \
		buf.mem[buf.len] = val;                                \
		buf.len++;                                             \
	} else {                                                       \
		buf.mem = malloc(sizeof(T));                           \
		buf.mem[0] = val;                                      \
		buf.len = 1;                                           \
	}

ccsv_InParseResult ccsv_Cell_parse(const char *src, size_t *index, ccsv_Cell *cell) {
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

	while (true) {
		char c = src[i];

		switch (c) {
		case '\n':
		case '\r':
		case '\0':
			cell->mem = &src[*index];
			cell->len = i - (*index);
			cell->is_allocated = false;
			*index = i;
			return CCSV_IPR_NEXT_CELL;
		case ',':
			cell->mem = &src[*index];
			cell->len = i - (*index);
			cell->is_allocated = false;
			*index = i + 1;
			return CCSV_IPR_NEXT_CELL;
		}

		i++;
	}

	// TODO: parse quotes n stuff (origin of CCSV_IPR_INVALID and CCSV_IPR_OOM)
}

bool ccsv_parse(const char *src, ccsv_Result *dest, ccsv_Error *err) {
	*dest = ccsv_Buf_new(ccsv_Result);

	size_t i = 0;
	ccsv_Line current_line = ccsv_Buf_new(ccsv_Line);

	while (true) {
		ccsv_Cell c;
		switch (ccsv_Cell_parse(src, &i, &c)) {
		case CCSV_IPR_INVALID:
			*err = CCSV_ERR_INVALID;
			return false;
		case CCSV_IPR_NEXT_CELL:
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
