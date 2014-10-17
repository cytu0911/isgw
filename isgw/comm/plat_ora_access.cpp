#include "plat_ora_access.h"

PlatOraAccess::PlatOraAccess()
{
    memset(section_, 0, sizeof(section_));
    for(int i=0; i<POOL_ORA_CONN_MAX; i++)
    {
        db_conn_[i] = NULL;
	db_conn_flag_[i] = 0; 
    }
    
    conn_nums_ = POOL_ORA_CONN_NUMS;
}

PlatOraAccess::~PlatOraAccess()
{
    for(int i=0; i<POOL_ORA_CONN_MAX; i++)
    {
        if (db_conn_[i] != NULL)
        {
            delete db_conn_[i];
            db_conn_[i] = NULL;
	    db_conn_flag_[i] = 0; 
        }
    }
}

int PlatOraAccess::init( char *section )
{
    ACE_Guard<ACE_Thread_Mutex> guard(db_access_lock_); 
    if (SysConf::instance()->get_conf_int(section, "conn_nums", &conn_nums_) != 0)
    {
        conn_nums_ = POOL_ORA_CONN_NUMS;
        ACE_DEBUG((LM_ERROR, "[%D][%N][%l] PlatOraAccess get conn_nums config error, section=%s"
            ", use default conn_nums %d\n"
            , section, conn_nums_));
    }

    char db_user[MAX_ORA_USER_LEN+1];
    if (SysConf::instance()->get_conf_str( section, "user", db_user, sizeof(db_user)) != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D][%N][%l] PlatOraAccess get user config error, section=%s\n", section)); 
        return -1;
    }

    char db_pswd[MAX_ORA_PSWD_LEN+1];
    if (SysConf::instance()->get_conf_str( section, "passwd", db_pswd, sizeof(db_pswd)) != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D][%N][%l] PlatOraAccess get passwd config error, section=%s\n", section)); 
        return -1;
    }

    char db_tns[MAX_ORA_TNS_LEN+1];
    if (SysConf::instance()->get_conf_str( section, "ora_tns", db_tns, sizeof(db_tns)) != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D][%N][%l] PlatOraAccess get ora_tns config error, section=%s\n", section)); 
        return -1;
    }

    strncpy(section_, section, 128);
    
    for(int i=0; i<conn_nums_; i++)
    {
        init(db_user, db_pswd, db_tns, i);
    }

    return 0;
}

int PlatOraAccess::init( char *user, char *passwd, char *tns, int index)
{    
    if ( user==NULL || passwd==NULL || tns==NULL || index<0 || index > POOL_ORA_CONN_MAX)
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatOraAccess init failed, para is invalid\n"));
        return -1;
    }

    ACE_Guard<ACE_Thread_Mutex> guard(db_conn_lock_[index]); //fini之前不能加锁，否则死锁

    if (db_conn_[index]!=NULL)
    {
        fini(index);
    }

    ACE_DEBUG((LM_DEBUG, "[%D] PlatOraAccess start to connect db"
		", user=%s"
		", passwd=%s"
		", tns=%s"
		"\n"
		, user
		, passwd
		, tns		
		));

    db_conn_[index] = new PlatOraConn();
    if (db_conn_[index]->connect(user, passwd, tns) != 0)        
    {
        ACE_DEBUG((LM_ERROR, "[%D] PlatOraAccess connect db failed, errmsg=%s\n", db_conn_[index]->get_err_msg()));
        fini(index);
        return -1;
    }

    ACE_DEBUG((LM_DEBUG, "[%D] PlatOraAccess connect db succ"
		", user=%s"
		", passwd=%s"
		", tns=%s"		
		"\n"
		, user
		, passwd
		, tns
		));
    
    return 0;
}

namespace {
class AutoRelease
{
	int & x_;
public:
	AutoRelease(int & x) : x_(x) { x = 1;}
	~AutoRelease() { x_ = 0;}
	
};
}

int PlatOraAccess::set_all_column_types(otl_stream * & otl_strm, const unsigned int amask)
{
    int index = get_conn_index();    
    AutoRelease ar(db_conn_flag_[index]);
    ACE_Guard<ACE_Thread_Mutex> guard(db_conn_lock_[index]); 
    
    ACE_DEBUG(( LM_DEBUG, "[%D] set_all_column_types amask =%d\n", amask ));

    PlatOraConn* db_conn = get_db_conn(index);
    if (db_conn == NULL)
    {
        ACE_DEBUG((LM_ERROR, "[%D][%N][%l] PlatOraAccess get db conn failed\n"));
        return -1;
    }

    int ret = 0;    
    ret = db_conn->set_all_column_types(otl_strm, amask);
    if (ret != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D][%N][%l] %s\n", db_conn->get_err_msg() ));
        if(ret == -1) // try again
        {
            ret = db_conn->set_all_column_types(otl_strm, amask);
            return ret;
        }
    }
    return ret;
}



int PlatOraAccess::set_conns_char_set(const int character_set)
{
    ACE_Guard<ACE_Thread_Mutex> guard(db_access_lock_); 
    int ret = 0;
    for(int i=0; i<conn_nums_; i++)
    {
        if(db_conn_[i]!=NULL)
        ret += db_conn_[i]->set_character_set(character_set);
    }    
    return ret;
}

