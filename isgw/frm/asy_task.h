/******************************************************************************
*  @file      asy_task.h
*  @author awayfang
*  @history 
*   201405 awayfang 跟后端的异步联接的返回消息处理类(单独线程处理)
*  
******************************************************************************/
#ifndef _ASY_TASK_H_
#define _ASY_TASK_H_
#include "sys_comm.h"
#include "isgw_comm.h"
#include "asy_prot.h"

#define MAX_ASYR_RECORED 10240*2 //map 里面最多的记录数 
#define DISCARD_TIME 5  //超时丢弃

class ASYTask : public ACE_Task_Base
{
public:
    static ASYTask* instance()
    {
        ACE_DEBUG((LM_TRACE,
		"[%D] in ASYTask::instance\n"
		));
        if (instance_ == NULL)
        {
            instance_ = new ASYTask();
        }
        ACE_DEBUG((LM_TRACE,
		"[%D] out ASYTask::instance\n"
		));		
        return instance_;
    }       
    virtual int init();
    virtual int fini();
    virtual int insert(ASYRMsg &rmsg);
    //设置缺省的回调函数
    virtual int set_proc(ASY_PROC asy_proc);
    
protected:
    virtual int svc (void);

protected:
    //ACE_Time_Value time_out_;
    
    static ASYTask *instance_;
    //存放异步消息的相关结果
    static ASYR_MAP asyr_map_; 
    static ACE_Thread_Mutex asyr_map_lock_;
    static ASY_PROC asy_proc_; //缺省的回调函数
    
protected:
    static const int DEFAULT_THREADS = 2;
};

#endif //_ASY_TASK_H_
