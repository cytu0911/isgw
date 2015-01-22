#include "plat_conn_mgr_ex.h"
#include "stat.h"
#include <fstream>

PlatConnMgrEx::PlatConnMgrEx()
{
    for(int i=0; i<IP_NUM_MAX; i++)
    {
        for(int j=0; j<POOL_CONN_MAX; j++)
        {
            conn_[i][j] = NULL;
    		conn_use_flag_[i][j] = 0;
        }
        memset(ip_[i], 0x0, sizeof(ip_[i]));
        fail_times_[i] = 0;
        last_fail_time_[i] = 0;
    }

    ip_num_ = IP_NUM_DEF;
    conn_nums_ = POOL_CONN_DEF;
    
    time_out_.set(SOCKET_TIME_OUT/1000, (SOCKET_TIME_OUT%1000)*1000);
    memset(section_, 0x0, sizeof(section_));	
    port_ = 0;

    use_strategy_ = DEF_USE_STRATEGY; //Ĭ��ʹ��
    max_fail_times_ = DEF_MAX_FAIL_TIMES; //Ĭ���������ʧ����ȴ�һ��ʱ������
	recon_interval_ = DEF_RECON_INTERVAL; //Ĭ��10s�������� 
	
}

PlatConnMgrEx::PlatConnMgrEx(const char*host_ip, int port, int conn_num)
{
    for(int i=0; i<IP_NUM_MAX; i++)
    {
        for(int j=0; j<POOL_CONN_MAX; j++)
        {
            conn_[i][j] = NULL;
            conn_use_flag_[i][j] = 0;            
        }
        memset(ip_[i], 0x0, sizeof(ip_[i]));
        fail_times_[i] = 0;
        last_fail_time_[i] = 0;
    }

    ip_num_ = 1;
    snprintf(ip_[0], sizeof(ip_[0]), "%s", host_ip);
    port_ = port;
    conn_nums_ = conn_num;
    time_out_.set(SOCKET_TIME_OUT/1000, (SOCKET_TIME_OUT%1000)*1000);

    use_strategy_ = DEF_USE_STRATEGY; //Ĭ��ʹ��
    max_fail_times_ = DEF_MAX_FAIL_TIMES; //Ĭ���������ʧ����ȴ�һ��ʱ������
	recon_interval_ = DEF_RECON_INTERVAL; //Ĭ��10s�������� 
}

PlatConnMgrEx::~PlatConnMgrEx()
{
    for(int i=0; i<IP_NUM_MAX; i++)
    {
        for(int j=0; j<POOL_CONN_MAX; j++)
        {
            if (conn_[i][j] != NULL)
            {
                conn_[i][j]->close();
                delete conn_[i][j];
                conn_[i][j] = NULL;
    			conn_use_flag_[i][j] = 0;
            }
        }
    }
    
}

