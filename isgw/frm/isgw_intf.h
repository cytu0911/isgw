/************************************************************
  Copyright (C), 2008-2018, Tencent Tech. Co., Ltd.
  FileName: isgw_intf.h
  Author: awayfang              Date: 2008-06-01
  Description:
      tcp socket handler/net interface 
***********************************************************/
#ifndef _ISGW_INTF_H_
#define _ISGW_INTF_H_
#include "isgw_comm.h"
#include "ace_sock_hdr_base.h"

#ifndef MAX_RECV_BUF_LEN
#define MAX_RECV_BUF_LEN MAX_INNER_MSG_LEN
#endif

class ISGWIntf : public AceSockHdrBase
{

public:
    ISGWIntf();
    virtual ~ISGWIntf();
    virtual int open(void * = 0);
    virtual int handle_input (ACE_HANDLE fd = ACE_INVALID_HANDLE);
    virtual int process(char* msg, int sock_fd, int sock_seq, int msg_len);
    int is_legal(char* msg);
    int is_auth();

private:
    char recv_buf_[MAX_RECV_BUF_LEN+1];
    //�Ѿ����յ�����Ϣ���ȣ�Ӧ�ô������Ϣ���֣�buf���������������Ϣ
    unsigned int  recv_len_; 
    //һ����������Ϣ���ĳ��ȣ�һ�㲻������Ϣ�����ֽ�,���ǰ���������
    unsigned int  msg_len_;
    //�˴��ǽ��Ƶ�ԭ�Ӳ��� ������Ŀ����� ACE_Atomic_Op<ACE_Thread_Mutex, int>  ��� 
    static int msg_seq_;//��Ϣ��������кź��������ӵ����к������� 
};

typedef ACE_Acceptor<ISGWIntf, ACE_SOCK_ACCEPTOR> ISGW_ACCEPTOR;

#endif  //_ISGW_INTF_H_
