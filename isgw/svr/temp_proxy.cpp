#include "temp_proxy.h"
#include "isgw_ack.h"
#include "asy_prot.h"
#include "asy_task.h"
using namespace EASY_UTIL;

PlatConnMgrAsy* TempProxy::conmgr_ = NULL;
ACE_Thread_Mutex TempProxy::conmgr_lock_;

/*
// ��Ҫ����Ľ������� 
#ifndef _ISGW_CINTF_PARSE_
#define _ISGW_CINTF_PARSE_
int isgw_cintf_parse(PriProAck * ack)
{
    QModeMsg qmode_ack(ack->msg);
    //�����Ҫֱ�ӷ��ظ��ͻ��˿��԰Ѵ��ֶ�����Ϊ1
    ack->rflag = atoi((*(qmode_ack.get_map()))["_rflag"].c_str());
    ack->sock_fd = strtoul((*(qmode_ack.get_map()))["_sockfd"].c_str(), NULL, 10);
    ack->protocol = strtoul((*(qmode_ack.get_map()))["_prot"].c_str(), NULL, 10);
    ack->sock_seq = strtoul((*(qmode_ack.get_map()))["_sock_seq"].c_str(), NULL, 10);
    ack->seq_no = strtoul((*(qmode_ack.get_map()))["_msg_seq"].c_str(), NULL, 10);
    ack->cmd = qmode_ack.get_cmd();
    ack->time = strtoul((*(qmode_ack.get_map()))["_time"].c_str(), NULL, 10);
    return 0;
}
#endif
*/

int TempProxy::init_conmgr()
{
    ACE_DEBUG((LM_INFO,"[%D] TempProxy start to init conmgr\n"));
    //
    if (conmgr_ != NULL)
    {
        ACE_DEBUG((LM_ERROR, "[%D] TempProxy delete old conmgr object\n"));
        delete conmgr_;
        conmgr_ = NULL;
    }
    
    //�������ļ���ȡ���������Ӳ���
    conmgr_ = new  PlatConnMgrAsy();
    char conf_section[32];
    memset(conf_section, 0x0, sizeof(conf_section));
    sprintf(conf_section, "igame_svr");
    
    if(0 != conmgr_->init(conf_section))
    {
        //��������һ�¶��� ��Ȼ��������ʹ�õ��� core 
        delete conmgr_;
        conmgr_ = NULL;
        return -1;
    }
    
    ACE_DEBUG((LM_INFO,"[%D] TempProxy init conmgr succ\n"));
    return 0;
}

PlatConnMgrAsy* TempProxy::get_conmgr()
{
    ACE_DEBUG((LM_TRACE, "[%D] TempProxy start to get conmgr\n"));

    ACE_Guard<ACE_Thread_Mutex> guard(conmgr_lock_);
    if (NULL == conmgr_)
    {
        init_conmgr();
    }

    ACE_DEBUG((LM_TRACE, "[%D] TempProxy get conmgr succ\n"));
    return conmgr_;
}

int TempProxy::init()
{
    ASYTask::instance()->set_proc(&TempProxy::cb_test);
    return 0;
}

TempProxy::TempProxy()
{
    
}

TempProxy::~TempProxy()
{
    //
}

// �����첽���ӹ�����
int TempProxy::test(QModeMsg &req)
{
    unsigned int uin = req.get_uin();
    char req_buf[1024];
    memset(req_buf, 0, sizeof(req_buf));
    
    // ��֪�����̽��յ���˵ķ�����Ϣ�Ƿ�Ҫ���мӹ�������ֱ��͸��
    int rflag = atoi((*(req.get_map()))["rflag"].c_str());
    int timeout = atoi((*(req.get_map()))["timeout"].c_str());
    if (timeout <= 0)
    {
        timeout = 1;
    }
    
    PlatConnMgrAsy* conn_mgr = get_conmgr();
    if (NULL == conn_mgr)
    {
        ACE_DEBUG((LM_ERROR, "[%D] TempProxy test failed"
            ",conn mgr is null"
            ",uin=%u\n"
            , uin
            ));
        return -1;
    }
    //��Ҫ����Ϣ��Ψһ��ʶ ������ˣ���Ȼ�޷����ֻ�������Ϣ 
    snprintf(req_buf, sizeof(req_buf)-1
        , "_sockfd=%d&_sock_seq=%d&_msg_seq=%d&_prot=%d&_time=%d&_rflag=%d&cmd=%d&uin=%u\n"
    , req.get_handle()
    , req.get_sock_seq()
    , req.get_msg_seq()
    , req.get_prot()
    , req.get_time()
    , rflag, 103, uin);

    // �����Ҫ�����ûص���ص���Ϣ 
    ASYRMsg rmsg;
    rmsg.key.sock_fd = req.get_handle();
    rmsg.key.sock_seq = req.get_sock_seq();
    rmsg.key.msg_seq = req.get_msg_seq();
    //rmsg.value.asy_proc = &TempProxy::cb_test;
    rmsg.value.content="test content string info";
    
    ACE_DEBUG((LM_DEBUG, "[%D] TempProxy test,uin=%u,req_buf=%s\n", uin, req_buf));

    //�����лص������ķ��ͽӿ�(���ÿ����Ϣָ���ص���Ϣ)
    int ret = conn_mgr->send(req_buf, strlen(req_buf), rmsg);
    if (ret == -1) //-1��Ϊʧ��
    {
        ACE_DEBUG((LM_ERROR, "[%D] TempProxy test send failed"
            ",uin=%u"
            ",req=%s\n"
            , uin
            , req_buf
            ));
        return -1;
    }
    
    ACE_DEBUG((LM_INFO, "[%D] TempProxy test send succ"
        ",uin=%u"
        ",req_buf=%s"
        "\n"
        , uin
        , req_buf
        ));
    return 0;
}

int TempProxy::cb_test(QModeMsg& ack, string& content, char* ack_buf, int& ack_len)
{
    snprintf(ack_buf, ack_len-1, "%s&info=cb_test async conn,%s\n", ack.get_body(), content.c_str());
    
    ACE_DEBUG((LM_INFO, "[%D] TempProxy cb_test, ack_buf=%s\n", ack_buf));
    return 0;
}