int PlatConnMgrEx::init(const char *section, const std::vector<std::string> *ip_vec)
{
    ACE_Guard<ACE_Thread_Mutex> guard(conn_mgr_lock_); 
    if (section != NULL)
    {
        strncpy(section_, section, sizeof(section_));
    }

    // Ĭ��ʹ���û�ָ����ip
    if(ip_vec != NULL && !ip_vec->empty())
    {
        ip_num_ = static_cast<int>(ip_vec->size());
        if(ip_num_ > IP_NUM_MAX) ip_num_ = IP_NUM_MAX;
        for(int i = 0;i < ip_num_;i++)
        {
            snprintf(ip_[i], sizeof(ip_[i]), "%s", (*ip_vec)[i].c_str());
        }
    }
    else /* δָ��ip��ʹ�����õ�ip */
    {
        if ((SysConf::instance()->get_conf_int(section_, "ip_num", &ip_num_) != 0)
    		|| (ip_num_ > IP_NUM_MAX))
    	{
    		ip_num_ = IP_NUM_DEF;
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
    			ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrEx get config failed"
    				",section=%s,ip_%d\n", section_, i));
    			ip_num_ = i; //ʵ�ʳɹ���ȡ���� ip ���� 
    			break;
    		}
    	}
    }
    
	if (ip_num_ == 0) //һ��ip��û�� 
	{
		ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrEx init failed"
			",section=%s,ip_num=%d\n", section_, ip_num_));
		ip_num_ = IP_NUM_DEF; //���ΪĬ��ֵ 
		return -1;
	}
	
	if (SysConf::instance()->get_conf_int(section_, "conn_nums", &conn_nums_) != 0)
	{
		conn_nums_ = POOL_CONN_DEF;
	}
	else if(conn_nums_ > POOL_CONN_MAX)
	{
		conn_nums_ = POOL_CONN_MAX;
	}
	
	int time_out = SOCKET_TIME_OUT;
	if (SysConf::instance()->get_conf_int(section_, "time_out", &time_out) == 0)
	{
		time_out_.set(time_out/1000, (time_out%1000)*1000);
	}

	if (SysConf::instance()->get_conf_int(section_, "port", &port_) != 0)
	{
		ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrEx get port config failed"
			",section=%s\n", section_)); 
		return -1;
	}

	if (SysConf::instance()->get_conf_int(section_, "use_strategy", &use_strategy_) == 0)
	{
		//ʹ�����Ӳ���
		ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrEx get use_strategy config succ,use strategy"
			",section=%s\n", section_));
		SysConf::instance()->get_conf_int(section_, "max_fail_times", &max_fail_times_);
		SysConf::instance()->get_conf_int(section_, "recon_interval", &recon_interval_);        
	}

    ACE_DEBUG((LM_INFO, "[%D] PlatConnMgrEx start to init conn"
        ",ip_num=%d"
        ",conn_nums=%d"
        "\n"
        , ip_num_
        , conn_nums_
        )); 
    for(int i=0; i<ip_num_; i++)
    {
        for(int j=0; j<conn_nums_; j++)
        {
            if (init_conn(j, i) !=0)
            {
                //ĳ�����Ӳ��� ���˳����� 
                //return -1;
            }
        }
    }
    
    //��ʼ��һ�� stat ��֤��ʹ�ÿ�ܵ�ģ��ʹ��Ҳ����
    Stat::instance()->init();
    return 0;
}

// ip port ��ָ��ʱ ��ʹ���ڲ��� ��ָ��ʱ���滻�ڲ��� 
int PlatConnMgrEx::init_conn(int index, int ip_idx, const char *ip, int port)
{
    if (index<0 || index>=POOL_CONN_MAX || ip_idx<0 || ip_idx >=IP_NUM_MAX)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrEx init conn failed,para is invalid"
            ",index=%d,ip_idx=%d\n", index, ip_idx));
        Stat::instance()->incre_stat(STAT_CODE_CONN_FAIL);
        return -1;
    }
    
    if (ip != NULL)
    {
        strncpy(ip_[ip_idx], ip, sizeof(ip_[ip_idx]));
    }
    if (port != 0)
    {
        port_ = port;
    }
	
    if (conn_[ip_idx][index] != NULL)
    {
        fini(index, ip_idx);
    }

    ACE_DEBUG((LM_INFO, "[%D] PlatConnMgrEx init conn"
        ",ip=%s"
        ",port=%d"
        ",index=%d"
        ",ip_idx=%d"
        "\n"
        , ip_[ip_idx]
        , port_
        , index
        , ip_idx
        ));

    // û�����õ� ip �Ͷ˿� 
    if (strlen(ip_[ip_idx]) == 0 || port_ == 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrEx init conn failed,invalid para"
            ",ip=%s,port=%d,index=%d,ip_idx=%d\n"
            , ip_[ip_idx], port_, index, ip_idx));
        Stat::instance()->incre_stat(STAT_CODE_CONN_FAIL);
        return -2; //���ò����� 
    }

    //�������Ӳ��ϣ����ӱ��� 
    ACE_INET_Addr svr_addr(port_, ip_[ip_idx]);
    ACE_SOCK_Connector connector;
    conn_[ip_idx][index] = new ACE_SOCK_Stream();    
    if (connector.connect(*conn_[ip_idx][index], svr_addr, &time_out_) != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrEx init conn failed,conn failed"
            ",ip=%s,port=%d,index=%d,ip_idx=%d\n"
            , ip_[ip_idx], port_, index, ip_idx));
        fini(index, ip_idx);
        Stat::instance()->incre_stat(STAT_CODE_CONN_FAIL);
        return -1;
    }
    
    ACE_DEBUG((LM_INFO, "[%D] PlatConnMgrEx init conn succ"
        ",ip=%s"
        ",port=%d"
        ",index=%d"
        ",ip_idx=%d"
        ",lock=0x%x"
        "\n"
        , ip_[ip_idx]
        , port_
        , index
        , ip_idx
        , &(conn_lock_[ip_idx][index].lock())
        ));
    // ���ӳɹ�ʵ������ʧ�ܴ���Ҫ�� 0  ʹ��״̬Ҫ�� 0 
    fail_times_[ip_idx] = 0;
    conn_use_flag_[ip_idx][index] = 0;
    return 0;
}

