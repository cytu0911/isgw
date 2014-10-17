/******************************************************************************
* MYSQL���ӳع����࣬������Ԫ�ĸ���(һ�����������߸�С�ļ���)�������ӹ���
* �����ӳص������������������Ͼ��жԵȵ��߼����ܣ�û�в��
* Ϊ�˱�֤�����ӣ����ڵ��ýӿ���ʵ��Ϊ��̬�������ȫ�ֶ���
* 
******************************************************************************/
#ifndef _PLAT_DB_ACCESS_H_
#define _PLAT_DB_ACCESS_H_

#include "easyace_all.h"
#include "plat_db_conn.h"

#define MAX_SECTION_NAME_LEN 32
#define MAX_DB_NAME_LEN 128
#define MAX_DB_HOST_LEN 64
#define MAX_DB_USER_LEN 32
#define MAX_DB_PSWD_LEN 64
#define MAX_ERROR_LEN 128
#define DB_CONN_MAX 50  //DB ���ӳ����������
#define POOL_CONN_DEF 10  //���ӳ�ȱʡ������

typedef unsigned int UINT;

class PlatDbAccess
{
public:
    PlatDbAccess();
    ~PlatDbAccess();
    int init(const char *section = "database"); //��ʼ����������
    int init(const char *host, const char *user, const char *passwd, int port
        , int index); //ָ��������ʼ������
    int set_conns_char_set(const char* character_set); //�Ѹ����ӳص�������������Ϊ��Ҫ���ַ���
    unsigned long escape_string(char *to, const char *from, unsigned long length); //�Ѹ����ӳص���������... 
    
    // SELECT caller need to free result_set
    int exec_query(const char* sql, MYSQL_RES*& result_set, unsigned int uin=0);
    int exec_multi_query(const char* sql, vector<MYSQL_RES*>& result_set_list, unsigned int uin=0);
    // INSERT DELETE UPDATE
    int exec_update(const char* sql, int& last_insert_id, int& affected_rows
        , unsigned int uin=0);

    // TRANS
    int exec_trans(const vector<string>& sqls, int& last_insert_id, int& affected_rows
        , unsigned int uin=0);

	int free_result(MYSQL_RES*& game_res);
    
private:
    int fini(int index); //��ֹ��index������
    PlatDbConn* get_db_conn(int index); //�����ӳػ�ȡһ�����ӣ���index������֤���������һ����
    unsigned int get_conn_index(unsigned int uin=0); //������ӵ��±�

private:
    PlatDbConn* db_conn_[DB_CONN_MAX];
    int db_conn_flag_[DB_CONN_MAX]; //DB���ӵ�ʹ��״̬ 
    ACE_Thread_Mutex db_conn_lock_[DB_CONN_MAX]; //������һһ��Ӧ��֤�����߳�ʹ������
    int conn_nums_; //�������� 
    char section_[MAX_SECTION_NAME_LEN]; 

    //����������
    char db_host_[MAX_DB_HOST_LEN];
    char db_user_[MAX_DB_USER_LEN];
    char db_pswd_[MAX_DB_PSWD_LEN];
    int port_; 
    int time_out_; //db����ĳ�ʱʱ��

    //��������
    int use_strategy_; //����ʹ�ñ�־ 
    int max_fail_times_; //�����������ʧ�ܴ������������ͣ����գ���
    int fail_times_; //����ʵ������ʧ�ܴ������������ͣ����գ���
    int recon_interval_; //�������
    int last_fail_time_; //���ʧ��ʱ�� 
    
    char err_msg_[MAX_ERROR_LEN+1];
};

#endif //_PLAT_DB_ACCESS_H_
