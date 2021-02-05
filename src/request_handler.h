#ifndef HTTP_RETURN_CODES
#define HTTP_OK 200
#define HTTP_NOT_FOUND 404
#define HTTP_NOT_VALID 400
#define HTTP_SERVER_ERROR 500
#endif

#ifndef MAX_REQUEST_LEN
#define MAX_REQUEST_LEN 100000
#endif

#ifndef MAX_RESPONSE_BODY
#define MAX_RESPONES_BODY 100000
#endif

#include <stddef.h>

enum http_verbs { GET, PUT, POST, DELETE };

struct http_header {
	size_t key_len;
	size_t value_len;
	char *key;
	char *value;
	struct http_header *next;
	struct http_header *prev;
};

struct http_headers {
	size_t header_count;
	struct http_header *first;
	struct http_header *last;
};

struct http_header *add_header(struct http_headers *, char *key, size_t key_len, char *value, size_t value_len);

struct http_request {
	int http_verb;
	size_t path_len;
	size_t body_len;
	size_t request_len;
	char *path;
	char *body;
	struct http_headers *headers;
};

struct http_request *string_to_request(char *req, size_t req_len);

struct http_response {
	int code;
	size_t body_len;
	size_t response_len;
	char *body;
	struct http_headers *headers;
};

struct http_response *handle_request(struct http_request *);

int validate_request(struct http_request *);

int free_request(struct http_request *);
int free_response(struct http_response *);
