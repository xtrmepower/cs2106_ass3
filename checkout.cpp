#include "libefs.h"

int main(int ac, char **av)
{
	if(ac != 3)
	{
		printf("\nUsage: %s <file to check out> <password>\n\n", av[0]);
		return -1;
	}

	initFS("part.dsk", av[2]);

	char *buffer = makeDataBuffer();

	int fp=openFile(av[1],MODE_READ_ONLY);
	if(fp!=-1)
		readFile(fp, buffer,0,0);
	else {
			printf("Error: File not found.\n");
			// Get file length
			unsigned int len = getFileLength(av[1]);

			// Write the buffer to a file.
			FILE *outFPtr = fopen(av[1], "w");
			fwrite(buffer, sizeof(char), len, outFPtr);
			fclose(outFPtr);
	}
	free(buffer);

	closeFS();
	return 0;
}
