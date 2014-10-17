/************************************************************
  Copyright (C), 2008-2018, Tencent Tech. Co., Ltd.
  FileName: ace_sock_hdr_base.h
  Author: awayfang              Date: 2008-06-01
  Description:
      tcp socket handler base 
***********************************************************/
#ifndef _ACE_SOCK_HDR_BASE_H_
#define _ACE_SOCK_HDR_BASE_H_
#include "easyace_all.h"

class AceSockHdrBase : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>
{
    typedef ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> super;
public:

    AceSockHdrBase();
    virtual ~AceSockHdrBase();
    virtual int open(void *p = 0);
    virtual int handle_output (ACE_HANDLE fd = ACE_INVALID_HANDLE);
    virtual int send(ACE_Message_Block* ack_msg = NULL);
    virtual int send_n(char* ack_msg, int len);
    virtual int get_seq();
	
protected:
    static unsigned int total_seq_; //ACE_Atomic_Op<ACE_Thread_Mutex, int> 
    int sock_seq_;
    ACE_INET_Addr remote_addr_;
    ACE_Time_Value *time_null_;
    ACE_Time_Value time_zero_;
    
};

#endif //_ACE_SOCK_HDR_BASE_H_
