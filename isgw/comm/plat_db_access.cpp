#include "plat_db_access.h"
#include "stat.h"

PlatDbAccess::PlatDbAccess()
{
    for(int i=0; i<DB_CONN_MAX; i++)
    {
        db_conn_[i] = NULL;
        db_conn_flag_[i] = 0; //Ĭ��Ϊ����״̬
    }
    
    conn_nums_ = POOL_CONN_DEF;
    memset(section_, 0x0, sizeof(section_));

    memset(db_host_, 0x0, sizeof(db_host_));
    memset(db_user_, 0x0, sizeof(db_user_));
    memset(db_pswd_, 0x0, sizeof(db_pswd_));
    port_ = 0;
    time_out_ = 2;

    use_strategy_ = 1; //Ĭ��ʹ��
    max_fail_times_ = 5; //Ĭ���������ʧ����ȴ�һ��ʱ������
    recon_interval_ = 10; //Ĭ��10s�������� 

    fail_times_ = 0;
    last_fail_time_ = 0;

    memset(err_msg_, 0x0, sizeof(err_msg_));	
}

PlatDbAccess::~PlatDbAccess()
{
    for(int i=0; i<DB_CONN_MAX; i++)
    {
        if (db_conn_[i] != NULL)
        {
            delete db_conn_[i];
            db_conn_[i] = NULL;
            db_conn_flag_[i] = 0;
        }
    }
}

int PlatDbAccess::init(const char *section )
{
    if (section != NULL)
    {
        strncpy(section_, section, sizeof(section_));
    }
	
    if (SysConf::instance()->get_conf_int(section_, "conn_nums", &conn_nums_) != 0)
    {
        conn_nums_ = POOL_CONN_DEF;
        ACE_DEBUG((LM_ERROR, "[%D] PlatDbAccess get conn_nums config failed"
            ",section=%s"
            ",use default conn_nums %d"
            "\n"
            , section_
            , conn_nums_
            ));
    }
    //���õ��ٴ�Ҳ���ܳ�������ڲ����Ƶ�������
    if (conn_nums_ > DB_CONN_MAX)
    {
        conn_nums_ = DB_CONN_MAX;
    }
    
    if (SysConf::instance()
        ->get_conf_str( section_, "ip", db_host_, sizeof(db_host_)) != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatDbAccess get ip config failed"
            ",section=%s\n", section_)); 
        return -1;
    }
    if (SysConf::instance()
        ->get_conf_str( section_, "user", db_user_, sizeof(db_user_)) != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatDbAccess get user config failed"
            ",section=%s\n", section_)); 
        return -1;
    }
    if (SysConf::instance()
        ->get_conf_str( section_, "passwd", db_pswd_, sizeof(db_pswd_)) != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatDbAccess get passwd config failed"
            ",section=%s\n", section_)); 
        return -1;
    }
	if (SysConf::instance()->get_conf_int(section_, "port", &port_) != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatDbAccess get port config failed"
            ",section=%s\n", section_)); 
    }
    
    if (SysConf::instance()->get_conf_int(section_, "timeout", &time_out_) != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatDbAccess get timeout config failed"
            ",section=%s\n", section_)); 
    }
    
    if (SysConf::instance()->get_conf_int(section_, "use_strategy", &use_strategy_) == 0)
    {
        //ʹ�����Ӳ���
        ACE_DEBUG((LM_ERROR, "[%D] PlatDbAccess get use_strategy config succ,use strategy"
            ",section=%s\n", section_));
        SysConf::instance()->get_conf_int(section_, "max_fail_times", &max_fail_times_);
        SysConf::instance()->get_conf_int(section_, "recon_interval", &recon_interval_);        
    }

    ACE_DEBUG((LM_INFO, "[%D] PlatDbAccess start to init conn"
        ",conn_nums=%d,timeout=%d\n", conn_nums_, time_out_)); 
    
    //��ʼ��һ������
    int ret = 0;
    for(int index=0; index<conn_nums_&&0==ret; index++)
    {
        ACE_Guard<ACE_Thread_Mutex> guard(db_conn_lock_[index]); 
        ret=init(db_host_, db_user_, db_pswd_, port_, index);        
    }
    return ret;
	
}

