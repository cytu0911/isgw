/******************************************************************************
* ��DB���ӳع����࣬Ϊ�˱�֤���ݿ�ĳ����ӣ����ڵ��ýӿ���
* ʵ��Ϊ��̬�������ȫ�ֶ���
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
#define POOL_ORA_CONN_MAX 1024  //���ӳ����������
#define POOL_ORA_CONN_NUMS 32  //���ӳ�ȱʡ������

typedef unsigned int UINT;

class PlatOraAccess
{
public:
    PlatOraAccess();
    ~PlatOraAccess();
    int init(char *section = "database"); //��ʼ����������
    int init(char *user, char *passwd, char *tns, int index = 0); //��ʼ����index�� 
    //����(һ��)����ȫ�����õ���Ҫ���ַ���
    int set_conns_char_set(const int character_set); //�Ѹ����ӳص�������������Ϊ��Ҫ���ַ��� 
    int fini(int index); //��ֹ��index������
    
    // SELECT caller don't need to free otl_strm
    int exec_query(const char* sql, otl_stream*& otl_strm);
    // exec Stored Procedure, only support simple int/char[] param datatype;
    int exec_proc(const char* sql, otl_stream * & otl_strm, ORA_PROC_PARAMS& params, ORA_PROC_PARAMS& outparams);    
    // INSERT DELETE UPDATE
    int exec_update(const char* sql);
    
    int set_all_column_types(otl_stream * & otl_strm, const unsigned int amask=0);
    
private:
    PlatOraConn* get_db_conn(int index);    //�����ӳػ�ȡһ�����ӣ���index������֤���������һ����
    unsigned int get_conn_index(void);      //������ӵ��±�
    int release_db_conn(int index);
private:
    int db_conn_flag_[POOL_CONN_MAX]; //DB���ӵ�ʹ��״̬ 
    PlatOraConn* db_conn_[POOL_ORA_CONN_MAX];

    ACE_Thread_Mutex db_conn_lock_[POOL_ORA_CONN_MAX]; //������һһ��Ӧ��֤�����߳�ʹ������
    ACE_Thread_Mutex db_access_lock_; //access������������������Ӳ���ʱ�ô���
    int conn_nums_; //��������
    char section_[128];

};

#endif //_PLAT_ORA_ACCESS_H_

