#ifndef HTTP_LEXER
#define HTTP_LEXER

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_TEST_STRING 1000000

typedef int token_t;

struct msg_token {
	token_t type;
	size_t value_len;
	unsigned int *value;
	struct msg_token *next;
};

// GET / HTTP/1.1 \r\n
struct request_line {
	struct msg_token method;
	struct msg_token request_uri;
	struct msg_token version;
};

struct message_header {
	struct msg_token name;
	struct msg_token value;
};


enum general_header_fields {
	CACHE_CONTROL, /* MUST override caching behavior */
	CONNECTION, /* either `keep-alive` or `close` when done */
	DATE, /* RFC-1123 Date, eg - Tue, 15 Nov 1994 08:12:31 GMT */
	PRAGMA, /* HTTP/1.0 directives - eg pragma: no-cache. */
	TRAILER, /* for chunked transfer-encoding, what headers to expect  */
	TRANSFER_ENCODING, /* order of transformations applied to _this_  request  */
	UPGRADE, /* client can signal that it wants to use a newer protocol  */
	VIA, /* protocols used by proxies between sender and receiver  */
	WARNING /* RFC 7234, section 5.5: Warning  */
};

struct general_header {

};

enum requst_header_fields { 
	ACCEPT, /* media types the client will 'accept' in a response */
	ACCEPT_CHARSET, /* documents returned _should_ use one of these */
	ACCEPT_ENCODING, /* server _should_ error if it can't satisfy  */
	ACCEPT_LANGUAGE, /* weighted natural language preferences */
	AUTHORIZATION, /* user credentials for the requested resource  */
	EXPECT, /* required server behaviors. must error if unable to satisfy  */
	FROM, /* email for user requesting the resource  */
	HOST, /* host and port number of requested resource  */
	IF_MATCH, /* request conditional on matching ETag  */
	IF_MODIFIED_SINCE, /* respond OK only if resource changed after given date  */
	IF_NONE_MATCH, /* return if ETag is different from given  */
	IF_RANGE, /* makes the range request conditional: 206 partial else full 200  */
	IF_UNMODIFIED_SINCE, /* opposite of modified_since  */
	MAX_FORWARDS, /* proxies _must_ update. Doubt it's actually enforcable  */
	PROXY_AUTHORIZATION, /* credentials for intermediary */
	RANGE, /* partial request */
	REFERRER, /* where we're coming from (if you wanna know)  */
	TE, /* accepted transfer codings  */
	USER_AGENT /* identifiers for user's application */
};


enum entity_header_fields {
	ALLOW, /* permissible methods: eg GET, PUT */
	CONTENT_ENCODING, /* additional encodings applied to message body */
	CONTENT_LANGUAGE, /* what natural language(s) is the resource written in */
	CONTENT_LENGTH, /* size of the entity body (in bytes) */
	CONTENT_LOCATION, /* can we access the requested resource anywhere else? */
	CONTENT_MD5, /* MD5 digest of the entity-body */
	CONTENT_RANGE, /* when a partial result is returned */
	CONTENT_TYPE, /* media type being returned */
	EXPIRES, /* request a fresh copy after the given date/time */
	LAST_MODIFIED, /* self-explanatory */
};

// requested resource metadata
struct entity_header {

};

/*
 * RFC - 2616 Hypertext Transfer Protocol -- HTTP/1.1
 */

struct http_request {
	struct request_line req_line;
	struct general_header **gen_headers;
	struct reuest_headers **req_headers;
	struct entity_header **ent_headers;
};

struct msg_token_list {
	unsigned int count;
	struct msg_token *head;
	struct msg_token *tail;
};

struct msg_token *new_token(int type, unsigned int *value, size_t val_len) {
	struct msg_token *token = (struct msg_token *)malloc(sizeof(struct msg_token));

	if(type < 0) return NULL;

	token->type = type;
	token->value_len = val_len;
	token->value = (unsigned int *)malloc(sizeof(char) * val_len);

	strncpy(token->value, value, val_len);

	return token;
}

int push_token(struct msg_token_list *list, struct msg_token *token) {
	if(list == 0) return 0;

	if(list->count == 0) {
		list->head = token;
		list->tail = token;

		list->count++;
	} else {
		list->tail->next = token;
		list->tail = list->tail->next;
		
		list->count++;
	}

	return list->count;
}

