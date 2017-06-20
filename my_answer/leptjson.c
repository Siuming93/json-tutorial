#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>  
#include <errno.h>  
#include "leptjson.h"

#define EXPECT(c, ch) do { assert(*c->json == (ch)); c->json++; } while(0)

typedef struct {
	const char* json;
	char* stack;
	rsize_t size, top;
}lept_context;

#ifndef LEPT_PARSE_STACK_INIT_SIZE
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif // LEPT_PARSE_STACK_INIT_SIZE


static void* lept_context_push(lept_context* c, size_t size)
{
	void* ret;
	assert(size > 0);
	if (c->top + size >= c->size)
	{
		if (c->size == 0)
			c->size = LEPT_PARSE_STACK_INIT_SIZE;
		while (c->top + size >= c->size)
		{
			c->size << 1;	/*c->size *2 */
		}
		c->stack = (char*)realloc(c->stack, c->size);
	}
	ret = c->stack + c->top;
	return ret;
}

static void* lept_context_pop(lept_context* c, rsize_t size)
{
	assert(c->top >= size);
}

/*ws = *(%x20 / %x09 / %x0A / %x0D) */
static void lept_parse_whitespace(lept_context* c) {
	char *p = c->json;
	while (*p == ' ' || *p == '\t' ||*p == '\n' || *p == '\r')
	{
		p++;
	}
	c->json = p;
}

static int lept_parse_literal(lept_context* c, lept_value* v, const char* literal, lept_type type)
{
	EXPECT(c, literal[0]);
	size_t i;
	for (i = 0; literal[i + 1]; i++)
		if (c->json[i] != literal[i + 1])
			return LEPT_PARSE_INVALID_VALUE;
	c->json += i;
	v->type = type;
	return LEPT_PARSE_OK;

}
/*null = "null"*/
static int lept_parse_null(lept_context* c, lept_value* v)
{
	EXPECT(c, 'n');
	if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
		return LEPT_PARSE_INVALID_VALUE;
	c->json += 3;
	v->type = LEPT_NULL;
	return LEPT_PARSE_OK;
}

/*true = "true"*/
static int lept_parse_true(lept_context* c, lept_value* v)
{
	EXPECT(c, 't');
	if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
		return LEPT_PARSE_INVALID_VALUE;
	c->json += 3;
	v->type = LEPT_TRUE;
	return LEPT_PARSE_OK;
}

/*false = "false"*/
static int lept_parse_false(lept_context* c, lept_value* v)
{
	EXPECT(c, 'f');
	if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
		return LEPT_PARSE_INVALID_VALUE;
	c->json += 4;
	v->type = LEPT_FALSE;
	return LEPT_PARSE_OK;
}

#define ISDIGIT(ch) (ch >= '0' && ch <= '9')
#define ISDIGIT1TO9(ch) (ch >= '1' && ch <='9') 

static int lept_parse_number(lept_context* c, lept_value* v)
{
	int step = 0;
	int errorCode = -1;
	char* p = c->json;
	if (*p == '-')
	{
		p++;
	}
	if (*p == '0')
	{
		p++;
	}
	else
	{
		if (ISDIGIT1TO9(*p))
			for (; ISDIGIT(*p); p++);
		else
			return LEPT_PARSE_INVALID_VALUE;
	}

	if (*p == '.')
	{
		p++;
		if (!ISDIGIT(*p))
			return LEPT_PARSE_INVALID_VALUE;
		for (p++; ISDIGIT(*p); p++);
	}

	if (*p == 'e' || *p == 'E')
	{
		p++;
		if (*p == '+' || *p == '-')
		{
			p++;
		}
		if (!ISDIGIT(*p))
			return LEPT_PARSE_INVALID_VALUE;
		for (p++; ISDIGIT(*p); p++);
	}

	char* end;
 	v->u.n = strtod(c->json, NULL);
	c->json = p;
	v->type = LEPT_NUMBER; 
	if (errno == ERANGE && (v->u.n == HUGE_VAL || v->u.n == -HUGE_VAL))
		return LEPT_PARSE_NUMBER_TOO_BIG;
	return LEPT_PARSE_OK;
}

void lept_free(lept_value* v)
{
	assert(v != NULL);
	if (v->type == LEPT_STRING)
		free(v->u.s.s);
	v->type = LEPT_NULL;
}

static int lept_parse_value(lept_context* c, lept_value* v)
{
	switch (*c->json)
	{
		case 'n':	return lept_parse_literal(c, v, "null", LEPT_NULL);
		case '\0':	return LEPT_PARSE_EXPECT_VALUE;
		case 't':	return lept_parse_literal(c, v, "true", LEPT_TRUE);
		case 'f':	return lept_parse_literal(c, v, "false", LEPT_FALSE);
		default:
			return lept_parse_number(c, v);
	}
}

int session = 1;
int lept_parse(lept_value* v, const char* json)
{
	lept_context c;
	int ret;
	assert(v != NULL);
	c.json = json;
	c.stack = NULL;
	c.size = c.top = 0;
	lept_init(v);
	lept_parse_whitespace(&c);
	if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK)
	{
		lept_parse_whitespace(&c);
		if (*c.json != '\0')
		{
			ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
			v->type = LEPT_NULL;
		}
	}
	assert(c.top == 0);
	free(c.stack);
	return ret;
}

#define lept_set_null(v) lept_free(v)

lept_type lept_get_type(const lept_value* v)
{
	assert(v != NULL);
	return v->type;
}

double lept_get_number(const lept_value* v)
{
	assert(v != NULL && v->type == LEPT_NUMBER);
	return v->u.n;
}
void lept_set_number(lept_value* v, double n)
{
	assert(v != NULL && v->type == LEPT_NUMBER);
	v->u.n = n;
}

const char* lept_get_string(const lept_value* v)
{
	assert(v != NULL && v->type == LEPT_STRING);
	return v->u.s.s;
}
size_t lept_get_string_length(const lept_value* v) 
{
	assert(v != NULL && v->type == LEPT_STRING);
	return v->u.s.len;
}
void lept_set_string(lept_value* v, const char* s, rsize_t len)
{
	assert(v != NULL && (s != NULL || len == 0));
	lept_free(v);
	v->u.s.s = (char*)malloc(len + 1);
	memcpy(v->u.s.s, s, len);
	v->u.s.s[len] = '\0';
	v->u.s.len = len;
	v->type = LEPT_STRING;
}