int PlatOraAccess::fini(int index)
{
    if ( index < 0 || index > POOL_ORA_CONN_MAX )
    {
        return -1;
    }
    
//    ACE_Guard<ACE_Thread_Mutex> guard(db_conn_lock_[index]);    
    if (db_conn_[index] != NULL)
    {  
        delete db_conn_[index];
        db_conn_[index] = NULL;
    }

    return 0;
}


int PlatOraAccess::exec_query(const char* sql, otl_stream*& otl_strm)
{
    int index = get_conn_index();    
    AutoRelease ar(db_conn_flag_[index]);
    ACE_Guard<ACE_Thread_Mutex> guard(db_conn_lock_[index]); 
    
    ACE_DEBUG(( LM_DEBUG, "[%D][%N][%l] sql=%s\n", sql ));

    PlatOraConn* db_conn = get_db_conn(index);
    if (db_conn == NULL)
    {
        ACE_DEBUG((LM_ERROR, "[%D][%N][%l] PlatOraAccess get db conn failed\n"));
        return -1;
    }

    int ret = 0;    
    ret = db_conn->exec_query(sql, otl_strm);
    if (ret != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D][%N][%l] %s\n", db_conn->get_err_msg() ));
        if(ret == -1) // try again
        {
            ret = db_conn->exec_query(sql, otl_strm);
            return ret;
        }
    }
    return ret;
}

int PlatOraAccess::exec_update( const char* sql )
{
    int index = get_conn_index();
    AutoRelease ar(db_conn_flag_[index]);
    ACE_Guard<ACE_Thread_Mutex> guard(db_conn_lock_[index]); 
    
    ACE_DEBUG((LM_TRACE, "[%D][%N][%l] sql=%s\n", sql ));

    PlatOraConn* db_conn = get_db_conn(index);
    if (db_conn == NULL)
    {
        ACE_DEBUG((LM_ERROR, "[%D][%N][%l] PlatOraAccess get db conn failed\n"));
        return -1;
    }

    int ret = 0;
    ret = db_conn->exec_update(sql);
    if (ret != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D][%N][%l] %s\n", db_conn->get_err_msg() ));
        if(ret == -1) // try again
        {
            ret = db_conn->exec_update(sql);
            return ret;
        }
    }
    return ret;
}

int PlatOraAccess::exec_proc(const char* sql, otl_stream * & otl_strm, ORA_PROC_PARAMS& params, ORA_PROC_PARAMS& outparams)
{
    int index = get_conn_index();    
    AutoRelease ar(db_conn_flag_[index]);
    ACE_Guard<ACE_Thread_Mutex> guard(db_conn_lock_[index]); 
    
    ACE_DEBUG(( LM_DEBUG, "[%D][%N][%l] sql=%s\n", sql ));

    PlatOraConn* db_conn = get_db_conn(index);
    if (db_conn == NULL)
    {
        ACE_DEBUG((LM_ERROR, "[%D][%N][%l] PlatOraAccess get db conn failed\n"));
        return -1;
    }

    int ret = 0;    
    ret = db_conn->exec_proc(sql, otl_strm, params, outparams);
    if (ret != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D][%N][%l] %s\n", db_conn->get_err_msg() ));
        if(ret == -1) // try again
        {
        //    ret = db_conn->exec_proc(sql, otl_strm, params, outparams);
            return ret;
        }
    }
    return ret;
}



PlatOraConn* PlatOraAccess::get_db_conn(int index) //从连接池获取一个连接
{
    //int index = get_conn_index();
    //保证连接池不溢出

    index = index%POOL_ORA_CONN_MAX;
    if ( db_conn_[index] == 0 ) {
	    char db_user[MAX_ORA_USER_LEN+1];
	    if (SysConf::instance()->get_conf_str( section_, "user", db_user, sizeof(db_user)) != 0)
	    {
		    ACE_DEBUG((LM_ERROR, "[%D][%N][%l] PlatOraAccess get user config error, section=%s\n", section_)); 
		    return 0;
	    }

	    char db_pswd[MAX_ORA_PSWD_LEN+1];
	    if (SysConf::instance()->get_conf_str( section_, "passwd", db_pswd, sizeof(db_pswd)) != 0)
	    {
		    ACE_DEBUG((LM_ERROR, "[%D][%N][%l] PlatOraAccess get passwd config error, section=%s\n", section_)); 
		    return 0;
	    }

	    char db_tns[MAX_ORA_TNS_LEN+1];
	    if (SysConf::instance()->get_conf_str( section_, "ora_tns", db_tns, sizeof(db_tns)) != 0)
	    {
		    ACE_DEBUG((LM_ERROR, "[%D][%N][%l] PlatOraAccess get ora_tns config error, section=%s\n", section_)); 
		    return 0;
	    }

	    init(db_user, db_pswd, db_tns, index);
    }
    return db_conn_[index];
}


unsigned int PlatOraAccess::get_conn_index(void)
{
    //return getpid()%conn_nums_;

    //suse下编译获取到的pid可能为同一个
#if 0
    return ACE_OS::getpid()%conn_nums_;//此算法依赖于线程号的顺序产生才能均匀分配请求     
#else
    int index = 0;
    //找到一个没有使用的连接就返回:
    ACE_Guard<ACE_Thread_Mutex> guard(db_access_lock_); 
    for(int i=0; i<conn_nums_; i++)
    {
        if(0 == db_conn_flag_[i])
        {
            index = i;
	    db_conn_flag_[i] = 1;
            break;
        }
    }
    //如果没找到，则随便返回一个
    if(index == conn_nums_)
    {
        index = rand()%conn_nums_;
    }
    return index;
#endif
}	

