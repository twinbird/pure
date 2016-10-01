#include <stdio.h>
#include <string.h>
#include "pureLisp.h"

int main() {
	initialize();

	char buf[MAX_TOKEN_LENGTH];

	memset(buf, '\0', MAX_TOKEN_LENGTH);
	while (getToken(buf, stdin) == 0) {
		ungetToken(buf);
		getToken(buf, stdin);
		printf("%s,", buf);
	}
	return 0;
}
