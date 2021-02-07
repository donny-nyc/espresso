#ifndef REQUEST_PARSER
#define REQUEST_PARSER

#include <stddef.h>

struct http_request *string_to_request(char *req, size_t req_len);

#endif
