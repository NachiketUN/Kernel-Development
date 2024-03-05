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
    file_id = _id;
    fileSystem = _fs;
    int block_num = fileSystem->LookupFile(file_id)->block;
    memset(block_cache, 0, SimpleDisk::BLOCK_SIZE);
    fileSystem->disk->read(block_num,block_cache);
}

File::~File() {
    Console::puts("Closing file.\n");
    /* Make sure that you write any cached data to disk. */
    /* Also make sure that the inode in the inode list is updated. */
    int block_num = fileSystem->LookupFile(file_id)->block;
    fileSystem->disk->write(block_num,block_cache);
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char *_buf) {
    Console::puts("reading from file\n");
    int char_read = 0;
    for(int i=0; i<_n; i++){
        if(EoF()) break;
        _buf[i] = block_cache[position];
        position++;
        char_read++;
    }
    return char_read;
}

int File::Write(unsigned int _n, const char *_buf) {
    Console::puts("writing to file\n");
    int char_write = 0;
    for(int i=0; i<_n; i++){
        if(EoF()) break;
        block_cache[position] = _buf[i];
        position++;
        char_write++;
    }
    return char_write;
}

void File::Reset() {
    Console::puts("resetting file\n");
    position = 0;
}

bool File::EoF() {
//    Console::puts("checking for EoF\n");
    return position == (SimpleDisk::BLOCK_SIZE - 1);
}
