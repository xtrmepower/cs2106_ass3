#include "libefs.h"

int main(int ac, char **av)
{
	if(ac != 3)
	{
		printf("\nUsage: %s <file to check set attrbute> <attribute>\n", av[0]);
		printf("Attribute: 'R' = Read only, 'W' = Read/Write\n\n");
		return -1;
	}

	mountFS("part.dsk", "");

	if (findFile(av[1]) == FS_FILE_NOT_FOUND) {
		printf("ERROR: File not found!\n");
		closeFS();
		return -1;
	}

	unsigned int newattr = 0;

	if (strcmp(av[2], "r") == 0 || strcmp(av[2], "R") == 0) {
		newattr = 1 << 2;
	} else if (strcmp(av[2], "w") == 0 || strcmp(av[2], "W") == 0) {
		newattr = 0 << 2;
	}

	setattr(av[1], newattr);

	closeFS();

	return 0;
}
