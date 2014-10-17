/************************************************************
  Copyright (C), 2008-2018, Tencent Tech. Co., Ltd.
  FileName: isgw_sighdl.h
  Author: ianyuan              Date: 2011-01-25
  Description:
      signal handler
      201308 扩展支持定时器      
***********************************************************/

#ifndef _ISGW_SIGHDL_H_
#define _ISGW_SIGHDL_H_

#include "isgw_comm.h"

class ISGWSighdl: public ACE_Event_Handler
{
public:
    ~ISGWSighdl();

    int handle_signal(int signum, siginfo_t * = 0, ucontext_t * = 0);
    int handle_timeout(const ACE_Time_Value& tv, const void *arg);
    //int handle_exception(ACE_HANDLE);

    static ISGWSighdl* instance();
        
private:
    ISGWSighdl(); //避免多个对象被创建
    
private:
    int handle_reload();

private:
    static ISGWSighdl* instance_; 
};

#endif // _ISGW_UINTF_H_
