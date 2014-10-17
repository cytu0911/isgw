#include "plat_conn_mgr_asy.h"
#include "isgw_ack.h"
#include "stat.h"
#include "asy_task.h"

// �첽���ӵ�ʱ��ÿ��ipֻ��Ҫһ�����Ӽ���
PlatConnMgrAsy::PlatConnMgrAsy()
{
    memset(section_, 0x0, sizeof(section_));
    memset(ip_, 0x0, sizeof(ip_));    
    ip_num_ = IP_NUM_DEF; // Ĭ��ÿ��2�������� ip 
    port_ = 0;    
    time_out_.set(SOCKET_TIME_OUT);
    memset(&conn_info_, 0x0, sizeof(conn_info_));    
}

PlatConnMgrAsy::PlatConnMgrAsy(const char*host_ip, int port)
{
    ip_num_ = 1;
    snprintf(ip_[0], sizeof(ip_[0]), "%s", host_ip);
    port_ = port;
    
    time_out_.set(SOCKET_TIME_OUT);
    memset(&conn_info_, 0x0, sizeof(conn_info_));
}

PlatConnMgrAsy::~PlatConnMgrAsy()
{
    for (int i=0; i<IP_NUM_MAX; i++)
    {
        fini_conn(i);
    }
}

int PlatConnMgrAsy::init(const char *section)
{
    ACE_Guard<ACE_Thread_Mutex> guard(conn_mgr_lock_); 
    if (section != NULL)
    {
        strncpy(section_, section, sizeof(section_));
    }
    
    if ((SysConf::instance()->get_conf_int(section_, "ip_num", &ip_num_) != 0)
        || (ip_num_ > IP_NUM_MAX))
    {
        ip_num_ = IP_NUM_MAX;
    }
    
    int time_out = SOCKET_TIME_OUT;
    if (SysConf::instance()->get_conf_int(section_, "time_out", &time_out) == 0)
    {
        time_out_.set(time_out);
    }
    
	//��ȡ ip �б� 
	char ip_str[16];
	for(int i=0; i<ip_num_; i++)
    {
        memset(ip_str, 0x0, sizeof(ip_str));
        snprintf(ip_str, sizeof(ip_str), "ip_%d", i);
        
        if (SysConf::instance()
            ->get_conf_str(section_, ip_str, ip_[i], sizeof(ip_[i])) != 0)
        {
            ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrAsy get config failed"
                ",section=%s,ip_%d\n", section_, i));
            ip_num_ = i; //ʵ�ʳɹ���ȡ���� ip ���� 
            break;
        }
    }
    
    if (ip_num_ == 0) //һ��ip��û�� 
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrAsy init failed"
            ",section=%s,ip_num=%d\n", section_, ip_num_));
        ip_num_ = IP_NUM_DEF; //���ΪĬ��ֵ 
        return -1;
    }
    
    if (SysConf::instance()->get_conf_int(section_, "port", &port_) != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrAsy get port config failed"
            ",section=%s\n", section_)); 
        return -1;
    }
    
    ACE_DEBUG((LM_INFO, "[%D] PlatConnMgrAsy start to init conn"
        ",ip_num=%d"
        "\n"
        , ip_num_
        )); 
    for(int i=0; i<ip_num_; i++)
    {
        if (init_conn(i) !=0)
        {
            //ĳ�����Ӳ��� ���˳����� 
            //return -1;
        }
    }

    //��ʼ��һ�� stat ��֤��ʹ�ÿ�ܵ�ģ��ʹ��Ҳ����
    Stat::instance()->init("");
    return 0;
}

// ip port ��ָ��ʱ ��ʹ���ڲ��� ��ָ��ʱ���滻�ڲ��� 
int PlatConnMgrAsy::init_conn(int ip_idx, const char *ip, int port)
{
    if (ip_idx<0 || ip_idx >=IP_NUM_MAX)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrAsy init conn failed,para is invalid"
            ",ip_idx=%d\n", ip_idx));
        Stat::instance()->incre_stat(STAT_CODE_CONN_FAIL);
        return -1;
    }
    
    // ��֤���ݳ�ʼ����һ���� 
    ACE_Guard<ACE_Thread_Mutex> guard(conn_lock_[ip_idx]);

    //������Ҫ���ж�һ��,����Ѿ��������������򷵻�:
    if (0 == is_estab(ip_idx))
    {
        ACE_DEBUG((LM_ERROR,
            "[%D] PlatConnMgrAsy send msg failed"
            ",intf is disconn"
            ",ip_idx=%d\n"
            , ip_idx
            ));
        return 0;
    }
    
    if (ip != NULL)
    {
        strncpy(ip_[ip_idx], ip, sizeof(ip_[ip_idx]));
    }
    if (port != 0)
    {
        port_ = port;
    }
	
    ACE_DEBUG((LM_INFO, "[%D] PlatConnMgrAsy start to init conn"
        ",ip=%s"
        ",port=%d"
        ",ip_idx=%d"
        "\n"
        , ip_[ip_idx]
        , port_
        , ip_idx
        ));

    // û�����õ� ip �Ͷ˿� 
    if (strlen(ip_[ip_idx]) == 0 || port_ == 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrAsy init conn failed"
            ",ip=%s,port=%d,ip_idx=%d\n"
            , ip_[ip_idx], port_, ip_idx));
        Stat::instance()->incre_stat(STAT_CODE_CONN_FAIL);
        return -2; //���ò����� 
    }
    
    ACE_INET_Addr svr_addr(port_, ip_[ip_idx]);
    ISGW_CONNECTOR connector;
    conn_info_[ip_idx].intf = new ISGWCIntf();
    if (connector.connect(conn_info_[ip_idx].intf, svr_addr) != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrAsy init conn failed"
            ",ip=%s,port=%d,ip_idx=%d\n"
            , ip_[ip_idx], port_, ip_idx
            ));
        fini_conn(ip_idx);
        Stat::instance()->incre_stat(STAT_CODE_CONN_FAIL);
        return -1;
    }
    conn_info_[ip_idx].sock_fd = conn_info_[ip_idx].intf->get_handle();
    conn_info_[ip_idx].sock_seq = conn_info_[ip_idx].intf->get_seq();
    
    ACE_DEBUG((LM_INFO, "[%D] PlatConnMgrAsy init conn succ"
        ",ip=%s,port=%d,ip_idx=%d,sock_fd=%d,sock_seq=%d\n"
        , ip_[ip_idx]
        , port_
        , ip_idx
        , conn_info_[ip_idx].sock_fd
        , conn_info_[ip_idx].sock_seq
        ));
    return 0;
}

