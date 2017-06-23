#ifndef LEPTJSON_H__
#define LEPTJSON_H__
typedef enum {LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT}lept_type;

typedef struct lept_member lept_member;
typedef struct lept_value lept_value;

struct lept_value {
	union {
		struct { lept_member* m; size_t size;}o;
		struct { lept_value* e; size_t size;}a;
		struct { char* s; size_t len;}s;
		double n;
	}u;
	lept_type type;
};
struct lept_member{
	char* k; size_t klen;
	lept_value v;
};

typedef enum {
	LEPT_PARSE_OK = 0,
	LEPT_PARSE_EXPECT_VALUE,
	LEPT_PARSE_INVALID_VALUE,
	LEPT_PARSE_ROOT_NOT_SINGULAR,
	LEPT_PARSE_NUMBER_TOO_BIG,
}lept_result;

#define lept_init(v) do{(v)->type = LEPT_NULL;} while(0)
#define lept_set_null(v) lept_free(v)

int lept_parse(lept_value* v, const char* json);
lept_type lept_get_type(const lept_value* v);

int lept_get_boolen(const lept_value* v);
void lept_set_boolen(lept_value* v, int b);

double lept_get_number(const lept_value* v);
void lept_set_number(lept_value* v, double n);

const char* lept_get_string(const lept_value* v);
size_t lept_get_string_length(const lept_value* v);
void lept_set_string(lept_value* v, const char* s, size_t len);

const lept_value* lept_get_array_value(const lept_value* v);
size_t lept_get_array_size(const lept_value* v);
void lept_set_array(lept_value* v, const lept_value* arr, size_t size);

const lept_member* lept_get_object_value(const lept_value* v);
size_t lept_get_object_size(const lept_value* v);
void lept_set_object(lept_value* v, const lept_member* m, size_t size);
#endif