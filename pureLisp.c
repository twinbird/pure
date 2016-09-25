#include <stdio.h>
#include <stdlib.h>
#include "pureLisp.h"

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
	int inString = 0; // 0: 通常処理中, 1: 文字列内処理中

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

// 新しいObjectを作成する
Object *allocate(ObjType type) {
	Object *obj = (Object *)malloc(sizeof(Object));
	obj->type = type;
	return obj;
}

void printList(Object *obj);

// Objectの内容を印字(atomのみ)
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

// Objectの内容を印字(listのみ)
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

// Objectの内容を印字
void print(Object *obj) {
	if (obj->type == TYPE_PAIR) {
		printf("(");
		printList(obj);
		printf(")");
	} else {
		printObj(obj);
	}
}
