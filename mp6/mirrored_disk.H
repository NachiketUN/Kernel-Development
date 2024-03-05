//
// Created by Nachiket Naganure on 11/19/23.
//

#ifndef MIRRORED_DISK
#define MIRRORED_DISK
#include "scheduler.H"
extern Scheduler * SYSTEM_SCHEDULER;

class MirroredDisk : public SimpleDisk {

public:
    BlockingDisk* leader_disk;
    BlockingDisk* follower_disk;
    MirroredDisk(DISK_ID _disk_id, unsigned int _size);

    virtual void read(unsigned long _block_no, unsigned char * _buf);


    virtual void write(unsigned long _block_no, unsigned char * _buf);

    virtual void wait_until_one_ready();


};

#endif
