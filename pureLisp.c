#include <stdio.h>

// スペース以外の文字列が来るまで入力を捨てる
void skipSpace(FILE *fp) {
	int c = fgetc(fp);
	// この後に続くスペースは捨てる
	while (c == ' ') {
		c = fgetc(fp);
	}
	// スペースではない文字列は戻しておく
	ungetc(c, fp);
}

/*
 * fpから入力を取り, 1綴りのトークン毎にbufへ入れる.
 * bufは呼び出し元の責任で初期化する必要がある.
 * 戻り値:
 *   0: 入力に続きがある.
 *   1: 入力が終端になった.
 */
int lexer(char *buf, FILE *fp) {
	int c = 0;
	int ptr = 0;

	skipSpace(fp);

	c = fgetc(fp);

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

	while (c != EOF && c != '(' && c != ')' && c != ' ') {
		buf[ptr++] = c;
		c = fgetc(fp);
	}
	buf[ptr++] = '\0';
	ungetc(c, fp);
	return 0;
}
