#include <stdio.h>

// �X�y�[�X�ȊO�̕����񂪗���܂œ��͂��̂Ă�
void skipSpace(FILE *fp) {
	int c = fgetc(fp);
	// ���̌�ɑ����X�y�[�X�͎̂Ă�
	while (c == ' ') {
		c = fgetc(fp);
	}
	// �X�y�[�X�ł͂Ȃ�������͖߂��Ă���
	ungetc(c, fp);
}

/*
 * fp������͂����, 1�Ԃ�̃g�[�N������buf�֓����.
 * buf�͌Ăяo�����̐ӔC�ŏ���������K�v������.
 * �߂�l:
 *   0: ���͂ɑ���������.
 *   1: ���͂��I�[�ɂȂ���.
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
