/******************************************************************************
*  @file object_que.h
*  @author awayfang
*  @history 
*   201009 awayfang 
*   对象池实现，主要用于管理空白对象，解决
*   频繁分配对象产生的一系列问题
*  
******************************************************************************/
#ifndef _OBJECT_QUE_H_
#define _OBJECT_QUE_H_
#include "sys_comm.h"

template <typename MESSAGE_TYPE>
class Object_Que
{
public:
    static Object_Que<MESSAGE_TYPE>* instance()
    {
        if (instance_ == NULL)
        {
            ACE_DEBUG((LM_INFO, "[%D] Object_Que new instance\n"));
            instance_ = new Object_Que<MESSAGE_TYPE>();
        }
        return instance_;
    }
    
    Object_Que();
    virtual ~Object_Que();
    virtual int init(int msg_counts = 5000);
    virtual int enqueue (MESSAGE_TYPE *&new_item);
    virtual int dequeue (MESSAGE_TYPE *&first_item);
	
private:
    deque<MESSAGE_TYPE*> que_imp_;
    ACE_Thread_Mutex queue_lock_;
    ACE_Time_Value timeout_;
    int ret_;
    int max_msg_count_;
    static Object_Que<MESSAGE_TYPE> *instance_;
};

template <typename MESSAGE_TYPE>
Object_Que<MESSAGE_TYPE>* Object_Que<MESSAGE_TYPE>::instance_ = NULL;

template <typename MESSAGE_TYPE>
Object_Que<MESSAGE_TYPE>::Object_Que ()
{
    ACE_DEBUG((LM_INFO, "[%D] Object_Que construct succ\n"));
}

template <typename MESSAGE_TYPE>
Object_Que<MESSAGE_TYPE>::~Object_Que ()
{
    ACE_DEBUG((LM_INFO, "[%D] Object_Que destruct succ\n"));
}

template <typename MESSAGE_TYPE>
int Object_Que<MESSAGE_TYPE>::init (int msg_counts)
{    
    ACE_Guard<ACE_Thread_Mutex> guard(queue_lock_);
    
    MESSAGE_TYPE *msg = NULL;
    for(int i=que_imp_.size(); i<msg_counts; i++)
    {
        ACE_DEBUG((LM_TRACE, "[%D] Object_Que init msg, start to new the %d msg\n", i));
        ACE_NEW_NORETURN(msg, MESSAGE_TYPE);
        que_imp_.push_back(msg);        
    }
    
    ACE_DEBUG((LM_INFO, "[%D] Object_Que init msg succ, msg counts=%d\n"
        ,que_imp_.size()
        ));
    max_msg_count_ = msg_counts;
    return 0;
}

template <typename MESSAGE_TYPE>
int Object_Que<MESSAGE_TYPE>::enqueue (MESSAGE_TYPE *&new_item)
{
    ACE_Guard<ACE_Thread_Mutex> guard(queue_lock_);
    if(new_item == NULL)
    {
        ACE_DEBUG((LM_ERROR, "[%D] Object_Que enqueue failed,new_item is NULL"));
        return -1;
    }

    int ret_ = que_imp_.size();
    if (ret_ >= max_msg_count_)
    {
        if (new_item != NULL)
        {
            delete new_item;
            new_item = NULL;
        }
        ACE_DEBUG((LM_ERROR, "[%D] ACE_Object_Que enqueue failed,que is full"
            ",delete this msg\n"));
        return ret_;
    }
    
    que_imp_.push_back(new_item);    
    if (que_imp_.empty())
    {
        ACE_DEBUG((LM_ERROR,
    		"[%D] Object_Que enqueue failed,msg count is %d\n"
    		, ret_
    		));
        new_item = NULL;
        return -1;
    }
    ACE_DEBUG((LM_TRACE, "[%D] Object_Que enqueue a msg succ,ret=%d\n", ret_));
    new_item = NULL;
    return ret_;
}

template <typename MESSAGE_TYPE>
int Object_Que<MESSAGE_TYPE>::dequeue (MESSAGE_TYPE *&first_item)
{
    ACE_Guard<ACE_Thread_Mutex> guard(queue_lock_);
    if ((first_item = que_imp_.front()) == NULL)
    {
        //获取不到则重新 产生 一个新的，并传递出去
        ACE_NEW_RETURN(first_item, MESSAGE_TYPE, ret_);
        ACE_DEBUG((LM_ERROR, "[%D] Object_Que dequeue failed,return a new blank msg\n"));
        if (que_imp_.size() > 0)
        {
            que_imp_.pop_front();
        }
        return que_imp_.size();
    }
    que_imp_.pop_front();
    ACE_DEBUG((LM_TRACE, "[%D] Object_Que dequeue a msg succ\n"));
    //返回内存之前先清理内存，确保返回的是空白的内存
    //memset(first_item, 0x0, sizeof(MESSAGE_TYPE));
    
    return que_imp_.size();
}

#endif//_OBJECT_QUE_H_
