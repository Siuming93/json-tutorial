﻿#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leptjson.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format)\
	do{\
		test_count++;\
		if(equality)\
			test_pass++;\
		else{\
			fprintf(stderr, "%s:%d: expect: "format" actual:"format"\n", __FILE__, __LINE__, expect,actual);\
			main_ret = 1;\
		}\
	}while (0)
#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")

#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%f")

#define EXPECT_EQ_STRING(expect, actual, alength) \
    EXPECT_EQ_BASE(sizeof(expect) - 1 == alength && memcmp(expect, actual, alength) == 0, expect, actual, "%s")


#define TEST_ERROR(error, json)\
	do {\
		lept_value v;\
		v.type = LEPT_FALSE;\
		EXPECT_EQ_INT(error, lept_parse(&v, json)); \
		EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));\
	}\
	while (0)

#define TEST_NUMBER(expect, json)\
	do{\
		lept_value v;\
		v.type = LEPT_FALSE;\
		EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, json));\
		EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(&v));\
		EXPECT_EQ_DOUBLE(expect, lept_get_number(&v));\
	}\
	while (0)

static void test_parse_expect_literal()
{
	lept_value v;
	v.type = LEPT_FALSE;
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "null"));
	EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
	v.type = LEPT_TRUE;
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "false"));
	EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(&v));
	v.type = LEPT_FALSE;
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "true"));
	EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(&v));
}

static void test_parse_expect_value()
{
	TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, "");
	TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, "  ");
}

static void test_parse_invalid_value()
{
	/*invalid number*/
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+0");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+1");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, ".123");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "123.");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "inf");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nan");

	/*invalida array*/
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "[12,22,false,]");
}

static void test_parse_root_not_singular()
{
	lept_value v;
	v.type = LEPT_FALSE;
	EXPECT_EQ_INT(LEPT_PARSE_ROOT_NOT_SINGULAR, lept_parse(&v, "null x"));
	EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
	EXPECT_EQ_INT(LEPT_PARSE_ROOT_NOT_SINGULAR, lept_parse(&v, "null 55"));
	EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
	EXPECT_EQ_INT(LEPT_PARSE_ROOT_NOT_SINGULAR, lept_parse(&v, "-1 5"));
	EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
	EXPECT_EQ_INT(LEPT_PARSE_ROOT_NOT_SINGULAR, lept_parse(&v, "00556123"));
	EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));

}

static void test_parse_number_value()
{ 
	TEST_NUMBER(0.0, "-0");
	TEST_NUMBER(1.0, "1");
	TEST_NUMBER(-1.0, "-1");
	TEST_NUMBER(-1.2, "-1.2");
	TEST_NUMBER(1.2E5, "1.2E5");
	TEST_NUMBER(-1.2E+5, "-1.2E+5");
	TEST_NUMBER(-1.2E-5, "-1.2E-5");
	TEST_NUMBER(0.0, "-0.0");
	TEST_NUMBER(0.0, "-0.0");
	TEST_NUMBER(0.0, "-0.0  ");
	TEST_NUMBER(0.0, "1E-100000");

	TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
	TEST_NUMBER(4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
	TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
	TEST_NUMBER(2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
	TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
	TEST_NUMBER(2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
	TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
	TEST_NUMBER(1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
	TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

static void test_parse_number_too_big()
{
	lept_value v;
	EXPECT_EQ_INT(LEPT_PARSE_NUMBER_TOO_BIG, lept_parse(&v, "-1e309"));
	EXPECT_EQ_INT(LEPT_PARSE_NUMBER_TOO_BIG, lept_parse(&v, "1e30009"));
}

static void test_access_string()
{
	lept_value v;
	lept_init(&v);
	lept_set_string(&v, "", 0);
	EXPECT_EQ_STRING("", lept_get_string(&v), lept_get_string_length(&v));
	lept_set_string(&v, "Hello", 5);
	EXPECT_EQ_STRING("Hello", lept_get_string(&v), lept_get_string_length(&v));
	lept_free(&v);
}

#define TEST_STRING(expect, json)\
    do {\
        lept_value v;\
        lept_init(&v);\
        EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, json));\
        EXPECT_EQ_INT(LEPT_STRING, lept_get_type(&v));\
        EXPECT_EQ_STRING(expect, lept_get_string(&v), lept_get_string_length(&v));\
        lept_free(&v);\
    } while(0)

static void test_parse_string() {
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
}

static void test_access_array()
{
	lept_value v;
	lept_value v1, v2, v3, v4;
	lept_parse(&v1, "12");
	lept_parse(&v2, "-0");
	lept_set_string(&v3, "hello", 5);
	lept_set_string(&v4, "", 0);
	lept_value arr[] = { v1,v2,v3,v4 };
	lept_set_array(&v, arr, 4);
	EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(&v));
	lept_value va = v.u.a.e[0];
	EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(&va));
	EXPECT_EQ_DOUBLE(12, va.u.n);
	va = v.u.a.e[1];
	EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(&va));
	EXPECT_EQ_DOUBLE(0, va.u.n);
	va = v.u.a.e[2];
	EXPECT_EQ_INT(LEPT_STRING, lept_get_type(&va));
	EXPECT_EQ_STRING("hello", lept_get_string(&va), lept_get_string_length(&va));
	va = v.u.a.e[3];
	EXPECT_EQ_INT(LEPT_STRING, lept_get_type(&va));
	EXPECT_EQ_STRING("", lept_get_string(&va), lept_get_string_length(&va));
	lept_free(&v);
}

static void test_parse_array()
{
	lept_value v;
	lept_parse(&v, "[12,22,false,\"hello\"]");
	EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(&v));
	EXPECT_EQ_INT(4, lept_get_array_size(&v));
	EXPECT_EQ_DOUBLE(12, lept_get_number(v.u.a.e));
	EXPECT_EQ_DOUBLE(22, lept_get_number(v.u.a.e + 1));
	EXPECT_EQ_INT(0, lept_get_boolen(v.u.a.e + 2));
	EXPECT_EQ_STRING("hello", lept_get_string(v.u.a.e + 3), lept_get_string_length(v.u.a.e + 3));
	lept_parse(&v, "[]");
	EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(&v));
	EXPECT_EQ_INT(0, lept_get_array_size(&v));
}

static void test_access_object()
{

}

static void test_parse_object()
{

}

#define TEST_ERROR(error, json)\
    do {\
        lept_value v;\
        lept_init(&v);\
        v.type = LEPT_FALSE;\
        EXPECT_EQ_INT(error, lept_parse(&v, json));\
        EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));\
        lept_free(&v);\
    } while(0)

static void test_parse() {
	test_parse_expect_literal();
	test_parse_expect_value();
	test_parse_number_value();
	test_parse_root_not_singular();
	test_parse_invalid_value();
	test_parse_number_too_big();
	test_access_string();
	test_parse_string();
	test_access_array();
	test_parse_array();
}

int main(){
	test_parse();
	printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass*100.0 / test_count);
	system("pause");
	return main_ret;
}
