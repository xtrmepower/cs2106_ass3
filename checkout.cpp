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
	readFile(fp, buffer,0,0);

	// Get file length
	unsigned int len = getFileLength(av[1]);

	// Open output file
	FILE *outFPtr = fopen(av[1], "w");

	// Write the data
	fwrite(buffer, sizeof(char), len, outFPtr);

	// Close the file
	fclose(outFPtr);
	free(buffer);

	closeFS();
	return 0;
}
