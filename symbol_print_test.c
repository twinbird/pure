#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pureLisp.h"

int main() {
	Object *obj = allocate(TYPE_SYMBOL);
	obj->symbol = (char *)malloc(strlen("hello") + 1);
	strcpy(obj->symbol, "hello");
	print(obj);

	return 0;
}
