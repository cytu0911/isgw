#include "sys_guard.h"
SysGuard::SysGuard(SYS_Thread_Mutex &lock):lock_(&lock)
{
    acquire();
}

int SysGuard::release()
{
    if ( owner_ == -1 )
    {
        return -1;
    }
    owner_ =  -1;
    return lock_->release(); 
}

int SysGuard::acquire()
{
    return owner_ = lock_->acquire();
}

SysGuard::~SysGuard()
{
    release();
}
