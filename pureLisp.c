#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
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

// ungetToken�p�̃o�b�t�@
char getTokenBuffer[MAX_TOKEN_LENGTH] = {'\0'};

/*
 * fp������͂����, 1�Ԃ�̃g�[�N������buf�֓����.
 * buf�͌Ăяo�����̐ӔC�ŏ���������K�v������.
 * �߂�l:
 *   0: ���͂ɑ���������.
 *   1: ���͂��I�[�ɂȂ���.
 */
int getToken(char *buf, FILE *fp) {
	int c = 0;
	int ptr = 0;
	int inString = 0; // 0: �ʏ폈����, 1: �������������

	// unget���ꂽ�o�b�t�@������΂�������Ԃ�
	if (strcmp(getTokenBuffer, "") != 0) {
		strcpy(buf, getTokenBuffer);
		//unget�p�̃o�b�t�@���N���A���Ă���
		memset(getTokenBuffer, '\0', MAX_TOKEN_LENGTH);
		return 0;
	}

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

/*
 * getToken�Ŏ擾�����g�[�N�����getToken�̃o�b�t�@�ɖ߂�.
 */
void ungetToken(char *buf) {
	strcpy(getTokenBuffer, buf);
}

// �V����Object���쐬����
Object *allocate(ObjType type) {
	Object *obj = (Object *)malloc(sizeof(Object));
	obj->type = type;
	return obj;
}

void printList(Object *obj);

// Object�̓��e����(atom�̂�)
void printObj(Object *obj) {
	switch (obj->type) {
		case TYPE_INTEGER:
			printf("%d", obj->integer);
			break;
		case TYPE_SYMBOL:
			printf("%s", obj->symbol);
			break;
		case TYPE_STRING:
			printf("%s", obj->string);
			break;
		case TYPE_PAIR:
			printList(obj);
			break;
		case TYPE_NIL:
			printf("nil");
			break;
		default:
			printf("bug.Unknown type %d.", obj->type);
			exit(1);
			break;
	}
}

// Object�̓��e����(list�̂�)
void printList(Object *obj) {
	print(obj->pair.car);
	if (obj->pair.cdr->type != TYPE_NIL) {
		if (obj->pair.cdr->type == TYPE_PAIR) {
			// list
			printf(" ");
			printObj(obj->pair.cdr);
		} else {
			// dot pair
			printf(" . ");
			printObj(obj->pair.cdr);
		}
	}
}

// Object�̓��e����
void print(Object *obj) {
	if (obj->type == TYPE_PAIR) {
		printf("(");
		printList(obj);
		printf(")");
	} else {
		printObj(obj);
	}
}

// ���������l�݂̂ō\������Ă����1��Ԃ�.
int isInteger(char *buf) {
	// 1�����ڂ�-(�}�C�i�X)�ł��悢
	if (isdigit(*buf) == 0 && *buf != '-') {
		return 0;
	}
	buf++;
	while (*buf != '\0') {
		if (isdigit(*buf) == 0) {
			return 0;
		}
		buf++;
	}
	return 1;
}

// ������������ł����1��Ԃ�
int isString(char *buf) {
	// 1�����ڂ�"�ł���
	if (*buf != '"') {
		return 0;
	}
	buf++;
	while (*buf != '\0') {
		buf++;
	}
	buf--;
	// �Ō�̕�����"�ł���
	if (*buf != '"') {
		return 0;
	}
	return 1;
}

Object *makeNil() {
	Object *obj = allocate(TYPE_NIL);
	return obj;
}

Object *makeInteger(char *buf) {
	Object *obj = allocate(TYPE_INTEGER);
	obj->integer = atoi(buf);
	return obj;
}

Object *makeString(char *buf) {
	Object *obj = allocate(TYPE_STRING);
	int n = strlen(buf);
	// �O���"�̕���-2, �I�[��\0�̕���+1
	obj->string = (char *)malloc(sizeof(char) * (n - 2 + 1));
	// �擪��"�̕�����炷
	buf++;
	// "��\0�ȊO���R�s�[
	memcpy(obj->string, buf, n-2);
	// NULL�ŏI�[����
	obj->string[n-1] = '\0';
	return obj;
}

Object *makeSymbol(char *buf) {
	Object *obj = allocate(TYPE_SYMBOL);
	strcpy(obj->symbol, buf);
	return obj;
}

Object *read(FILE *fp);

// read��list�����p�֐�
Object *readList(FILE *fp) {
	Object *obj = allocate(TYPE_PAIR);
	char buf[MAX_TOKEN_LENGTH];
	memset(buf, '\0', MAX_TOKEN_LENGTH);

	obj->pair.car = read(fp);

	// cdr�ւ̐ݒ�̔��f�̂��߂�1�g�[�N����ǂ݂��Ė߂�
	getToken(buf, fp);
	if (strcmp(buf, ")") == 0) {
		obj->pair.cdr = allocate(TYPE_NIL);
	} else if (strcmp(buf, ".") == 0) {
		obj->pair.cdr = read(fp);
	} else {
		ungetToken(buf);
		obj->pair.cdr = readList(fp);
	}
	return obj;
}

/*
 * fp������Ȃ�����͂�Object�ɕϊ�����.
 * fp��open���ꂽ��ԂłȂ���΂Ȃ�Ȃ�.
 */
Object *read(FILE *fp) {
	char buf[MAX_TOKEN_LENGTH];
	memset(buf, '\0', MAX_TOKEN_LENGTH);

	if (getToken(buf, fp) == 0) {
		if (strcmp(buf, "(") == 0) {
			return readList(fp);
		}
		if (strcmp(buf, "nil") == 0) {
			return makeNil();
		}
		if (isInteger(buf)) {
			return makeInteger(buf);
		}
		if (isString(buf)) {
			return makeString(buf);
		}
		return makeSymbol(buf);
	}
	printf("malform.Token is %s\n", buf);
	exit(1);
}
