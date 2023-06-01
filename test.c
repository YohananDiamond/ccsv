#include <stdio.h>
#include "ccsv.h"

int main(void) {
	const char* csv = " aargh ,\" vlarg \"\" \",morb \nfoo,bar,baz,\nfoo,";

	printf("Input:\n");

	size_t line = 0;
	bool stop = false;
	for (size_t i = 0; ; i++) {
		if (i == 0) {
			printf("%4ld | ", line);
		}

		switch (csv[i]) {
		case '\n':
			line++;
			printf("\n%4ld | ", line);
			break;
		case '\0':
			printf("\n");
			stop = true;
			break;
		default:
			printf("%c", csv[i]);
		}

		if (stop)
			break;
	}

	printf("\n");

	ccsv_Result r;
	ccsv_Error e;
	size_t i;
	if (!ccsv_parse(csv, &r, &e, &i)) {
		fprintf(stderr, "Failed to parse (at %ld): %s\n", i, ccsv_Error_tostring(e));
		fprintf(stderr, "%s\n^", &csv[i]);
		return 1;
	}

	printf("Output:\n");
	for (size_t i = 0; i < r.len; i++) {
		printf("%4ld | ", i);
		const ccsv_Line *l = &r.mem[i];
		for (size_t j = 0; j < l->len; j++) {
			const ccsv_Cell *c = &l->mem[j];
			printf("[.%.*s.] ", (int)c->len, c->mem.ref);
		}
		printf("\n");
	}

	ccsv_Result_destroy(&r);
}
