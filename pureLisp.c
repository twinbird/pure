#include <stdio.h>
#include <stdlib.h>
#include "pureLisp.h"

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
	int inString = 0; // 0: �ʏ폈����, 1: �������������

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

	while (inString == 1 || (c != EOF && c != '(' && c != ')' && c != ' ')) {
		if (c == '"') {
			inString = inString == 0 ? 1 : 0;
		}
		buf[ptr++] = c;
		c = fgetc(fp);
		if (ptr >= MAX_TOKEN_LENGTH) {
			buf[MAX_TOKEN_LENGTH -1] = '\0';
			printf("token %s is too long.", buf);
			exit(1);
		}
	}
	buf[ptr++] = '\0';
	ungetc(c, fp);
	return 0;
}
