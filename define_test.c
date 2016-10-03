#include <stdio.h>
#include <string.h>
#include "pureLisp.h"

int main() {
	initialize();
	Object *ret;

	Object *asym = define(makeSymbol("a"), makeInteger("1"));
	ret = lookup(TopEnv, asym);
	if (ret->integer != 1) {
		printf("Symbol 'a' expect value is 1.Actual %d", ret->integer);
		return 1;
	}

	Object *bsym = define(makeSymbol("b"), makeInteger("2"));
	ret = lookup(TopEnv, bsym);
	if (ret->integer != 2) {
		printf("Symbol 'b' expect value is 2.Actual %d", ret->integer);
		return 1;
	}

	Object *csym = define(makeSymbol("c"), makeString("\"hello, world\""));
	ret = lookup(TopEnv, makeSymbol("c"));
	if (strcmp(ret->string, "hello, world") != 0) {
		printf("Symbol 'c' expect value is 'hello, world'.Actual %s", ret->string);
		return 1;
	}
	printf("Success");

	return 0;
}
