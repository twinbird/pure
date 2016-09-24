#include <stdio.h>

/*
 * fpから入力を取り, 1綴りのトークン毎にbufへ入れる.
 * 戻り値:
 *   0: 入力に続きがある.
 *   1: 入力が終端になった.
 */
int lexer(char *buf, FILE *fp) {
	int c = fgetc(fp);
	int ptr = 0;

	if (c == EOF) {
		return 1;
	}
	if (c == '(') {
		buf[ptr++] = c;
		buf[ptr++] = '\0';
		return 0;
	}
	if (c == ')') {
		buf[ptr++] = c;
		buf[ptr++] = '\0';
		return 0;
	}

	while (c != EOF && c != '(' && c != ')') {
		buf[ptr++] = c;
		c = fgetc(fp);
	}
	buf[ptr++] = '\0';
	ungetc(c, fp);
	return 0;
}