int PlatConnMgrEx::send(const void * buf, int len, const unsigned int uin)
{
    int ip_idx = get_ip_idx(uin);
    int index = get_index(ip_idx, uin);
    ACE_SOCK_Stream* conn = get_conn(index, ip_idx);
    if (conn == NULL)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrEx get conn failed"
            ",can't get a useful conn"
            ",index=%d,ip_idx=%d,uin=%u\n"
            , index, ip_idx, uin
            ));
        return -1;
    }
    ACE_Guard<ACE_Thread_Mutex> guard(conn_lock_[ip_idx][index]);// ������Ҫ���ڻ�ȡ�ĺ��� 
    if (conn_[ip_idx][index] == NULL) // ����������һ�ηǷ��ж� 
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrEx send failed"
            ",conn is null"
            ",index=%d,ip_idx=%d,uin=%u\n"
            , index, ip_idx, uin
            ));
        return -1;
    }
    conn_use_flag_[ip_idx][index] = 1;

    ACE_DEBUG((LM_TRACE, "[%D] PlatConnMgrEx start to send msg"
        ",index=%d,ip_idx=%d,uin=%u\n", index, ip_idx, uin));
    int ret = conn_[ip_idx][index]->send(buf, len, &time_out_);
    if (ret <= 0) //�쳣���߶Զ˹ر�
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrEx send msg failed"
            ",index=%d,ip_idx=%d,ret=%d,uin=%u,errno=%d\n", index, ip_idx, ret, uin, errno));
        //�ر�����
        fini(index, ip_idx);
        Stat::instance()->incre_stat(STAT_CODE_SEND_FAIL);
        return ret;
    }
    ACE_DEBUG((LM_TRACE, "[%D] PlatConnMgrEx send msg succ"
        ",index=%d,ip_idx=%d,ret=%d,uin=%u\n", index, ip_idx, ret, uin));

    conn_use_flag_[ip_idx][index] = 0;//�˳�ǰ����ʹ��״̬
    return ret;
}

int PlatConnMgrEx::send(const void * buf, int len, const std::string& ip)
{
    int ip_idx = get_ip_idx(ip);
    int index = get_index(ip_idx);
    ACE_SOCK_Stream* conn = get_conn(index, ip_idx);
    if (conn == NULL)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrEx get conn failed"
            ",can't get a useful conn"
            ",index=%d,ip_idx=%d,ip=%s\n"
            , index, ip_idx, ip.c_str()
            ));
        return -1;
    }
    ACE_Guard<ACE_Thread_Mutex> guard(conn_lock_[ip_idx][index]);// ������Ҫ���ڻ�ȡ�ĺ��� 
    if (conn_[ip_idx][index] == NULL) // ����������һ�ηǷ��ж� 
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrEx send failed"
            ",conn is null"
            ",index=%d,ip_idx=%d,ip=%s\n"
            , index, ip_idx, ip.c_str()
            ));
        return -1;
    }
    conn_use_flag_[ip_idx][index] = 1;

    ACE_DEBUG((LM_TRACE, "[%D] PlatConnMgrEx start to send msg"
        ",index=%d,ip_idx=%d,ip=%s\n", index, ip_idx, ip.c_str()));
    int ret = conn_[ip_idx][index]->send(buf, len, &time_out_);
    if (ret <= 0) //�쳣���߶Զ˹ر�
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrEx send msg failed"
            ",index=%d,ip_idx=%d,ret=%d,ip=%s,errno=%d\n", index, ip_idx, ret, ip.c_str(), errno));
        //�ر�����
        fini(index, ip_idx);
        Stat::instance()->incre_stat(STAT_CODE_SEND_FAIL);
        return ret;
    }
    ACE_DEBUG((LM_TRACE, "[%D] PlatConnMgrEx send msg succ"
        ",index=%d,ip_idx=%d,ret=%d,ip=%s\n", index, ip_idx, ret, ip.c_str()));

    conn_use_flag_[ip_idx][index] = 0;//�˳�ǰ����ʹ��״̬
    return ret;
}


