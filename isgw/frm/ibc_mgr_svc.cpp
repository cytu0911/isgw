/******************************************************************************
*  @file      ibc_mgr_svc.cpp
*  @author awayfang
*  @history 
*  
******************************************************************************/
#include "ibc_mgr_svc.h"
#include "isgw_ack.h" //���Ϳͻ�����Ӧ��Ϣ
#include "ibc_oper_fac.h" //���󴴽����� 
#include "stat.h" 

IBCR_MAP IBCMgrSvc::ibcr_map_;
ACE_Thread_Mutex IBCMgrSvc::ibcr_map_lock_;
IBCMgrSvc* IBCMgrSvc::instance_ = NULL;
int IBCMgrSvc::max_ibcr_record_ = MAX_IBCR_RECORED;
int IBCMgrSvc::discard_flag_ = 1; //Ĭ�Ͻ��ж��� 
int IBCMgrSvc::discard_time_ = 2; //Ĭ�ϳ���2�붪��

///Ĭ����Ϣ����صĴ�С
#ifndef OBJECT_QUEUE_SIZE
#define OBJECT_QUEUE_SIZE 5000
#endif

IBCMgrSvc* IBCMgrSvc::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new IBCMgrSvc();
    }
    return instance_;
}

int IBCMgrSvc::init()
{
    ACE_DEBUG((LM_INFO,
		"[%D] IBCMgrSvc start to init\n"
		));

    // ��ʱ���� 
    SysConf::instance()->get_conf_int("ibc_mgr_svc", "discard_flag", &discard_flag_);
    SysConf::instance()->get_conf_int("ibc_mgr_svc", "discard_time", &discard_time_);
    
    // ��ȡ��󻺴�Ľ����¼�� 
    SysConf::instance()->get_conf_int("ibc_mgr_svc", "max_ibcr_record", &max_ibcr_record_); 
    ACE_DEBUG((LM_INFO, "[%D] IBCMgrSvc set max_ibcr_record=%d\n", max_ibcr_record_));
    
    int quesize = MSG_QUE_SIZE;
    //������Ϣ���д�С
    SysConf::instance()->get_conf_int("ibc_mgr_svc", "quesize", &quesize); 
    ACE_DEBUG((LM_INFO, "[%D] IBCMgrSvc set quesize=%d(byte)\n", quesize));
    open(quesize, quesize, NULL); 
    
    //�����߳�����Ĭ��Ϊ 10 
    int threads = DEFAULT_THREADS; // ACESvc �� ���� 
    SysConf::instance()->get_conf_int("ibc_mgr_svc", "threads", &threads);    
    ACE_DEBUG((LM_INFO, "[%D] IBCMgrSvc set number of threads=%d\n", threads));
	//�����߳�
    activate(THR_NEW_LWP | THR_JOINABLE, threads);
    ACE_DEBUG((LM_INFO,
    	"[%D] IBCMgrSvc init succ\n"
    	));
    return 0;
}

