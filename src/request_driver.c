#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "request_handler.h"

#ifndef PASS
#define PASS 0
#endif

#ifndef FAIL
#define FAIL 1
#endif


struct http_request *get_request() {
	struct http_request *my_request = (struct http_request *)malloc(sizeof(struct http_request));	

	char *path = "https://google.com";
	char *body = "testing123!";

	my_request->body = (char *)malloc(sizeof(char) * strlen(body));
	my_request->body_len = strlen(my_request->body);
	my_request->headers = (struct http_headers *)malloc(sizeof(struct http_headers));
	my_request->headers->header_count = 0;
	my_request->http_verb = GET;
	my_request->path = (char *)malloc(sizeof(char) * strlen(path));
	my_request->path_len = strlen(my_request->path);
	my_request->request_len = my_request->body_len + my_request->path_len; // incomplete, but good to start

	return my_request;
}

int test_echo_handler() {
	struct http_request *my_request;
	my_request = get_request();

	struct http_response *my_response;
	my_response = handle_request(my_request);

	if(my_response->code != HTTP_OK) {
		fprintf(stderr, "[fail] got %d expected %d\n", my_response->code, HTTP_OK);
		exit(1);
	}

	if(strncmp(my_response->body, my_request->body, my_request->body_len) != 0) {
		fprintf(stderr, "[fail] the echo response does not equal the request body.\n");
		exit(1);
	}

	free_request(my_request);

	fprintf(stderr, "[pass] test request\n");

	return PASS;
}

int test_add_header() {
	struct http_request *my_request;
	my_request = get_request();

	struct http_header *header;
	char *test_key = "test_key";
	char *test_value = "test_value";

	header = add_header(my_request->headers, test_key, strlen(test_key), test_value, strlen(test_value));

	if(strncmp(header->key, test_key, strlen(test_key)) != 0) {
		fprintf(stderr, "[fail] header keys do not match expected. Got %s, expected %s\n", header->key, test_key);
		return FAIL;
	}

	if(strncmp(header->key, test_key, strlen(test_key)) != 0) {
		fprintf(stderr, "[fail] header values do not match expected. Got %s, expected %s\n", header->value, test_value);
		return FAIL;
	}

	fprintf(stderr, "[pass] test add header to request\n");

	return PASS;
}

int test_free_request() {
	struct http_request *my_request;
	my_request = get_request();

	char *test_key = "test_key";
	char *test_value = "test_value";
	add_header(my_request->headers, test_key, strlen(test_key), test_value, strlen(test_value));

	free_request(my_request);

	fprintf(stderr, "[pass] free request\n");

	return PASS;
}

int test_request_parse() {
	char *sample_request = "GET / HTTP/1.1\r\n\
Host: www.example.com\r\n\
User-Agent: curl/7.69.1\r\n\
Accept: */*\r\n\
\r\n";

	string_to_request(sample_request, strlen(sample_request));

	return PASS;
}

int test_malformed_string_parse() {
	char *bad_request = "efneN39okGESSFKgKY9cCf$QTdr?koytdx#$x$asKahi6rhRdmNJn4d#K5f7g$AhTxSn?XmTAJKzsrcd";

	string_to_request(bad_request, strlen(bad_request));

	return PASS;
}

int main(int argc, char **argv) {
	(void)argc;
	(void)argv;

	test_echo_handler();
	test_add_header();
	test_free_request();
	test_request_parse();
	test_malformed_string_parse();
}
