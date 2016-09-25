#include <stdio.h>
#include <string.h>
#include "pureLisp.h"

int main() {
	// (lambda (x) (+ x 1)) ‚ð\¬
	Object *obj = allocate(TYPE_PAIR);
	obj->pair.car = allocate(TYPE_SYMBOL);
	strcpy(obj->pair.car->symbol, "lambda");

	obj->pair.cdr = allocate(TYPE_PAIR);

	obj->pair.cdr->pair.car = allocate(TYPE_PAIR);
	obj->pair.cdr->pair.car->pair.car = allocate(TYPE_SYMBOL);
	strcpy(obj->pair.cdr->pair.car->pair.car->symbol, "x");
	obj->pair.cdr->pair.car->pair.cdr = allocate(TYPE_NIL);

	obj->pair.cdr->pair.cdr = allocate(TYPE_PAIR); 

	obj->pair.cdr->pair.cdr->pair.car = allocate(TYPE_PAIR);
	obj->pair.cdr->pair.cdr->pair.car->pair.car = allocate(TYPE_SYMBOL);
	strcpy(obj->pair.cdr->pair.cdr->pair.car->pair.car->symbol, "+");
	obj->pair.cdr->pair.cdr->pair.car->pair.cdr = allocate(TYPE_PAIR);
	obj->pair.cdr->pair.cdr->pair.car->pair.cdr->pair.car = allocate(TYPE_SYMBOL);
	strcpy(obj->pair.cdr->pair.cdr->pair.car->pair.cdr->pair.car->symbol, "x");
	obj->pair.cdr->pair.cdr->pair.car->pair.cdr->pair.cdr = allocate(TYPE_PAIR);
	obj->pair.cdr->pair.cdr->pair.car->pair.cdr->pair.cdr->pair.car = allocate(TYPE_INTEGER);
	obj->pair.cdr->pair.cdr->pair.car->pair.cdr->pair.cdr->pair.car->integer = 1;
	obj->pair.cdr->pair.cdr->pair.car->pair.cdr->pair.cdr->pair.cdr = allocate(TYPE_NIL);

	obj->pair.cdr->pair.cdr->pair.cdr = allocate(TYPE_NIL);
	
	print(obj);

	return 0;
}
