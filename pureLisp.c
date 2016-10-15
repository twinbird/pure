#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <setjmp.h>
#include "pureLisp.h"

#ifdef DEBUG
#define DEBUG_PRINT(msg) fprintf(stderr, msg)
#else
#define DEBUG_PRINT
#endif

// ungetToken用のバッファ
static char getTokenBuffer[MAX_TOKEN_LENGTH] = {'\0'};

// symbolテーブル(initializeで初期化)
static Object *SymbolTable = NULL;

// トップレベルの環境(initializeで初期化)
Object *TopEnv = NULL;

// GC用 オブジェクトテーブル
// 全てのオブジェクトはこのテーブルに属する
static Object *ObjectTable = NULL;

// GC用 割り当て済みヒープサイズカウンタ
static unsigned long AllocatedHeapSize = 0;

// GC用 スタックの開始位置
static Object *StackStartPosition = NULL;

// NIL定数(initializeで初期化)
static Object *NIL = NULL;
// T定数(initializeで初期化)
static Object *T = NULL;

// 1の間はGCしない
static int GCLockFlag = 0;

static void GCLock() {
	GCLockFlag = 1;
}

static void GCUnlock() {
	GCLockFlag = 0;
}

static Object *allocate(Object *env, ObjType type);

// スペース以外の文字列が来るまで入力を捨てる
static void skipSpace(FILE *fp) {
	int c = fgetc(fp);
	// この後に続くスペースと改行とタブは捨てる
	while (c == ' ' || c == '\n' || c == '\t') {
		c = fgetc(fp);
	}
	// スペースではない文字列は戻しておく
	ungetc(c, fp);
}

