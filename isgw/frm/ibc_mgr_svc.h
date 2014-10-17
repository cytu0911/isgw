/******************************************************************************
*  @file      isgw_mgr_svc.h
*  @author awayfang <awayfang@tencent.com>
*  @history 
*  isgw 框架的 批量处理模块 
*  
******************************************************************************/
#ifndef _IBC_MGR_SVC_H_
#define _IBC_MGR_SVC_H_
#include "isgw_comm.h"
#include "ibc_prot.h"

#define MSG_QUE_SIZE 10*1024*1024
#define MAX_IBCR_RECORED 1024 //map 里面最多的记录数 

//此模块负责把从网络接收的消息调用业务逻辑提供的接口进行处理，并回送给网络收发接口
class IBCMgrSvc : public ACESvc<QModeMsg, PriProAck>
{
public:
    static IBCMgrSvc* instance();
    virtual ~IBCMgrSvc(){}
    virtual int init();
    
private:
    IBCMgrSvc(){}
    virtual PriProAck* process(QModeMsg*& req);
    virtual int send(PriProAck* ack);
    
    int encode_ppack(const IBCRKey& key
        , IBCRValue* prvalue, PriProAck* ack);
    
private:
    static IBCMgrSvc *instance_;
    static IBCR_MAP ibcr_map_;
    static ACE_Thread_Mutex ibcr_map_lock_;
    static int max_ibcr_record_;
    static int discard_flag_; // 超时消息丢弃标志 
    static int discard_time_; // 超时时间判断 
};

#endif //_IBC_MGR_SVC_H_
