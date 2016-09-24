#include <stdio.h>
#include <string.h>
#include "pureLisp.h"

int main() {
	// (1 2)‚ð\¬
	Object *obj = allocate(TYPE_PAIR);
	obj->pair.car = allocate(TYPE_INTEGER);
	obj->pair.car->integer = 1;

	obj->pair.cdr = allocate(TYPE_PAIR);
	obj->pair.cdr->pair.car = allocate(TYPE_INTEGER);
	obj->pair.cdr->pair.car->integer = 2; 

	obj->pair.cdr->pair.cdr = allocate(TYPE_NIL); 

	print(obj);

	return 0;
}