int PlatConnMgrEx::send_recv(const void * send_buf, int send_len
    , void * recv_buf, int recv_len, const unsigned int uin)
{
    int ip_idx = get_ip_idx(uin);
    int index = get_index(ip_idx, uin);
    ACE_SOCK_Stream* conn = get_conn(index, ip_idx);
    if (conn == NULL)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrEx get conn failed"
            ",can't get a useful conn"
            ",index=%d,ip_idx=%d,uin=%u\n"
            , index, ip_idx, uin
            ));
        return -1;
    }
    ACE_Guard<ACE_Thread_Mutex> guard(conn_lock_[ip_idx][index]);// ������Ҫ���ڻ�ȡ�ĺ��� 
    if (conn_[ip_idx][index] == NULL) // ����������һ�ηǷ��ж� 
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrEx send_recv failed"
            ",conn is null"
            ",index=%d,ip_idx=%d,uin=%u\n"
            , index, ip_idx, uin
            ));
        return -1;
    }
    conn_use_flag_[ip_idx][index] = 1;
    
    //���ԭ���� add by awayfang 2010-03-02 
    ACE_Time_Value zero;
    zero.set(0);
    int max_recv_len = recv_len;
    int tmp_ret = conn_[ip_idx][index]->recv(recv_buf, max_recv_len, &zero);//���ȴ�ֱ�ӷ�����
    // ��⵽���ӹر�ʱ, �ؽ�����
    if((tmp_ret < 0 && errno != ETIME)   // �����������ݵ����,�᷵�س�ʱ,�������� 
        || tmp_ret == 0)                 // �����Ѿ����Զ˹ر�, ����close wait״̬
    {
        ACE_DEBUG((LM_INFO, "[%D] PlatConnMgrEx send_recv connection close detected,"
            "uin=%u,ip=%s,index=%d\n", uin, ip_[ip_idx], index));
        init_conn(index, ip_idx);
	 if(conn_[ip_idx][index] == NULL)
        {
            ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrEx send_recv reconnect failed,"
                "index=%d,ip_idx=%d\n", index, ip_idx));
            return -1;
        }
    }
    
    ACE_DEBUG((LM_TRACE, "[%D] PlatConnMgrEx send_recv msg"
        ",index=%d,ip_idx=%d,uin=%u\n", index, ip_idx, uin));
    int ret = conn_[ip_idx][index]->send(send_buf, send_len, &time_out_);
    if (ret <= 0) //�쳣���߶Զ˹ر�
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrEx send_recv send msg failed"
            ",index=%d,ip_idx=%d,ret=%d,uin=%u,errno=%d\n", index, ip_idx, ret, uin, errno));
        //�ر�����
        fini(index, ip_idx);
        Stat::instance()->incre_stat(STAT_CODE_SEND_FAIL);
        return ret;
    }
	// 
    ret = conn_[ip_idx][index]->recv(recv_buf, recv_len, &time_out_);
    if (ret <= 0) //�쳣���߶Զ˹ر�
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrEx send_recv recv msg failed"
            ",index=%d,ip_idx=%d,ret=%d,uin=%u,errno=%d\n", index, ip_idx, ret, uin, errno));
        //�ر�����
        fini(index, ip_idx);
        Stat::instance()->incre_stat(STAT_CODE_RECV_FAIL);
        return ret;
    }

    ACE_DEBUG((LM_TRACE, "[%D] PlatConnMgrEx send_recv msg succ"
        ",index=%d,ip_idx=%d,ret=%d,uin=%u\n", index, ip_idx, ret, uin));
        
    conn_use_flag_[ip_idx][index] = 0;//�˳�ǰ����ʹ��״̬
    return ret;
    
}

