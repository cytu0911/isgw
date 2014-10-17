/******************************************************************************
* 单DB连接池管理类，为了保证数据库的长连接，请在调用接口中
* 实现为静态对象或者全局对象
*
******************************************************************************/
#ifndef _PLAT_ORA_ACCESS_H_
#define _PLAT_ORA_ACCESS_H_

#include "easyace_all.h"
#include "plat_ora_conn.h"

//#define MAX_DB_NAME_LEN 128
//#define MAX_ORA_HOST_LEN 32
#define MAX_ORA_USER_LEN 32
#define MAX_ORA_PSWD_LEN 64
#define MAX_ORA_TNS_LEN 64
//#define MAX_ERROR_LEN 128
#define POOL_ORA_CONN_MAX 1024  //连接池最大连接数
#define POOL_ORA_CONN_NUMS 32  //连接池缺省连接数

typedef unsigned int UINT;

class PlatOraAccess
{
public:
    PlatOraAccess();
    ~PlatOraAccess();
    int init(char *section = "database"); //初始化所有连接
    int init(char *user, char *passwd, char *tns, int index = 0); //初始化第index个 
    //该类(一池)连接全部设置到需要的字符集
    int set_conns_char_set(const int character_set); //把该连接池的所有连接设置为需要的字符集 
    int fini(int index); //终止第index个连接
    
    // SELECT caller don't need to free otl_strm
    int exec_query(const char* sql, otl_stream*& otl_strm);
    // exec Stored Procedure, only support simple int/char[] param datatype;
    int exec_proc(const char* sql, otl_stream * & otl_strm, ORA_PROC_PARAMS& params, ORA_PROC_PARAMS& outparams);    
    // INSERT DELETE UPDATE
    int exec_update(const char* sql);
    
    int set_all_column_types(otl_stream * & otl_strm, const unsigned int amask=0);
    
private:
    PlatOraConn* get_db_conn(int index);    //从连接池获取一个连接，第index个，保证和锁的序号一致性
    unsigned int get_conn_index(void);      //获得连接的下标
    int release_db_conn(int index);
private:
    int db_conn_flag_[POOL_CONN_MAX]; //DB连接的使用状态 
    PlatOraConn* db_conn_[POOL_ORA_CONN_MAX];

    ACE_Thread_Mutex db_conn_lock_[POOL_ORA_CONN_MAX]; //和连接一一对应保证单个线程使用连接
    ACE_Thread_Mutex db_access_lock_; //access本身的锁，对所有连接操作时用此锁
    int conn_nums_; //连接总数
    char section_[128];

};

#endif //_PLAT_ORA_ACCESS_H_

