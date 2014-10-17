/******************************************************************************
*  @file      isgw_mgr_svc.h
*  @author awayfang <awayfang@tencent.com>
*  @history 
*  
******************************************************************************/
#ifndef _ISGW_MGR_SVC_H_
#define _ISGW_MGR_SVC_H_
#include "isgw_comm.h"
#include "cmd_amount_contrl.h"
#include "plat_conn_mgr_asy.h"

#define MSG_QUE_SIZE 10*1024*1024

typedef int (*OPER_INIT)();
//typedef int (*OPER_PROC)(char* req, char* ack, int& ack_len);
typedef int (*OPER_PROC)(QModeMsg& req, char* ack, int& ack_len);

//此模块负责把从网络接收的消息调用业务逻辑提供的接口进行处理，并回送给网络收发接口
class ISGWMgrSvc : public ACESvc<PriProReq, PriProAck>
{
public:
    static ISGWMgrSvc* instance();
    virtual ~ISGWMgrSvc(){}
    virtual int init();
    friend class IsgwOperBase;
    
    // get message queue size
    size_t message_count() { return queue_.message_count(); }
    
protected:
    ISGWMgrSvc()
    {
#ifdef ISGW_USE_DLL
        memset(dllname_, 0x0, sizeof(dllname_));
        oper_proc_ = NULL;
        
        dll_hdl_ = ACE_SHLIB_INVALID_HANDLE;
#endif
    }
    virtual PriProAck* process(PriProReq*& pp_req);
    virtual int send(PriProAck* ack);
    
    int check(QModeMsg& qmode_req);
    int forward(QModeMsg& qmode_req, int rflag, unsigned int uin=0);

#ifdef ISGW_USE_DLL
private:
    virtual int init_dll(const char* dllname);
    ACE_DLL dll_;
    char dllname_[128]; //
    OPER_PROC oper_proc_;
    
    ACE_SHLIB_HANDLE dll_hdl_;
    virtual int init_dll_os(const char* dllname);
#endif
    
private:
    static ISGWMgrSvc *instance_;
    static int discard_flag_; // 超时消息丢弃标志 
    static int discard_time_; // 超时时间判断 

    static int control_flag_; //默认不做流量控制 
    static CmdAmntCntrl *freq_obj_; //频率控制的对象

    //路由功能的设置开关
    //rflag_=0,关闭路由功能
    //rflag_=1,打开路由功能，并且只做路由转发
    //rflag_=2,打开路由功能，并且也做消息处理
    static int rflag_; //路由功能的开关
    static int ripnum_; //配置的路由ip的数量 
    //key:appname 
    static map<string, PlatConnMgrAsy*> route_conn_mgr_map_;
    static ACE_Thread_Mutex conn_mgr_lock_;

	static int local_port_; // 本地端口，用于消息转发
};

#endif //_ISGW_MGR_SVC_H_
