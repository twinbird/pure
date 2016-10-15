#define MAX_TOKEN_LENGTH 256
#define GC_THRESHOLD_BYTES 1000000 // 1MB

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

void initialize();
Object *read(Object *env, FILE *fp);
Object *eval(Object *env, Object *obj);
void print(Object *obj);
