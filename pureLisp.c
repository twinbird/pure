#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
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

// symbolテーブル(initializeで初期化)
Object *SymbolTable = NULL;

// トップレベルの環境(initializeで初期化)
Object *TopEnv = NULL;

// NIL定数(initializeで初期化)
Object *NIL = NULL;

// トップレベルの環境で変数を定義する
// 定義済みなら値を上書きする
// 返り値はシンボル
Object *define(Object *sym, Object *val) {
	Object *newCell = allocate(TYPE_PAIR);
	// 新しい変数(シンボルと値)のペア
	Object *newVar = allocate(TYPE_PAIR);
	newVar->pair.car = sym;
	newVar->pair.cdr = val;

	// すでに定義済みかどうかを調べる
	// 値とシンボルのペアを順に調べていく
	Object *cell, *p;
	Object *tbl = TopEnv->env.vars;
	for (cell = tbl; cell->type != TYPE_NIL; cell = cell->pair.cdr) {
		p = cell->pair.car;
		if (p->pair.car == sym) {
			// 見つかった
			// 値を上書きする
			p->pair.cdr = val;
			return sym;
		}
	}

	// 見つからなかったので環境の変数テーブルにくっつける
	newCell->pair.car = newVar;
	newCell->pair.cdr = TopEnv->env.vars;
	TopEnv->env.vars = newCell;

	return sym;
}

// envの下のスコープ(環境)を作成する.
Object *makeEnv(Object *env, Object *vars, Object *vals) {
	// 新しい環境
	Object *newEnv = allocate(TYPE_ENV);
	// 値とシンボルのペアのリスト(NILを入れて空リストにしておく)
	Object *tbl = NIL;

	Object *var = vars;
	Object *val = vals;
	while (var->type != TYPE_NIL) {
		// tblへの追加用のペア
		Object *cell = allocate(TYPE_PAIR);
		// 値とシンボルのペア
		Object *p = allocate(TYPE_PAIR);
		p->pair.car = var->pair.car;
		p->pair.cdr = val->pair.car;
		// ペアのリストへ追加
		cell->pair.car = p;
		cell->pair.cdr = tbl;
		tbl = cell;
		// 次の変数へ
		var = var->pair.cdr;
		val = val->pair.cdr;
	}
	// 新しい環境へ設定
	newEnv->env.up = env;
	newEnv->env.vars = tbl;
	// 新しい環境を返す
	return newEnv;
}

// envを調べて変数を参照する
Object *lookup(Object *env, Object *symbol) {
	Object *tbl, *p, *cell;
	// 見つからなかったらNILを返す
	if (env->type == TYPE_NIL) {
		return NIL;
	}
	assert(env->type == TYPE_ENV);
	// 値とシンボルのペアを順に調べていく
	tbl = env->env.vars;
	for (cell = tbl; cell->type != TYPE_NIL; cell = cell->pair.cdr) {
		p = cell->pair.car;
		// 見つかった
		if (p->pair.car == symbol) {
			// 値を返す
			return p->pair.cdr;
		}
	}
	// 見つからなかったら上の環境を調べる
	return lookup(env->env.up, symbol);
}

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

// symbolテーブルへ登録する.
// 登録済みなら登録済みのObjectを返す.
Object *makeSymbol(char *buf) {
	// テーブルに登録済みか調べる
	for (Object *o = SymbolTable; o->type != TYPE_NIL; o = o->pair.cdr) {
		if (strcmp(o->pair.car->symbol, buf) == 0) {
			return o->pair.car;
		}
	}
	// 未登録なので新しく作って登録
	Object *obj = allocate(TYPE_SYMBOL);
	obj->symbol = (char *)malloc(strlen(buf) + 1); // \0 分+1
	strcpy(obj->symbol, buf);
	// ペアを作ってシンボルテーブルのリストへつなげる
	Object *p = allocate(TYPE_PAIR);
	p->pair.cdr = SymbolTable;
	p->pair.car = obj;
	SymbolTable = p;
	// 生成したsymbolを返す
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
		obj->pair.cdr = NIL;
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
			return NIL;
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

Object *apply(Object *func, Object *param) {
	if (func->type != TYPE_SYMBOL) {
		printf("malform");
		exit(1);
	}
	// Special form
	if (strcmp(func->symbol, "quote") == 0) {
		return param->pair.car;
	}
	if (strcmp(func->symbol, "define") == 0) {
		Object *sym = param->pair.car;
		Object *val = eval(param->pair.cdr->pair.car);
		return define(sym, val);
	}
	if (strcmp(func->symbol, "if") == 0) {
		Object *t = param->pair.cdr->pair.car;
		Object *f = param->pair.cdr->pair.cdr->pair.car;
		if (param->pair.car->type == TYPE_NIL) {
			return eval(f);
		} else {
			return eval(t);
		}
	}
	printf("not implement.");
	exit(1);
}

Object *eval(Object *obj) {
	if (obj->type == TYPE_INTEGER) {
		return obj;
	}
	if (obj->type == TYPE_STRING) {
		return obj;
	}
	if (obj->type == TYPE_NIL) {
		return obj;
	}
	if (obj->type == TYPE_PAIR) {
		return apply(obj->pair.car, obj->pair.cdr);
	}
	printf("malform.");
	exit(1);
}

// 初期化関数
// 他の関数を呼び出す前に必ず呼び出すこと
void initialize() {
	NIL = allocate(TYPE_NIL);
	SymbolTable = NIL;
	TopEnv = makeEnv(NIL, NIL, NIL);
}
