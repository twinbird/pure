#include <stdio.h>
#include "pureLisp.h"

int main() {
	initialize();

	Object *obj = allocate(TYPE_INTEGER);
	obj->integer = 1;
	print(obj);

	return 0;
}
