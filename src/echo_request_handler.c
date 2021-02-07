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


#endif
