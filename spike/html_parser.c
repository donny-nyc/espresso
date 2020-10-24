#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define SCANNING 			0
#define OPEN_BRACKET 	1
#define CLOSE_BRACKET 2
#define OPEN_TAG 			3
#define CLOSE_TAG			4
#define NAME					5
#define ATTRIBUTE			6

struct attribute {
	char *name;
	char *value;
};

typedef struct attribute attribute_t;

#define OPEN 0
#define CLOSED 1
struct tag {
	char 					*name;
	attribute_t 	**attributes;
	int						closed;
};

typedef struct tag tag_t;

tag_t **tags;

int before_start(int argc, char **argv) {
	if (argc != 2) {
		perror("usage: ./a.out [html file]");
	  exit(1);
	}

	return 0;
}

int main(int argc, char **argv) {
	before_start(argc, argv);

	char *file_name = argv[1];

	int fd;

	fd = open(file_name, O_RDONLY);

	if (fd == -1) {
		perror(strerror(errno));
		exit(1);
	}

	char window[10];
	size_t n_read;

	unsigned int status = SCANNING;

	while ( (n_read = read(fd, window, sizeof(window))) > 0) {
		for(int i = 0; i < n_read; i++) {
			switch(status) {
				case SCANNING:
					switch(window[i]) {
						case '<':
							printf("<");
							status = OPEN_BRACKET;
							break;
						case '>':
							break;
					}
					break;
				case OPEN_BRACKET:
					switch(window[i]) {
						case '/':
							break;
						case '>':
							status = OPEN_TAG;
							break;
						default:
							printf("%c", window[i]);
							break;
					}
					break;
				case CLOSE_BRACKET:
					break;
				case OPEN_TAG:
					break;
				case CLOSE_TAG:
					break;
			}
		}

		bzero(window, sizeof(window));
	}

	return 0;
}
