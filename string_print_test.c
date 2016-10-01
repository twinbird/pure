#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pureLisp.h"

int main() {
	initialize();

	char expect[] = "hello string";
	Object *obj = allocate(TYPE_STRING);
	obj->string = (char *)malloc(sizeof(expect));
	strcpy(obj->string, expect);
	print(obj);

	return 0;
}