PriProAck* IBCMgrSvc::process(QModeMsg*& req)
{
    int ret = 0;
    char ack_buf[MAX_INNER_MSG_LEN+1]; // ����Ե�ַ&�ָ�Ľ����Ϣ
    memset(ack_buf, 0x0, sizeof(ack_buf));    
    int ack_len = MAX_INNER_MSG_LEN;

    //���ݿͻ�������ָ�������ز���
    ACE_DEBUG((LM_TRACE, "[%D] IBCMgrSvc process cmd %d\n"
        , req->get_cmd()));
    IBCOperBase* oper = IBCOperFac::create_oper(req->get_cmd());
    if (oper == NULL)
    {
        ret = ERROR_IBC_FAC;
        Stat::instance()->incre_stat(STAT_CODE_IBCSVC_FAC);
        return NULL;
    }

    //�ж��Ƿ�ʱ 
    unsigned int now_time = ISGWAck::instance()->get_time();
    if (discard_flag_==1 && ((now_time - req->get_time()) > discard_time_))
    {
        // ���ڳ�ʱ����Ϣ��ʲô�������������˷�
        ACE_DEBUG((LM_ERROR, "[%D] IBCMgrSvc process msg failed,time out"
            ",sock_fd=%u,prot=%u,ip=%u,sock_seq=%u,seq_no=%u,time=%u"
        	"\n"
        	, req->get_handle()
        	, req->get_prot()
        	, req->get_sock_ip()
        	, req->get_sock_seq()
        	, req->get_msg_seq()
        	, req->get_time()
            ));
        ret = ERROR_TIMEOUT_FRM;
        Stat::instance()->incre_stat(STAT_CODE_IBCSVC_TIMEOUT);
        return NULL;
    }
    else
    {
        ret = oper->process(*req, ack_buf, ack_len);
    }
    
    //���ҵ������Ľ��
    IBCRKey key;           
    memset(&key, 0x0, sizeof(IBCRKey));
    key.sock_fd = req->get_handle();
    key.sock_seq = req->get_sock_seq();
    key.msg_seq = req->get_msg_seq();
    //����ȷ���Խ���Ĳ�����ԭ�ӵ� 
    ACE_Guard<ACE_Thread_Mutex> guard(ibcr_map_lock_);    
    PriProAck *pp_ack = NULL;
        
    IBCR_MAP::iterator it;
    IBCRValue *prvalue = NULL;
    
    it = ibcr_map_.find(key);
    if(it != ibcr_map_.end()) 
    {
        ACE_DEBUG((LM_TRACE, "[%D] IBCMgrSvc find match record\n"));        
        prvalue = &(it->second); //(IBCRValue)        
    }
    else //û�ҵ� ˵�����µļ�¼ 
    {        
        //�жϽ����¼���Ƿ���࣬������ֱ�ӷ��ز������ݸ�ǰ�ˣ���ɾ�������¼
        if (ibcr_map_.size() > max_ibcr_record_)
        {
            IBCR_MAP::iterator tmp_it;
            ACE_DEBUG((LM_ERROR, "[%D] IBCMgrSvc delete some records"
                ",num=%d\n", ibcr_map_.size()
                ));
        	tmp_it = ibcr_map_.end();
        	--tmp_it;

            //������Ӧ��Ϣ��Դ 
            ACE_Object_Que<PriProAck>::instance()->dequeue(pp_ack);
            if ( pp_ack == NULL )
            {
                ACE_DEBUG((LM_ERROR,
                    "[%D] ISGWMgrSvc dequeue msg failed"
                    ",maybe system has no memory\n"
                    ));
                //�������ص�ʱ�������Դ 
                if (req != NULL)
                {
                    delete req;
                    req = NULL;
                }
                if (oper != NULL)
                {
                    delete oper;
                    oper = NULL;
                }
                return NULL;
            }
            memset(pp_ack, 0x0, sizeof(PriProAck));
            pp_ack->tv_time = *(req->get_tvtime());
            prvalue = &(tmp_it->second);
            // �ڻ�����Ϣǰ������ж��Ƿ���Ҫ����end()����
            // ��Ҫ����cmd�ҵ�������
            encode_ppack(tmp_it->first, prvalue, pp_ack);
            //�ŵ��ͻ�����Ӧ��Ϣ������������Ӧ��Ϣ���͸��ͻ���
            ISGWAck::instance()->putq(pp_ack);
            //���꼰ʱ����Ϊ�� ��Ȼ�����п��ܱ������������·�������� 
            //��Ϊ pp_ack ���ջᱻ return 
            pp_ack = NULL; 
            
            ibcr_map_.erase(tmp_it);
        }
        
        //����һ��Ĭ��ֵ���¼�¼������ʼ�� total ��ֵ 
        IBCRValue rvalue;
        //memset(&rvalue, 0x0, sizeof(IBCRValue));    
        rvalue.time = req->get_time();
        rvalue.cmd = req->get_cmd();
        rvalue.uin = req->get_uin();
        rvalue.total = atoi((*(req->get_map()))["total"].c_str());
        //��Ҫ͸������Ϣ
        rvalue.sock_fd_ = strtoul((*(req->get_map()))["_sockfd"].c_str(), NULL, 10);
        rvalue.sock_seq_ = strtoul((*(req->get_map()))["_sock_seq"].c_str(), NULL, 10);
        rvalue.msg_seq_ = strtoul((*(req->get_map()))["_msg_seq"].c_str(), NULL, 10);
        rvalue.prot_ = strtoul((*(req->get_map()))["_prot"].c_str(), NULL, 10);
        rvalue.time_ = strtoul((*(req->get_map()))["_time"].c_str(), NULL, 10);
        rvalue.rflag_ = atoi((*(req->get_map()))["_rflag"].c_str());
        rvalue.endflag_ = atoi((*(req->get_map()))["endflag"].c_str());
        
        it = ibcr_map_.insert(ibcr_map_.begin(), pair<IBCRKey, IBCRValue>(key, rvalue));
        prvalue = &(it->second);
    }

    //��¼״̬���ϲ���� ���صĽ�������зǷ�����ʧ�ܵĽ�� ҵ����Ҫ�����쳣 
    prvalue->cnum++;
    if (ret == 0)
    {
        prvalue->snum++;
    }
    ret = oper->merge(*prvalue, ack_buf);

    //�жϹ�����¼�Ƿ��Ѿ�ȫ������ ȫ�������򷵻����ݸ��ͻ��� 
    if ( prvalue->cnum >= prvalue->total )
    {
        //�ص�һ�����մ�����
        if (prvalue->endflag_ == 1)
        {
            ret = oper->end(*prvalue);
        }
        
        //������Ӧ��Ϣ��Դ         
        ACE_Object_Que<PriProAck>::instance()->dequeue(pp_ack);
        if ( pp_ack == NULL )
        {
            ACE_DEBUG((LM_ERROR,
                "[%D] ISGWMgrSvc dequeue msg failed"
                ",maybe system has no memory\n"
                ));
            //�������ص�ʱ�������Դ 
            if (req != NULL)
            {
                delete req;
                req = NULL;
            }
            if (oper != NULL)
            {
                delete oper;
                oper = NULL;
            }
            return NULL;
        }
    	memset(pp_ack, 0x0, sizeof(PriProAck));
        pp_ack->tv_time = *(req->get_tvtime());
        encode_ppack(key, prvalue, pp_ack);

        ACE_DEBUG((LM_INFO,"[%D] IBCMgrSvc process succ"
            ",time=%u"
            ",cmd=%u"
            ",uin=%u"
            ",total=%u"
            ",cnum=%u"
            ",snum=%u"
            ",msg_len=%u"
            ",sock_fd_=%u"
            ",sock_seq_=%u"
            ",msg_seq_=%u"
            ",prot_=%u"
            ",time_=%u"
            ",rflag_=%d"
            "\n"
            , prvalue->time
            , prvalue->cmd
            , prvalue->uin
            , prvalue->total
            , prvalue->cnum
            , prvalue->snum
            , prvalue->msg_len
            , prvalue->sock_fd_
            , prvalue->sock_seq_
            , prvalue->msg_seq_
            , prvalue->prot_
            , prvalue->time_
            , prvalue->rflag_
            ));
        
        //ɾ��������¼
        ibcr_map_.erase(it);
    }
    
    ACE_DEBUG((LM_TRACE,"[%D] IBCMgrSvc process cmd %d succ\n"
        , req->get_cmd()));

    //�������ص�ʱ�������Դ 
    if (req != NULL)
    {
        delete req;
        req = NULL;
    }
    if (oper != NULL)
    {
        delete oper;
        oper = NULL;
    }
    
    return pp_ack;
}

