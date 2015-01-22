/******************************************************************************
*  @file      isgw_app.cpp
*  @author awayfang
*  @history 
*  
******************************************************************************/
#include "isgw_app.h"
#include "isgw_intf.h"
#include "isgw_uintf.h"
#include "isgw_ack.h"
#include "isgw_mgr_svc.h"
#include "stat.h"
#include "ace/Dev_Poll_Reactor.h"
#include "ace/High_Res_Timer.h"
#include "isgw_sighdl.h"
#include "isgw_oper_base.h"
#ifdef ISGW_USE_IBC
#include "ibc_mgr_svc.h"
#endif 
#include "isgw_cintf.h"
#ifdef ISGW_USE_ASY
#include "asy_task.h"
#endif 

using namespace EASY_UTIL;

ISGWApp::ISGWApp() : max_wait_mgr_time_(DEF_MAX_WAIT_MGR_TIME)
{

}

int  ISGWApp::init_sys_path(const char* program)//app_path_env_name
{
    char bin_path_env_name[256];
    snprintf(bin_path_env_name, sizeof(bin_path_env_name), "%s", "ISGW_BIN");
    
    char* bin_path_env = ACE_OS::getenv(bin_path_env_name);
    if (bin_path_env == NULL)
    {
        ACE_DEBUG((LM_ERROR, "[%D] env %s not set\n", bin_path_env_name));
        return -1;
    }

    if (chdir(bin_path_env) != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] chdir(%s) error, %d, %s\n",
                  bin_path_env, errno, strerror(errno)));
        return -1;
    }

    ACE_DEBUG((LM_INFO, "[%D] path changed to %s\n", bin_path_env));
    return 0;
}

int ISGWApp::init_conf()
{
    char cfg_path_env_name[256];
    snprintf(cfg_path_env_name, sizeof(cfg_path_env_name), "%s", "ISGW_CFG");
    
    char* cfg_path_env = ACE_OS::getenv(cfg_path_env_name);
    if (cfg_path_env == NULL)
    {
        ACE_DEBUG((LM_INFO, "[%D] env %s not set, use default working path\n", cfg_path_env_name));
        cfg_path_env = "./";
    }

    string conf_file = program_;
    string conf_path = cfg_path_env;
    conf_file += ".ini";
    conf_file = conf_path + "/" + conf_file;

    // 加载系统配置文件
    if(SysConf::instance()->load_conf(conf_file) != 0)
    {
        return -1;
    }
        
    // 增量加载服务器配置文件
    string svrs_file(cfg_path_env);
    svrs_file.append("/svrlist.ini");
    
    char svrs_file_path[128] = {0};
    if(SysConf::instance()->get_conf_str("common", "svrs_file", svrs_file_path, sizeof(svrs_file_path)) == 0)
    {
        svrs_file.assign(svrs_file_path);
    }
    
    SysConf::instance()->load_conf(svrs_file, 1);     
    return 0;
}

int ISGWApp::init_stat()
{
    char path_env_name[256];
    snprintf(path_env_name, sizeof(path_env_name), "%s", "ISGW_STAT");
    
    char* path_env = ACE_OS::getenv(path_env_name);
    if (path_env == NULL)
    {
        ACE_DEBUG((LM_INFO, "[%D] env %s not set, use default working path\n", path_env_name));
        path_env = "./";
    }

    string stat_file = program_;
    string stat_path = path_env;
    stat_file += ".stat";
    stat_file = stat_path + "/" + stat_file;

    return Stat::instance()->init(stat_file.c_str());
}

