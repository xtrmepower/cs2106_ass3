#include "libefs.h"

// FS Descriptor
TFileSystemStruct *_fs;

// Open File Table
TOpenFile *_oft;

// Open file table counter
int _oftCount=0;

// Mounts a paritition given in fsPartitionName. Must be called before all
// other functions
void initFS(const char *fsPartitionName, const char *fsPassword)
{
	// Check to see if partition given in fsPartitionName exists.

	// If exists, attempt to mount partition.
	mountFS(fsPartitionName, fsPassword);

	_oft=NULL;
}

// Opens a file in the partition. Depending on mode, a new file may be created
// if it doesn't exist, or we may get FS_FILE_NOT_FOUND in _result. See the enum above for valid modes.
// Return -1 if file open fails for some reason. E.g. file not found when mode is MODE_NORMAL, or
// disk is full when mode is MODE_CREATE, etc.

int openFile(const char *filename, unsigned char mode)
{
	// Search the directory for the file
	unsigned int fileNdx = findFile(filename);

	//Checking if it exists.
	switch (mode) {
		case MODE_NORMAL:
			if(fileNdx == FS_FILE_NOT_FOUND)
			{
				printf("Cannot find encrypted file %s\n", filename);
				exit(-1);
			}
			break;
		case MODE_CREATE:
			if(fileNdx == FS_FILE_NOT_FOUND)
			{
				printf("Cannot find encrypted file %s\nCreating a new file '%s'\n", filename);
			}
			break;
		case MODE_READ_ONLY:
			if(fileNdx == FS_FILE_NOT_FOUND)
			{
				printf("Cannot find read-only encrypted file %s\n", filename);
				exit(-1);
			}
			break;
	}

	//Creating OFT in the system.
	_oftCount++;
	TOpenFile *newoft = new TOpenFile[_oftCount];

	//Check if the directory already has items.
	if(_oft!=NULL){
		for(int i=0;i<_oftCount;i++){
			newoft[i]=_oft[i];
		}
		delete[] _oft;
	} else {
		//Copy the new directory.
		_oft=newoft;
	}

	// Get the attributes
	TFileSystemStruct *fs = getFSInfo();
	_oft[_oftCount-1].blockSize = fs->blockSize;
	_oft[_oftCount-1].openMode = mode;
	_oft[_oftCount-1].inodeBuffer = makeInodeBuffer();
	_oft[_oftCount-1].buffer = makeDataBuffer();
	_oft[_oftCount-1].filePtr = fileNdx;

	return (_oftCount-1);
}

// Write data to the file. File must be opened in MODE_NORMAL or MODE_CREATE modes. Does nothing
// if file is opened in MODE_READ_ONLY mode.
void writeFile(int fp, void *buffer, unsigned int dataSize, unsigned int dataCount)
{
}

// Flush the file data to the disk. Writes all data buffers, updates directory,
// free list and inode for this file.
void flushFile(int fp)
{
}

// Read data from the file.
void readFile(int fp, void *buffer, unsigned int dataSize, unsigned int dataCount)
{
	if(fp>=0&&fp<_oftCount){
		// Load the inode
		loadInode(_oft[fp].inodeBuffer, _oft[fp].filePtr);

		// Get the block number
		unsigned long blockNum = _oft[fp].inodeBuffer[0];

		// Read the block
		readBlock(_oft[fp].buffer, blockNum);
		readBlock((char*)buffer, blockNum);
	}
}

// Delete the file. Read-only flag (bit 2 of the attr field) in directory listing must not be set.
// See TDirectory structure.
void delFile(const char *filename) {

	unsigned int toDelInodeIndex = delDirectoryEntry(filename);

	// Check if file exists
	if (toDelInodeIndex == FS_FILE_NOT_FOUND) {
		printf("ERROR: File does not exist.");
		return;
	}

	// Check to see if it is READ_ONLY
	//TODO

	unsigned long *inodeBuffer = makeInodeBuffer();

	loadInode(inodeBuffer, toDelInodeIndex);

	unsigned long blockNum = returnBlockNumFromInode(inodeBuffer, 0);

	markBlockFree(blockNum);

	updateFreeList();
}

// Close a file. Flushes all data buffers, updates inode, directory, etc.
void closeFile(int fp){
	flushFile(fp);

	//Check if OFT has this file.
	if(fp>=0 &&fp < _oftCount)
	{
			//Check if that file is not a read only
			if(_oft[fp].openMode != MODE_READ_ONLY){
				_oftCount--;
				TOpenFile *newoft = new TOpenFile[_oftCount];

				//Copy existing items infront of the index.
					for(int i=0;i<fp;i++){
						newoft[i]=_oft[i];
					}
				//Copy existing items behind of the index.
					for(int i=fp;i<_oftCount;i++){
						newoft[i]=_oft[i+1];
					}

					//Delete old OFT
					free(_oft[fp].inodeBuffer);
					free(_oft[fp].buffer);
					delete[] _oft;
					_oft = newoft;
			}
	}
}

// Unmount file system.
void closeFS(){

	for (int i = 0; i < _oftCount; i++) {
		free(_oft[i].inodeBuffer);
		free(_oft[i].buffer);
	}
	delete[] _oft;
	_oft = NULL;

	unmountFS();
}
