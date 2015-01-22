#ifndef _SYS_CLI_DEF_H_
#define _SYS_CLI_DEF_H_
#ifndef WIN32
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>
#else
#include "winsock2.h"
#include <process.h>
#endif

#include <cstdlib>
#include <cstdio>
#include <time.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
using namespace std;

#include "sys_guard.h"
#include "sys_thread_mutex.h"
#include "sys_cli_err.h"

#define SYS_CONF_SRV_FILE "sys_svr.conf"
#define SYS_CONF_PATH_ENV_NAME "SYS_CONF_PATH" //配置文件存放目录的环境变量
#define SYS_SOCKET_TIMEOUT_ENV_NAME "SYS_SOCKET_TIMEOUT" //网络超时时间设置的环境变量

//缺省值定义
#define SYS_MAX_BUF_LEN 8192 
#define SYS_MAX_MSG_LEN 8188 
#define SYS_SOCKET_TIMEOUT 5 //缺省值
//path 
#ifndef WIN32
#define SYS_CONF_SRV_PATH "/etc/sys/" 
#define SYS_CONF_KEY_PATH "/etc/sys/" 
#else
#define SYS_CONF_SRV_PATH "c:\\sys\\" 
#define SYS_CONF_KEY_PATH "c:\\sys\\" 
#endif

//tcp socket
#ifndef WIN32
typedef int SYS_HANDLE;
typedef SYS_HANDLE SYS_SOCKET;
#define SYS_INVALID_HANDLE -1 
#define SYS_INVALID_SOCKET -1 
#define SYS_SOCKET_ERROR -1 
#define port_t  in_port_t
#define addr_t  in_addr_t
#else
typedef HANDLE SYS_HANDLE;
typedef SOCKET SYS_SOCKET;
#define SYS_INVALID_HANDLE INVALID_HANDLE_VALUE
#define SYS_INVALID_SOCKET INVALID_SOCKET
#define EINPROGRESS WSAEINPROGRESS
#define EWOULDBLOCK WSAEWOULDBLOCK
#define socklen_t int 
#define SYS_SOCKET_ERROR SOCKET_ERROR 
#define port_t  u_short
#define addr_t  u_long

class WSocketInit
{
public:
    WSocketInit()
    {
        int ret = 0;
        WSADATA wsaData;
        ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
#ifdef TRACE
        if ( ret != 0 )
        {
            cout <<"in WSocketInit(), WSAStartup failed, ret="<<ret<<endl;
        }
        else
        {
            cout <<"in WSocketInit(), WSAStartup ok"<<endl;
        }
#endif
    }
    
    ~WSocketInit()
    {
        WSACleanup();
    }
};
#endif

#define MAX_SERVER_NUM 10 //最大保存的服务器数
//服务器列表信息数据结构
typedef struct tagSYS_CONF_SERVER
{
    unsigned long server_addr_[MAX_SERVER_NUM];
    short server_port_[MAX_SERVER_NUM];
    short server_status_[MAX_SERVER_NUM];//记录服务器连续不可连接的次数
    int server_num_; //当前配置的服务器总数
    int current_index_;//当前使用的ip addr index
    int init_flag_;//0 需从文件初始化 1 已经初始化
    char reserved_[10];
}SYS_CONF_SERVER;

#define MAX_SOCKET_NUM 128 //使用本api所能创建的最大连接数
typedef struct tagSYS_SOCKET_INFO
{
    int index;
    SYS_SOCKET socket_fd;
    int sock_time_begin;
}SYS_SOCKET_INFO;

extern char sys_conf_path[256];
extern SYS_CONF_SERVER sys_server_conf;

// 网络通信相关函数，创建非阻塞模式的连接，发送，接受指定的长度数据，对网络句柄进行管理等
int SYS_connect(SYS_SOCKET& sock_fd, SYS_CONF_SERVER& server_conf
	, char* error);
int SYS_send_n(SYS_SOCKET sock_fd, const char* buf, int len, char* error);
int SYS_recv_n(SYS_SOCKET sock_fd, char* buf, int len, char* error);

int SYS_get_socket_env();
int SYS_socket_list_init();
int SYS_get_sock(SYS_SOCKET& sock_fd);
int SYS_close(SYS_SOCKET sock_fd);
int SYS_close_all_socket();
bool SYS_is_sock_valid();

// 系统 comm 函数定义
int SYS_get_pid();
int SYS_thr_self();
int SYS_get_errno();

// 服务器列表相关管理函数定义
int SYS_read_svrconf(SYS_CONF_SERVER* conf, char* error);
int SYS_save_svrstatus(const int index = 0, const int count = 0
	, SYS_CONF_SERVER &server_conf = sys_server_conf);
int SYS_get_svraddr(addr_t &addr, port_t &port, SYS_CONF_SERVER &server_conf
	, int index = 0);	

class SystemInit
{
public:
    SystemInit()
    {
        SYS_get_socket_env();
        SYS_read_svrconf(NULL, error_);
        SYS_socket_list_init();        
#ifdef TRACE
        cout<<"system init finished"<<endl;
#endif
    }
    ~SystemInit()
    {
        SYS_close_all_socket();
    }
private:
    char error_[256];
};

#endif // _SYS_CLI_DEF_H_ 
