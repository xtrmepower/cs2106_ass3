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
	TFileSystemStruct *fs = getFSInfo();

	// Search the directory for the file
	unsigned int fileNdx = findFile(filename);

	//Checking if it exists.
	if(fileNdx == FS_FILE_NOT_FOUND)
	{
		switch (mode) {
			case MODE_NORMAL:
			case MODE_READ_ONLY:
					return -1;
					break;
			case MODE_CREATE:
						// File isn't found, create a new directory and set its length to 0 to be updated later.
						fileNdx = makeDirectoryEntry(filename, 0x0, 0);
						if(fileNdx==FS_DIR_FULL){
							return -1;
						}
				break;
		}
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
	//Check if fp is valid.
	if(fp>=0&&fp<_oftCount){

		//Check if file is not READ Only
		if(_oft[fp].openMode!=MODE_READ_ONLY){

				TFileSystemStruct *fs = getFSInfo();
				printf("FS Size: %lu\n",fs->fsSize);
				printf("Block Size: %i\n---\n",fs->blockSize);
				printf("Length of file: %i\n",dataCount);

			int totalFileSize = dataCount * dataSize;
			int requiredBlocks = totalFileSize / fs->blockSize;
			if(totalFileSize % fs->blockSize!=0){
				requiredBlocks+=1;
			}

			if(totalFileSize> fs->numInodeEntries*fs->blockSize){
				//Todo: Handle if the size of the file to be written is larger than all the blocks in the inode combined.
				return;
			}

			// Load the inode
			loadInode(_oft[fp].inodeBuffer, _oft[fp].filePtr);

			//Create a temp block.
			char tBlock[fs->blockSize];

			for(int cycle=0;cycle<requiredBlocks;cycle++){
				// Find a free block
				unsigned long freeBlock = findFreeBlock();

				if(freeBlock==FS_FULL)
					return;

				// Mark the free block now as busy
				markBlockBusy(freeBlock);

				// Set the cycle entry of the inode to the free block
				_oft[fp].inodeBuffer[cycle]=freeBlock;

				//Extract the data from the buffer.
				memcpy(tBlock, &(((char*)buffer)[fs->blockSize*cycle]),fs->blockSize);

				// Write the data to the block
				writeBlock((char*)tBlock, freeBlock);
			}

			printf("Total block(s) written: %i\n",requiredBlocks);
			printf("Total byte(s) written: %i\n",dataCount);

			// Write the inode
			saveInode(_oft[fp].inodeBuffer, _oft[fp].filePtr);
		}
	}
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
		TFileSystemStruct *fs = getFSInfo();

		int totalFileSize = dataCount * dataSize;
		int requiredBlocks = totalFileSize / fs->blockSize;
		if(totalFileSize % fs->blockSize!=0){
			requiredBlocks+=1;
		}

		// Load the inode
		loadInode(_oft[fp].inodeBuffer, _oft[fp].filePtr);

		//Create a temp block.
		char tBlock[fs->blockSize];

		for(int cycle=0;cycle<requiredBlocks;cycle++){
			// Get the block number
			unsigned long blockNum = _oft[fp].inodeBuffer[cycle];

			// Read the block
			readBlock(tBlock, blockNum);

			//Combine block memory.
			memcpy(&(((char*)buffer)[fs->blockSize*cycle]), tBlock, fs->blockSize);
		}
		printf("Total block(s) loaded: %i\n",requiredBlocks);
		printf("Total byte(s) loaded: %i\n",dataCount);
	}
}

// Delete the file. Read-only flag (bit 2 of the attr field) in directory listing must not be set.
// See TDirectory structure.
void delFile(const char *filename) {
	unsigned int toDelInodeIndex = delDirectoryEntry(filename);

	// Check if file exists
	if (toDelInodeIndex == FS_FILE_NOT_FOUND) {
		printf("Error: File not found.\n");
		return;
	}

	//Todo: Clear up oft entry.


	// Check to see if it is READ_ONLY
	//TODO: use bit mask to check if the file is READ-_ONLY

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
