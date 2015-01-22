/************************************************************
  Copyright (C), 2008-2018
  FileName: isgw_uintf.h
  Author: awayfang              Date: 2008-06-25
  Description:
      udp socket handler/net interface 
***********************************************************/

#ifndef _ISGW_UINTF_H_
#define _ISGW_UINTF_H_
#include "isgw_comm.h"

#define MAX_UDP_BUF_LEN MAX_INNER_MSG_LEN

class ISGWUIntf: public ACE_Event_Handler
{
public:
    virtual ~ISGWUIntf();
    int open(const ACE_INET_Addr &svr_addr);

    virtual int handle_input(ACE_HANDLE fd = ACE_INVALID_HANDLE);
    virtual int process(char* msg, int sock_fd, int sock_seq);

    int is_legal(char* msg);
    int is_auth();
    
    virtual ACE_HANDLE get_handle (void) const;
    static ISGWUIntf* instance();
    int send_udp(char* ack_msg, int send_len, const ACE_INET_Addr& to_addr);
    
protected:
    virtual int handle_close (ACE_HANDLE handle,ACE_Reactor_Mask mask); 
    
private:
    ISGWUIntf(); //避免多个对象被创建

private:
    static ISGWUIntf* instance_; 
    ACE_SOCK_Dgram dgram_; // udp sock
    ACE_INET_Addr remote_addr_;
    
    int ret_;
    char recv_buf_[MAX_UDP_BUF_LEN+1];
    static ACE_UINT32 msg_seq_; 
};

#endif // _ISGW_UINTF_H_
