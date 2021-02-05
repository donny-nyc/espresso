#include <string.h>

#include "request_handler.h"

#ifndef HANDLE_REQUEST_F

struct http_response echo_response;

struct http_response *handle_request(struct http_request *req) {
	char *res = strncpy(echo_response.body, req->body, req->body_len);

	echo_response.code = HTTP_OK;
	return HTTP_OK;
}
#endif
