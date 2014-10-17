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

//��ģ�鸺��Ѵ�������յ���Ϣ����ҵ���߼��ṩ�Ľӿڽ��д��������͸������շ��ӿ�
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
    static int discard_flag_; // ��ʱ��Ϣ������־ 
    static int discard_time_; // ��ʱʱ���ж� 

    static int control_flag_; //Ĭ�ϲ����������� 
    static CmdAmntCntrl *freq_obj_; //Ƶ�ʿ��ƵĶ���

    //·�ɹ��ܵ����ÿ���
    //rflag_=0,�ر�·�ɹ���
    //rflag_=1,��·�ɹ��ܣ�����ֻ��·��ת��
    //rflag_=2,��·�ɹ��ܣ�����Ҳ����Ϣ����
    static int rflag_; //·�ɹ��ܵĿ���
    static int ripnum_; //���õ�·��ip������ 
    //key:appname 
    static map<string, PlatConnMgrAsy*> route_conn_mgr_map_;
    static ACE_Thread_Mutex conn_mgr_lock_;

	static int local_port_; // ���ض˿ڣ�������Ϣת��
};

#endif //_ISGW_MGR_SVC_H_
