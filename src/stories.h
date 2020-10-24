#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

struct story {
	size_t len;
	char *title;	
	char *body;
};

typedef struct story story_t;

size_t read_stories_to_buf(story_t *buf, size_t b_len);

int build_response(char *body_buffer, size_t b_len);
