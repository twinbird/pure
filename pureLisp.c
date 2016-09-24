#include <stdio.h>

int lexer(char *buf, FILE *fp) {
	int c = fgetc(fp);
	int ptr = 0;

	if (c == '(') {
		buf[ptr++] = c;
		return 0;
	}
	if (c == ')') {
		buf[ptr++] = c;
		return 0;
	}
	if (c == '.') {
		buf[ptr++] = c;
		return 0;
	}

	return 1;
}
