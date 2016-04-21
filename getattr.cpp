#include "libefs.h"

int main(int ac, char **av)
{
	if(ac != 2)
	{
		printf("\nUsage: %s <file to check>\n", av[0]);
		printf("Prints: 'R' = Read only, 'W' = Read/Write\n\n");
		return -1;
	}

	if (findFile(av[1]) == FS_FILE_NOT_FOUND) {
		printf("ERROR: File not found!\n");
		return -1;
	}

	unsigned int attr = getattr(av[1]);
	bool isReadOnly = (attr & (1 << 2));

	if (isReadOnly) {
		printf("R\n");
	} else {
		printf("W\n");
	}

	return 0;
}
