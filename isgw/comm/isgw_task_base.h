#ifndef ISGW_TASK_BASE_H
#define	ISGW_TASK_BASE_H

#include <string>
#include <ace/Task.h>

class IsgwTaskBase : public ACE_Task<ACE_MT_SYNCH> 
{
public:
    IsgwTaskBase(const std::string& conf_section = "comm") : conf_section_(conf_section), 
            thread_num_(DEF_THREAD_NUM) {}
    
public:
    virtual int init();
    virtual int stop();
    
public:
    virtual int open(void* p = 0);
    virtual int svc();
    virtual int put (ACE_Message_Block *mblk, ACE_Time_Value *timeout = 0);
    
protected:
    virtual int process(ACE_Message_Block* mblk) = 0;

protected:
    std::string conf_section_;
    unsigned int thread_num_;
    
private:
    static const unsigned int DEF_THREAD_NUM = 1;
};

#endif	/* ISGW_TASK_BASE_H */

