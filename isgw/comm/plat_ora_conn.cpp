#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "plat_ora_conn.h"

int PlatOraConn::init_flag = 0;

PlatOraConn::PlatOraConn()
{
    conn_flag_ = 0;
    memset(db_user_, 0x0, sizeof(db_user_));
    memset(db_pswd_, 0x0, sizeof(db_pswd_));
    memset(db_tns_, 0x0, sizeof(db_tns_));    
    
    auto_commit_ = 0;
    char_set_ = SQLCS_IMPLICIT;
    
    
    memset(err_msg_, 0x0, sizeof(err_msg_));
}

PlatOraConn::~PlatOraConn()
{
    disconnect();
}

int PlatOraConn::set_all_column_types(otl_stream*& otl_strm, const unsigned int amask)
{
    try
    {
        //otl_all_num2str：数字类型转换成string；
        //otl_all_date2str：日期类型转换成string；
        //otl_all_num2str|otl_all_date2str：数组、日期全部转换成string；
        otl_strm->set_all_column_types(amask);
    }
    catch(otl_exception& p)
    {
        // intercept OTL exceptions
        sprintf(err_msg_, "PlatOraConn set_all_column_types[%d] error: %d, %s",
            amask, p.code, p.msg);
        return -1;
    }
    
    return 0;
}


    
int PlatOraConn::set_character_set(const int char_set)
{
    //save char set info
    char_set_ = char_set;
    
    return set_chrcter_set();
}

int PlatOraConn::set_chrcter_set()
{
    try
    {
        //设置字符集，对oracle有效，并且需要定义宏OTL_UNICODE。
        otl_conn_.set_character_set(char_set_);
    }
    catch(otl_exception& p)
    {
        // intercept OTL exceptions
        sprintf(err_msg_, "PlatOraConn set_character_set error: %d, %s",
            p.code, p.msg);

        return -1;
    }    
    
    return 0;
}


int PlatOraConn::exec_query(const char* sql, otl_stream*& otl_strm)
{
    
    if(conn_flag_ == 0)
    {
        if(connect() != 0)
        {
            return -1;
        }
    }

    try
    {
        if(otl_strm->good()) { otl_strm->close(); }
        otl_strm->open(1, sql, otl_conn_);
        
        if(!otl_strm->good())
        {
            sprintf(err_msg_, "PlatOraConn exec_query error: unknown.");
            
            disconnect(); //清理，便于下次重连
            return -1;
        }
    }
    catch(otl_exception& p)
    {
        // intercept OTL exceptions
        sprintf(err_msg_, "PlatOraConn exec_query error: %d, %s",
            p.code, p.msg);

        disconnect(); //判断连接错误或已断开 须清理，便于下次重连
        return -1;
    }   
    
    return 0;

}


int PlatOraConn::exec_update( const char* sql )
{
    if(conn_flag_ == 0)
    {
        if(connect() != 0)
        {
            return -1;
        }
    }

    try
    {
        otl_conn_.direct_exec(sql);
        otl_conn_.commit();
    }
    catch(otl_exception& p)
    {
        // intercept OTL exceptions
        sprintf(err_msg_, "PlatOraConn exec_update error: %d, %s",
            p.code, p.msg);

        disconnect(); //判断连接错误或已断开 须清理，便于下次重连
        return -1;
    }   

    return 0;
}

int PlatOraConn::exec_proc(const char* sql, otl_stream * & otl_strm, ORA_PROC_PARAMS& params, ORA_PROC_PARAMS& outparams)
{

    if(conn_flag_ == 0)
    {
        if(connect() != 0)
        {
            return -1;
        }
    }

    try
    {
        if(otl_strm->good()) { otl_strm->close(); }
        otl_strm->open(1, sql, otl_conn_);
        
        // Since there is no transactions, unset the stream auto-commit
        otl_strm->set_commit(0); 

        //input
        for( int i = 0; i< params.size(); i++)
        {
            const ORA_PROC_PARAM& param = params[i] ;
ACE_DEBUG((LM_DEBUG, "[%D] exec_proc param[%d]: param.ppt:%d, param.pdt:%d, param.index:%d, param.value:%s  \n"
    , i 
    , param.ppt
    , param.pdt
    , param.index
    , param.value));

            if ( (param.ppt == PPTIn) || (param.ppt == PPTInOut) )
            {
                if( param.pdt == PDTString )
                {
                    (*otl_strm) << param.value;
                }
                else if ( param.pdt == PDTInt )
                {
                    (*otl_strm) << atoi(param.value);
                }       
                else
                {
                    //异常退出
                    sprintf(err_msg_, "PlatOraConn exec_proc error:parametertype error.");
                    return -1;
                }
            }
        }      

        //output
        for( int i = params.size() -1 ; i >= 0 ; i--)
        {
            ORA_PROC_PARAM& param = params[i] ;
            if ( (param.ppt == PPTOut) || (param.ppt == PPTInOut) )
            {
                if ( param.pdt == PDTInt )
                {
                    if ( !otl_strm->eof() )
                    {
                        int value;
                        (*otl_strm) >> value;
                        snprintf(param.value, sizeof(param.value), "%d", value);
                    }
                    else break;
                }
                else //PDTString
                {
                    if ( !otl_strm->eof() )
                    {
                        (*otl_strm) >> param.value;
                    }
                    else break;
                }
            }
            else
            {
                continue;
            }
            outparams.push_back(param);
        }
        
        //提交当前事务
        otl_conn_.commit();
    }
    catch(otl_exception& p)
    {
        // intercept OTL exceptions
        sprintf(err_msg_, "PlatOraConn exec_proc error: %d, %s",
            p.code, p.msg);

        disconnect(); //判断连接错误或已断开 须清理，便于下次重连
        return -1;
    }

    return 0;
    
}


int PlatOraConn::connect(const char* db_user, const char* db_pswd,    
    const char* db_tns, const int auto_commit )    
{
    //save connect info    
    strncpy(db_user_, db_user, sizeof(db_user_)-1);
    strncpy(db_pswd_, db_pswd, sizeof(db_pswd_)-1);
    strncpy(db_tns_, db_tns, sizeof(db_tns_)-1);
    auto_commit_ = auto_commit;
        
    return connect();
}

void PlatOraConn::disconnect()
{
    if (conn_flag_ == 1)
    {
        otl_conn_.logoff();

        conn_flag_ = 0;
    }
}

///
int PlatOraConn::connect()
{
    if (!init_flag) 
    {
        //多线程模式 只需在连接数据库之前调用一次
        otl_connect::otl_initialize(1); // initialize OCI environment
        init_flag = 1;
    }

    try
    {
        char connect_str[PLAT_ORA_MAX_CONN_STR_LEN]={0};
        snprintf(connect_str, sizeof(connect_str), "%s/%s@%s", db_user_, db_pswd_, db_tns_);
        
        otl_conn_.rlogon(connect_str,auto_commit_);
    }
    catch(otl_exception& p)
    {
        // intercept OTL exceptions
        sprintf(err_msg_, "PlatOraConn connect error: %d, %s",
            p.code, p.msg);

        disconnect(); //判断连接错误或已断开 须清理，便于下次重连
        return -1;
    }   

    conn_flag_ = 1;
    return 0;

}

