#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "request_handler.h"

#ifndef HANDLE_REQUEST_F

const struct http_response empty_response_template;

int free_request(struct http_request *req) {
	if(req) {
		if(req->path) {
			free(req->path);
		}

		if(req->body) {
			free(req->body);
		}

		if(req->headers) {
			struct http_header *h;
			struct http_header *i;
			for(h = req->headers->first; h != NULL; h = i) {
				i = h->next;
				if(h->key) {
					free(h->key);
				}

				if(h->value) {
					free(h->value);
				}

				free(h);
			}
		}

		free(req);
	}

	return 0;
}

struct http_header *add_header(struct http_headers *headers, char *key, size_t key_len, char *value, size_t value_len) {
	struct http_header *last = headers->last;
	if(!last) {
		headers->first = (struct http_header *)malloc(sizeof(struct http_header));
		headers->last = headers->first;
		last = headers->last;
	} else {
		last = headers->last->next;
		last = (struct http_header *)malloc(sizeof(struct http_header));
	}


	last->key = (char *)malloc(sizeof(char) * key_len);
	strncpy(last->key, key, key_len);

	last->value = (char *)malloc(sizeof(char) * value_len);
	strncpy(last->value, value, value_len);

	headers->last = last;

	return last;
}

/* don't forget to free the response
 * when you're done with it
 */
struct http_response *get_empty_http_response() {
	struct http_response *res = (struct http_response*)malloc(sizeof(struct http_response));

	memcpy((void *)res, (const void *)&empty_response_template, sizeof(struct http_response));

	return res;
}

/*
 * should obliterate the existing body,
 * if it exists
 */
size_t load_response_body(struct http_response *res, char *body, size_t body_len) {
	if(res->body) {
		free(res->body);
	}

	res->body = (char *)malloc(sizeof(char) * body_len);

	strncpy(res->body, body, body_len);

	return strlen(res->body);	
}

int test_invalid_request(struct http_request *req) {
	if(req->request_len > MAX_REQUEST_LEN) {
		return 1;
	}

	return 0;
}

struct http_response *handle_request(struct http_request *req) {
	struct http_response *echo_response = get_empty_http_response();

	if(!echo_response) {
		echo_response->code = HTTP_SERVER_ERROR;

		return echo_response;
	}

	if(test_invalid_request(req)) {
		echo_response->code = HTTP_NOT_VALID;

		return echo_response;
	}

	size_t copy_body_len = load_response_body(echo_response, req->body, req->body_len);

	if(copy_body_len == req->body_len) {
		echo_response->code = HTTP_OK;
	} else {
		echo_response->code = HTTP_SERVER_ERROR;
	}

	return echo_response;
}

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
#endif
