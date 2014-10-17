#ifndef _VIP_DB_OPER_
#define _VIP_DB_OPER_
#include "isgw_comm.h"
#include "plat_db_access.h"

#define MAX_DB_ACCESS  128
#define MAX_DB_NAME_LEN  128

struct VIPINFO
{
    unsigned int uin;
    int level; //vip 级别
    char service[128];
};

class VipDbOper
{
public:
    VipDbOper();
    ~VipDbOper();
    int get_vip_info(QModeMsg &req, char* ack);
    //int itc_oper(QModeMsg &req, char* ack);
    char * get_errmsg();

private:

    //acct db
    static int init_acct_db_access(int area = 1); //一个大区一个
    static PlatDbAccess* get_acct_db_access(int area = 1); //一个大区一个
    int get_acct_db_name(char* db_name, int name_len, int area = 1, const char* db_type = "db_name");
    
private:
    static ACE_Thread_Mutex acct_db_lock_;
    static PlatDbAccess* acct_db_access_[MAX_DB_ACCESS];
    
private:
    char err_msg_[PLAT_MAX_ERROR_LEN+1];
    int ret_;
};

#endif //_VIP_DB_OPER_

