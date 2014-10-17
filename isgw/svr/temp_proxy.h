/************************************************************
  Copyright (C), 2008-2018, Tencent Tech. Co., Ltd.
  FileName: temp_proxy.h
  Author: awayfang              Date: 2010-08-20
  Description: Զ�̴�����ģ��  
  Function List:   
  History:     
      <author>     <time>     <version >   <desc>

***********************************************************/
#ifndef _TEMP_PROXY_H_
#define _TEMP_PROXY_H_
#include "isgw_comm.h"
#include "plat_conn_mgr_asy.h"
#include "pdb_prot.h"

class TempProxy
{
public:
    TempProxy();
    ~TempProxy();
    // ���Իص� 
    int test(QModeMsg &req);
    //�ص� ����
    static int cb_test(QModeMsg &ack, string& content, char* ack_buf, int& ack_len);
    
public:
    static int init();
private:
    static PlatConnMgrAsy* get_conmgr(); 
    static int init_conmgr(); 

private:
    static PlatConnMgrAsy* conmgr_;
    static ACE_Thread_Mutex conmgr_lock_;
};

#endif //_TEMP_PROXY_H_