int ISGWApp::init_reactor()
{
    
    #if ((ACE_MAJOR_VERSION > 5 || (ACE_MAJOR_VERSION==5 && ACE_MINOR_VERSION>=6)) && defined (ACE_HAS_EVENT_POLL) )
    
    ACE_DEBUG((LM_INFO, "[%D] ISGWApp use dev poll reactor, ACE_FD_SETSIZE=%d\n", ACE_FD_SETSIZE));
    //ACE5.6 之上的版本使用EPOLL 
    
    //如果实现高并发,这里就不 需要限制FD的最大值
    //ACE默认是使用系统限制的最大限制值
    //dev_poll_reactor 默认定义的timer池大小44个,因为isgw并不需要太多timer
    //所以也可以不预先申请太大的timer池
    //ACE_Timer_Queue          *timer_queue;
    //timer_queue = new ACE_Timer_Heap(MAX_FD_SETSIZE);
    //timer_queue->gettimeofday(ACE_High_Res_Timer::gettimeofday_hr); // 替换时间获取的函数 
    //ACE_Dev_Poll_Reactor *dp_reactor = new ACE_Dev_Poll_Reactor(MAX_FD_SETSIZE, 0, NULL, timer_queue);
    ACE_Dev_Poll_Reactor *dp_reactor = new ACE_Dev_Poll_Reactor();
    int ret = dp_reactor->initialized();
    //ACE 这个畜牲，居然用 true 表示成功,害得我调试半天(sailzeng 语录)
    if (ret != true )
    {
        ACE_DEBUG((LM_ERROR,"ISGWApp init ACE_Dev_Poll_Reactor failed, errno:%u|%m, ret=%d\n", ACE_OS::last_error(), ret));
        return ret;
    }
    ACE_Reactor::instance(new  ACE_Reactor(dp_reactor,1),1);
    
    #else
    
    ACE_DEBUG((LM_INFO, "[%D] ISGWApp use select reactor, ACE_FD_SETSIZE=%d\n", ACE_FD_SETSIZE));
    ACE_Reactor::instance(new ACE_Reactor(new ACE_Select_Reactor_N()));
    
    #endif
    
    return 0;
}

