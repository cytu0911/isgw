/******************************************************************************
*  @file      isgw_app.h
*  @author awayfang
*  @history 
*  
******************************************************************************/
#ifndef _ISGW_APP_H_
#define _ISGW_APP_H_
#include "isgw_comm.h"

#define SYS_MODULE_VERSION "V3.2"

///默认消息对象池的大小
#ifndef OBJECT_QUEUE_SIZE
#define OBJECT_QUEUE_SIZE 3000
#endif 

#define MAX_FD_SETSIZE 10240
#define DEF_MAX_WAIT_MGR_TIME 5

class ISGWApp : public ACEApp
{
public:
    ISGWApp();
    virtual int init_app(int argc, ACE_TCHAR* argv[]);
    virtual int  init_sys_path(const char* program);
    virtual int  init_conf();
    virtual int  init_stat();
    virtual int init_reactor();
    virtual int quit_app();
    virtual void disp_version();
    
private:
    unsigned int max_wait_mgr_time_;
};

#endif  //_ISGW_APP_H_
