#include "libefs.h"

int main(int ac, char **av) {

	if (ac != 3) {
		printf("\nUsage: %s <file to check in> <password>\n\n", av[0]);
		return -1;
	}

	initFS("part.dsk", av[2]);

	char *buffer = makeDataBuffer();
	FILE *inFPtr = fopen(av[1], "r");
	TFileSystemStruct *fs = getFSInfo();

	//Open the current file to read.
	unsigned int len = fread(buffer, sizeof(char), fs->blockSize, inFPtr);

	//Write into partition
	int fp=openFile(av[1],MODE_CREATE);
	updateDirectoryFileLength(av[1],len);
	writeFile(fp, buffer,sizeof(char),len);
	printf("Buffer data: %s\n",buffer);

	// Close the file
	fclose(inFPtr);
	free(buffer);

	closeFS();

	return 0;
}
