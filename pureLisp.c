#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
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

// ungetToken用のバッファ
char getTokenBuffer[MAX_TOKEN_LENGTH] = {'\0'};

/*
 * fpから入力を取り, 1綴りのトークン毎にbufへ入れる.
 * bufは呼び出し元の責任で初期化する必要がある.
 * 戻り値:
 *   0: 入力に続きがある.
 *   1: 入力が終端になった.
 */
int getToken(char *buf, FILE *fp) {
	int c = 0;
	int ptr = 0;
	int inString = 0; // 0: 通常処理中, 1: 文字列内処理中

	// ungetされたバッファがあればそこから返す
	if (strcmp(getTokenBuffer, "") != 0) {
		strcpy(buf, getTokenBuffer);
		//unget用のバッファをクリアしておく
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
 * getTokenで取得したトークン一つをgetTokenのバッファに戻す.
 */
void ungetToken(char *buf) {
	strcpy(getTokenBuffer, buf);
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

// 引数が数値のみで構成されていれば1を返す.
int isInteger(char *buf) {
	// 1文字目は-(マイナス)でもよい
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

// 引数が文字列であれば1を返す
int isString(char *buf) {
	// 1文字目が"である
	if (*buf != '"') {
		return 0;
	}
	buf++;
	while (*buf != '\0') {
		buf++;
	}
	buf--;
	// 最後の文字も"である
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
	// 前後の"の分を-2, 終端の\0の分で+1
	obj->string = (char *)malloc(sizeof(char) * (n - 2 + 1));
	// 先頭の"の分一つずらす
	buf++;
	// "と\0以外をコピー
	memcpy(obj->string, buf, n-2);
	// NULLで終端する
	obj->string[n-1] = '\0';
	return obj;
}

Object *makeSymbol(char *buf) {
	Object *obj = allocate(TYPE_SYMBOL);
	strcpy(obj->symbol, buf);
	return obj;
}

Object *read(FILE *fp);

// readのlist処理用関数
Object *readList(FILE *fp) {
	Object *obj = allocate(TYPE_PAIR);
	char buf[MAX_TOKEN_LENGTH];
	memset(buf, '\0', MAX_TOKEN_LENGTH);

	obj->pair.car = read(fp);

	// cdrへの設定の判断のために1トークン先読みして戻す
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
 * fpを消費しながら入力をObjectに変換する.
 * fpはopenされた状態でなければならない.
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
