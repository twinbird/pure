#define MAX_TOKEN_LENGTH 256

typedef enum _objType {
	TYPE_PAIR,
	TYPE_INTEGER,
	TYPE_SYMBOL,
	TYPE_STRING,
	TYPE_NIL
} ObjType;

typedef struct _object {
	ObjType type;
	union {
		struct {
			struct _object *car;
			struct _object *cdr;
		} pair;
		int integer;
		char symbol[MAX_TOKEN_LENGTH];
		char *string;
	};
} Object;

int getToken(char *buf, FILE *fp);
void ungetToken(char *buf);
Object *allocate(ObjType type);
void print(Object *obj);
Object *read(FILE *fp);