int PlatDbAccess::init(const char *host, const char *user, const char *passwd, int port
        , int index)
{
    if (host==NULL || user==NULL || passwd==NULL 
        || index > DB_CONN_MAX || index <0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatDbAccess init failed, para is invalid\n"));
        return -1;
    }
	
	//��ʼ��һ������
    if (db_conn_[index] != NULL)
    {
        fini(index);
    }

    ACE_DEBUG((LM_DEBUG, "[%D] PlatDbAccess start to connect db"
    	",host=%s"
    	",user=%s"
    	",passwd=%s"
    	",index=%d"
    	"\n"
    	, host
    	, user
    	, passwd
    	, index
    	));

    db_conn_[index] = new PlatDbConn();
    if (db_conn_[index]->connect(host, user, passwd, port, time_out_) != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatDbAccess connect db failed, errmsg=%s\n"
            , db_conn_[index]->get_err_msg()));
        fini(index);
        return -1;
    }

    ACE_DEBUG((LM_INFO, "[%D] PlatDbAccess connect db succ"
        ",host=%s"
        ",user=%s"
        ",passwd=%s"
        ",index=%d"
        "\n"
        , host
        , user
        , passwd
        , index
        ));
    
    return 0;
}

int PlatDbAccess::set_conns_char_set(const char* character_set)
{
    int ret = 0;
    for(int i=0; i<conn_nums_&&0==ret; i++)
    {
        ACE_Guard<ACE_Thread_Mutex> guard(db_conn_lock_[i]); 
        if(db_conn_[i]!=NULL)
        ret = db_conn_[i]->set_character_set(character_set);
    }    
    return ret;
}

unsigned long PlatDbAccess::escape_string(char *to, const char *from
	, unsigned long length)
{
    unsigned long ret = 0;
    for(int i=0; i<conn_nums_&&0==ret; i++)
    {
        ACE_Guard<ACE_Thread_Mutex> guard(db_conn_lock_[i]); 
        if(db_conn_[i]!=NULL)
        ret = db_conn_[i]->escape_string(to, from, length);
    }    
    return ret;
}

unsigned long PlatDbAccess::escape_string(string to, const string from)
{
    int length = from.length();
    char to_tmp[length*2+1];
    int ret =  escape_string(to_tmp, from.c_str(), length);
    to = to_tmp;
    return ret;
}

int PlatDbAccess::fini(int index)
{
    if ( index < 0 || index >= DB_CONN_MAX )
    {
        return -1;
    }
	
    // ���Ӳ��� 
    fail_times_++;
    last_fail_time_ = time(0); 

    if (db_conn_[index] != NULL)
    {  
        delete db_conn_[index];
        db_conn_[index] = NULL;
        db_conn_flag_[index] = 0;
    }

    return 0;
}

//ʹ�õ�ʱ������״̬db_conn_flag_ ����Ϊ 1   ʹ��������Ϊ 0 
//�̺߳����Ӱ󶨵�ʱ���ܴ˱���Ӱ��
int PlatDbAccess::exec_query(const char* sql, MYSQL_RES*& result_set
    , unsigned int uin)
{
    int index = get_conn_index(uin);    
    ACE_Guard<ACE_Thread_Mutex> guard(db_conn_lock_[index]); 
    // ��ʶ�Ƿ���ʹ�� �����ʹ���� ��ע������ 
    db_conn_flag_[index] = 1; // 1��ʾ����ʹ��
    
    ACE_DEBUG(( LM_TRACE, "[%D] PlatDbAccess exec query sql=%s\n", sql ));

    PlatDbConn* db_conn = get_db_conn(index);
    if (db_conn == NULL)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatDbAccess get db conn failed\n"));
        db_conn_flag_[index] = 0;
        return -1;
    }

    int ret = 0;    
    ret = db_conn->exec_query(sql, result_set);
    if (ret != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatDbAccess exec query failed,idx=%d,%s\n"
            , index, db_conn->get_err_msg() ));
        if(ret == -1) // -1 ��ʾ���������ӶϿ����������� try again
        {
            ret = db_conn->exec_query(sql, result_set);
            if(ret == -1) //������ʧ�ܣ�����������Ӷ���
            {
                fini(index);
            }
        }
    }
    db_conn_flag_[index] = 0;
    return ret;
}

