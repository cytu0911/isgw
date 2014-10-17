/******************************************************************************
* �����࣬�������ӵĹ����࣬������֤�̰߳�ȫ����Ҫ�ϲ㱣֤
* ��Ҫʵ�����¹���:
* 1 ���ݿ�����ԭʼapi�ķ�װ
* 2 ���ӵ�(״̬)���ҹ����Զ�����������ʱ������Ҫ���ַ�����
******************************************************************************/

#ifndef _PLAT_DB_CONN_H_
#define _PLAT_DB_CONN_H_

#ifdef WIN32
#include <winsock2.h>
#endif

#include "mysql.h"
#include "errmsg.h"

#include <string>
#include <vector>
using namespace std;

#define PLAT_MAX_ERROR_LEN 255
#define PLAT_MAX_IP_LEN 64  //gcs��db�������ܻ�ܳ�
#define PLAT_MAX_USER_LEN 24
#define PLAT_MAX_PSWD_LEN 24
#define PLAT_MAX_DBNM_LEN 24
#define PLAT_MAX_UXSK_LEN 128
#define PLAT_MAX_CHARSET_LEN 32 //�ַ����ַ�����С

class PlatDbConn
{
public:
    PlatDbConn();
    ~PlatDbConn();
    // SELECT caller need to free result_set
    int exec_query(const char* sql, MYSQL_RES*& result_set);
    int exec_multi_query(const char* sql, vector<MYSQL_RES*> & result_set_list);
    // INSERT DELETE UPDATE
    int exec_update(const char* sql, int& last_insert_id, int& affected_rows);
    // TRANS 
    int exec_trans(const vector<string>& sqls, int& last_insert_id, int& affected_rows);
    int connect(const char* db_ip, const char* db_user, const char* db_pswd, 
        unsigned int port=0, int timeout=2, const char *db=NULL
        , const char *unix_socket=NULL, unsigned long client_flag=0); //CLIENT_MULTI_STATEMENTS
    int set_character_set(const char* character_set);
    unsigned long escape_string(char *to, const char *from, unsigned long length);

    char *get_err_msg(void) 
    {
        return err_msg_;
    }

private:
    int connect(); //�ڲ��Զ�������
    void disconnect(); //
	int set_character_set(); 

private:
    MYSQL mysql_;
    int conn_flag_; //�����Ƿ���Ч�ı�ʶ
    char db_ip_[PLAT_MAX_IP_LEN];
    char db_user_[PLAT_MAX_USER_LEN];
    char db_pswd_[PLAT_MAX_PSWD_LEN];
    
    char db_[PLAT_MAX_DBNM_LEN];
    unsigned int port_;
    char unix_socket_[PLAT_MAX_UXSK_LEN];
    unsigned long client_flag_;
    int time_out_; //��ʱʱ�䣬ĿǰΪ������Ч

    char char_set_[PLAT_MAX_CHARSET_LEN];
    
    char err_msg_[PLAT_MAX_ERROR_LEN+1];
};

#endif //_PLAT_DB_CONN_H_

