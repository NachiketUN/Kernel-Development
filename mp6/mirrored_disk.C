
#include "assert.H"
#include "utils.H"
#include "console.H"
#include "blocking_disk.H"
#include "mirrored_disk.H"

MirroredDisk::MirroredDisk(DISK_ID _disk_id, unsigned int _size)
        : SimpleDisk(_disk_id, _size) {
    leader_disk = new BlockingDisk(DISK_ID::MASTER, _size);
    follower_disk = new BlockingDisk(DISK_ID::DEPENDENT, _size);
}
void MirroredDisk::wait_until_one_ready()
{
    if (!leader_disk->is_ready() || !follower_disk->is_ready())
    {
        SYSTEM_SCHEDULER->resume(Thread::CurrentThread());
        SYSTEM_SCHEDULER->yield();
    }
}
void MirroredDisk::read(unsigned long _block_no, unsigned char * _buf) {
    leader_disk->issue_operation(DISK_OPERATION::READ, _block_no);
    follower_disk->issue_operation(DISK_OPERATION::READ, _block_no);
    wait_until_one_ready();
    int i;
    unsigned short tmpw;
    for (i = 0; i < 256; i++) {
        tmpw = Machine::inportw(0x1F0);
        _buf[i*2]   = (unsigned char)tmpw;
        _buf[i*2+1] = (unsigned char)(tmpw >> 8);
    }
}
void MirroredDisk::write(unsigned long _block_no, unsigned char * _buf)
{
    leader_disk->write(_block_no, _buf);
    follower_disk->write(_block_no, _buf);
}

