#include <stdio.h>
#include "pureLisp.h"

int main() {
	Object *obj = allocate(TYPE_INTEGER);
	obj->integer = 1;
	printObj(obj);

	return 0;
}
