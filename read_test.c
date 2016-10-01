#include <stdio.h>
#include <string.h>
#include "pureLisp.h"

int main() {
	initialize();

	print(read(stdin));
	return 0;
}
