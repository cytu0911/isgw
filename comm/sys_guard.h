/******************************************************************************
*  @file      sys_prot.h
*  @author awayfang
*  @history 
*  通用的线程锁保护机制类
*  
******************************************************************************/

#ifndef _SYS_GUARD_H_
#define _SYS_GUARD_H_
#include "sys_thread_mutex.h"
class SysGuard
{
public:
    SysGuard(SYS_Thread_Mutex &lock);
    ~SysGuard();
    
private:
    int acquire ();
    int release ();
    
private:
    SYS_Thread_Mutex *lock_;
    int owner_;
};
#endif // _SYS_GUARD_H_
