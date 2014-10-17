/******************************************************************************
*  @file      asy_task.h
*  @author awayfang
*  @history 
*   201405 awayfang ����˵��첽���ӵķ�����Ϣ������(�����̴߳���)
*  
******************************************************************************/
#ifndef _ASY_TASK_H_
#define _ASY_TASK_H_
#include "sys_comm.h"
#include "isgw_comm.h"
#include "asy_prot.h"

#define MAX_ASYR_RECORED 10240*2 //map �������ļ�¼�� 
#define DISCARD_TIME 5  //��ʱ����

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
    //����ȱʡ�Ļص�����
    virtual int set_proc(ASY_PROC asy_proc);
    
protected:
    virtual int svc (void);

protected:
    //ACE_Time_Value time_out_;
    
    static ASYTask *instance_;
    //����첽��Ϣ����ؽ��
    static ASYR_MAP asyr_map_; 
    static ACE_Thread_Mutex asyr_map_lock_;
    static ASY_PROC asy_proc_; //ȱʡ�Ļص�����
    
protected:
    static const int DEFAULT_THREADS = 2;
};

#endif //_ASY_TASK_H_