int PlatConnMgrAsy::send(const void * buf, int len, const unsigned int uin)
{
    int ip_idx = get_ip_idx(uin);
    if (is_estab(ip_idx) != 0)
    {
        ACE_DEBUG((LM_ERROR,
            "[%D] PlatConnMgrAsy send msg failed"
            ",intf is disconn"
            ",ip_idx=%d\n"
            , ip_idx
            ));
        if (init_conn(ip_idx) !=0)
        {
            return -1;
        }
    }
    
    PriProAck *ack = NULL; // ����ʵ�������̨��������Ϣ req 
    ACE_Object_Que<PriProAck>::instance()->dequeue(ack);
    if ( ack == NULL )
    {
        ACE_DEBUG((LM_ERROR,
            "[%D] PlatConnMgrAsy dequeue msg failed"
            ",maybe system has no memory\n"
            ));
        return -1;
    }
    memset(ack, 0x0, sizeof(PriProAck));
    
    ack->sock_fd = conn_info_[ip_idx].sock_fd;
    ack->protocol = PROT_TCP_IP;
    ack->sock_seq = conn_info_[ip_idx].sock_seq;
    gettimeofday(&(ack->tv_time), NULL);
    ack->time = ISGWAck::instance()->get_time();
    snprintf(ack->msg, sizeof(ack->msg), "%s", buf);
    ACE_DEBUG((LM_NOTICE, "[%D] PlatConnMgrAsy send msg to ISGWAck"
        ",sock_fd=%u,protocol=%u,ip=%u,port=%u,sock_seq=%u,seq_no=%u,time=%u"
        ",msg=%s\n"
        , ack->sock_fd
        , ack->protocol
        , ack->sock_ip
        , ack->sock_port
        , ack->sock_seq
        , ack->seq_no
        , ack->time
        , ack->msg
        ));
    
    //���������Ϣ������ �������Ϣ�͵����ӶԶ� (����ָ����˵�����)
    ISGWAck::instance()->putq(ack);
    return 0;
}

//#ifdef ISGW_USE_ASY
int PlatConnMgrAsy::send(const void * buf, int len, ASYRMsg & rmsg, const unsigned int uin)
{
	ASYTask::instance()->insert(rmsg);
	return send(buf, len, uin);
}
//#endif

int PlatConnMgrAsy::get_ip_idx(const unsigned int uin)
{
    if (uin == 0)
    {
        return time(0)%ip_num_;
    }
    
    return uin%ip_num_;
}

int PlatConnMgrAsy::is_estab(int ip_idx)
{
    ACE_Event_Handler* eh = ACE_Reactor::instance()
            ->find_handler(conn_info_[ip_idx].sock_fd);
    AceSockHdrBase* intf = dynamic_cast<AceSockHdrBase*>(eh); //
    if (intf == NULL || intf->get_seq() != conn_info_[ip_idx].sock_seq)
    {
        ACE_DEBUG((LM_ERROR,
            "[%D] PlatConnMgrAsy find intf failed"
            ",sock_fd=%u,sock_seq=%u\n"
            , conn_info_[ip_idx].sock_fd
            , conn_info_[ip_idx].sock_seq
            ));
        return -1;
    }
    return 0;
}

int PlatConnMgrAsy::fini_conn(int ip_idx)
{
    /*
    // ACE_Connector �Ḻ�� ���� intf ���ڴ�ռ� �˴�ֻ��Ҫ����Ϊ NULL ����
    // ����һ������ ���� ACE_Connector ����û����ɾ� 
    if (conn_info_[ip_idx].intf != NULL)
    {
        delete conn_info_[ip_idx].intf;
        conn_info_[ip_idx].intf = NULL;
    }
    */
    
    conn_info_[ip_idx].sock_fd = 0;
    conn_info_[ip_idx].sock_seq = 0;
    return 0;
}

