#define MAX_TOKEN_LENGTH 256

typedef enum _objType {
	TYPE_PAIR,
	TYPE_INTEGER,
	TYPE_SYMBOL,
	TYPE_STRING,
	TYPE_NIL,
	TYPE_ENV,
	TYPE_PRIMITIVE,
	TYPE_T,
	TYPE_FUNCTION
} ObjType;

// Primitive function type
typedef struct _object *(*Primitive)(struct _object*);

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
		Primitive primitive;
		struct {
			struct _object *params;
			struct _object *body;
		} function;
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
