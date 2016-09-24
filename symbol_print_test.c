#include <stdio.h>
#include <string.h>
#include "pureLisp.h"

int main() {
	Object *obj = allocate(TYPE_SYMBOL);
	strcpy(obj->symbol, "hello");
	printObj(obj);

	return 0;
}
