#include "stories.h"

size_t read_tmpl_to_buf(char *tmpl_name, char *tmpl_buf, size_t b_len) {
	int tmpl_fd;

	if (strlen(tmpl_name) == 0) {
		perror("No template name given.\n");
		return -1;
	}

	if (tmpl_buf == NULL || b_len == 0) {
		perror("No buffer available for the template.\n");
		return -1;
	}

	tmpl_fd = open(tmpl_name, O_RDONLY);

	if (tmpl_fd == -1) {
		perror(strerror(errno));
		return -1;
	}

	size_t n_read = read(tmpl_fd, tmpl_buf, b_len);

	if (n_read == -1) {
		perror(strerror(errno));
		return -1;
	}

	return n_read;
}

unsigned int MAX_BODY = 10000;
size_t read_stories_to_buf(story_t *story_buf, size_t b_len) {
	DIR *dir;
	struct dirent *ent;

	int s_fd;

	char *prefix = "data/stories";
	char path[10000];

	int story_count = 0;
	if ((dir = opendir("data/stories")) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			if (ent->d_type == DT_REG) {
				printf("reading %s\n", ent->d_name);

				// must build the path to the story file using the prefix above
				bzero(path, sizeof(path));
				size_t n = snprintf(path, sizeof(path), "%s/%s", prefix, ent->d_name);

				if (n <= 0) {
					perror("we could not build the story path.\n");
					return 1;
				}

				s_fd = open(path, O_RDONLY);

				if (s_fd == -1) {
					perror("could not load story.\n");
					return 1;
				}

				char b_buf[MAX_BODY];
				size_t s_read = read(s_fd, b_buf, sizeof(b_buf));

				if (s_read <= 0) {
					perror("could not read from story.\n");
					return 1;
				}

				close(s_fd);

				// prepare story to receive data
				char *title = "My First Story";
				story_t read_story;
				// will need to free this once we've built the response
				read_story.body = malloc(sizeof(char) * strlen(b_buf));
				read_story.title = malloc(sizeof(char) * strlen(title));

				strncpy(read_story.title, title, strlen(title));
				strncpy(read_story.body, b_buf, strlen(b_buf));

				memcpy(story_buf + (story_count++), &read_story, sizeof(read_story));
			}
		}
		closedir(dir);
	} else { /* could not open dir */
		perror("could not open dir\n");
		return 1;
	}
	return story_count;
}

char *load_dynamic_content(char *tmpl, story_t *dyn, size_t t_len, size_t d_len) {
	char *delim = "{{:stories:}}";

	char *context;

	char *tok = strtok_r(tmpl, " ", &context);
	printf("%s\n", tok);
	while((tok = strtok_r(NULL, " ", &context)) != NULL) {
		printf("%s\n", tok);
	}
	
	return 0;
}

unsigned int TMPL_MAX = 10000;
unsigned int STORY_MAX = 10;
int build_response(char *body_buffer, size_t b_len) {
	// Load Template
	char tmpl_buf[TMPL_MAX];

	char *tmpl_name = "templ/stories.tmpl";

	size_t n_read = read_tmpl_to_buf(tmpl_name, tmpl_buf, sizeof(tmpl_buf));

	if (n_read <= 0) {
		perror("Template not read\n");
		return 1;
	}

	// Load Dynamic Content
	story_t story_buf[STORY_MAX];
	size_t s_read = read_stories_to_buf(story_buf, sizeof(story_buf));

	// Return formatted body
	size_t d_read = load_dynamic_content(tmpl_buf, story_buf, n_read, s_read);

	return 0;
}
