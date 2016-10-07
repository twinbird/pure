#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include "pureLisp.h"

// �X�y�[�X�ȊO�̕����񂪗���܂œ��͂��̂Ă�
void skipSpace(FILE *fp) {
	int c = fgetc(fp);
	// ���̌�ɑ����X�y�[�X�Ɖ��s�͎̂Ă�
	while (c == ' ' || c == '\n') {
		c = fgetc(fp);
	}
	// �X�y�[�X�ł͂Ȃ�������͖߂��Ă���
	ungetc(c, fp);
}

// ungetToken�p�̃o�b�t�@
char getTokenBuffer[MAX_TOKEN_LENGTH] = {'\0'};

// symbol�e�[�u��(initialize�ŏ�����)
Object *SymbolTable = NULL;

// �g�b�v���x���̊�(initialize�ŏ�����)
Object *TopEnv = NULL;

// NIL�萔(initialize�ŏ�����)
Object *NIL = NULL;
// T�萔(initialize�ŏ�����)
Object *T = NULL;

// �g�b�v���x���̊��ŕϐ����`����
// ��`�ς݂Ȃ�l���㏑������
// �Ԃ�l�̓V���{��
Object *define(Object *sym, Object *val) {
	Object *newCell = allocate(TYPE_PAIR);
	// �V�����ϐ�(�V���{���ƒl)�̃y�A
	Object *newVar = allocate(TYPE_PAIR);
	newVar->pair.car = sym;
	newVar->pair.cdr = val;

	// ���łɒ�`�ς݂��ǂ����𒲂ׂ�
	// �l�ƃV���{���̃y�A�����ɒ��ׂĂ���
	Object *cell, *p;
	Object *tbl = TopEnv->env.vars;
	for (cell = tbl; cell->type != TYPE_NIL; cell = cell->pair.cdr) {
		p = cell->pair.car;
		if (p->pair.car == sym) {
			// ��������
			// �l���㏑������
			p->pair.cdr = val;
			return sym;
		}
	}

	// ������Ȃ������̂Ŋ��̕ϐ��e�[�u���ɂ�������
	newCell->pair.car = newVar;
	newCell->pair.cdr = TopEnv->env.vars;
	TopEnv->env.vars = newCell;

	return sym;
}

// env�̉��̃X�R�[�v(��)���쐬����.
Object *makeEnv(Object *env, Object *vars, Object *vals) {
	// �V������
	Object *newEnv = allocate(TYPE_ENV);
	// �l�ƃV���{���̃y�A�̃��X�g(NIL�����ċ󃊃X�g�ɂ��Ă���)
	Object *tbl = NIL;

	Object *var = vars;
	Object *val = vals;
	while (var->type != TYPE_NIL) {
		// tbl�ւ̒ǉ��p�̃y�A
		Object *cell = allocate(TYPE_PAIR);
		// �l�ƃV���{���̃y�A
		Object *p = allocate(TYPE_PAIR);
		p->pair.car = var->pair.car;
		p->pair.cdr = val->pair.car;
		// �y�A�̃��X�g�֒ǉ�
		cell->pair.car = p;
		cell->pair.cdr = tbl;
		tbl = cell;
		// ���̕ϐ���
		var = var->pair.cdr;
		val = val->pair.cdr;
	}
	// �V�������֐ݒ�
	newEnv->env.up = env;
	newEnv->env.vars = tbl;
	// �V��������Ԃ�
	return newEnv;
}

// env�𒲂ׂĕϐ����Q�Ƃ���
Object *lookup(Object *env, Object *symbol) {
	Object *tbl, *p, *cell;
	// ������Ȃ�������NIL��Ԃ�
	if (env->type == TYPE_NIL) {
		return NIL;
	}
	assert(env->type == TYPE_ENV);
	// �l�ƃV���{���̃y�A�����ɒ��ׂĂ���
	tbl = env->env.vars;
	for (cell = tbl; cell->type != TYPE_NIL; cell = cell->pair.cdr) {
		p = cell->pair.car;
		// ��������
		if (p->pair.car == symbol) {
			// �l��Ԃ�
			return p->pair.cdr;
		}
	}
	// ������Ȃ��������̊��𒲂ׂ�
	return lookup(env->env.up, symbol);
}

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
			printf("func");
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

