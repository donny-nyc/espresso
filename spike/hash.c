#include <stdio.h>

int main(int argc, char **argv) {
	unsigned long hash = 5381;
	int c = 0;

	char *path = argv[1];

	printf("%s", path);

	while ((c = *path++))
		hash = ((hash << 5) + hash) + c;

	printf(":%lu\n", hash);

	return 0;
}
