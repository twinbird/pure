#include <stdio.h>
#include <string.h>
#include "pureLisp.h"

int main() {
	initialize();

	Object *env = allocate(TYPE_NIL);
	Object *vals1 = allocate(TYPE_PAIR);
	Object *syms1 = allocate(TYPE_PAIR);
	Object *vals2 = allocate(TYPE_PAIR);
	Object *syms2 = allocate(TYPE_PAIR);
	Object *vals3 = allocate(TYPE_PAIR);
	Object *syms3 = allocate(TYPE_PAIR);
	Object *ret;

	// b => 9, c => 10
	syms3->pair.car = (struct _object *)makeSymbol("b");
	vals3->pair.car = (struct _object *)makeInteger("9");
	syms3->pair.cdr = (struct _object *)allocate(TYPE_PAIR);
	vals3->pair.cdr = (struct _object *)allocate(TYPE_PAIR);
	syms3->pair.cdr->pair.car = (struct _object *)makeSymbol("c");
	vals3->pair.cdr->pair.car = (struct _object *)makeInteger("10");
	syms3->pair.cdr->pair.cdr = (struct _object *)allocate(TYPE_NIL);
	vals3->pair.cdr->pair.cdr = (struct _object *)allocate(TYPE_NIL);
	env = makeEnv(env, syms3, vals3);

	// a => 5
	syms2->pair.car = (struct _object *)makeSymbol("a");
	vals2->pair.car = (struct _object *)makeInteger("5");
	syms2->pair.cdr = (struct _object *)allocate(TYPE_NIL);
	vals2->pair.cdr = (struct _object *)allocate(TYPE_NIL);
	env = makeEnv(env, syms2, vals2);
	
	// a => 1, b => 2
	syms1->pair.car = (struct _object *)makeSymbol("a");
	vals1->pair.car = (struct _object *)makeInteger("1");
	syms1->pair.cdr = (struct _object *)allocate(TYPE_PAIR);
	vals1->pair.cdr = (struct _object *)allocate(TYPE_PAIR);
	syms1->pair.cdr->pair.car = (struct _object *)makeSymbol("b");
	vals1->pair.cdr->pair.car = (struct _object *)makeInteger("2");
	syms1->pair.cdr->pair.cdr = (struct _object *)allocate(TYPE_NIL);
	vals1->pair.cdr->pair.cdr = (struct _object *)allocate(TYPE_NIL);
	env = makeEnv(env, syms1, vals1);

	ret = lookup(env, makeSymbol("a"));
	if (ret->integer != 1) {
		printf("Symbol 'a' expect value is 1.Actual %d", ret->integer);
		return 1;
	}
	ret = lookup(env, makeSymbol("b"));
	if (ret->integer != 2) {
		printf("Symbol 'b' expect value is 2.Actual %d", ret->integer);
		return 1;
	}
	ret = lookup(env, makeSymbol("c"));
	if (ret->integer != 10) {
		printf("Symbol 'c' expect value is 10.Actual %d", ret->integer);
		return 1;
	}
	printf("Success");

	return 0;
}
