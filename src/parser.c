#include "request_handler.h"
#include "parser.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int http_verb_to_i(char *verb) {
	if (strcmp(verb, "GET") == 0) {
		return GET;
	} else if (strcmp(verb, "PUT") == 0) {
		return PUT;
	} else if (strcmp(verb, "POST") == 0) {
		return POST;
	} else if (strcmp(verb, "DELETE") == 0) {
		return DELETE;
	}

	return -1;
}

struct http_request *string_to_request(char *req, size_t req_len) {
	char *front = req;
	const char *delim = "\r\n";
	size_t delim_len = strlen(delim);

	struct http_request *http_req = (struct http_request *)malloc(sizeof(struct http_request));

	char *c = strstr(front, delim);

	char *request_line;
	if (c > front) {
		size_t request_line_len = c - front + 1;
		request_line = (char *)malloc(sizeof(char) * request_line_len);
		bzero(request_line, request_line_len);
		strncpy(request_line, front, request_line_len - 1);

		printf("%s\n", request_line);
		front += strlen(request_line) + delim_len;

		c += delim_len;
	}

	char *context;
	char d = ' ';
	char *http_verb = strtok_r(request_line, &d, &context);
	http_req->http_verb = http_verb_to_i(http_verb);

	char *http_path = strtok_r(NULL, &d, &context); 
	http_req->path = (char *)malloc(sizeof(char) * strlen(http_path));
	strncpy(http_req->path, http_path, strlen(http_path));

	c = strstr(front, delim);
	while(c) {
		size_t header_line_len = c - front + 1;
		char header[header_line_len];
		bzero(header, header_line_len);
		strncpy(header, front, header_line_len - 1);

		char *key;
		char *value;
		char *colon = strstr(front, ":");

		if (!colon) {
			c = strstr(front, delim);
			continue;
		}

		colon++;

		size_t key_len = colon - front;

		while(*colon == ' ') colon++;
		size_t value_len = c - colon - 1;

		key = (char *)malloc(sizeof(char) * key_len);
		bzero(key, key_len + 1);
		strncpy(key, front, key_len);

		value = (char *)malloc(sizeof(char) * value_len);
		bzero(value, value_len + 1);
		strncpy(value, colon + 1, value_len);

		printf("key: %s, val: %s\n", key, value);

		front += strlen(header) + delim_len;
		c += delim_len;

	}

	return 0;
}
