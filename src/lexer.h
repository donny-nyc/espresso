#ifndef HTTP_LEXER
#define HTTP_LEXER

#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <time.h>
#include <unistd.h>

#define MAX_TEST_STRING 100

typedef int token_t;

struct http_token {
	token_t type;
	size_t value_len;
	char *value;
};

// <stdin.h>
int test_getchar() {
	time_t start, end;
	char test_string[MAX_TEST_STRING];

	start = time(NULL);
	char *c = test_string;
	while((*c = getchar()) != EOF) {
		if (c - test_string >= MAX_TEST_STRING) break;
		c++;
	}
	end = time(NULL);

	printf("test getchar() -- %f\n", difftime(end, start));

	return 0;
}

// <unistd.h.>
// <errno.h>
int test_read() {
	time_t start, end;
	char test_string[MAX_TEST_STRING];

	start = time(NULL);
	int res = read(0, test_string, MAX_TEST_STRING);
	end = time(NULL);

	printf("test read() -- %d %f\n", res, difftime(end, start));

	return 0;
}

int main(int argc, char **argv) {
	(void) argc;
	(void) argv;

	test_getchar();
	test_read();

	return 0;
}

#endif