// ֧��ָ�����յĽ��������������� add by awayfang 2010-03-02 
int PlatConnMgrEx::send_recv_ex(const void * send_buf, int send_len
    , void * recv_buf, int recv_len, const char * separator, const unsigned int uin)
{
    int ip_idx = get_ip_idx(uin); // ip Ĭ�ϸ��� uin ��ѯ 
    int index = get_index(ip_idx, uin); 
    ACE_SOCK_Stream* conn = get_conn(index, ip_idx);
    if (conn == NULL)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrEx get conn failed"
            ",can't get a useful conn"
            ",index=%d,ip_idx=%d,uin=%u\n"
            , index, ip_idx, uin
            ));
        return -1;
    }
    
    // ��Ϊ ip_idx ���ܱ��޸ģ�������Ҫ���ڻ�ȡ�ĺ��� 
    ACE_Guard<ACE_Thread_Mutex> guard(conn_lock_[ip_idx][index]); 
    // ����������һ�ηǷ��ж� ��Ȼ�п��ܱ������߳��ͷ����Ӷ���
    if (conn_[ip_idx][index] == NULL) 
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrEx send_recv_ex failed"
            ",conn is null"
            ",index=%d,ip_idx=%d,uin=%u\n"
            , index, ip_idx, uin
            ));
        return -1;
    }
    conn_use_flag_[ip_idx][index] = 1;

    //���ԭ���� add by awayfang 2010-03-02 
    ACE_Time_Value zero;
    zero.set(0);
    int max_recv_len = recv_len;
    int tmp_ret = conn_[ip_idx][index]->recv(recv_buf, max_recv_len, &zero);//������ֱ�ӷ�����
    // ��⵽���ӹر�ʱ, �ؽ�����
    // �ڶ����������ʱ, �п��ܷ���
    if((tmp_ret < 0 && errno != ETIME)   // �����������ݵ����,�᷵�س�ʱ,�������� 
        || tmp_ret == 0)                 // �����Ѿ����Զ˹ر�, ����close wait״̬
    {
        ACE_DEBUG((LM_INFO, "[%D] PlatConnMgrEx send_recv_ex connection close detected,"
            "uin=%u,ip=%s,index=%d\n", uin, ip_[ip_idx], index));
        init_conn(index, ip_idx);
    	if(conn_[ip_idx][index] == NULL)
    	{
    		ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrEx send_recv_ex reconnect failed,"
                "index=%d,ip_idx=%d\n", index, ip_idx));
    		return -1;
    	}
    }

    ACE_DEBUG((LM_TRACE, "[%D] PlatConnMgrEx send_recv_ex msg"
        ",index=%d,ip_idx=%d,uin=%u\n", index, ip_idx, uin));
    int ret = conn_[ip_idx][index]->send_n(send_buf, send_len, &time_out_);
    if (ret <= 0) //�쳣���߶Զ˹ر�
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrEx send_recv_ex send msg failed"
            ",index=%d,ip_idx=%d,ret=%d,uin=%u,errno=%d\n", index, ip_idx, ret, uin, errno));
        //�ر����ӣ�����״̬
        fini(index, ip_idx);
        Stat::instance()->incre_stat(STAT_CODE_SEND_FAIL);
        return ret;
    }
    ACE_DEBUG((LM_TRACE, "[%D] PlatConnMgrEx send_recv_ex send msg succ"
        ",index=%d,ip_idx=%d,ret=%d,uin=%u\n", index, ip_idx, ret, uin));
	// 
    ret = conn_[ip_idx][index]->recv(recv_buf, recv_len, &time_out_);
    if (ret <= 0) //�쳣���߶Զ˹ر�
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrEx send_recv_ex recv msg failed"
            ",index=%d,ip_idx=%d,ret=%d,uin=%u,errno=%d\n", index, ip_idx, ret, uin, errno));
        //�ر����ӣ�����״̬
        fini(index, ip_idx);
        Stat::instance()->incre_stat(STAT_CODE_RECV_FAIL);
        return ret;
    }

    // �жϽ����� 
    if (separator != NULL)
    {
        int tmp_recv_len = ret;
        //�ж���Ϣ�Ƿ����
        while (strstr((const char*)recv_buf, separator) == NULL 
            && tmp_recv_len < max_recv_len) //δ��������������
        {
            ret = conn_[ip_idx][index]->recv((char*)recv_buf + tmp_recv_len
                , max_recv_len - tmp_recv_len, &time_out_);
            if (ret <= 0) //�쳣���߶Զ˹ر�
            {
                ACE_DEBUG((LM_ERROR, "[%D] [%N,%l]PlatConnMgrEx send_recv_ex"
                    " recv msg failed"
                    ",index=%d,ip_idx=%d,ret=%d,uin=%u,errno=%d\n", index, ip_idx, ret, uin, errno));
                //�ر����ӣ�����״̬
                fini(index, ip_idx);
                Stat::instance()->incre_stat(STAT_CODE_RECV_FAIL);
                return ret;
            }
            tmp_recv_len += ret;
            ACE_DEBUG((LM_TRACE, "[%D] PlatConnMgrEx send_recv_ex msg"
                ",index=%d,ip_idx=%d,ret=%d,uin=%u\n", index, ip_idx, ret, uin));
        }
        ret = tmp_recv_len;
    }

    ACE_DEBUG((LM_TRACE, "[%D] PlatConnMgrEx send_recv_ex msg succ"
        ",index=%d,ip_idx=%d,ret=%d,uin=%u\n", index, ip_idx, ret, uin));
    
    conn_use_flag_[ip_idx][index] = 0;//�˳�ǰ����ʹ��״̬
    return ret;
}

