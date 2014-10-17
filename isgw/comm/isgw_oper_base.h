//=============================================================================
/**
 *  @file    isgw_oper_base.h
 *
 *  此文件为处理业务逻辑的基础类
 *  
 *  @author awayfang
 */
//=============================================================================

#ifndef _ISGW_OPER_BASE_H_
#define _ISGW_OPER_BASE_H_
#include "easyace_all.h"
#include "qmode_msg.h"
#include "cmd_amount_contrl.h"
#include "isgw_mgr_svc.h"

#ifndef FIELD_NAME_SVC
#define FIELD_NAME_SVC "service"
#endif

//消息类型指令定义
typedef enum _PDB_BASE_CMD
{
    // 100 以内内部保留 
    CMD_TEST = 1, //内部压力测试用指令 

    CMD_SELF_TEST = 2,					// 业务自测命令
    CMD_GET_SERVER_VERSION = 3,		// 获取当前svr的版本号
    CMD_SYS_LOAD_CONF = 10,             //重新加载配置信息 
    CMD_SYS_GET_CONTRL_STAT = 11,       //查询命令流量状态
    CMD_SYS_SET_CONTRL_STAT = 12,       //设置命令状态
    CMD_SYS_SET_CONTRL_SWITCH = 13,       //运行时设置流量控制功能开关
    
    CMD_PDB_BASE_END
};

class IsgwOperBase
{
protected:
    static IsgwOperBase* oper_;
    
public:
    IsgwOperBase();
    virtual ~IsgwOperBase();
    
    static IsgwOperBase* instance(IsgwOperBase* oper);
    static IsgwOperBase* instance();
    static int  init_auth_cfg();//操作权限控制配置项读取

    virtual int init();
    virtual int process(QModeMsg& req, char* ack, int& ack_len);
    virtual int reload_config();
    virtual int time_out();
    // wait task
    virtual int wait_task();

    int internal_process(QModeMsg& req, char* ack, int& ack_len);
    int is_auth(QModeMsg& req, char* ack, int& ack_len);

private:
	int self_test(int testlevel, std::string& msg);
	
private:
    // 命令字限制标志，为1限制命令字处理
    static int cmd_auth_flag_;
    // 可支持命令字列表
    static std::map<int, int> cmd_auth_map_;
    // 业务限制标志，为1限制业务处理
    static int svc_auth_flag_;
    // 可支持业务列表
    static std::map<int, int> svc_auth_map_;
    
};

#endif // _ISGW_OPER_BASE_H_
