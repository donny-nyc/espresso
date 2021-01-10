#include "stories.h"

#include <syslog.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>

unsigned int SERV_PORT 	= 80;
unsigned int LISTEN_Q  	= 100;
unsigned int MAXLINE 		= 10000;

struct favicon {
	char *path;
	size_t size;
	char *buffer;
} favicon;

struct page {
	char *path;
	size_t size;
	char *buffer;
};

typedef struct page page_t;
typedef struct favicon favicon_t;

void print_usage() {
	printf("usage: ./a.out [-p listen port number]\n");
	printf("\t -p listen port number: an available port \n \
\tthat we will bind to for receiving connection requests\n");
}

/* djb2 - http://www.cse.yorku.ca/~oz/hash.html */
/* https://www.reddit.com/r/C_Programming/comments/2syijd/understanding_this_hash_function/cnu1nri/ */
unsigned long hash(char *str) {
	unsigned long hash = 5381;
	int c;

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c;

	return hash;
}

// Returns:
// 0 - no cli arguments given, or args were recognized
// 1 - unrecognized arg. Print usage
int before_start_hook(int argc, char **argv) {
	for(int i = 0; i < argc; i++) {
		switch(hash(argv[i])) {
			case 5861506:
				SERV_PORT = atoi(argv[++i]);
				break;
		}
	}
	return 0;
}

int build_http_header(char *header_buf, const char *header_format_string, unsigned int status, size_t buf_size, size_t body_len, char *content_type) {
	char current_date[1000];

	time_t t;
	struct tm *tmp;

	t = time(NULL);
	tmp = localtime(&t);

	if (tmp == NULL) {
		perror("localtime");
		return 1;
	}

	if (strftime(current_date, sizeof(current_date), "%c", tmp)  == 0) {
		perror("strftime returned 0");
		return 1;
	}

	snprintf(header_buf, buf_size, header_format_string, status, status == 200 ? "OK" : "NOT FOUND", current_date, content_type, body_len);

	return 0;
}

int build_200_ok(char *header_buf, size_t buf_size, size_t body_len, char *content_type) {
	const char *format_string = "HTTP/2.0 %d %s\r\n\
server: espresso\r\n\
date: %s\r\n\
content-type: %s\r\n\
content-length: %d\r\n\
\r\n";

	return build_http_header(header_buf, format_string, 200, buf_size, body_len, content_type);
}

int build_404_not_found(char *header_buf, size_t buf_size, size_t body_len) {
	char *content_type = "text/html; charset=UTF-8";
	const char *format_string = "HTTP/2.0 %d %s\r\n\
server: espresso\r\n\
date: %s\r\n\
content-type: %s\r\n\
content-length: %d\r\n\
\r\n";

	return build_http_header(header_buf, format_string, 404, buf_size, body_len, content_type);
}

int build_301_redirect(char *header_buf, size_t buf_size, size_t body_len) {
	char *content_type = "text/html; charset=UTF-8";
	const char *format_string = "HTTP/2.0 %d %s\r\n\
location: http://104.131.30.208/\r\n\
server: espresso\r\n\
date: %s\r\n\
content-type: %s\r\n\
content-length: %d\r\n\
\r\n";

	return build_http_header(header_buf, format_string, 301, buf_size, body_len, content_type);
}

size_t load_favicon(char *path) {
	FILE *fp;
	long lSize;
	size_t result;

	fp = fopen(path, "r");
	if (fp == NULL) {
		syslog(LOG_ERR, "failed to open %s", path);
		return 0;
	}

	// sort out the file size
	fseek(fp, 0, SEEK_END);
	lSize = ftell(fp);
	rewind(fp); // equivalent to fseek(stream, 0L, SEEK_SET);
	syslog(LOG_INFO, "reading %ld bytes from %s", lSize, path);

	// prepare a buffer for the file
	favicon.buffer  = (char *)malloc(sizeof(char) * lSize);
	if (favicon.buffer == NULL) {
		syslog(LOG_ERR, "failed to allocate buffer for %s", path);
		return 0;
	}

	// copy the file to the buffer
	result = fread((void *)favicon.buffer, sizeof(char), lSize, fp);
	if (result != lSize) {
		syslog(LOG_ERR, "Failed to read the file from %s", path);
		return 0;
	}
	syslog(LOG_INFO, "read %ld bytes into buffer", result);

	syslog(LOG_INFO, "load file (%s): %s", path, favicon.buffer);
	syslog(LOG_INFO, "result: %ld", result);

	favicon.path = (char *)malloc(sizeof(char) * strlen(path));
	strcpy(favicon.path, path);
	favicon.size = result;

	return result;
}

