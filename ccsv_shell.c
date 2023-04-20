#include <stdio.h>
#include "ccsv.h"

int main(void) {
	const char* csv = "aargh,vlarg,morb\nfoo,bar,baz";

	ccsv_Result r;
	ccsv_Error e;
	if (!ccsv_parse(csv, &r, &e)) {
		fprintf(stderr, "Failed to parse.\n");
		return 1;
	}

	for (size_t i = 0; i < r.len; i++) {
		printf("Line #%ld: ", i);
		const ccsv_Line *l = &r.mem[i];
		for (size_t j = 0; j < l->len; j++) {
			const ccsv_Cell *c = &l->mem[j];
			printf("[ %.*s ] ", (int)c->len, c->mem);
		}
		printf("\n");
	}

	ccsv_Result_destroy(&r);
}
