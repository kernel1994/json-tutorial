#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leptjson.h"

static int test_count = 0;
static int test_pass = 0;
static int main_ret = 0;

/* 
* multi line macro code should be warpped by do {} while(0)
* void to break if-else statement.
*/
#define EXPECT_EQ_BASE(equality, expect, actual, format) \
	do {\
		test_count++;\
		if(equality) {\
			test_pass++;\
		} else {\
			fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
			main_ret = 1;\
		}\
	} while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")

#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")

#define TEST_ERROR(error, json) \
	do {\
		lept_value v;\
		v.type = LEPT_NULL;\
		EXPECT_EQ_INT(error, lept_parse(&v, json));\
	} while(0)

#define TEST_NUMBER(expect, json) \
	do {\
		lept_value v;\
		v.type = LEPT_NUMBER;\
		EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, json));\
		EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(&v));\
		EXPECT_EQ_DOUBLE(expect, lept_get_number(&v));\
	} while(0)

void test_parse_expect_value() {
	TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, "");
	TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, " ");
	TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, "\n");
	TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, "\0");
	TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, "\t");
	TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, "\r");
}

void test_parse_invalid_value() {
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nul");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nulx");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nullx");

	 /* invalid number */
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+0");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+1");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "0.");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "1.");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, ".1");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "INF");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "NAN");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nan");
}

void test_parse_root_not_singular() {
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "null x");
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "false a");
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "null true");

	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0123"); /* after zero should be '.' or nothing */
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0x0");
    TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0x123");
}

void test_parse_null() {
	TEST_ERROR(LEPT_PARSE_OK, "null");
}

void test_parse_true() {
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "tru");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "trux");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "truex");
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "true x");
	TEST_ERROR(LEPT_PARSE_OK, "true");
}

void test_parse_false() {
	TEST_ERROR(LEPT_PARSE_OK, "false");
}

void test_parse_number() {
	TEST_NUMBER(0.0, "0");
	TEST_NUMBER(0.0, "-0");
	TEST_NUMBER(0.0, "-0.0");
	TEST_NUMBER(1.0, "1");
	TEST_NUMBER(1.0, "1.0");
	TEST_NUMBER(-1.0, "-1");
	TEST_NUMBER(3.14159, "3.14159");
	TEST_NUMBER(1E10, "1E10");
	TEST_NUMBER(1e10, "1e10");
	TEST_NUMBER(1E+10, "1E+10");
	TEST_NUMBER(1E-10, "1E-10");
	TEST_NUMBER(-1E10, "-1E10");
	TEST_NUMBER(-1e10, "-1e10");
	TEST_NUMBER(12.34E10, "12.34E10");
	TEST_NUMBER(12.34e10, "12.34e10");
	TEST_NUMBER(0.0, "1e-10000"); /* must underflow */
}

void test_parse() {
	test_parse_null();
	test_parse_true();
	test_parse_false();
	test_parse_expect_value();
	test_parse_invalid_value();
	test_parse_root_not_singular();
	test_parse_number();
}

int main() {
	test_parse();
	printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);

	return main_ret;
}