int PlatDbAccess::exec_multi_query(const char* sql, vector<MYSQL_RES*>& result_set_list
    , unsigned int uin)
{
    int index = get_conn_index(uin);    
    ACE_Guard<ACE_Thread_Mutex> guard(db_conn_lock_[index]); 
    // ��ʶ�Ƿ���ʹ�� �����ʹ���� ��ע������ 
    db_conn_flag_[index] = 1; // 1��ʾ����ʹ��
    
    ACE_DEBUG(( LM_TRACE, "[%D] PlatDbAccess exec query sql=%s\n", sql ));

    PlatDbConn* db_conn = get_db_conn(index);
    if (db_conn == NULL)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatDbAccess get db conn failed\n"));
        db_conn_flag_[index] = 0;
        return -1;
    }

    int ret = 0;    
    ret = db_conn->exec_multi_query(sql, result_set_list);
    if (ret != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatDbAccess exec query failed,idx=%d,%s\n"
            , index, db_conn->get_err_msg() ));
        if(ret == -1) // -1 ��ʾ���������ӶϿ����������� try again
        {
            ret = db_conn->exec_multi_query(sql, result_set_list);
            if(ret == -1) //������ʧ�ܣ�����������Ӷ���
            {
                fini(index);
            }
        }
    }
    db_conn_flag_[index] = 0;
    return ret;
}

//ʹ�õ�ʱ������״̬db_conn_flag_ ����Ϊ 1   ʹ��������Ϊ 0 
//�̺߳����Ӱ󶨵�ʱ���ܴ˱���Ӱ��
int PlatDbAccess::exec_update(const char* sql, int& last_insert_id
	, int& affected_rows, unsigned int uin)
{
    int index = get_conn_index(uin);
    ACE_Guard<ACE_Thread_Mutex> guard(db_conn_lock_[index]); 
    // ��ʶ�Ƿ���ʹ�� �����ʹ���� ��ע������ 
	db_conn_flag_[index] = 1;
    
    ACE_DEBUG((LM_TRACE, "[%D] PlatDbAccess exec update sql=%s,conn index=%d\n", sql, index));

    PlatDbConn* db_conn = get_db_conn(index);
    if (db_conn == NULL)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatDbAccess get db conn failed\n"));
		db_conn_flag_[index] = 0;
        return -1;
    }

    int ret = 0;
    ret = db_conn->exec_update(sql, last_insert_id, affected_rows);
    if (ret != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatDbAccess exec update failed,ret=%d,%s\n"
            , ret, db_conn->get_err_msg() ));
        if(ret == -1) // -1 ��ʾ���������ӶϿ����������� try again
        {
            ret = db_conn->exec_update(sql, last_insert_id, affected_rows);
            if(ret == -1) //������ʧ�ܣ�����������Ӷ���
            {
                fini(index);
            }
        }
    }
    db_conn_flag_[index] = 0;
    return ret;
}

