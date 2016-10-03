#define MAX_TOKEN_LENGTH 256

typedef enum _objType {
	TYPE_PAIR,
	TYPE_INTEGER,
	TYPE_SYMBOL,
	TYPE_STRING,
	TYPE_NIL,
	TYPE_ENV
} ObjType;

typedef struct _object {
	ObjType type;
	union {
		struct {
			struct _object *car;
			struct _object *cdr;
		} pair;
		int integer;
		char *symbol;
		char *string;
		struct {
			struct _object *vars;
			struct _object *up;
		} env;
	};
} Object;

extern Object *TopEnv;

int getToken(char *buf, FILE *fp);
void ungetToken(char *buf);
Object *allocate(ObjType type);
void print(Object *obj);
Object *read(FILE *fp);
Object *eval(Object *obj);
Object *lookup(Object *env, Object *symbol);
Object *makeEnv(Object *env, Object *vars, Object *vals);
Object *makeInteger(char *buf);
Object *makeSymbol(char *buf);
Object *makeString(char *buf);
Object *define(Object *sym, Object *val);
void initialize();