int IBCMgrSvc::send(PriProAck* ack)
{
    ACE_DEBUG((LM_TRACE,"[%D] in IBCMgrSvc::send\n"
		));
    
    //�ŵ��ͻ�����Ӧ��Ϣ������������Ӧ��Ϣ���͸��ͻ���
    ISGWAck::instance()->putq(ack);
    
    ACE_DEBUG((LM_TRACE,"[%D] out IBCMgrSvc::send\n"));
    return 0;
}

int IBCMgrSvc::encode_ppack(const IBCRKey& key
    , IBCRValue* prvalue, PriProAck* pp_ack)
{
    pp_ack->sock_fd = key.sock_fd;
    pp_ack->sock_seq = key.sock_seq;
    pp_ack->seq_no = key.msg_seq;
    //pp_ack->time = prvalue->time;
    pp_ack->cmd = prvalue->cmd;
    
    //ǰ��͸����Ϣ 
    if (prvalue->sock_fd_ != 0 || prvalue->sock_seq_ != 0 || prvalue->msg_seq_ != 0)
    {
        pp_ack->msg_len += snprintf(pp_ack->msg+pp_ack->msg_len
            , MAX_INNER_MSG_LEN-pp_ack->msg_len
            , "_sockfd=%d&_sock_seq=%d&_msg_seq=%d&_prot=%d&_time=%d&_rflag=%d&"
            , prvalue->sock_fd_, prvalue->sock_seq_, prvalue->msg_seq_
            , prvalue->prot_, prvalue->time_, prvalue->rflag_);
    }
    
    //������Ϣ���ͻ��˵�ʱ��Ҫ����Э��ͷ�� cmd result uin ... ��β�� \n 
    pp_ack->msg_len += snprintf(pp_ack->msg+pp_ack->msg_len
        , MAX_INNER_MSG_LEN-pp_ack->msg_len
        , "%s=%d&%s=%d&%s=%u&total=%d&cnum=%d&snum=%d"
        , FIELD_NAME_CMD, prvalue->cmd
        , FIELD_NAME_RESULT, 0 //���������Ϊ�ǳɹ��� 
        , FIELD_NAME_UIN, prvalue->uin
        , prvalue->total, prvalue->cnum, prvalue->snum
        );
    
    //��Ϣ�壬���ȴ���list��Ĭ�ϼ�¼�ָ���Ϊ"|"(�Ż�: ��ָ���ָ���)
    if(!prvalue->msg_list.empty())
    {
        pp_ack->msg_len += snprintf(pp_ack->msg+pp_ack->msg_len
            , MAX_INNER_MSG_LEN-pp_ack->msg_len-INNER_RES_MSG_LEN, "&info=");
        std::list<std::string>::iterator itor = prvalue->msg_list.begin();
        for(;itor != prvalue->msg_list.end();++itor)
        {
            // ��ֹЭ�鳬��
            if (pp_ack->msg_len + INNER_RES_MSG_LEN*4 >= MAX_INNER_MSG_LEN)
            {
                break;
            }
            pp_ack->msg_len += snprintf(pp_ack->msg+pp_ack->msg_len
                , MAX_INNER_MSG_LEN-pp_ack->msg_len-INNER_RES_MSG_LEN
                , "%s|", itor->c_str());
        }
    }
    else
    {
        pp_ack->msg_len += snprintf(pp_ack->msg+pp_ack->msg_len
            , MAX_INNER_MSG_LEN-pp_ack->msg_len-INNER_RES_MSG_LEN
            , "&info=%s", prvalue->msg);
    }
    
    if (pp_ack->msg_len > MAX_INNER_MSG_LEN-sizeof(MSG_SEPARATOR))
    {
        pp_ack->msg_len = MAX_INNER_MSG_LEN-sizeof(MSG_SEPARATOR);
    }
    
    //��Ϣβ�� 
    pp_ack->msg_len += snprintf(pp_ack->msg+pp_ack->msg_len
        , MAX_INNER_MSG_LEN-pp_ack->msg_len
        , "%s", MSG_SEPARATOR);
    
    return 0;
}
