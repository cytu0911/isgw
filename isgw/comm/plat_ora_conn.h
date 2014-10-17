/******************************************************************************
* ORACLE������(��������ociģʽ)���������ӵĹ����࣬������֤�̰߳�ȫ����Ҫ�ϲ㱣֤
* ��Ҫʵ�����¹���:
* 1 ���ݿ�����ԭʼapi�ķ�װ
* 2 ���ӵ�(״̬)���ҹ����Զ�����������ʱ������Ҫ���ַ�����
******************************************************************************/

#ifndef _PLAT_ORA_CONN_H_
#define _PLAT_ORA_CONN_H_

#include "otlv4.h"
#include "errmsg.h"
#include "isgw_comm.h"

//#define OTL_ORA9I // Compile OTL 4.0/OCI9i
//#define OTL_UNICODE // Enable Unicode OTL for OCI9i
//#define OTL_ORA_UTF8 // Enable UTF8 OTL for OCI9i

#define PLAT_ORA_MAX_ERROR_LEN 2048
#define PLAT_ORA_MAX_USER_LEN 24
#define PLAT_ORA_MAX_PSWD_LEN 24
#define PLAT_ORA_MAX_TNS_LEN 36
#define PLAT_ORA_MAX_CONN_STR_LEN 128
#define PLAT_ORA_SP_PARAM_STR_LEN 128

typedef enum  _PROC_PARAMTYPE
{
    PPTIn = 1   ,
    PPTOut      ,
    PPTInOut
}PROC_PARAMTYPE;

typedef enum _PROC_DATATYPE
{
    PDTInt = 1,
    PDTString
}PROC_DATATYPE;

typedef struct _ORA_PROC_PARAM
{
    PROC_PARAMTYPE  ppt;        //param Parametertype
    PROC_DATATYPE   pdt;        //param datatype
    int             index;      //param index
    char            value[PLAT_ORA_SP_PARAM_STR_LEN+1];      //param value
    //string          value;      //param value
}ORA_PROC_PARAM;

typedef vector<ORA_PROC_PARAM> ORA_PROC_PARAMS;


class PlatOraConn
{
public:
    PlatOraConn();
    ~PlatOraConn();
    
    // SELECT caller don't need to free otl_strm
    int exec_query(const char* sql, otl_stream * & otl_strm);
    // exec Stored Procedure, only support simple int/char[] param datatype;
    int exec_proc(const char* sql, otl_stream * & otl_strm, ORA_PROC_PARAMS& params, ORA_PROC_PARAMS& outparams);
    // INSERT DELETE UPDATE
    int exec_update(const char* sql);
    
    int connect(const char* db_user, const char* db_pswd, 
        const char* db_tns,const int auto_commit = 0 );
    int set_character_set(const int char_set = SQLCS_IMPLICIT);

    int set_all_column_types(otl_stream*& otl_strm, const unsigned int amask=0);

    char *get_err_msg(void) 
    {
        return err_msg_;
    }

private:
    int connect(); //�ڲ��Զ�������
    void disconnect(); //
    int set_chrcter_set(); 

private:
    otl_connect otl_conn_;
    
    int conn_flag_; //�����Ƿ���Ч�ı�ʶ
    char db_user_[PLAT_ORA_MAX_USER_LEN];
    char db_pswd_[PLAT_ORA_MAX_PSWD_LEN];
    char db_tns_[PLAT_ORA_MAX_TNS_LEN];
    int auto_commit_; //�Զ��ύ����

    int char_set_; //�����ַ�������Ҫ�����OTL_UNICODE
    
    char err_msg_[PLAT_ORA_MAX_ERROR_LEN+1];

    static int init_flag; //��ʼ��OTL������ʶ
};

#endif //_PLAT_ORA_CONN_H_