page_t *load_page_to_buffer(char *path) {
	FILE *fp;
	long lSize;
	char *buffer;
	size_t result;

	fp = fopen(path, "r");
	if (fp == NULL) {
		syslog(LOG_ERR, "failed to open %s", path);
		return NULL;
	}

	// sort out the file size
	fseek(fp, 0, SEEK_END);
	lSize = ftell(fp);
	rewind(fp);

	syslog(LOG_INFO, "reading %ld bytes from %s", lSize, path);

	// prepare buffer for the file
	buffer = (char *)malloc(sizeof(char) * lSize);
	if (buffer == NULL) {
		syslog(LOG_ERR, "failed to allocate buffer for %s", path);
		return NULL;
	}

	// copy the file to the buffer
	result = fread((void *) buffer, 1, lSize, fp);
	if (result != lSize) {
		syslog(LOG_ERR, "failed to read file from %s", path);
		free(buffer);
		return NULL;
	}
	syslog(LOG_INFO, "read %ld bytes from %s", result, path);
	syslog(LOG_INFO, "%s", buffer);

	// prepare the page struct
	page_t *page = (page_t *)malloc(sizeof(page_t));
	if (page == NULL) {
		syslog(LOG_ERR, "failed to allocate page for %s", path);
		free(buffer);
		return NULL;
	}

	page->path = (char *)malloc(sizeof(char) * strlen(path));
	if (page->path == NULL) {
		syslog(LOG_ERR, "failed to write path %s", path);
		free(buffer);
		free(page);
		return NULL;
	}

	strcpy(page->path, path);
	page->buffer = buffer;
	page->size = result;

	syslog(LOG_INFO, "page->buffer [%lu]: %s", page->size, page->buffer);

	return page;
}

int parse_request(char *request, char *path, size_t r_len, size_t p_len) {
	char *context;
	const char delim = ' ';
	char *result = strtok_r(request, &delim, &context);
	char *token;

	syslog(LOG_INFO, "parse_request: %s", path);

	switch(hash(result)) {
		case 193456677: /* GET */
			token = strtok_r(NULL, &delim, &context);
			printf("Found %s\n", token);
			strncpy(path, token, p_len);
			break;
		default:
			printf("Found %ld\n", hash(result));
			break;
	}



	return 0;
}

