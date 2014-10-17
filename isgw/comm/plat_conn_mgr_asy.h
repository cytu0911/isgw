/******************************************************************************
* 异步连接管理类 plat_conn_mgr_asy.h 
* send 时 只负责找到相关的连接 sockfd 及 sockseq 把消息放到 ISGWAck 队列即可
* recv 时 取本身的队列中的消息 返回给上层用 
* 应用模块需要自己匹配接收到的消息 是属于哪个 前端应用的 
* 使用时 请 生成自身的 静态类 来使用 确保能保存自身的连接状态 
******************************************************************************/
#ifndef _PLAT_CONN_MGR_ASY_H_
#define _PLAT_CONN_MGR_ASY_H_
#include "easyace_all.h"
#include "isgw_cintf.h"
#include "asy_prot.h"

#define IP_NUM_MAX 20  //后端服务器 ip 最大个数 
#define IP_NUM_DEF 2  //后端服务器 ip 缺省个数 
#define MAX_SECTION_NAME_LEN 32 
#define SOCKET_TIME_OUT 1 

struct stConnInfo
{
    ISGWCIntf* intf;
    uint32 sock_fd;
    uint32 sock_seq;
};

class PlatConnMgrAsy
{
public:
    PlatConnMgrAsy();
    PlatConnMgrAsy(const char*host_ip, int port);
    ~PlatConnMgrAsy();
    // 通过配置初始化连接 
    int init(const char *section = NULL); 
    // 判断 intf 是否正常 放入 ISGWAck 的队列即可 由接口层自己负责发送消息  
    int send(const void *buf, int len, const unsigned int uin=0);
//#ifdef ISGW_USE_ASY
    int send(const void *buf, int len, ASYRMsg &rmsg, const unsigned int uin=0);
//#endif 
    
private:
    ///通过指定ip和port 初始化第index个连接，不指定则用内部的 
    int init_conn(int ip_idx, const char *ip=NULL, int port=0);
    inline int get_ip_idx(const unsigned int uin);
    // 判断连接是否正常 
    inline int is_estab(int ip_idx);
    int fini_conn(int ip_idx);
    
private:
    // stream 处理器句柄信息 
    stConnInfo conn_info_[IP_NUM_MAX];
    ACE_Thread_Mutex conn_lock_[IP_NUM_MAX]; 
    
    ACE_Thread_Mutex conn_mgr_lock_; // conn mgr 本身的锁，对所有连接操作时用此锁

    char section_[MAX_SECTION_NAME_LEN];
    ACE_Time_Value time_out_;
    int ip_num_; // 配置的服务器的 ip 总数 
 
    char ip_[IP_NUM_MAX][16]; 
    int port_; 
};

#endif //_PLAT_CONN_MGR_ASY_H_
