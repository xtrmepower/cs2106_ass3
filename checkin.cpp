#include "libefs.h"

int main(int ac, char **av) {

	if (ac != 3) {
		printf("\nUsage: %s <file to check in> <password>\n\n", av[0]);
		return -1;
	}

	initFS("part.dsk", av[2]);
	TFileSystemStruct *fs = getFSInfo();
	unsigned int len;

	//Open the current file to read data and obtain length.
	char *buffer = makeDataBuffer();
	FILE *inFPtr = fopen(av[1], "r");
	len = fread(buffer, sizeof(char), fs->blockSize, inFPtr);
	fclose(inFPtr);

	//Write into partition
	int fp=openFile(av[1],MODE_CREATE);
	//Check the file length in partition, if it's not 0, something already exist.
	if(getFileLength(av[1])!=0){
		printf("Error: Duplicated file.\n");
		free(buffer);
		closeFS();
		return 0;
	} else {
		updateDirectoryFileLength(av[1],len);
	}
	writeFile(fp, buffer,sizeof(char),len);
	printf("Buffer data: %s\n",buffer);

	// Close the file
	free(buffer);

	closeFS();

	return 0;
}
