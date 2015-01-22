#include "vip_db_oper.h"
//#include "pay.h"

using namespace EASY_UTIL;

ACE_Thread_Mutex VipDbOper::acct_db_lock_;
PlatDbAccess* VipDbOper::acct_db_access_[MAX_DB_ACCESS];

int VipDbOper::init_acct_db_access(int area)
{
    int index = area;
    if (index >= MAX_DB_ACCESS || index < 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] VipDbOper init acct db access failed, index is invalid\n"));
        return -1;
    }

    //don't use lock, avoid lock recursion
    if ( acct_db_access_[index]!= NULL )
    {
        ACE_DEBUG((LM_INFO, "[%D] VipDbOper delete old service_cfg object\n"));
        delete acct_db_access_[index];
        acct_db_access_[index]=NULL;
    }
    
    //读配置文件获取数据库连接参数:
    acct_db_access_[index] = new  PlatDbAccess();
    char db_conf_section[32];
    memset(db_conf_section, 0x0, sizeof(db_conf_section));
    sprintf(db_conf_section, "vip_db_%d", area);
    
    //读入数据库连接串
    if(0 == acct_db_access_[index]->init(db_conf_section))
    {
        ACE_DEBUG((LM_INFO,"[%D] VipDbOper init acct db access success\n"));
    }
    else
    {
        ACE_DEBUG((LM_ERROR,"[%D] VipDbOper init acct db access failed\n"));
        delete acct_db_access_[index];
        acct_db_access_[index] = NULL;
        return -1;
    }

    return 0;
}
PlatDbAccess* VipDbOper::get_acct_db_access(int area)
{
    int index = area;
    if (index >= MAX_DB_ACCESS || index < 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] VipDbOper get acct db access failed, index is invalid\n"));
        return NULL;
    }
    
    ACE_DEBUG((LM_DEBUG, "[%D] VipDbOper start to get_acct_db_access\n"));
    
    ACE_Guard<ACE_Thread_Mutex> guard(acct_db_lock_);
    if ( NULL == acct_db_access_[index] )
    {
        init_acct_db_access(area);
    }

    ACE_DEBUG((LM_DEBUG, "[%D] VipDbOper get_acct_db_access succ \n"));
    return acct_db_access_[index];
}

int VipDbOper::get_acct_db_name(char* db_name, int name_len, int area, const char* db_type)
{
    char db_conf_section[32];
    memset(db_conf_section, 0x0, sizeof(db_conf_section));
    
    sprintf(db_conf_section, "vip_db_%d", area);
    return SysConf::instance()->get_conf_str( db_conf_section, db_type, db_name, name_len);
}

VipDbOper::VipDbOper()
{
    
}

VipDbOper::~VipDbOper()
{
    //注意数据库连接的释放
}

char * VipDbOper::get_errmsg()
{
    return err_msg_;
}

int  VipDbOper::get_vip_info(QModeMsg &req, char* ack)
{
    char sql[1024];
    char db_name[32];
    memset(sql, 0x0, sizeof(sql));
    memset(db_name, 0x0, sizeof(db_name));   

    VIPINFO info;
    memset(&info, 0x0, sizeof(info));
    
    strncpy(info.service, (*(req.get_map()))["game"].c_str(), sizeof(info.service)) ;
    info.uin = strtoul((*(req.get_map()))["uin"].c_str(), NULL, 10);//atoi((*(req.get_map()))["uin"].c_str());
    
    //获取所有角色id
    PlatDbAccess* db_access_ = NULL;
    get_acct_db_name(db_name, sizeof(db_name));
    sprintf(sql,"select Uin,Type from %s.tbVipUser where Uin=%u", db_name, info.uin); // and Game=\'%s\' , info.service
    ACE_DEBUG((LM_DEBUG, "[%D] execute sql=%s\n", sql));
    db_access_ = get_acct_db_access();
    if (db_access_ == NULL)
    {
        ACE_DEBUG((LM_ERROR,"[%D] VipDbOper get dr access failed, dr_access is null\n"));
        sprintf(ack, "uin=%u&info=%s", info.uin, "get vip info error, get db access failed");
        return -1;
    }
    MYSQL_RES* game_res = NULL;
    ret_ = db_access_->exec_query(sql, game_res);
    if(0 != ret_)
    {
        ACE_DEBUG((LM_ERROR, "[%D] Warnning: execute sql=%s failed, ret=%d\n", sql, ret_));
        sprintf(ack, "uin=%u&info=%s", info.uin, "get vip info error, query failed");
        mysql_free_result(game_res); //别忘记释放资源
        return ret_;
    }
    int row_num = mysql_num_rows(game_res);
    if(0 == row_num)
    {
        ACE_DEBUG((LM_ERROR, "[%D] Warnning: get res num rows, row_num is 0\n"));
        sprintf(ack, "uin=%u&info=%s", info.uin, "get vip info error, row num 0");
        mysql_free_result(game_res); //别忘记释放资源
        return 1;
    }

    MYSQL_ROW game_row = NULL; 
    game_row = mysql_fetch_row(game_res);
    info.level = atoi(game_row[1]);
    
    ACE_DEBUG((LM_DEBUG, "[%D] VipDbOper get vip info level=%d\n", info.level));
    
    //返回最高的等级
    sprintf(ack, "uin=%u&level=%d", info.uin, info.level);
    
    mysql_free_result(game_res);
    
    return 0;
}
