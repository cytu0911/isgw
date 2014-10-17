#include "sys_thread_mutex.h"
SYS_Thread_Mutex::SYS_Thread_Mutex(const char *name, pthread_mutexattr_t *attrs)
{
    pthread_mutex_init (&lock_, attrs);
}

SYS_Thread_Mutex::~SYS_Thread_Mutex()
{
    pthread_mutex_destroy (&lock_);
}

int SYS_Thread_Mutex::acquire()
{
    return pthread_mutex_lock (&lock_);
}
int SYS_Thread_Mutex::release()
{
    return pthread_mutex_unlock (&lock_);
}