int PlatConnMgrEx::recv(void * buf, int len, const unsigned int uin)
{
    int ip_idx = get_ip_idx(uin); 
    int index = get_index(ip_idx); 
    ACE_SOCK_Stream* conn = get_conn(index, ip_idx);
    if (conn == NULL)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrEx get conn failed"
            ",can't get a useful conn"
            ",index=%d,ip_idx=%d,uin=%u\n"
            , index, ip_idx, uin
            ));
        return -1;
    }
    ACE_Guard<ACE_Thread_Mutex> guard(conn_lock_[ip_idx][index]);// ������Ҫ���ڻ�ȡ�ĺ��� 
    if (conn_[ip_idx][index] == NULL) // ����������һ�ηǷ��ж� 
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrEx recv failed"
            ",conn is null"
            ",index=%d,ip_idx=%d,uin=%u\n"
            , index, ip_idx, uin
            ));
        return -1;
    }
    conn_use_flag_[ip_idx][index] = 1;
    
    ACE_DEBUG((LM_TRACE, "[%D] PlatConnMgrEx start to recv msg"
        ",index=%d,ip_idx=%d,uin=%u\n", index, ip_idx, uin));
    int ret = conn_[ip_idx][index]->recv(buf, len, &time_out_);
    if (ret <= 0) //�쳣���߶Զ˹ر�
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrEx recv msg failed"
            ",index=%d,ip_idx=%d,ret=%d,uin=%u,errno=%d\n", index, ip_idx, ret, uin, errno));
        //�ر�����
        fini(index, ip_idx);
        Stat::instance()->incre_stat(STAT_CODE_RECV_FAIL);
        return ret;
    }
    ACE_DEBUG((LM_TRACE, "[%D] PlatConnMgrEx recv msg succ"
        ",index=%d,ip_idx=%d,ret=%d,uin=%u\n", index, ip_idx, ret, uin));

    conn_use_flag_[ip_idx][index] = 0;//�˳�ǰ����ʹ��״̬
    return ret;
}

ACE_SOCK_Stream* PlatConnMgrEx::get_conn(int index, int & ip_idx) //�����ӳػ�ȡһ������
{    
    ACE_DEBUG((LM_TRACE, "[%D] PlatConnMgrEx start to get conn"
        ",index=%d,ip_idx=%d\n", index, ip_idx));
	if (index<0 || index>=POOL_CONN_MAX || ip_idx<0 || ip_idx >=IP_NUM_MAX)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrEx get conn failed, para is invalid"
            ",index=%d,ip_idx=%d\n", index, ip_idx));
        return NULL;
    }
    
    // ���Ӳ��� 
    if (use_strategy_ == 1 //ʹ�ò���
        && fail_times_[ip_idx] > max_fail_times_  //ʧ�ܴ��������������ʧ�ܴ���
        && (time(0) - last_fail_time_[ip_idx]) < recon_interval_ //ʱ����С������ʱ���� 
    )
    {
        ACE_DEBUG((LM_TRACE, "[%D] PlatConnMgrEx get conn failed,because of strategy"
            ",index=%d"
            ",ip_idx=%d"
            ",fail_times=%d"
            ",last_fail_time=%d"
			"\n"	
            , index
            , ip_idx
            , fail_times_[ip_idx]
            , last_fail_time_[ip_idx]
            ));
        // ������һ�� ip 
        ip_idx = (++ip_idx)%ip_num_;
        
        // ������һ����ʱ�����κ��������� ֱ��ʹ�� ������һ��Ҳ�쳣 ����Ƶ�����������
        return conn_[ip_idx][index]; // NULL; //
    }
    
    if (conn_[ip_idx][index] == NULL)
    {
        // ��֤ÿ��ֻ��һ���߳��ڳ�ʼ��
        ACE_Guard<ACE_Thread_Mutex> guard(conn_lock_[ip_idx][index]);
        if (conn_[ip_idx][index] == NULL)
        {
            init_conn(index, ip_idx);
        }
    }
    
    return conn_[ip_idx][index];
}

