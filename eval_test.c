#include <stdio.h>
#include <string.h>
#include "pureLisp.h"

int main() {
	print(eval(read(stdin)));
	return 0;
}
