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
    inodes = new Inode[SimpleDisk::BLOCK_SIZE];
    free_blocks = new unsigned char [SimpleDisk::BLOCK_SIZE];
}

FileSystem::~FileSystem() {
    Console::puts("unmounting file system\n");
    /* Make sure that the inode list and the free list are saved. */
    delete inodes;
    delete free_blocks;
}


/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/


bool FileSystem::Mount(SimpleDisk * _disk) {
    Console::puts("mounting file system from disk\n");
    disk = _disk;
    /* Here you read the inode list and the free list into memory */
    unsigned char block_buffer[SimpleDisk::BLOCK_SIZE];
    memset(block_buffer,0,SimpleDisk::BLOCK_SIZE);
    disk->read(0,block_buffer);
    inodes = (Inode*) block_buffer;
    memset(block_buffer,0,SimpleDisk::BLOCK_SIZE);
    disk->read(1,block_buffer);
    free_blocks = (unsigned char *) block_buffer;
    return true;
}

bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size) { // static!
    Console::puts("formatting disk\n");
    /* Here you populate the disk with an initialized (probably empty) inode list
       and a free list. Make sure that blocks used for the inodes and for the free list
       are marked as used, otherwise they may get overwritten. */
    long num_blocks = _size/SimpleDisk::BLOCK_SIZE;
    unsigned char block_buffer[SimpleDisk::BLOCK_SIZE];
    memset(block_buffer,0,SimpleDisk::BLOCK_SIZE);
    for(int i = 3; i < num_blocks; i++){
        _disk->write(i,block_buffer);
    }

    Inode * temp_inodes = (Inode*) block_buffer;
    for(int i = 0; i < MAX_INODES;i++){
        temp_inodes[i].id = -1;
        temp_inodes[i].block = -1;
        temp_inodes[i].size = 0;
    }
    _disk->write(0, block_buffer);
    memset(block_buffer,0,SimpleDisk::BLOCK_SIZE);
    block_buffer[0] = 1;
    block_buffer[1] = 1;
    for(int i = 3; i < MAX_INODES;i++){
        block_buffer[i] = 0;
    }
    _disk->write(1,block_buffer);
    return true;
}

Inode * FileSystem::LookupFile(int _file_id) {
    Console::puts("looking up file with id = "); Console::puti(_file_id); Console::puts("\n");
    /* Here you go through the inode list to find the file. */
    unsigned char block_buffer[SimpleDisk::BLOCK_SIZE];
    disk->read(0, block_buffer);
    inodes = (Inode *) block_buffer;
    for(int i=0;i<MAX_INODES;i++){
        if(inodes[i].id == _file_id){
            return &inodes[i];
        }
    }
    return NULL;
}

int FileSystem::GetFreeBlock(){
    unsigned char block_buffer[SimpleDisk::BLOCK_SIZE];
    memset(block_buffer,1, SimpleDisk::BLOCK_SIZE);
    disk->read(1,block_buffer);
    for(int i=0;i<MAX_INODES;i++){
        if(block_buffer[i] == 0){
            block_buffer[i] = 1;
            disk->write(1,block_buffer);
            return i;
        }
    }
    return -1;
}

bool FileSystem::CreateFile(int _file_id) {
    Console::puts("creating file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* Here you check if the file exists already. If so, throw an error.
       Then get yourself a free inode and initialize all the data needed for the
       new file. After this function there will be a new file on disk. */
    unsigned char block_buffer[SimpleDisk::BLOCK_SIZE];
    memset(block_buffer,0, SimpleDisk::BLOCK_SIZE);
    disk->read(0,block_buffer);
    inodes = (Inode *) block_buffer;
    int free_inode = -1;
    for(int i=0;i<MAX_INODES;i++){
        if(inodes[i].id == -1){
            free_inode = i;
        }
        if(inodes[i].id == _file_id){
            return false;
        }
    }
    inodes[free_inode].id = _file_id;
    inodes[free_inode].size = SimpleDisk::BLOCK_SIZE;
    inodes[free_inode].block = GetFreeBlock();
    disk->write(0,block_buffer);
    return true;


}

bool FileSystem::DeleteFile(int _file_id) {
    Console::puts("deleting file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* First, check if the file exists. If not, throw an error. 
       Then free all blocks that belong to the file and delete/invalidate 
       (depending on your implementation of the inode list) the inode. */
    unsigned char block_buffer[SimpleDisk::BLOCK_SIZE];
    memset(block_buffer,0, SimpleDisk::BLOCK_SIZE);
    disk->read(0,block_buffer);
    unsigned char data_block[SimpleDisk::BLOCK_SIZE];
    memset(data_block,0, SimpleDisk::BLOCK_SIZE);
    disk->read(1,data_block);

    inodes = (Inode *) block_buffer;
    int inode_num = -1;
    for(int i=0;i<MAX_INODES;i++){
        if(inodes[i].id == _file_id){
            inode_num = i;
        }
    }
    if(inode_num == -1) return false;
    inodes[inode_num].id = -1;
    data_block[inodes[inode_num].block] = 0;
    inodes[inode_num].block = -1;
    inodes[inode_num].size = 0;
    disk->write(0,block_buffer);
    disk->write(1,data_block);
    return true;

}
