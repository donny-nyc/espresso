#include <errno.h>
#include <limits.h>	/* ARG_MAX */
#include <stdio.h>	/* perror(), printf() */
#include <stdlib.h> /* exit() */
#include <string.h>	/* strlen() */
#include <unistd.h> /* sysconf() */

struct attribute {
	char *name;
	char *value;
};

typedef struct attribute attribute_t;

struct tag {
	char *name;
	attribute_t **attributes;
};

typedef struct tag tag_t;

#define DEBUG 				1
	

#define SCANNING 								0
#define OPEN_BRACKET 						1
#define OPEN_BRACKET_NAME				2
#define OPEN_BRACKET_ATTR				3
#define OPEN_BRACKET_WHITESPACE	4
#define CLOSE_BRACKET 					5
#define OPEN_TAG								6
#define CLOSE_TAG 							7
#define OVERRUN 								8
#define STOP				  					9

#define VALID							0
#define INVALID						1

#define MAX_ATTR 				100

/*
 * ->(start) [SCANNING]
 *  |
 *  | do while !read('<')
 *  | 
 *  -> if read('<') -> [OPEN_BRACKET / OPEN_TAG]
 *  |
 *  | create empty tag_t
 *  | read(/[a-zA-Z]+/) as tag.name
 *  | read(/\s* /)
 *  | do while read(/[a-zA-Z]+(=[a-ZA-Z0-9]+)?/) as attribute
 *  |
 *  |-> do while read(/\s* /)
 *  |
 *  |-> if read('/>') -> [CLOSE_BRACKET / CLOSE_TAG]
 */

/* Scan input from argv and parse as HTML tags */
int main(int argc, char **argv) {
	if (argc != 2) {
		perror("usage: ./a.out \"[tag string]\"");
		exit(1);
	}

	long ARG_MAX = sysconf(_SC_ARG_MAX);
	if(DEBUG) printf("ARG_MAX %ld\n", ARG_MAX); /* man(3) sysconf */
	if(DEBUG) printf("%s\n", argv[1]);

	char *candidate = argv[1];
	size_t c_len = strlen(candidate);

	char buffer[ARG_MAX];
	bzero(buffer, ARG_MAX * sizeof(char));
	unsigned long b_idx = 0;

	short state = SCANNING;
	int idx = 0;

	tag_t *new_tag;
	size_t n_len;					// name length;

	attribute_t *new_attr = NULL;
	size_t attr_len;
	unsigned int attr_i = 0; 	// attribute count
	while (state != STOP) {
		// Bail if idx has overrun
		if (idx > c_len) {
			state = OVERRUN;
		}

		switch (state) {
			case SCANNING:
				switch (candidate[idx]) {
					case '<':
						state = OPEN_BRACKET;

						idx++;
						break;
					default:

						idx++;
						break;
				}
				break;
			case OPEN_BRACKET:
				new_tag = (tag_t *)malloc(sizeof(tag_t));

				switch (candidate[idx]) {
					case ' ': /* consume leading whitespace */
						idx++;
						break;
					default:
					state = OPEN_BRACKET_NAME;
					break;
				};
				break;
			case OPEN_BRACKET_NAME:
				switch (candidate[idx]) {
					case '>': /* done - no attrs. wrap up the tag */
						n_len = strlen(buffer);
						new_tag->name = (char *)malloc(sizeof(char) * n_len);
						strncpy(new_tag->name, buffer, n_len);

						bzero(buffer, ARG_MAX * sizeof(char));
						b_idx = 0;

						state = CLOSE_BRACKET;
						idx++;
						break;
					case ' ':
						n_len = strlen(buffer);
						new_tag->name = (char *)malloc(sizeof(char) * n_len);
						strncpy(new_tag->name, buffer, n_len);

						bzero(buffer, ARG_MAX * sizeof(char));
						b_idx = 0;

						state = OPEN_BRACKET_WHITESPACE;
						break;
					default:
						buffer[b_idx++] = candidate[idx];
						idx++;
						break;
				}
				break;
			case OPEN_BRACKET_WHITESPACE:
				switch (candidate[idx]) {
					case ' ':
						idx++;
						break;
					case '>':
						state = CLOSE_BRACKET;
						break;
					default: /* anything else that is read, assume to be an attr */
						state = OPEN_BRACKET_ATTR;
						break;
				}
				break;
			case OPEN_BRACKET_ATTR:
				// prepare storage for new attribute
				if (!new_attr) {
					new_attr = (attribute_t *)malloc(sizeof(attribute_t));
					bzero(new_attr, sizeof(attribute_t));
				}
				switch (candidate[idx]) {
					/* attr are separated by spaces, but should also
					 * end once the tag closes - ie a right-square bracket
					 * is read from the candidate string
					 */
					case '>':
						state = CLOSE_BRACKET;
					case ' ':
						// if new_attr is not null, we've just finished parsing it
						// get ready to write whatever is in the buffer to
						// the name or value:
						// assume value if name is not null, otherwise assume that
						// the tag is only a name (instead of a name=value pair)
						if (new_attr != NULL) {
							attr_len = strlen(buffer);
							if(new_attr->name) {
								new_attr->value = (char *)malloc(sizeof(char) * attr_len);
								bzero(new_attr->value, attr_len);

								strncpy(new_attr->value, buffer, attr_len);

							} else { /* write to name */
								new_attr->name = (char *)malloc(sizeof(char) * attr_len);
								bzero(new_attr->name, attr_len);

								strncpy(new_attr->name, buffer, attr_len);
							}
							bzero(buffer, ARG_MAX * sizeof(char));
							b_idx = 0;

							if (!new_tag->attributes) {
								new_tag->attributes = (attribute_t **)malloc(MAX_ATTR * sizeof(attribute_t *));
							}

							new_tag->attributes[attr_i++] = new_attr;

							new_attr = NULL;
						} else { /* just consume the empty whitespace
												until something meaningful is read */
						}

						state = OPEN_BRACKET_WHITESPACE;
						break;
					case '=':
						attr_len = strlen(buffer);
						new_attr->name = (char *)malloc(sizeof(char) * attr_len);
						bzero(new_attr->name, attr_len);

						strncpy(new_attr->name, buffer, attr_len);

						bzero(buffer, ARG_MAX * sizeof(char));
						b_idx = 0;

						idx++;
						break;
					default:
						/* read a continuous stream of input until we've reached
						 * the end of the value (marked by whitespace, an equals sign
						 * or a right-angle bracket
						 */
						buffer[b_idx++] = candidate[idx];
						idx++;
						break;
				}
				break;
			case CLOSE_BRACKET:
				if(DEBUG) {
					printf("result => %s\n", new_tag->name);
					for (int a = 0; a < attr_i; a++) {
						printf("attr[%d]-> %s\n", a, new_tag->attributes[a]->name);	
						if(new_tag->attributes[a]->value) {
							printf("value-> %s\n", new_tag->attributes[a]->value);
						}
					}
				}

				idx++;
				state = STOP;
				break;
			case OPEN_TAG:
				break;
			case CLOSE_TAG:
				break;
			case OVERRUN:
				if(DEBUG) perror("Nothing left to read. Unable to parse tag.");
				exit(1);
			default:
				perror("undefined state.");
				exit(1);
		}
	}

	free(new_tag);
	return 0;
}
