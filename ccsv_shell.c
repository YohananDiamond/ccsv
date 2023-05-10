#include <stdio.h>
#include "ccsv.h"

int main(void) {
	const char* csv = "aargh,\" vlarg \"\" \",morb\nfoo,bar,baz";

	ccsv_Result r;
	ccsv_Error e;
	size_t i;
	if (!ccsv_parse(csv, &r, &e, &i)) {
		fprintf(stderr, "Failed to parse: %s\n", ccsv_Error_tostring(e));
		fprintf(stderr, "%s\n^", &csv[i]);
		return 1;
	}

	for (size_t i = 0; i < r.len; i++) {
		printf("Line #%ld: ", i);
		const ccsv_Line *l = &r.mem[i];
		for (size_t j = 0; j < l->len; j++) {
			const ccsv_Cell *c = &l->mem[j];
			printf("[.%.*s.] ", (int)c->len, c->mem);
		}
		printf("\n");
	}

	ccsv_Result_destroy(&r);
}
