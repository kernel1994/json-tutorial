#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <errno.h>   /* errno, ERANGE */
#include <math.h>    /* HUGE_VAL */
#include "leptjson.h"

typedef struct {
	const char* json;
} lept_context;

#define EXPECT(c, ch) \
	do {\
		assert(*c->json == (ch));\
		c->json++;\
	} while(0)

#define ISDIGIT(ch)     ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')

/**
* the judgement of wheather char is white space
* parm: char the char needed judge
* return: 1 white space. 0 otherwise
*/
static int isWhiteSpace(char ch) {
	if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
		return 1;
	return 0;
}

/*
* ws = *(%x20 / %x09 / %x0A / %x0D)
*/
static void lept_parse_whitespace(lept_context* c) {
	const char* p = c->json;
	while (isWhiteSpace(*p)) {
		p++;
	}
	c->json = p;
}

/**
* reafactor parse function of null\ true\ false
* 
*/
static int lept_parse_literal(lept_context* c, lept_value* v, const char* literal, lept_type type) {
	// note: use size_t type on array length or some index, rather than int or unsigned type
	size_t i;
	EXPECT(c, literal[0]);
	for (i = 0; literal[i + 1]; i++) {
		if (c->json[i] != literal[i + 1]) {
			return LEPT_PARSE_INVALID_VALUE;
		}
	}
	c->json += i;
	v->type = type;

	return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_context* c, lept_value* v) {
	const char* p = c->json;

	/* ���ţ�ֻ�� -(����) �Ϸ� */
	if (*p == '-') p++;

	/* 
	* ��������
	* 1. ������0�������һ��
	*/
	if (*p == '0') {
		p++;
	} else {
		/*
		* 2. ��һ�����ֱ���������1-9
		*/
		if (!ISDIGIT1TO9(*p)) {
			return LEPT_PARSE_INVALID_VALUE;
		}

		for (p++; ISDIGIT(*p); p++);
	}

	/*
	* С������
	* �������С���㣬�������ٳ���һ������
	*/
	if (*p == '.') {
		// ����.(С����)
		p++;

		if (!ISDIGIT(*p)) {
			return LEPT_PARSE_INVALID_VALUE;
		}

		for (p++; ISDIGIT(*p); p++);
	}

	/*
	* ָ������
	* �������e ��E��������Գ���������
	* Ȼ�����ٳ���һ������
	*/
	if (*p == 'e' || *p == 'E') {
		// ����e ��E
		p++;

		if (*p == '-' || *p == '+') p++;

		if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;

		for (p++; ISDIGIT(*p); p++);
	}

	/* ���ֹ���Ĵ��� */
	errno = 0;
	v->n = strtod(c->json, NULL);
	if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL)) {
		return LEPT_PARSE_NUMBER_TOO_BIG;
	}
	
	v->type = LEPT_NUMBER;
	c->json = p;

	return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
	switch(*c->json) {
		case 'n': return lept_parse_literal(c, v, "null", LEPT_NULL);
		case 't': return lept_parse_literal(c, v, "true", LEPT_TRUE);
		case 'f': return lept_parse_literal(c, v, "false", LEPT_FALSE);
		default: return lept_parse_number(c, v);
		case '\0': return LEPT_PARSE_EXPECT_VALUE;
	}
}

int lept_parse(lept_value* v, const char* json) {
	int ret;
	lept_context c;
	
	assert(v != NULL);
	c.json = json;
	v->type = LEPT_NULL;
	lept_parse_whitespace(&c);

	if((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
		lept_parse_whitespace(&c);
		
		if (*c.json != '\0') {
			v->type = LEPT_NULL;
			ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
		}
	}

	return ret;
}

lept_type lept_get_type(const lept_value* v) {
	assert(v != NULL);

	return v->type;
}

double lept_get_number(const lept_value* v) {
	assert(v != NULL && v->type == LEPT_NUMBER);

	return v->n;
}

