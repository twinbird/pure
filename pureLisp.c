#include <stdio.h>

/*
 * fp������͂����, 1�Ԃ�̃g�[�N������buf�֓����.
 * �߂�l:
 *   0: ���͂ɑ���������.
 *   1: ���͂��I�[�ɂȂ���.
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
