#include <stdio.h>
#include <string.h>
#include "pureLisp.h"

int main() {
	Object *obj = allocate(TYPE_NIL);
	print(obj);

	return 0;
}