// symbol�e�[�u���֓o�^����.
// �o�^�ς݂Ȃ�o�^�ς݂�Object��Ԃ�.
Object *makeSymbol(char *buf) {
	// �e�[�u���ɓo�^�ς݂����ׂ�
	for (Object *o = SymbolTable; o->type != TYPE_NIL; o = o->pair.cdr) {
		if (strcmp(o->pair.car->symbol, buf) == 0) {
			return o->pair.car;
		}
	}
	// ���o�^�Ȃ̂ŐV��������ēo�^
	Object *obj = allocate(TYPE_SYMBOL);
	obj->symbol = (char *)malloc(strlen(buf) + 1); // \0 ��+1
	strcpy(obj->symbol, buf);
	// �y�A������ăV���{���e�[�u���̃��X�g�ւȂ���
	Object *p = allocate(TYPE_PAIR);
	p->pair.cdr = SymbolTable;
	p->pair.car = obj;
	SymbolTable = p;
	// ��������symbol��Ԃ�
	return obj;
}

Object *makeFunction(Object *params, Object *body) {
	Object *func = allocate(TYPE_FUNCTION);
	func->function.params = params;
	func->function.body = body;
	return func;
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

Object *eval(Object *env, Object *obj);

Object *evalList(Object *env, Object *obj) {
	Object *newList, *tmp;
	int count = 0;
	// �y�A�łȂ����eval�����l��Ԃ�
	if (obj->type != TYPE_PAIR) {
		return eval(env, obj);
	}
	newList = allocate(TYPE_PAIR);
	tmp = newList;
	// �]�����Ēl��V�������X�g�֓���Ă���
	for (Object *p = obj; p->type != TYPE_NIL; p = p->pair.cdr) {
		tmp->pair.car = eval(env, p->pair.car);
		tmp->pair.cdr = allocate(TYPE_PAIR);
		tmp = tmp->pair.cdr;
	}
	tmp->pair.car = NIL;
	// �V�������X�g��Ԃ�
	return newList;
}

Object *apply(Object *env, Object *sym, Object *param) {
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
		return makeFunction(args, body);
	}
	Object *func = lookup(env, sym);
	// Primitive function
	if (func->type == TYPE_PRIMITIVE) {
		return func->primitive(evalList(env, param));
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
		return lookup(env, obj);
	}
	if (obj->type == TYPE_PAIR) {
		return apply(env, obj->pair.car, obj->pair.cdr);
	}
	printf("malform on eval.");
	exit(1);
}

//�v���~�e�B�u�֐�

// List�Ȃ�nil�A���̑���t
Object *primitiveAtom(Object *args) {
	Object *arg = args->pair.car;
	if (arg->type == TYPE_PAIR) {
		return NIL;
	}
	return makeSymbol("t");
}

// ���l�A������͓��l�Ȃ�t.
// ���̑��̓A�h���X�Ŕ�r.(�܂蓯��A�h���X�Ȃ�t)
Object *primitiveEq(Object *args) {
	Object *one = args->pair.car;
	Object *two = args->pair.cdr->pair.car;
	// �^���Ⴆ�ΈقȂ�
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

Object *primitiveCons(Object *args) {
	Object *one = args->pair.car;
	Object *two = args->pair.cdr->pair.car;
	Object *pair = allocate(TYPE_PAIR);
	pair->pair.car = one;
	pair->pair.cdr = two;
	return pair;
}

Object *primitiveCar(Object *args) {
	return args->pair.car->pair.car;
}

Object *primitiveCdr(Object *args) {
	return args->pair.car->pair.cdr;
}

// �v���~�e�B�u�֐���`
void definePrimitive(Object *sym, Primitive func) {
	Object *obj = allocate(TYPE_PRIMITIVE);
	obj->primitive = func;
	define(sym, obj);
}

// �������֐�
// ���̊֐����Ăяo���O�ɕK���Ăяo������
void initialize() {
	T = allocate(TYPE_T);
	NIL = allocate(TYPE_NIL);
	SymbolTable = NIL;
	TopEnv = makeEnv(NIL, NIL, NIL);
	// �\���̒�`
	define(makeSymbol("nil"), NIL);
	define(makeSymbol("t"), T);
	definePrimitive(makeSymbol("atom"), primitiveAtom);
	definePrimitive(makeSymbol("eq"), primitiveEq);
	definePrimitive(makeSymbol("cons"), primitiveCons);
	definePrimitive(makeSymbol("car"), primitiveCar);
	definePrimitive(makeSymbol("cdr"), primitiveCdr);
}
