/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
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
#include "file.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File(FileSystem *_fs, int _id) {
    Console::puts("Opening file.\n");
    fs = _fs;

    fileSize = 512;
    current_pos = 0; //indicates the current start position. Useful in cases where not all 512 bytes are occupied.
    inode = fs->LookupFile(_id);
    current_block = inode->blockIdx;
}

File::~File() {
    Console::puts("Closing file.\n");
    fs->disk->write(current_block, block_cache);
    /* Make sure that you write any cached data to disk. */
    /* Also make sure that the inode in the inode list is updated. */
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char *_buf) {
    Console::puts("reading from file\n");
    fs->disk->read(current_block, block_cache);	
    
    int leftInBlock = 512 - current_pos;
    
    if(_n <= leftInBlock) {
	    memcpy(_buf, block_cache + current_pos, _n);
	    current_pos += _n;	
	    return _n;
    } else {
	    Console::puts("read exceed one block\n");
    }
    return _n;
}

int File::Write(unsigned int _n, const char *_buf) {
   Console::puts("writing to file\n");
   
   fs->disk->read(current_block, block_cache);
   
   unsigned int leftInBlock = 512 - current_pos;
		
   if(_n < leftInBlock) {
	   memcpy(block_cache + current_pos, _buf, _n);
	   current_pos += _n;
	   Console::puts("WRITE COMPLETE \n");
	   return _n;
   } else {
	   Console::puts("read file exceed one block\n");
   }
   return _n;
}

void File::Reset() {
    Console::puts("resetting file\n");
    current_pos = 0;
}

bool File::EoF() {
    Console::puts("checking for EoF\n");
    return  current_pos == fileSize;
}