int PlatDbAccess::exec_trans(const vector<string>& sqls, int & last_insert_id
    , int & affected_rows, unsigned int uin)
{
    int index = get_conn_index(uin);
    ACE_Guard<ACE_Thread_Mutex> guard(db_conn_lock_[index]); 
    // ��ʶ�Ƿ���ʹ�� �����ʹ���� ��ע������ 
	db_conn_flag_[index] = 1; 
    
    ACE_DEBUG((LM_TRACE, "[%D] PlatDbAccess exec_trans sql vec size=%d,conn index=%d\n", sqls.size(), index));

    PlatDbConn* db_conn = get_db_conn(index);
    if (db_conn == NULL)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatDbAccess get db conn failed\n"));
		db_conn_flag_[index] = 0;
        return -1;
    }

    int ret = 0;
    ret = db_conn->exec_trans(sqls, last_insert_id, affected_rows);
    if (ret != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatDbAccess exec_trans failed,%s\n", db_conn->get_err_msg() ));
        if(ret == -1) // -1 ��ʾ���������ӶϿ�
        {
            ret = db_conn->exec_trans(sqls, last_insert_id, affected_rows);
            if(ret == -1) //������ʧ�ܣ�����������Ӷ���
            {
                fini(index);
            }
        }
    }
    db_conn_flag_[index] = 0;
    return ret;
}

int PlatDbAccess::free_result(MYSQL_RES*& game_res)
{
    if (game_res == NULL)
    {
        return 1; //����Դ������Ҫ�ͷ�
    }
	
    mysql_free_result(game_res);
    game_res = NULL;
    return 0;
}

int PlatDbAccess::free_result(vector<MYSQL_RES*>& result_set_vec)
{
    for(int i=0; i<result_set_vec.size(); i++)
    {
        free_result(result_set_vec[i]);
    }
    return 0;
}

PlatDbConn* PlatDbAccess::get_db_conn(int index) //�����ӳػ�ȡһ������
{
    index %= conn_nums_;
    int ret = 0;

    // ���Ӳ��� 
    if (use_strategy_ == 1 //ʹ�ò���
        && fail_times_ > max_fail_times_  //ʧ�ܴ����������ʧ�ܴ���
        && (time(0) - last_fail_time_) < recon_interval_ //ʱ����С������ʱ���� 
    )
    {
        ACE_DEBUG((LM_TRACE, "[%D] PlatDbAccess get conn failed,because of strategy"
            ",index=%d"
            ",fail_times=%d"
            ",last_fail_time=%d"
			"\n" 
            , index
            , fail_times_
            , last_fail_time_
            ));
        return NULL;//NULL 
    }
    
    if (db_conn_[index] == NULL)
    {
        ret = init(db_host_, db_user_, db_pswd_, port_, index);
    }

    //���ӳɹ�ʵ������ʧ�ܴ���Ҫ�� 0 	
    if (ret == 0) //˵�����ӳɹ�
    {
        fail_times_ = 0; 
    }
    
    return db_conn_[index];	
}

unsigned int PlatDbAccess::get_conn_index(unsigned int uin)
{
#ifdef THREAD_BIND_CONN
    return syscall(SYS_gettid)%conn_nums_;//���㷨�������̺߳ŵ�˳��������ܾ��ȷ�������     
#else
    // �� uin ·�� 
    if (uin != 0)
    {
        return uin%conn_nums_;
    }

    // ���·�� 
    int index = rand()%conn_nums_;
    if(0 == db_conn_flag_[index])
    {
        return index;
    }
    // �ҵ�һ��û��ʹ�õ����Ӿͷ���:
    int i = 0;
    for (i=0; i<conn_nums_; i++)
    {
        if (0 == db_conn_flag_[i])
        {
            index = i;
            break;
        }
    }
    // ���û�ҵ���������øղ������
    if (i == conn_nums_)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatDbAccess get_conn_index conn runout"
            ",section=%s"
            ",index=%d"
            "\n"
            , section_
            , index));
        Stat::instance()->incre_stat(STAT_CODE_DB_CONN_RUNOUT);
    }
    return index;

#endif
    
}