int ISGWApp::init_app(int , ACE_TCHAR* [])
{
    ACE_DEBUG((LM_INFO, "[%D] %s init ...\n", program_));
    int ret = 0;

    //统计模块初始化
    ret = init_stat();
    if (ret != 0 )
    {
        ACE_DEBUG((LM_ERROR, "[%D] ISGWApp init stat failed,go on.\n"));
    }
    else
    {
        ACE_DEBUG(( LM_INFO, "[%D] ISGWApp init stat succ\n" ));
    }
    
    //消息响应类
    int ack_interval = 0;
    SysConf::instance()->get_conf_int("system", "ack_interval", &ack_interval);
    ISGWAck::instance()->init(ack_interval);
    
    ACE_Object_Que<PriProReq>::instance()->init(OBJECT_QUEUE_SIZE);
    ACE_Object_Que<PriProAck>::instance()->init(OBJECT_QUEUE_SIZE);

    // modify by awayfang 先初始化内部类 再启动监听模块 
    ret = ISGWMgrSvc::instance()->init();
    if (ret != 0 )
    {
        ACE_DEBUG((LM_ERROR, "[%D] ISGWApp init ISGWMgrSvc failed,program exit.\n"));
        return -1;
    }
    ACE_DEBUG(( LM_INFO, "[%D] ISGWApp init ISGWMgrSvc succ\n" ));
    
#ifdef ISGW_USE_IBC
    int ibc_svc_flag = 0;
    SysConf::instance()->get_conf_int("system", "ibc_svc_flag", &ibc_svc_flag);
    if(ibc_svc_flag != 0)
    {
        ret = IBCMgrSvc::instance()->init();
        if (ret != 0 )
        {
            ACE_DEBUG((LM_ERROR, "[%D] ISGWApp init IBCMgrSvc failed,program exit.\n"));
            return -1;
        }
        ACE_DEBUG(( LM_INFO, "[%D] ISGWApp init IBCMgrSvc succ\n" ));        
    }
#endif 
    
    int port;
    char ip[16];    
    string ipstr;
    
    int tcp_flag = 1; //默认使用tcp
    SysConf::instance()->get_conf_int("system", "tcp_flag", &tcp_flag);
    if (tcp_flag == 1)
    {
        memset(ip, 0x0, sizeof(ip));
        // initialize isgw tcp listening 
        SysConf::instance()->get_conf_int("system", "port", &port);
        SysConf::instance()->get_conf_str("system", "ip", ip, sizeof(ip));
        // 配置获取不到则直接获取本机 eth1 的 ip 
        if (strlen(ip) == 0)
        {
            ACE_DEBUG((LM_ERROR, "[%D] ISGWIntf get cfg ip failed,get eth1 ip\n"));
            ipstr = get_local_ip("eth1");
            snprintf(ip, sizeof(ip), "%s", ipstr.c_str());
        }
        
        ISGW_ACCEPTOR* isgw_acceptor = new ISGW_ACCEPTOR();
        if (isgw_acceptor->open(ACE_INET_Addr(port, ip),
            ACE_Reactor::instance(), ACE_NONBLOCK) != 0)
        {
        	ACE_DEBUG((LM_ERROR, "[%D] ISGWIntf open listen failed"
        		",ip=%s,port=%d,program exit.\n"
        		, ip
        		, port
        		));
        	return -1;
        }
        ACE_DEBUG((LM_INFO, "[%D] ISGWApp tcp listen succ,ip=%s,port=%d\n", ip, port));
    }
    
    int udp_flag = 0; //默认不使用udp
    SysConf::instance()->get_conf_int("system", "udp_flag", &udp_flag);
    if (udp_flag == 1)
    {
        memset(ip, 0x0, sizeof(ip));
        // initialize isgw udp listening 
        SysConf::instance()->get_conf_int("system", "udp_port", &port);
        SysConf::instance()->get_conf_str("system", "udp_ip", ip, sizeof(ip)); 
        // 配置获取不到则直接获取本机 eth1 的 ip 
        if (strlen(ip) == 0)
        {
            ACE_DEBUG((LM_ERROR, "[%D] ISGWUIntf get cfg ip failed,get eth1 ip\n"));
            ipstr = get_local_ip("eth1");
            snprintf(ip, sizeof(ip), "%s", ipstr.c_str());
        }
        
        ACE_INET_Addr local_addr(port,ip);                
        if (ISGWUIntf::instance()->open(local_addr) != 0)
        {
            ACE_DEBUG((LM_ERROR, "[%D] ISGWUIntf open listen failed"
        		",ip=%s,port=%d,program exit.\n"
        		, ip
        		, port
        		));
            return -1;
        }
        ACE_DEBUG((LM_INFO, "[%D] ISGWApp udp listen succ,ip=%s,port=%d\n", ip, port));
    }

    ISGWCIntf::init(); // 框架路由模块默认会用到
#ifdef ISGW_USE_ASY //ISGW_USE_CINTF
    ret = ASYTask::instance()->init();
    if (ret != 0 )
    {
        ACE_DEBUG((LM_ERROR, "[%D] ISGWApp init ASYTask failed,program exit.\n"));
        return -1;
    }
    ACE_DEBUG(( LM_INFO, "[%D] ISGWApp init ASYTask succ\n" ));
#endif
    
    //取退出时等待ISGWMgrSvc处理完消息的最长时间，单位秒
    SysConf::instance()->get_conf_int("system", "max_wait_mgr_time", (int*)&max_wait_mgr_time_);
    
    //初始化信号处理器，为了配置重加载的功能
    ISGWSighdl::instance();
    
    ACE_DEBUG((LM_INFO, "[%D] %s init succ ...\n", program_));
    return 0;
}

int ISGWApp::quit_app()
{
    //等待ISGWMgrSvc处理完所有消息
    unsigned int times = 0;
    while(times < max_wait_mgr_time_)
    {
        size_t count = ISGWMgrSvc::instance()->message_count();
        ACE_DEBUG((LM_INFO, "[%D] ISGWApp::quit_app wait ISGWMgrSvc exit,message count=%u\n", count));
        if(count == 0)
        {
            //退出前再等待1s,以便正在处理的消息可以被处理完
            ACE_OS::sleep(1);
            break;
        }
        ACE_OS::sleep(1);
    }
    
    //等待业务的其他线程退出
    IsgwOperBase::instance()->wait_task();
    
    ACE_DEBUG((LM_INFO, "[%D] %s quit\n", program_));
    return 0;
}

void ISGWApp::disp_version()
{
    ACE_DEBUG((LM_INFO, "**********************************************\n"));
    ACE_DEBUG((LM_INFO, "* ISGW Module Name: %s\n", program_));
    ACE_DEBUG((LM_INFO, "* ISGW Module Version: %s\n", SYS_MODULE_VERSION));
    ACE_DEBUG((LM_INFO, "* ISGW Module Build Time: %s %s\n", __DATE__, __TIME__));
    ACE_DEBUG((LM_INFO, "**********************************************\n"));
}
