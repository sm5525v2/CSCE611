/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file_system.H"

/*--------------------------------------------------------------------------*/
/* CLASS Inode */
/*--------------------------------------------------------------------------*/

/* You may need to add a few functions, for example to help read and store 
   inodes from and to disk. */

/*--------------------------------------------------------------------------*/
/* CLASS FileSystem */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem() {
    Console::puts("In file system constructor.\n");
    disk = nullptr;
    inodes = new Inode[MAX_INODES];
    free_blocks = new unsigned char[512];
    memset(free_blocks, 0, sizeof(free_blocks));
    freeBlock = DATA_START;
    nInode = 0; 
}

FileSystem::~FileSystem() {
    Console::puts("unmounting file system\n");
    /* Make sure that the inode list and the free list are saved. */

    delete[] inodes;
    delete[] free_blocks;
}


/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/


bool FileSystem::Mount(SimpleDisk * _disk) {
    Console::puts("mounting file system from disk\n");

    /* Here you read the inode list and the free list into memory */
    if(disk) return false;

   disk = _disk;
   
   unsigned char * buf = new unsigned char[512];
   disk->read(0, buf); //read block 0, inode list
   disk->read(1, free_blocks); //read block, free list

   return true;
}

bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size) { // static!
    Console::puts("formatting disk\n");
    /* Here you populate the disk with an initialized (probably empty) inode list
       and a free list. Make sure that blocks used for the inodes and for the free list
       are marked as used, otherwise they may get overwritten. */
    unsigned int numBlocks = _size/SimpleDisk::BLOCK_SIZE;
    unsigned char* buf = new unsigned char[512];
   
    //  First block:INODES block, second block : FREELIST block(bitmap)
	   
	unsigned int block;
	for(unsigned int block = 0; block< 2; block++) {
		_disk->write(block, buf);
	}

   //   other data blocks	
	for(block = 2 ; block < numBlocks ; block++) {
		_disk->write(block,buf);
	}
	return true;   
}

Inode * FileSystem::LookupFile(int _file_id) {
    Console::puts("looking up file with id = "); Console::puti(_file_id); Console::puts("\n");
    /* Here you go through the inode list to find the file. */
    if(inodes == nullptr) return nullptr;
    
    for(int i = 0; i < nInode; i++) {
    	if(inodes[i].valid && inodes[i].id == _file_id) return &inodes[i];
    }
    return nullptr;
}

bool FileSystem::CreateFile(int _file_id) {
    Console::puts("creating file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* Here you check if the file exists already. If so, throw an error.
       Then get yourself a free inode and initialize all the data needed for the
       new file. After this function there will be a new file on disk. */
    if(LookupFile(_file_id)) return false;

    unsigned char* buf = new unsigned char[512];
    freeBlock = GetFreeBlock();	
    free_blocks[freeBlock] = 1; //used
    disk->read(freeBlock,buf);

    Inode* curNode = &inodes[GetFreeInode()]; //setting Inode info first to map inode with file in File constructor
    curNode->id = _file_id;
    curNode->valid = true;
    curNode->blockIdx = freeBlock;

    /*Creating New file by sending this block numbers*/
    File *newFile = new File(this, _file_id);

    curNode->file = newFile;
    return true;
}

bool FileSystem::DeleteFile(int _file_id) {
    Console::puts("deleting file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* First, check if the file exists. If not, throw an error. 
       Then free all blocks that belong to the file and delete/invalidate 
       (depending on your implementation of the inode list) the inode. */
    Inode* deleteNode = LookupFile(_file_id);
    if(!deleteNode) return false;
    delete deleteNode->file; //data block deleted
    free_blocks[deleteNode->blockIdx] = 0; // free
    if(deleteNode->blockIdx == maxUsed) maxUsed--; //decrease maxUsed
    deleteNode->blockIdx = -1;
    deleteNode->valid = false;
    return true;
}
