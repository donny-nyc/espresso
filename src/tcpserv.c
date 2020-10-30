#include "stories.h"

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

unsigned int SERV_PORT 	= 9090;
unsigned int LISTEN_Q  	= 100;
unsigned int MAXLINE 		= 10000;

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

int build_http_header(char *header_buf, unsigned int status, size_t buf_size, size_t body_len) {
	const char *header_format_string = "HTTP/2.0 %d %s\r\n\
server: caffeine.d\r\n\
date: %s\r\n\
content-type: text/html; charset=UTF-8\r\n\
content-length: %d\r\n\
\r\n\
\r\n";

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

	snprintf(header_buf, buf_size, header_format_string, status, status == 200 ? "OK" : "NOT FOUND", current_date, body_len +2);

	return 0;
}

int build_response_body(int body_fd, char *buffer, size_t max_len) {
	char p_buff[1000];

	read(body_fd, buffer, max_len);


	close(body_fd);
	return 0;
}

int parse_request(char *request, char *path, size_t r_len, size_t p_len) {
	char *context;
	const char delim = ' ';
	char *result = strtok_r(request, &delim, &context);
	char *token;

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
	ssize_t n;
	read(sockfd, request_buffer, sizeof(request_buffer));

	char path_buffer[10000];
	parse_request(request_buffer, path_buffer, sizeof(request_buffer), sizeof(path_buffer));

	char path[10000];

	char *prefix = "public";
	char *root = "public/";
	snprintf(path, sizeof(path), "%s%s", prefix, path_buffer);

	// given an empty path, just return the index
	if (strncmp(path, root, strlen(path)) == 0) {
		bzero(path, sizeof(path));
		strcpy(path, "public/index.html");
		printf("%s <=> %s\n", path, root);
	} 

	// char body[10000];
	// We're getting prolific - 10kb should have been enough for anybody,,
	// but we just had to have more...
	//
	// Should make this configurable
	char body[100000];

	printf("path: %s\n", path);

	int body_fd;
	if((body_fd = open(path, O_RDONLY)) == -1) { /* 404 Not Found */
		status = 404;
		if( (body_fd = open("public/404.html", O_RDONLY)) == -1) {
				perror(strerror(errno));
				exit(1);
		}
	}

	build_response_body(body_fd, body, sizeof(body));

	char header_buf[1000];
	if(build_http_header(header_buf, status, sizeof(header_buf), strlen(body)) == -1) {
				perror("build_http_header");
				exit(1);
	}

	if((write(sockfd, header_buf, strlen(header_buf))) == -1) {
		perror(strerror(errno));
		exit(1);
	}

	if((write(sockfd, body, strlen(body))) == -1) {
		perror(strerror(errno));
		exit(1);
	}
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

	struct sigaction sa;

	sigemptyset(&sa.sa_mask);

	sa.sa_flags = SA_RESTART;
	sa.sa_handler = sigchldHandler;

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
