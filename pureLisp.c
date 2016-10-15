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

// ungetToken�p�̃o�b�t�@
static char getTokenBuffer[MAX_TOKEN_LENGTH] = {'\0'};

// symbol�e�[�u��(initialize�ŏ�����)
static Object *SymbolTable = NULL;

// �g�b�v���x���̊�(initialize�ŏ�����)
Object *TopEnv = NULL;

// GC�p �I�u�W�F�N�g�e�[�u��
// �S�ẴI�u�W�F�N�g�͂��̃e�[�u���ɑ�����
static Object *ObjectTable = NULL;

// GC�p ���蓖�čς݃q�[�v�T�C�Y�J�E���^
static unsigned long AllocatedHeapSize = 0;

// GC�p �X�^�b�N�̊J�n�ʒu
static Object *StackStartPosition = NULL;

// NIL�萔(initialize�ŏ�����)
static Object *NIL = NULL;
// T�萔(initialize�ŏ�����)
static Object *T = NULL;

// 1�̊Ԃ�GC���Ȃ�
static int GCLockFlag = 0;

static void GCLock() {
	GCLockFlag = 1;
}

static void GCUnlock() {
	GCLockFlag = 0;
}

static Object *allocate(Object *env, ObjType type);

// �X�y�[�X�ȊO�̕����񂪗���܂œ��͂��̂Ă�
static void skipSpace(FILE *fp) {
	int c = fgetc(fp);
	// ���̌�ɑ����X�y�[�X�Ɖ��s�ƃ^�u�͎̂Ă�
	while (c == ' ' || c == '\n' || c == '\t') {
		c = fgetc(fp);
	}
	// �X�y�[�X�ł͂Ȃ�������͖߂��Ă���
	ungetc(c, fp);
}