#define DEBUG 1

#define START 0
#define FIRST_SCAN 1
#define FIRST_TOKEN 2
#define NEW_TOKEN 3
#define SCAN_WHITESPACE 4
#define SCAN_VALUE 5
#define SCAN_ASSIGNMENT 6
#define SCAN_EOL 7 /* * split `raw` on any value given in `delim.`
 * Do not read past `raw_len`
 */
struct msg_token *test_parse(char *raw, size_t raw_len, struct msg_token_list *list) {
	unsigned int state = START;
	struct msg_token *head = 0;
	struct msg_token *last = 0;

	char *front = 0, *back = 0;

	state = FIRST_SCAN;

	for(int i = 0; i <= raw_len; i++) {
		if(i == raw_len) {
			state = SCAN_EOL;
			back = raw + i - 1;
		}

		switch (state) {
			case FIRST_SCAN:
				if(raw[i] == ' ' || raw[i] == '\t' || raw[i] == '\n' || raw[i] == '\r') {
					state = SCAN_WHITESPACE;
				} else {
					state = SCAN_VALUE;
					front = raw + i;
				}
				break;
			case SCAN_WHITESPACE:
				if(raw[i] > 33 && raw[i] < 127) {
					state = SCAN_VALUE;
					front = raw + i;
					back = front;
				}
				break;
			case SCAN_ASSIGNMENT:

				break;
			case SCAN_VALUE:
				if(raw[i] == ' ' || raw[i] == '\t' || raw[i] == '\n' || raw[i] == '\r') {
					size_t val_len = 0;
					if(front != 0 && back != 0) {
						val_len = back - front + 1;
					}

					struct msg_token *token;

					if(val_len > 0) {
						token = new_token(1, (unsigned int *)front, val_len);

						push_token(list, token);
					}

					front = raw + i;

					/*
					if(head == 0) {
						state = FIRST_TOKEN;
						head = (struct msg_token *)malloc(sizeof(struct msg_token));
						new_token = head;
					} else {
						last->next = (struct msg_token *)malloc(sizeof(struct msg_token));
						new_token = last->next;
					}


					new_token->type = 1;
					new_token->value_len = val_len;
					new_token->value = (unsigned int *)malloc(sizeof(char) * val_len);

					strncpy((char *)new_token->value, front, val_len);


					last = new_token;

					*/
					front = back = 0;
					state = SCAN_WHITESPACE;
				} else {
					// must advance the rear so long as there
					// are values to be read
					back = raw + i;
				}
				break;
			case SCAN_EOL:
				if(front != 0 && back != 0 && front != back) {
					size_t val_len = back - front + 1;

					struct msg_token *token;

					token = new_token(1, (unsigned int *)front, val_len);

					push_token(list, token);

					front = raw + i;

					/*
					if(head == 0) {
						state = FIRST_TOKEN;
						head = (struct msg_token *)malloc(sizeof(struct msg_token));
						last = head;
						new_token = head;
					} else {
						state = NEW_TOKEN;
						last->next = (struct msg_token *)malloc(sizeof(struct msg_token));
						new_token = last->next;
					}

					new_token->type = 1;
					new_token->value_len = val_len;
					new_token->value = (unsigned int *)malloc(sizeof(char) * val_len);

					new_token->next = 0;
					last = 0;
					strncpy((char *)new_token->value, front, val_len);
					if(DEBUG) {
						printf("[debug] new_token->value: %s\n", (char *)new_token->value);
					}

					front = back = 0;
					*/
				}
				break;
		}
	}
	return head;
}

int main(int argc, char **argv) {
	(void) argc;
	(void) argv;

	char *sample_request = "GET / HTTP/1.1\r\n\
Host: www.example.com\r\n\
User-Agent: curl/7.69.1\r\n\
Accept: */*\r\n\
\r\n";

	struct msg_token_list *list = (struct msg_token_list *)malloc(sizeof(struct msg_token_list));

	struct msg_token *head = test_parse(sample_request,strlen(sample_request), list);

	/*
	while(head != 0) {
		printf("test_parse: %s\n", (char *)head->value);
		head = head->next;
	}
	*/

	head = list->head;

	while(head != NULL) {
		printf("[test] %lu : %s\n", head->value_len, (char *)head->value);
		head = head->next;
	}
	return 0;
}

#endif
