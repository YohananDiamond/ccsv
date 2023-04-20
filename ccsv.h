#ifndef _CCSV_H

#include <stdlib.h>
#include <stdbool.h>

typedef enum ccsv_Error {
	CCSV_ERR_OOM,
	CCSV_ERR_INVALID
} ccsv_Error;

typedef struct ccsv_Cell {
	const char *mem;
	size_t len;
	bool is_allocated;
} ccsv_Cell;

typedef struct ccsv_Line {
	ccsv_Cell *mem;
	size_t len;
} ccsv_Line;

typedef struct ccsv_Result {
	ccsv_Line *mem;
	size_t len;
} ccsv_Result;

bool ccsv_parse(const char *src, ccsv_Result *dest, ccsv_Error *err);
void ccsv_Result_destroy(ccsv_Result* r);

#define _CCSV_H
#endif // _CCSV_H
