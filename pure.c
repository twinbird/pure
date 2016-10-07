#include <stdio.h>
#include "pureLisp.h"

int main(int argc, char *argv[]) {
	Object *readExp, *evalExp;
	FILE *fp = stdin;

	if (argc == 2) {
		fp = fopen(argv[1], "r");
	}

	initialize();

	while (1) {
		printf(">");
		readExp = read(fp);
		if (readExp == NULL) {
			break;
		}
		evalExp = eval(TopEnv, readExp);
		print(evalExp);
		printf("\n");
	}
	fclose(fp);

	return 0;
}
