#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <string.h>
#include "leptjson.h"

typedef struct {
	const char* json;
} lept_context;

#define EXPECT(c, ch) \
	do {\
		assert(*c->json == (ch));\
		c->json++;\
	} while(0)

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

/*
* null = "null"
*/
static int lept_parse_null(lept_context* c, lept_value* v) {
	EXPECT(c, 'n');
	if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l') {
		return LEPT_PARSE_INVALID_VALUE;
	}
	c->json += 3;
	v->type = LEPT_NULL;

	return LEPT_PARSE_OK;
}

/*
* true = "true"
*/
static int lept_parse_true(lept_context* c, lept_value* v) {
	EXPECT(c, 't');
	if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e') {
		return LEPT_PARSE_INVALID_VALUE;
	}
	c->json += 3;
	v->type = LEPT_TRUE;

	return LEPT_PARSE_OK;
}

/*
* false = "false"
*/
static int lept_parse_false(lept_context* c, lept_value* v) {
	EXPECT(c, 'f');
	if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e') {
		return LEPT_PARSE_INVALID_VALUE;
	}
	c->json += 4;
	v->type = LEPT_FALSE;

	return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_context* c, lept_value* v) {
	char* end;
	
	/* 
	* begin with '+'\ '.'  is invalid number
	* end with '.' is invalid number
	*/
	if (c->json[0] == '+' || c->json[0] == '.' || c->json[strlen(c->json) - 1] == '.') {
		return LEPT_PARSE_INVALID_VALUE;
	}

	/*
	* after the first zero should be '.' or nothing
	*/
	if (c->json[0] == '0') {
		if (c->json[1] == '.' || c->json[1] == '\0') {
			// It's allowed. We should always try to write truthy conditions.
		} else {
			return LEPT_PARSE_ROOT_NOT_SINGULAR;
		}
	}

	v->n = strtod(c->json, &end);
	if (c->json == end) {
		return LEPT_PARSE_INVALID_VALUE;
	}
	c->json = end;
	v->type = LEPT_NUMBER;

	return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
	switch(*c->json) {
		case 'n': return lept_parse_null(c, v);
		case 't': return lept_parse_true(c, v);
		case 'f': return lept_parse_false(c, v);
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
		if (*c.json == '\0') {
			return LEPT_PARSE_OK;
		}
		
		if (!isWhiteSpace(*c.json)) {
			return LEPT_PARSE_INVALID_VALUE;
		}

		lept_parse_whitespace(&c);
		
		if (*c.json != '\0') {
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

