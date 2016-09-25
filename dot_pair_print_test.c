#include <stdio.h>
#include <string.h>
#include "pureLisp.h"

int main() {
	Object *obj = allocate(TYPE_PAIR);
	obj->pair.car = allocate(TYPE_INTEGER);
	obj->pair.car->integer = 1;
	obj->pair.cdr = allocate(TYPE_INTEGER);
	obj->pair.cdr->integer = 2;

	print(obj);

	return 0;
}