void str_echo(int sockfd) {
	unsigned int status = 200;

	char request_buffer[10000];
	read(sockfd, request_buffer, sizeof(request_buffer));

	char path_buffer[10000];
	parse_request(request_buffer, path_buffer, sizeof(request_buffer), sizeof(path_buffer));

	page_t *page;
	char *body_buffer = NULL;
	char *content_type = "text/html; charset=UTF-8";
	size_t body_size = 0;

	char *path;
	syslog(LOG_INFO, "hash path [%lu] %s", hash(path_buffer), path_buffer);
	switch(hash(path_buffer)) {
	 case 12332232679284166093:
		 syslog(LOG_INFO, "serving favicon");
		 syslog(LOG_INFO, "%s", favicon.buffer);
		 path = "/home/espresso/static/favicon.png";
		 content_type = "image/png";
		 body_buffer = favicon.buffer; // *shrug*
		 body_size = favicon.size;
		 syslog(LOG_INFO, "body_size %lu", favicon.size);
		 break;
	 case 177620:
		 path = "/home/espresso/public/index.html";
		 page = load_page_to_buffer(path);
		 body_buffer = page->buffer;
		 body_size = page->size;
		 break;
	 default:
		 syslog(LOG_INFO, "hash %s [%lu]", path_buffer, hash(path_buffer));
		 status = 301;
		 path = "/home/espresso/public/index.html";
		 page = load_page_to_buffer(path);
		 body_buffer = page->buffer;
		 body_size = page->size;
	}

	syslog(LOG_INFO, "body buffer: %s", body_buffer);


	syslog(LOG_INFO, "path: %s", path);
	// build_response_body(body_fd, body, sizeof(body));


	char header_buf[1000];
	switch(status) {
		case 200:
			build_200_ok(header_buf, sizeof(header_buf), body_size, content_type);
			break;
		case 404:
			build_404_not_found(header_buf, sizeof(header_buf), body_size);
			break;
		case 301:
			build_301_redirect(header_buf, sizeof(header_buf), body_size);
			break;
	}

	syslog(LOG_INFO, "write header");
	syslog(LOG_INFO, "%s", header_buf);
	if((write(sockfd, header_buf, strlen(header_buf))) == -1) {
		syslog(LOG_ERR, "failed to write header to stream");

		exit(1);
	}

	if((write(sockfd, body_buffer, body_size)) == -1) {
		syslog(LOG_ERR, "failed to write body to stream");
		exit(1);
	}

	if (page != NULL) {
		free(body_buffer);
		free(page->path);
		free(page);
	}

	syslog(LOG_INFO, "request [%d] [%s] [%ld] %s", status, request_buffer, body_size, path_buffer);
}

static void
sigchldHandler(int sig) {
	int status, savedErrno;
	pid_t childPid;

	savedErrno = errno;

	while ((childPid = waitpid(-1, &status, WNOHANG)) > 0) {
		printf("%d handler reaped child %d - ", sig, childPid);
	}

	if (childPid == -1 && errno != ECHILD) {
		perror(strerror(errno));
	}

	errno = savedErrno;
}

int main(int argc, char **argv) {
	int listenfd, connfd;
	pid_t childpid;
	socklen_t clilen;

	char *path = "/home/espresso/static/favicon.png";
	load_favicon(path);

	syslog(LOG_INFO, "favicon.buffer %s", favicon.buffer);
	syslog(LOG_INFO, "favicon.path %s", favicon.path);
	syslog(LOG_INFO, "favicon.size %lu", favicon.size);

	struct sigaction sa;

	sigemptyset(&sa.sa_mask);

	sa.sa_flags = SA_RESTART;
	sa.sa_handler = sigchldHandler;

	openlog("[espresso]", 0, 0);
	syslog(LOG_INFO, "started");

	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	if( before_start_hook(argc, argv) != 0) {
		perror("There was a problem starting the script.\n");
		exit(1);
	}


	// <netinet/in.h>
	// Structure describing an Internet socket address
	struct sockaddr_in cliaddr, servaddr;

	printf("socket ");
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror(strerror(errno));
		exit(1);
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl (INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	//printf("bind socket\n");
	if((bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr))) == -1) {
		perror(strerror(errno));
		exit(1);
	}
	syslog(LOG_INFO, "listening on port %d", SERV_PORT);

	//printf("bound to host on port %d\n", SERV_PORT);

	//printf("flag socket as listen ");
	if((listen(listenfd, LISTEN_Q)) == -1) {
		perror(strerror(errno));
		exit(1);
	}

	for( ; ; ) {
		clilen = sizeof(cliaddr);
		if((connfd = accept(listenfd, (struct sockaddr*) &cliaddr, &clilen)) == -1) {
			perror(strerror(errno));
			exit(1);
		}

		if ( (childpid = fork()) == 0) { /* child process */
			close(listenfd);	/* close listening socket */

			str_echo(connfd);
			exit(0);
		}
		close(connfd);	/* parent closes connected socket */
	}

	return 0;
}
