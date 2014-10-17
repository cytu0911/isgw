/******************************************************************************
*  @file      isgw_mgr_svc.h
*  @author awayfang <awayfang@tencent.com>
*  @history 
*  isgw ��ܵ� ��������ģ�� 
*  
******************************************************************************/
#ifndef _IBC_MGR_SVC_H_
#define _IBC_MGR_SVC_H_
#include "isgw_comm.h"
#include "ibc_prot.h"

#define MSG_QUE_SIZE 10*1024*1024
#define MAX_IBCR_RECORED 1024 //map �������ļ�¼�� 

//��ģ�鸺��Ѵ�������յ���Ϣ����ҵ���߼��ṩ�Ľӿڽ��д��������͸������շ��ӿ�
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
    static int discard_flag_; // ��ʱ��Ϣ������־ 
    static int discard_time_; // ��ʱʱ���ж� 
};

#endif //_IBC_MGR_SVC_H_
