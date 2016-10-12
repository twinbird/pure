#define MAX_TOKEN_LENGTH 256
#define GC_THRESHOLD_BYTES 48

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

typedef enum _gcmark {
	USED,
	UNUSED
} GCMark;

// Primitive function type
typedef struct _object *(*Primitive)(struct _object*, struct _object*);

typedef struct _object {
	ObjType type;
	GCMark gcmark;
	struct _object *next;
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
extern Object *NIL;
extern Object *T;
extern Object *EvaluatingExpression;
extern unsigned long AllocatedHeapSize;

int getToken(char *buf, FILE *fp);
void ungetToken(char *buf);
Object *allocate(Object *env, ObjType type);
void print(Object *obj);
Object *read(Object *env, FILE *fp);
Object *eval(Object *env, Object *obj);
Object *lookup(Object *env, Object *symbol);
Object *makeEnv(Object *env, Object *vars, Object *vals);
Object *makeInteger(Object *env, char *buf);
Object *makeSymbol(Object *env, char *buf);
Object *makeString(Object *env, char *buf);
Object *define(Object *sym, Object *val);
void initialize();