// トップレベルの環境で変数を定義する
// 定義済みなら値を上書きする
// 返り値はシンボル
static Object *define(Object *sym, Object *val) {
	Object *newCell = allocate(TopEnv, TYPE_PAIR);
	// 新しい変数(シンボルと値)のペア
	Object *newVar = allocate(TopEnv, TYPE_PAIR);
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
static Object *makeEnv(Object *env, Object *vars, Object *vals) {
	// 新しい環境
	Object *newEnv = allocate(env, TYPE_ENV);
	// 値とシンボルのペアのリスト(NILを入れて空リストにしておく)
	Object *tbl = NIL;

	Object *var = vars;
	Object *val = vals;
	while (var->type != TYPE_NIL) {
		// tblへの追加用のペア
		Object *cell = allocate(env, TYPE_PAIR);
		// 値とシンボルのペア
		Object *p = allocate(env, TYPE_PAIR);
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
// 未定義ならNULLを返す
static Object *lookup(Object *env, Object *symbol) {
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
	if (env == TopEnv) {
		// 未定義だった
		return NULL;
	} else {
		// 見つからなかったら上の環境を調べる
		return lookup(env->env.up, symbol);
	}
}

/*
 * fpから入力を取り, 1綴りのトークン毎にbufへ入れる.
 * bufは呼び出し元の責任で初期化する必要がある.
 * 戻り値:
 *   0: 入力に続きがある.
 *   1: 入力が終端になった.
 */
static int getToken(char *buf, FILE *fp) {
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

	while (inString == 1 || (c != EOF && c != '(' && c != ')' && c != ' ' && c != '\n')) {
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
static void ungetToken(char *buf) {
	strcpy(getTokenBuffer, buf);
}

// 未使用のsymbolをsymbolテーブルから取り除く
static void removeUnusedSymbol() {
	Object *tmp, *prev;
	Object *symPair;
	for (prev = tmp = SymbolTable; tmp->type != TYPE_NIL; prev = tmp, tmp = tmp->pair.cdr) {
		symPair = tmp->pair.car;
		if (symPair->pair.car->gcmark == UNUSED) {
			// リストをつなぎ変える
			prev->pair.cdr = tmp->pair.cdr;
			// 次のループで取り除いたセルを飛ばすためにtmpを次のものに入れ替え
			tmp = tmp->pair.cdr;
		}
	}
}

// オブジェクトを開放する
static void deallocate(Object *obj) {
	Object *prev, *tmp;
	// 予約語は回収しない
	switch (obj->type) {
		case TYPE_PRIMITIVE:
			return;
		case TYPE_T:
			return;
		case TYPE_NIL:
			return;
	}
	// ObjectTableのリストから外すためにobjのひとつ前のObjectを探索
	for (prev = tmp = ObjectTable; tmp != NULL; prev = tmp, tmp = tmp->next) {
		if (tmp == obj) {
			break;
		}
	}
	assert(tmp != NULL);
	// リストをつなぎ変える
	if (tmp == ObjectTable) {
		// 先頭の場合
		ObjectTable = tmp->next;
	} else {
		prev->next = tmp->next;
	}
	// オブジェクトを開放し、割り当て済みヒープサイズを調整
	switch (obj->type) {
		case TYPE_STRING:
			// 文字列領域も開放しておく
			free(obj->string);
			break;
		case TYPE_SYMBOL:
			// シンボル名領域も開放しておく
			free(obj->symbol);
			break;
	}
	free(obj);
	AllocatedHeapSize -= sizeof(Object);
}

// 引数のポインタがObjectTableの領域内に入っていたら1を返す
static int maybeObject(Object *obj) {
	unsigned long addr = (unsigned long)obj;
	unsigned long tmpAddr;
	// ObjectTableを1つずつ調べて確保した領域内にあるか調べる.
	for (Object *tmp = ObjectTable; tmp != NULL; tmp = tmp->next) {
		tmpAddr = (unsigned long)tmp;
		if (tmpAddr <= addr && addr <= (tmpAddr + sizeof(Object))) {
			return 1;
		}
	}
	return 0;
}

static void GCMarkingEnv(Object *basePair);

static void GCMarkingSub(Object *obj) {
	obj->gcmark = USED;
	if (obj->type == TYPE_PAIR) {
		if (maybeObject(obj->pair.car) == 1) {
			GCMarkingSub(obj->pair.car);
		}
		if (maybeObject(obj->pair.cdr) == 1) {
			GCMarkingSub(obj->pair.cdr);
		}
	} else if (obj->type == TYPE_ENV) {
		GCMarkingEnv(obj);
	} else if (obj->type == TYPE_FUNCTION) {
		if (maybeObject(obj->function.params) == 1) {
			GCMarkingSub(obj->function.params);
		}
		if (maybeObject(obj->function.body) == 1) {
			GCMarkingSub(obj->function.body);
		}
	}
}

// 現在の環境を起点に再帰的にマークする
static void GCMarkingEnv(Object *basePair) {
	// この環境は使っている
	basePair->gcmark = USED;
	// 現在の環境をすべてマーキング
	if (basePair->env.vars != NIL && (maybeObject(basePair->env.vars) == 1)) {
		GCMarkingSub(basePair->env.vars);
	}
	// 上位環境もマーキング
	if (basePair->env.up != NIL && (maybeObject(basePair->env.up) == 1)) {
		GCMarkingEnv(basePair->env.up);
	}
}

// レジスタ領域をマークする
static void GCMarkingRegister() {
	int i;
	// 現時点でのレジスタのスナップショットをとる
	jmp_buf registerSnapshot;
	setjmp(registerSnapshot);
	// スナップショットの先頭をポインタに入れておく
	Object **chkTgt = (Object **)registerSnapshot;
	// オブジェクトへのポインタがレジスタに入る最大個数を求める
	unsigned long maxFittingSize = sizeof(registerSnapshot) / sizeof(Object *);

	// レジスタ内を順に調べる
	for (i = 0; i < maxFittingSize; i++) {
		if (maybeObject(*chkTgt) == 1) {
			// Objectっぽかったらマーク
			GCMarkingSub(*chkTgt);
		}
		chkTgt++;
	}
}

// スタック領域をマークする
static void GCMarkingStack() {
	// GC用 スタック終了(先頭)のアドレス確認用
	Object stackEndObject;
	unsigned long stackEndAddr = (unsigned long)&stackEndObject;
	unsigned long stackStartAddr = (unsigned long)StackStartPosition;
	unsigned long tmpAddr = 0;
	int i;

	// スタックの伸びる方向を調べてマーキング開始位置を決める
	if (stackStartAddr > stackEndAddr) {
		tmpAddr = stackStartAddr;
		stackStartAddr = stackEndAddr;
		stackEndAddr = tmpAddr;
	}
	// オブジェクトへのポインタがスタックに入る最大個数を求める
	unsigned long maxFittingSize = (stackEndAddr - stackStartAddr) / sizeof(Object *);
	Object **chkTgt = (Object **)stackStartAddr;
	// スタックをマーキング
	for (i = 0; i < maxFittingSize; i++) {
		if (maybeObject(*chkTgt) == 1) {
			// Objectっぽかったらマーク
			GCMarkingSub(*chkTgt);
		}
		chkTgt++;
	}
}

// マーク&スイープのマーク処理
static void GCMarking(Object *rootEnv) {
	// 全て一度UNUSEDにする
	for (Object *tmp = ObjectTable; tmp != NULL; tmp = tmp->next) {
		tmp->gcmark = UNUSED;
	}
	// 引数の環境を起点にマーキング
	if (rootEnv != NULL && rootEnv != NIL) {
		GCMarkingEnv(rootEnv);
	}
	// スタックとレジスタもマーキング
	GCMarkingStack();
	GCMarkingRegister();
	// 上記まででマークされていないシンボルをシンボルテーブルから取り外す
	removeUnusedSymbol();
	// 利用中のシンボルのみ含んだシンボルテーブル全体をマーキング
	GCMarkingSub(SymbolTable);
}

// マーク&スイープのスイープ処理
// ObjectTableの順に辿ってMark済みのものを開放する
static void GCSweep() {
	Object *tmp = ObjectTable;
	Object *freeTgt = NULL;
	while (tmp != NULL) {
		// 削除対象へのポインタとして退避
		freeTgt = tmp;
		// ループのために退避
		tmp = tmp->next;
		// マークされていなければ開放
		if (freeTgt->gcmark == UNUSED) {
			DEBUG_PRINT("[GC] deallocated.\n");
			deallocate(freeTgt);
		}
	}
}

// ObjectTableの状況を見る(Debug用)
static void DebugPrintObjectTable() {
	fprintf(stderr, "-----------------------------\n");
	if (ObjectTable == NULL) {
		fprintf(stderr, "ObjectTable is null\n");
		return;
	}
	for (Object *tmp = ObjectTable; tmp != NULL; tmp = tmp->next) {
		switch (tmp->type) {
			case TYPE_INTEGER:
				fprintf(stderr, "%d", tmp->integer);
				break;
			case TYPE_SYMBOL:
				fprintf(stderr, "%s", tmp->symbol);
				break;
			case TYPE_STRING:
				fprintf(stderr, "\"%s\"", tmp->string);
				break;
			case TYPE_PAIR:
				fprintf(stderr, "pair");
				break;
			case TYPE_NIL:
				fprintf(stderr, "nil");
				break;
			case TYPE_T:
				fprintf(stderr, "t");
				break;
			case TYPE_PRIMITIVE:
				fprintf(stderr, "primitive");
				break;
			case TYPE_FUNCTION:
				fprintf(stderr, "function");
				break;
			case TYPE_ENV:
				fprintf(stderr, "environment");
				break;
			default:
				fprintf(stderr, "Unknown type %d.", tmp->type);
				break;
		}
		if (tmp->gcmark == USED) {
			fprintf(stderr, " is used.\n");
		} else {
			fprintf(stderr, " is unused.\n");
		}
	}
	fprintf(stderr, "Now Heap Size: %d\n", AllocatedHeapSize);
}

// 新しいObjectを作成する
static Object *allocate(Object *env, ObjType type) {
	if (GCLockFlag == 0) {
		// 割り当て済み容量が一定以上になっていたらGCをかける
		if (AllocatedHeapSize >= GC_THRESHOLD_BYTES) {
			GCMarking(env);
			GCSweep();
		}
	}

	// 新しいオブジェクトを作成
	Object *obj = (Object *)malloc(sizeof(Object));
	obj->type = type;
	obj->gcmark = USED;
	// ObjectTableへ登録
	obj->next = ObjectTable;
	ObjectTable = obj;
	// 割り当て済みヒープサイズを更新
	AllocatedHeapSize += sizeof(Object);

	return obj;
}

static void printList(Object *obj);

// Objectの内容を印字(atomのみ)
static void printObj(Object *obj) {
	switch (obj->type) {
		case TYPE_INTEGER:
			printf("%d", obj->integer);
			break;
		case TYPE_SYMBOL:
			printf("%s", obj->symbol);
			break;
		case TYPE_STRING:
			printf("\"%s\"", obj->string);
			break;
		case TYPE_PAIR:
			printList(obj);
			break;
		case TYPE_NIL:
			printf("nil");
			break;
		case TYPE_T:
			printf("t");
			break;
		case TYPE_PRIMITIVE:
			printf("primitive");
			break;
		case TYPE_FUNCTION:
			printf("function");
			break;
		case TYPE_ENV:
			printf("environment");
			break;
		default:
			printf("bug.Unknown type %d.", obj->type);
			exit(1);
			break;
	}
}

// Objectの内容を印字(listのみ)
static void printList(Object *obj) {
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
static int isInteger(char *buf) {
	// 1文字目は-(マイナス)でもよい
	if (isdigit(*buf) == 0 && *buf != '-') {
		return 0;
	}
	// 1文字目が-(マイナス)の時は2文字目は数字でないとならない
	if (isdigit(*buf) == 0 &&
			(*(buf + 1) == '\0' || isdigit(*(buf + 1)) == 0)) {
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
static int isString(char *buf) {
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

static Object *makeInteger(Object *env, char *buf) {
	Object *obj = allocate(env, TYPE_INTEGER);
	obj->integer = atoi(buf);
	return obj;
}

static Object *makeString(Object *env, char *buf) {
	Object *obj = allocate(env, TYPE_STRING);
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
static Object *makeSymbol(Object *env, char *buf) {
	// テーブルに登録済みか調べる
	for (Object *o = SymbolTable; o->type != TYPE_NIL; o = o->pair.cdr) {
		if (strcmp(o->pair.car->symbol, buf) == 0) {
			return o->pair.car;
		}
	}
	// 未登録なので新しく作って登録
	Object *obj = allocate(env, TYPE_SYMBOL);
	obj->symbol = (char *)malloc(strlen(buf) + 1); // \0 分+1
	strcpy(obj->symbol, buf);
	// ペアを作ってシンボルテーブルのリストへつなげる
	Object *p = allocate(env, TYPE_PAIR);
	p->pair.cdr = SymbolTable;
	p->pair.car = obj;
	SymbolTable = p;
	// 生成したsymbolを返す
	return obj;
}

static Object *makeFunction(Object *env, Object *params, Object *body) {
	Object *func = allocate(env, TYPE_FUNCTION);
	func->function.params = params;
	func->function.body = body;
	return func;
}

// readのlist処理用関数
static Object *readList(Object *env, FILE *fp) {
	Object *obj = allocate(env, TYPE_PAIR);
	char buf[MAX_TOKEN_LENGTH];
	memset(buf, '\0', MAX_TOKEN_LENGTH);

	// 空リスト判断のために1トークン先読みして戻す
	getToken(buf, fp);
	if (strcmp(buf, ")") == 0) {
		return NIL;
	} else {
		ungetToken(buf);
	}
	obj->pair.car = read(env, fp);

	// cdrへの設定の判断のために1トークン先読みして戻す
	getToken(buf, fp);
	if (strcmp(buf, ")") == 0) {
		obj->pair.cdr = NIL;
	} else if (strcmp(buf, ".") == 0) {
		obj->pair.cdr = read(env, fp);
	} else {
		ungetToken(buf);
		obj->pair.cdr = readList(env, fp);
	}
	return obj;
}

/*
 * fpを消費しながら入力をObjectに変換する.
 * fpはopenされた状態でなければならない.
 * EOFに達した時にはNULLを返す.
 */
Object *read(Object *env, FILE *fp) {
	char buf[MAX_TOKEN_LENGTH];
	memset(buf, '\0', MAX_TOKEN_LENGTH);

	if (getToken(buf, fp) == 0) {
		if (strcmp(buf, "(") == 0) {
			return readList(env, fp);
		}
		if (strcmp(buf, "nil") == 0) {
			return NIL;
		}
		if (isInteger(buf)) {
			return makeInteger(env, buf);
		}
		if (isString(buf)) {
			return makeString(env, buf);
		}
		return makeSymbol(env, buf);
	}
	return NULL;
}

static Object *evalList(Object *env, Object *obj) {
	Object *newList, *tmp;
	int count = 0;
	// ペアでなければevalした値を返す
	if (obj->type != TYPE_PAIR) {
		return eval(env, obj);
	}
	newList = allocate(env, TYPE_PAIR);
	tmp = newList;
	// 評価して値を新しいリストへ入れていく
	for (Object *p = obj; p->type != TYPE_NIL; p = p->pair.cdr) {
		tmp->pair.car = eval(env, p->pair.car);
		tmp->pair.cdr = allocate(env, TYPE_PAIR);
		tmp = tmp->pair.cdr;
	}
	tmp->pair.car = NIL;
	// 新しいリストを返す
	return newList;
}

static Object *apply(Object *env, Object *sym, Object *param) {
	if (sym->type != TYPE_SYMBOL && sym->type != TYPE_PRIMITIVE && sym->type != TYPE_FUNCTION) {
		printf("malform on apply. type:%d", sym->type);
		exit(1);
	}
	// Special form
	if (strcmp(sym->symbol, "quote") == 0) {
		return param->pair.car;
	}
	if (strcmp(sym->symbol, "define") == 0) {
		Object *sym = param->pair.car;
		Object *val = eval(env, param->pair.cdr->pair.car);
		return define(sym, val);
	}
	if (strcmp(sym->symbol, "if") == 0) {
		Object *t = param->pair.cdr->pair.car;
		Object *f = param->pair.cdr->pair.cdr->pair.car;
		if (param->pair.car->type == TYPE_NIL) {
			return eval(env, f);
		} else {
			return eval(env, t);
		}
	}
	if (strcmp(sym->symbol, "lambda") == 0) {
		Object *args = param->pair.car;
		Object *body = param->pair.cdr->pair.car;
		return makeFunction(env, args, body);
	}
	Object *func = lookup(env, sym);
	if (func == NULL) {
		printf("undefined function '%s'\n", sym->symbol);
		exit(1);
	}
	// Primitive function
	if (func->type == TYPE_PRIMITIVE) {
		Object *newEnv = makeEnv(env, NIL, NIL);
		return func->primitive(newEnv, evalList(env, param));
	}
	// Function
	if (func->type == TYPE_FUNCTION) {
		Object *newEnv = makeEnv(env, func->function.params, evalList(env, param));
		return eval(newEnv, func->function.body);
	}
	
	printf("not implement type:%d.", func->type);
	exit(1);
}

Object *eval(Object *env, Object *obj) {
	if (obj->type == TYPE_INTEGER) {
		return obj;
	}
	if (obj->type == TYPE_STRING) {
		return obj;
	}
	if (obj->type == TYPE_NIL) {
		return obj;
	}
	if (obj->type == TYPE_T) {
		return obj;
	}
	if (obj->type == TYPE_SYMBOL) {
		Object *sym = lookup(env, obj);
		if (sym == NULL) {
			printf("undefined symbol '%s'\n", obj->symbol);
			exit(1);
		}
		return sym;
	}
	if (obj->type == TYPE_PAIR) {
		return apply(env, obj->pair.car, obj->pair.cdr);
	}
	printf("malform on eval.");
	exit(1);
}

//プリミティブ関数

// Listならnil、その他はt
static Object *primitiveAtom(Object *env, Object *args) {
	Object *arg = args->pair.car;
	if (arg->type == TYPE_PAIR) {
		return NIL;
	}
	return makeSymbol(env, "t");
}

// 数値、文字列は同値ならt.
// その他はアドレスで比較.(つまり同一アドレスならt)
static Object *primitiveEq(Object *env, Object *args) {
	Object *one = args->pair.car;
	Object *two = args->pair.cdr->pair.car;
	// 型が違えば異なる
	if (one->type != two->type) {
		return NIL;
	}
	switch (one->type) {
		case TYPE_INTEGER:
			if (one->integer == two->integer) {
				return T;
			}
			break;
		case TYPE_STRING:
			if (strcmp(one->string, two->string) == 0) {
				return T;
			}
			break;
		default:
			if (one == two) {
				return T;
			}
			break;
	}
	return NIL;
}

static Object *primitiveCons(Object *env, Object *args) {
	Object *one = args->pair.car;
	Object *two = args->pair.cdr->pair.car;
	Object *pair = allocate(env, TYPE_PAIR);
	pair->pair.car = one;
	pair->pair.cdr = two;
	return pair;
}

static Object *primitiveCar(Object *env, Object *args) {
	return args->pair.car->pair.car;
}

static Object *primitiveCdr(Object *env, Object *args) {
	return args->pair.car->pair.cdr;
}

static Object *primitivePrint(Object *env, Object *args) {
	print(args->pair.car);
	return args->pair.car;
}

static Object *primitiveAdd(Object *env, Object *args) {
	Object *one = args->pair.car;
	Object *two = args->pair.cdr->pair.car;
	if (one->type != TYPE_INTEGER || two->type != TYPE_INTEGER) {
		printf("+ is only apply integer\n");
		exit(1);
	}
	Object *newObj = allocate(env, TYPE_INTEGER);
	newObj->integer = one->integer + two->integer;
	return newObj;
}

static Object *primitiveSubtract(Object *env, Object *args) {
	Object *one = args->pair.car;
	Object *two = args->pair.cdr->pair.car;
	if (one->type != TYPE_INTEGER || two->type != TYPE_INTEGER) {
		printf("- is only apply integer\n");
		exit(1);
	}
	Object *newObj = allocate(env, TYPE_INTEGER);
	newObj->integer = one->integer - two->integer;
	return newObj;
}

static Object *primitiveMultiple(Object *env, Object *args) {
	Object *one = args->pair.car;
	Object *two = args->pair.cdr->pair.car;
	if (one->type != TYPE_INTEGER || two->type != TYPE_INTEGER) {
		printf("* is only apply integer\n");
		exit(1);
	}
	Object *newObj = allocate(env, TYPE_INTEGER);
	newObj->integer = one->integer * two->integer;
	return newObj;
}

static Object *primitiveDivide(Object *env, Object *args) {
	Object *one = args->pair.car;
	Object *two = args->pair.cdr->pair.car;
	if (one->type != TYPE_INTEGER || two->type != TYPE_INTEGER) {
		printf("/ is only apply integer\n");
		exit(1);
	}
	if (two->integer == 0) {
		printf("zero divide error.\n");
		exit(1);
	}
	Object *newObj = allocate(env, TYPE_INTEGER);
	newObj->integer = one->integer / two->integer;
	return newObj;
}

static Object *primitiveModulo(Object *env, Object *args) {
	Object *one = args->pair.car;
	Object *two = args->pair.cdr->pair.car;
	if (one->type != TYPE_INTEGER || two->type != TYPE_INTEGER) {
		printf("% is only apply integer\n");
		exit(1);
	}
	Object *newObj = allocate(env, TYPE_INTEGER);
	newObj->integer = one->integer % two->integer;
}

static Object *primitiveLessThan(Object *env, Object *args) {
	Object *one = args->pair.car;
	Object *two = args->pair.cdr->pair.car;
	if (one->type != TYPE_INTEGER || two->type != TYPE_INTEGER) {
		printf("% is only apply integer\n");
		exit(1);
	}
	if (one->integer < two->integer) {
		return T;
	} else {
		return NIL;
	}
}

static Object *primitiveGreaterThan(Object *env, Object *args) {
	Object *one = args->pair.car;
	Object *two = args->pair.cdr->pair.car;
	if (one->type != TYPE_INTEGER || two->type != TYPE_INTEGER) {
		printf("% is only apply integer\n");
		exit(1);
	}
	if (one->integer > two->integer) {
		return T;
	} else {
		return NIL;
	}
}

static Object *primitiveLessOrEqual(Object *env, Object *args) {
	Object *one = args->pair.car;
	Object *two = args->pair.cdr->pair.car;
	if (one->type != TYPE_INTEGER || two->type != TYPE_INTEGER) {
		printf("% is only apply integer\n");
		exit(1);
	}
	if (one->integer <= two->integer) {
		return T;
	} else {
		return NIL;
	}
}

static Object *primitiveGreaterOrEqual(Object *env, Object *args) {
	Object *one = args->pair.car;
	Object *two = args->pair.cdr->pair.car;
	if (one->type != TYPE_INTEGER || two->type != TYPE_INTEGER) {
		printf("% is only apply integer\n");
		exit(1);
	}
	if (one->integer >= two->integer) {
		return T;
	} else {
		return NIL;
	}
}

// プリミティブ関数定義
static void definePrimitive(Object *sym, Primitive func) {
	Object *obj = allocate(TopEnv, TYPE_PRIMITIVE);
	obj->primitive = func;
	define(sym, obj);
}

// 初期化関数
// 他の関数を呼び出す前に必ず呼び出すこと
void initialize() {
	// GC用 スタックのはじめに置かれるオブジェクト
	Object stackStartObject;
	// スタック開始位置のアドレスを記録
	StackStartPosition = &stackStartObject;

	// 初期化完了まではGCを行わない
	GCLock();
	T = allocate(TopEnv, TYPE_T);
	NIL = allocate(TopEnv, TYPE_NIL);
	SymbolTable = NIL;
	TopEnv = makeEnv(NIL, NIL, NIL);
	// 予約語の定義
	define(makeSymbol(TopEnv, "nil"), NIL);
	define(makeSymbol(TopEnv, "t"), T);
	definePrimitive(makeSymbol(TopEnv, "atom"), primitiveAtom);
	definePrimitive(makeSymbol(TopEnv, "eq"), primitiveEq);
	definePrimitive(makeSymbol(TopEnv, "cons"), primitiveCons);
	definePrimitive(makeSymbol(TopEnv, "car"), primitiveCar);
	definePrimitive(makeSymbol(TopEnv, "cdr"), primitiveCdr);
	// もはやpureではないやつら
	definePrimitive(makeSymbol(TopEnv, "print"), primitivePrint);
	definePrimitive(makeSymbol(TopEnv, "+"), primitiveAdd);
	definePrimitive(makeSymbol(TopEnv, "-"), primitiveSubtract);
	definePrimitive(makeSymbol(TopEnv, "*"), primitiveMultiple);
	definePrimitive(makeSymbol(TopEnv, "/"), primitiveDivide);
	definePrimitive(makeSymbol(TopEnv, "%"), primitiveModulo);
	definePrimitive(makeSymbol(TopEnv, "<"), primitiveLessThan);
	definePrimitive(makeSymbol(TopEnv, ">"), primitiveGreaterThan);
	definePrimitive(makeSymbol(TopEnv, "<="), primitiveLessOrEqual);
	definePrimitive(makeSymbol(TopEnv, ">="), primitiveGreaterOrEqual);
	GCUnlock();
}
