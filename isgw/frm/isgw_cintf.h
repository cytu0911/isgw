/************************************************************
  Copyright (C), 2008-2018
  FileName: isgw_cintf.h
  Author: awayfang              Date: 2010-08-20
  Description:
      tcp conn socket handler/net interface 
      2014-06-06 ���Կ��Ǽ̳���ISGWIntf �� �򻯴����ʵ��
***********************************************************/
#ifndef _ISGW_CINTF_H_
#define _ISGW_CINTF_H_
#include "isgw_comm.h"
#include "ace_sock_hdr_base.h"

#ifndef MAX_RECV_BUF_LEN
#define MAX_RECV_BUF_LEN MAX_INNER_MSG_LEN
#endif

#ifndef MSG_QUE_SIZE
#define MSG_QUE_SIZE 10*1024*1024
#endif

typedef ACE_Message_Queue_Ex<PriProAck, ACE_MT_SYNCH> ISGWC_MSGQ;

// ������Ӵ����� ����Ӻ���첽���ص���Ϣ 
class ISGWCIntf : public AceSockHdrBase
{    
public:
    ISGWCIntf();
    virtual ~ISGWCIntf();
    virtual int open(void * = 0);
    virtual int handle_input (ACE_HANDLE fd = ACE_INVALID_HANDLE);
    // ������Ϣ�����뱾�����Ϣ�����л���ֱ��͸����ǰ�� 
    virtual int process(char* msg, int sock_fd, int sock_seq, int msg_len);
    int is_legal(char* msg);
    int is_auth();
    // �ӱ��� queue_ ������Ϣ  time_out == NULL Ĭ������ 
    static int recvq(PriProAck*& msg, ACE_Time_Value* time_out=NULL);
    static int init();

private:
    char recv_buf_[MAX_RECV_BUF_LEN+1];
    unsigned int  recv_len_;
    unsigned int  msg_len_; //һ����������Ϣ���ĳ��ȣ�һ�㲻������Ϣ�����ֽ�
    static int msg_seq_;//��Ϣ��������кź��������ӵ����к�������
    
    static ISGWC_MSGQ queue_; //��Ÿ������Ͻ��յ�����Ϣ 
    static ACE_Time_Value zero_; 
};

typedef  ACE_Connector<ISGWCIntf, ACE_SOCK_CONNECTOR> ISGW_CONNECTOR;

#endif  //_ISGW_CINTF_H_
