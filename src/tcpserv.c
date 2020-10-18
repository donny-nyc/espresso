#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
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

int build_http_200_header(char *header_buf, size_t buf_size, size_t body_len) {
	const char *header_format_string = "HTTP/2.0 200 OK\r\n\
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

	snprintf(header_buf, buf_size, header_format_string, current_date, body_len +2);

	return 0;
}

int build_response_body(char *buffer, size_t max_len) {
	int body_fd;

	if((body_fd = open("index.html", O_RDONLY)) == -1) {
		perror(strerror(errno));
		return 1;
	}

	read(body_fd, buffer, max_len);

	close(body_fd);
	return 0;
}

void str_echo(int sockfd) {
	char body[10000];

	build_response_body(body, sizeof(body));

	char header_buf[1000];
	if(build_http_200_header(header_buf, sizeof(header_buf), strlen(body)) == -1) {
		perror("build_http_200_header");
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

int main(int argc, char **argv) {
	int listenfd, connfd;
	pid_t childpid;
	socklen_t clilen;

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
