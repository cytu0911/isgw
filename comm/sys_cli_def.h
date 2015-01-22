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
#define SYS_CONF_PATH_ENV_NAME "SYS_CONF_PATH" //�����ļ����Ŀ¼�Ļ�������
#define SYS_SOCKET_TIMEOUT_ENV_NAME "SYS_SOCKET_TIMEOUT" //���糬ʱʱ�����õĻ�������

//ȱʡֵ����
#define SYS_MAX_BUF_LEN 8192 
#define SYS_MAX_MSG_LEN 8188 
#define SYS_SOCKET_TIMEOUT 5 //ȱʡֵ
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

#define MAX_SERVER_NUM 10 //��󱣴�ķ�������
//�������б���Ϣ���ݽṹ
typedef struct tagSYS_CONF_SERVER
{
    unsigned long server_addr_[MAX_SERVER_NUM];
    short server_port_[MAX_SERVER_NUM];
    short server_status_[MAX_SERVER_NUM];//��¼�����������������ӵĴ���
    int server_num_; //��ǰ���õķ���������
    int current_index_;//��ǰʹ�õ�ip addr index
    int init_flag_;//0 ����ļ���ʼ�� 1 �Ѿ���ʼ��
    char reserved_[10];
}SYS_CONF_SERVER;

#define MAX_SOCKET_NUM 128 //ʹ�ñ�api���ܴ��������������
typedef struct tagSYS_SOCKET_INFO
{
    int index;
    SYS_SOCKET socket_fd;
    int sock_time_begin;
}SYS_SOCKET_INFO;

extern char sys_conf_path[256];
extern SYS_CONF_SERVER sys_server_conf;

// ����ͨ����غ���������������ģʽ�����ӣ����ͣ�����ָ���ĳ������ݣ������������й����
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

// ϵͳ comm ��������
int SYS_get_pid();
int SYS_thr_self();
int SYS_get_errno();

// �������б���ع���������
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
