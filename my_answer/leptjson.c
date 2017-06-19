#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>  
#include <errno.h>  
#include "leptjson.h"

typedef struct {
	const char* json;
}lept_context;

#define EXPECT(c, ch) do { assert(*c->json == (ch)); c->json++; } while(0)

#define ISWHITESPACE(ch) (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')

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
 	v->n = strtod(c->json, NULL);
	c->json = p;
	v->type = LEPT_NUMBER; 
	if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL))
		return LEPT_PARSE_NUMBER_TOO_BIG;
	return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v){
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
	v->type = LEPT_NULL;
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
	return ret;
}

lept_type lept_get_type(const lept_value* v)
{
	assert(v != NULL);
	return v->type;
}

double lept_get_number(const lept_value* v)
{
	assert(v != NULL && v->type == LEPT_NUMBER);
	return v->n;
}

