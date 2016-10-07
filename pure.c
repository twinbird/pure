#include <stdio.h>
#include "pureLisp.h"

int main() {
	Object *readExp, *evalExp;
	initialize();

	while (1) {
		printf(">");
		readExp = read(stdin);
		evalExp = eval(TopEnv, readExp);
		print(evalExp);
		printf("\n");
	}
	return 0;
}
