/************************************************************
  Copyright (C), 2008-2018, Tencent Tech. Co., Ltd.
  FileName: isgw_ack.h
  Author: awayfang              Date: 2008-06-01
  Description:
      send msg back to client 
***********************************************************/
#ifndef _ISGW_ACK_H_
#define _ISGW_ACK_H_
#include "isgw_comm.h"

// ����С�����ֵ(1000) ��Ȼcpuռ�û�ܸ� ���׵�����ѭ��
#define DEFAULT_TIME_INTERVAL 1000 //��λ΢��
#ifndef ALARM_TIMEOUT
#define ALARM_TIMEOUT 1 //��λ��
#endif

class ISGWAck : public ACE_Event_Handler
{
public:
    static ISGWAck* instance(); 
    static ISGWAck* instance(int tv);
    void putq(PriProAck* ack_msg);
    virtual int handle_input(ACE_HANDLE fd = ACE_INVALID_HANDLE);
    virtual int handle_timeout(const ACE_Time_Value& tv, const void *arg);
    unsigned int get_time();
    unsigned int get_utime();

private:
    ISGWAck() : notification_strategy_(ACE_Reactor::instance(),
        this, ACE_Event_Handler::READ_MASK)
    {
        
        ACE_Time_Value delay(0,0);
        ACE_Time_Value interval(0,DEFAULT_TIME_INTERVAL);
        ACE_Reactor::instance()->schedule_timer(this, 0, delay, interval);
    }
    ISGWAck(int tv) : notification_strategy_(ACE_Reactor::instance(),
        this, ACE_Event_Handler::READ_MASK)
    {
        
        ACE_Time_Value delay(0,0);
        ACE_Time_Value interval(0, tv);
        ACE_Reactor::instance()->schedule_timer(this, 0, delay, interval);
    }
    int process();
    uint32 statisitc(PriProAck* ack_msg);

private:
    ACE_Reactor_Notification_Strategy notification_strategy_;
    ACE_Thread_Mutex queue_lock_;
    deque<PriProAck*> msg_queue_;

    static ISGWAck* instance_;
    static struct timeval time_; //��ǰ��ʱ�� ������ ����ʱ���Ļ��������� 
};

#endif //_ISGW_ACK_H_

