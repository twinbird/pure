#include <stdio.h>
#include <string.h>
#include "pureLisp.h"

int main() {
	char buf[MAX_TOKEN_LENGTH];

	memset(buf, '\0', MAX_TOKEN_LENGTH);
	while (lexer(buf, stdin) == 0) {
		printf("%s", buf);
	}
	return 0;
}