//ϵͳ��������֮����ô˽ӿ�˵������������
int PlatConnMgrEx::fini(int index, int ip_idx)
{    
    if (index<0 || index>=POOL_CONN_MAX || ip_idx<0 || ip_idx >=IP_NUM_MAX)
    {
        return -1;
    }
    
    // ���Ӳ��� 
    fail_times_[ip_idx]++;
	last_fail_time_[ip_idx] = time(0);
    
    if (conn_[ip_idx][index] != NULL)
    {
        conn_[ip_idx][index]->close();
        delete conn_[ip_idx][index];
        conn_[ip_idx][index] = NULL;
        conn_use_flag_[ip_idx][index] = 0;
    }
    
    ACE_DEBUG((LM_INFO, "[%D] PlatConnMgrEx fini conn succ"
		",ip=%s"
		",port=%d"
		",index=%d"
		",ip_idx=%d"
		",fail_times=%d"
		"\n"
		, ip_[ip_idx]
		, port_
		, index
		, ip_idx
		, fail_times_[ip_idx]
		));

    return 0;
}

// �������� index ���㷨����ԱȽϴ��Եģ�������ô�ϸ�
// ��������ʹ�õ�ʱ���ͨ���������ƣ�һ������ͬʱֻ��һ���߳�ʹ��
unsigned int PlatConnMgrEx::get_index(int ip_idx, unsigned uin)
{
    ip_idx = ip_idx % ip_num_;

#ifdef THREAD_BIND_CONN
    // ������� getpid() suse�±����ȡ����pidΪͬһ�� 
    // ACE_OS::thr_self()
    return syscall(SYS_gettid)%conn_nums_;//���㷨�������̺߳ŵ�˳��������ܾ��ȷ�������         
#else
    // ����������Ƿ��пյ�    
    int index = 0;
    if(0==uin) index = rand()%conn_nums_;
    else index= (uin/100)%conn_nums_;
    
    if(0 == conn_use_flag_[ip_idx][index])
    {
        return index;
    }
    
    //�����ҵ�һ��û��ʹ�õ����Ӿͷ���:
    int i = 0;
    for(i=0; i<conn_nums_; i++)
    {
        if(0 == conn_use_flag_[ip_idx][i])
        {
            index = i;
            break;
        }
    }
    //���û�ҵ�������Ȼ���ظղ�����ģ���Ϊ����ʹ��ʱ����������������̫��Ӱ��
    if(i == conn_nums_)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatConnMgrEx get_index conn runout"
    		",ip=%s"
    		",port=%d"
    		",index=%d"
    		"\n"
    		, ip_[ip_idx]
    		, port_
    		, index
    		));
        Stat::instance()->incre_stat(STAT_CODE_TCP_CONN_RUNOUT);
    }
    return index;

#endif
}	

// ������֤·��������� ��Ȼ�����׵��¼��з��ʵ�ĳ�� ip ���� 
int PlatConnMgrEx::get_ip_idx(unsigned int uin)
{
    if (uin == 0)
    {
        return rand()%ip_num_;
    }
    
    return uin%ip_num_;
}

int PlatConnMgrEx::get_ip_idx(const std::string& ip)
{
    for(int i = 0; i < ip_num_; ++i)
    {
        if(ip.compare(ip_[i]) == 0)
        {
            return i;
        }
    }

    return rand()%ip_num_;
}

