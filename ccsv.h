#ifndef _CCSV_H

#include <stdlib.h>
#include <stdbool.h>

#define ccsv_def_Buf(T)       \
	struct ccsv_Buf_##T { \
		T *mem;       \
		size_t len;   \
	}

#define ccsv_Buf(T) struct ccsv_Buf_##T

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

typedef enum ccsv_Error {
	CCSV_ERR_OOM,
	CCSV_ERR_INVALID,
	CCSV_ERR_INTERNAL
} ccsv_Error;

typedef struct ccsv_Cell {
	union {
		char *alloc;
		const char *ref;
	} mem;
	bool is_allocated;
	size_t len;
} ccsv_Cell;

ccsv_def_Buf(ccsv_Cell);
typedef ccsv_Buf(ccsv_Cell) ccsv_Line;

ccsv_def_Buf(ccsv_Line);
typedef ccsv_Buf(ccsv_Line) ccsv_Result;

/**
 * Try to parse the CSV string `src`.
 *
 * `idx` is an index variable to use for parsing, and to track which part of the
 * input is invalid.
 *
 * On success: sets `*dest` to the result, and returns true;
 *
 * On failure: sets `*err` to the result, `*idx` to the last point before the
 * parse error, and returns true;
 */
bool ccsv_parse(const char *src, ccsv_Result *dest, ccsv_Error *err, size_t *idx);

/**
 * Convert a ccsv_Error to a string describing it.
 *
 * If an unexpected value is given, returns "???".
 */
const char *ccsv_Error_tostring(ccsv_Error e);

/**
 * Destroys a `ccsv_Result`, along with all allocated memory residing within it.
 */
void ccsv_Result_destroy(ccsv_Result* r);

#define _CCSV_H
#endif // _CCSV_H