// �g�b�v���x���̊��ŕϐ����`����
// ��`�ς݂Ȃ�l���㏑������
// �Ԃ�l�̓V���{��
static Object *define(Object *sym, Object *val) {
	Object *newCell = allocate(TopEnv, TYPE_PAIR);
	// �V�����ϐ�(�V���{���ƒl)�̃y�A
	Object *newVar = allocate(TopEnv, TYPE_PAIR);
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
static Object *makeEnv(Object *env, Object *vars, Object *vals) {
	// �V������
	Object *newEnv = allocate(env, TYPE_ENV);
	// �l�ƃV���{���̃y�A�̃��X�g(NIL�����ċ󃊃X�g�ɂ��Ă���)
	Object *tbl = NIL;

	Object *var = vars;
	Object *val = vals;
	while (var->type != TYPE_NIL) {
		// tbl�ւ̒ǉ��p�̃y�A
		Object *cell = allocate(env, TYPE_PAIR);
		// �l�ƃV���{���̃y�A
		Object *p = allocate(env, TYPE_PAIR);
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
// ����`�Ȃ�NULL��Ԃ�
static Object *lookup(Object *env, Object *symbol) {
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
	if (env == TopEnv) {
		// ����`������
		return NULL;
	} else {
		// ������Ȃ��������̊��𒲂ׂ�
		return lookup(env->env.up, symbol);
	}
}

/*
 * fp������͂����, 1�Ԃ�̃g�[�N������buf�֓����.
 * buf�͌Ăяo�����̐ӔC�ŏ���������K�v������.
 * �߂�l:
 *   0: ���͂ɑ���������.
 *   1: ���͂��I�[�ɂȂ���.
 */
static int getToken(char *buf, FILE *fp) {
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
static void ungetToken(char *buf) {
	strcpy(getTokenBuffer, buf);
}

// ���g�p��symbol��symbol�e�[�u�������菜��
static void removeUnusedSymbol() {
	Object *tmp, *prev;
	Object *symPair;
	for (prev = tmp = SymbolTable; tmp->type != TYPE_NIL; prev = tmp, tmp = tmp->pair.cdr) {
		symPair = tmp->pair.car;
		if (symPair->pair.car->gcmark == UNUSED) {
			// ���X�g���Ȃ��ς���
			prev->pair.cdr = tmp->pair.cdr;
			// ���̃��[�v�Ŏ�菜�����Z�����΂����߂�tmp�����̂��̂ɓ���ւ�
			tmp = tmp->pair.cdr;
		}
	}
}

// �I�u�W�F�N�g���J������
static void deallocate(Object *obj) {
	Object *prev, *tmp;
	// �\���͉�����Ȃ�
	switch (obj->type) {
		case TYPE_PRIMITIVE:
			return;
		case TYPE_T:
			return;
		case TYPE_NIL:
			return;
	}
	// ObjectTable�̃��X�g����O�����߂�obj�̂ЂƂO��Object��T��
	for (prev = tmp = ObjectTable; tmp != NULL; prev = tmp, tmp = tmp->next) {
		if (tmp == obj) {
			break;
		}
	}
	assert(tmp != NULL);
	// ���X�g���Ȃ��ς���
	if (tmp == ObjectTable) {
		// �擪�̏ꍇ
		ObjectTable = tmp->next;
	} else {
		prev->next = tmp->next;
	}
	// �I�u�W�F�N�g���J�����A���蓖�čς݃q�[�v�T�C�Y�𒲐�
	switch (obj->type) {
		case TYPE_STRING:
			// ������̈���J�����Ă���
			free(obj->string);
			break;
		case TYPE_SYMBOL:
			// �V���{�����̈���J�����Ă���
			free(obj->symbol);
			break;
	}
	free(obj);
	AllocatedHeapSize -= sizeof(Object);
}

// �����̃|�C���^��ObjectTable�̗̈���ɓ����Ă�����1��Ԃ�
static int maybeObject(Object *obj) {
	unsigned long addr = (unsigned long)obj;
	unsigned long tmpAddr;
	// ObjectTable��1�����ׂĊm�ۂ����̈���ɂ��邩���ׂ�.
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

// ���݂̊����N�_�ɍċA�I�Ƀ}�[�N����
static void GCMarkingEnv(Object *basePair) {
	// ���̊��͎g���Ă���
	basePair->gcmark = USED;
	// ���݂̊������ׂă}�[�L���O
	if (basePair->env.vars != NIL && (maybeObject(basePair->env.vars) == 1)) {
		GCMarkingSub(basePair->env.vars);
	}
	// ��ʊ����}�[�L���O
	if (basePair->env.up != NIL && (maybeObject(basePair->env.up) == 1)) {
		GCMarkingEnv(basePair->env.up);
	}
}

// ���W�X�^�̈���}�[�N����
static void GCMarkingRegister() {
	int i;
	// �����_�ł̃��W�X�^�̃X�i�b�v�V���b�g���Ƃ�
	jmp_buf registerSnapshot;
	setjmp(registerSnapshot);
	// �X�i�b�v�V���b�g�̐擪���|�C���^�ɓ���Ă���
	Object **chkTgt = (Object **)registerSnapshot;
	// �I�u�W�F�N�g�ւ̃|�C���^�����W�X�^�ɓ���ő�������߂�
	unsigned long maxFittingSize = sizeof(registerSnapshot) / sizeof(Object *);

	// ���W�X�^�������ɒ��ׂ�
	for (i = 0; i < maxFittingSize; i++) {
		if (maybeObject(*chkTgt) == 1) {
			// Object���ۂ�������}�[�N
			GCMarkingSub(*chkTgt);
		}
		chkTgt++;
	}
}

// �X�^�b�N�̈���}�[�N����
static void GCMarkingStack() {
	// GC�p �X�^�b�N�I��(�擪)�̃A�h���X�m�F�p
	Object stackEndObject;
	unsigned long stackEndAddr = (unsigned long)&stackEndObject;
	unsigned long stackStartAddr = (unsigned long)StackStartPosition;
	unsigned long tmpAddr = 0;
	int i;

	// �X�^�b�N�̐L�т�����𒲂ׂă}�[�L���O�J�n�ʒu�����߂�
	if (stackStartAddr > stackEndAddr) {
		tmpAddr = stackStartAddr;
		stackStartAddr = stackEndAddr;
		stackEndAddr = tmpAddr;
	}
	// �I�u�W�F�N�g�ւ̃|�C���^���X�^�b�N�ɓ���ő�������߂�
	unsigned long maxFittingSize = (stackEndAddr - stackStartAddr) / sizeof(Object *);
	Object **chkTgt = (Object **)stackStartAddr;
	// �X�^�b�N���}�[�L���O
	for (i = 0; i < maxFittingSize; i++) {
		if (maybeObject(*chkTgt) == 1) {
			// Object���ۂ�������}�[�N
			GCMarkingSub(*chkTgt);
		}
		chkTgt++;
	}
}

// �}�[�N&�X�C�[�v�̃}�[�N����
static void GCMarking(Object *rootEnv) {
	// �S�Ĉ�xUNUSED�ɂ���
	for (Object *tmp = ObjectTable; tmp != NULL; tmp = tmp->next) {
		tmp->gcmark = UNUSED;
	}
	// �����̊����N�_�Ƀ}�[�L���O
	if (rootEnv != NULL && rootEnv != NIL) {
		GCMarkingEnv(rootEnv);
	}
	// �X�^�b�N�ƃ��W�X�^���}�[�L���O
	GCMarkingStack();
	GCMarkingRegister();
	// ��L�܂łŃ}�[�N����Ă��Ȃ��V���{�����V���{���e�[�u��������O��
	removeUnusedSymbol();
	// ���p���̃V���{���̂݊܂񂾃V���{���e�[�u���S�̂��}�[�L���O
	GCMarkingSub(SymbolTable);
}

// �}�[�N&�X�C�[�v�̃X�C�[�v����
// ObjectTable�̏��ɒH����Mark�ς݂̂��̂��J������
static void GCSweep() {
	Object *tmp = ObjectTable;
	Object *freeTgt = NULL;
	while (tmp != NULL) {
		// �폜�Ώۂւ̃|�C���^�Ƃ��đޔ�
		freeTgt = tmp;
		// ���[�v�̂��߂ɑޔ�
		tmp = tmp->next;
		// �}�[�N����Ă��Ȃ���ΊJ��
		if (freeTgt->gcmark == UNUSED) {
			DEBUG_PRINT("[GC] deallocated.\n");
			deallocate(freeTgt);
		}
	}
}

// ObjectTable�̏󋵂�����(Debug�p)
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

// �V����Object���쐬����
static Object *allocate(Object *env, ObjType type) {
	if (GCLockFlag == 0) {
		// ���蓖�čςݗe�ʂ����ȏ�ɂȂ��Ă�����GC��������
		if (AllocatedHeapSize >= GC_THRESHOLD_BYTES) {
			GCMarking(env);
			GCSweep();
		}
	}

	// �V�����I�u�W�F�N�g���쐬
	Object *obj = (Object *)malloc(sizeof(Object));
	obj->type = type;
	obj->gcmark = USED;
	// ObjectTable�֓o�^
	obj->next = ObjectTable;
	ObjectTable = obj;
	// ���蓖�čς݃q�[�v�T�C�Y���X�V
	AllocatedHeapSize += sizeof(Object);

	return obj;
}

static void printList(Object *obj);

// Object�̓��e����(atom�̂�)
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

// Object�̓��e����(list�̂�)
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
static int isInteger(char *buf) {
	// 1�����ڂ�-(�}�C�i�X)�ł��悢
	if (isdigit(*buf) == 0 && *buf != '-') {
		return 0;
	}
	// 1�����ڂ�-(�}�C�i�X)�̎���2�����ڂ͐����łȂ��ƂȂ�Ȃ�
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

// ������������ł����1��Ԃ�
static int isString(char *buf) {
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

static Object *makeInteger(Object *env, char *buf) {
	Object *obj = allocate(env, TYPE_INTEGER);
	obj->integer = atoi(buf);
	return obj;
}

static Object *makeString(Object *env, char *buf) {
	Object *obj = allocate(env, TYPE_STRING);
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
static Object *makeSymbol(Object *env, char *buf) {
	// �e�[�u���ɓo�^�ς݂����ׂ�
	for (Object *o = SymbolTable; o->type != TYPE_NIL; o = o->pair.cdr) {
		if (strcmp(o->pair.car->symbol, buf) == 0) {
			return o->pair.car;
		}
	}
	// ���o�^�Ȃ̂ŐV��������ēo�^
	Object *obj = allocate(env, TYPE_SYMBOL);
	obj->symbol = (char *)malloc(strlen(buf) + 1); // \0 ��+1
	strcpy(obj->symbol, buf);
	// �y�A������ăV���{���e�[�u���̃��X�g�ւȂ���
	Object *p = allocate(env, TYPE_PAIR);
	p->pair.cdr = SymbolTable;
	p->pair.car = obj;
	SymbolTable = p;
	// ��������symbol��Ԃ�
	return obj;
}

static Object *makeFunction(Object *env, Object *params, Object *body) {
	Object *func = allocate(env, TYPE_FUNCTION);
	func->function.params = params;
	func->function.body = body;
	return func;
}

// read��list�����p�֐�
static Object *readList(Object *env, FILE *fp) {
	Object *obj = allocate(env, TYPE_PAIR);
	char buf[MAX_TOKEN_LENGTH];
	memset(buf, '\0', MAX_TOKEN_LENGTH);

	// �󃊃X�g���f�̂��߂�1�g�[�N����ǂ݂��Ė߂�
	getToken(buf, fp);
	if (strcmp(buf, ")") == 0) {
		return NIL;
	} else {
		ungetToken(buf);
	}
	obj->pair.car = read(env, fp);

	// cdr�ւ̐ݒ�̔��f�̂��߂�1�g�[�N����ǂ݂��Ė߂�
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
 * fp������Ȃ�����͂�Object�ɕϊ�����.
 * fp��open���ꂽ��ԂłȂ���΂Ȃ�Ȃ�.
 * EOF�ɒB�������ɂ�NULL��Ԃ�.
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
	// �y�A�łȂ����eval�����l��Ԃ�
	if (obj->type != TYPE_PAIR) {
		return eval(env, obj);
	}
	newList = allocate(env, TYPE_PAIR);
	tmp = newList;
	// �]�����Ēl��V�������X�g�֓���Ă���
	for (Object *p = obj; p->type != TYPE_NIL; p = p->pair.cdr) {
		tmp->pair.car = eval(env, p->pair.car);
		tmp->pair.cdr = allocate(env, TYPE_PAIR);
		tmp = tmp->pair.cdr;
	}
	tmp->pair.car = NIL;
	// �V�������X�g��Ԃ�
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

//�v���~�e�B�u�֐�

// List�Ȃ�nil�A���̑���t
static Object *primitiveAtom(Object *env, Object *args) {
	Object *arg = args->pair.car;
	if (arg->type == TYPE_PAIR) {
		return NIL;
	}
	return makeSymbol(env, "t");
}

// ���l�A������͓��l�Ȃ�t.
// ���̑��̓A�h���X�Ŕ�r.(�܂蓯��A�h���X�Ȃ�t)
static Object *primitiveEq(Object *env, Object *args) {
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

// �v���~�e�B�u�֐���`
static void definePrimitive(Object *sym, Primitive func) {
	Object *obj = allocate(TopEnv, TYPE_PRIMITIVE);
	obj->primitive = func;
	define(sym, obj);
}

// �������֐�
// ���̊֐����Ăяo���O�ɕK���Ăяo������
void initialize() {
	// GC�p �X�^�b�N�̂͂��߂ɒu�����I�u�W�F�N�g
	Object stackStartObject;
	// �X�^�b�N�J�n�ʒu�̃A�h���X���L�^
	StackStartPosition = &stackStartObject;

	// �����������܂ł�GC���s��Ȃ�
	GCLock();
	T = allocate(TopEnv, TYPE_T);
	NIL = allocate(TopEnv, TYPE_NIL);
	SymbolTable = NIL;
	TopEnv = makeEnv(NIL, NIL, NIL);
	// �\���̒�`
	define(makeSymbol(TopEnv, "nil"), NIL);
	define(makeSymbol(TopEnv, "t"), T);
	definePrimitive(makeSymbol(TopEnv, "atom"), primitiveAtom);
	definePrimitive(makeSymbol(TopEnv, "eq"), primitiveEq);
	definePrimitive(makeSymbol(TopEnv, "cons"), primitiveCons);
	definePrimitive(makeSymbol(TopEnv, "car"), primitiveCar);
	definePrimitive(makeSymbol(TopEnv, "cdr"), primitiveCdr);
	// ���͂�pure�ł͂Ȃ����
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
