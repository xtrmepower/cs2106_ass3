#include "libefs.h"

int main(int ac, char **av) {
	if (ac != 3) {
		printf("\nUsage: %s <file to check in> <password>\n\n", av[0]);
		return -1;
	}

	initFS("part.dsk", av[2]);
	TFileSystemStruct *fs = getFSInfo();
	long len;

	//Open the current file to read data and obtain length.
	FILE *inFPtr = fopen(av[1], "r");

	//Obtain file size:
  fseek (inFPtr , 0 , SEEK_END);
  len = ftell (inFPtr);
  rewind (inFPtr);

	//Create a buffer to hold the whole file.
	char *buffer = (char *) malloc(sizeof(char)*len);
	fread(buffer, sizeof(char), len, inFPtr);

	fclose(inFPtr);

	//Write into partition
	int fp=openFile(av[1],MODE_CREATE);
	//Check the file length in partition, if it's not 0, something already exist.
	if(getFileLength(av[1])!=0){
		printf("Error: Duplicated file.\n");
	} else {
		updateDirectoryFileLength(av[1],len);
		writeFile(fp, buffer,sizeof(char),len);
	}

	// Close the file
	free(buffer);
	closeFS();
	return 0;
}
