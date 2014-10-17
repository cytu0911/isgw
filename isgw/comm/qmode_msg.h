#ifndef _QMODE_MSG_H_
#define _QMODE_MSG_H_
#include "pp_prot.h"

#include <string>
#include <cstring>
#include <map>
#include <sys/time.h>

using namespace std;

#ifdef MAX_INNER_MSG_LEN 
#define QMSG_MAX_LEN MAX_INNER_MSG_LEN
#else
#define QMSG_MAX_LEN 8400
#endif
#define QMSG_NAME_LEN 64
#define QMSG_VALUE_LEN 8000
#define QMSG_SEP "&\r\n"

#ifndef FIELD_NAME_CMD
#define FIELD_NAME_CMD "cmd"
#endif
#ifndef FIELD_NAME_UIN
#define FIELD_NAME_UIN "uin"
#endif
#ifndef FIELD_NAME_RESULT
#define FIELD_NAME_RESULT "result"
#endif

typedef map<string, string> QMSG_MAP;

class QModeMsg
{
public:
    #ifdef BINARY_PROTOCOL
    //�����ȵ�qmode�����ڶ�����
    QModeMsg(const int len, const char* body, unsigned int sock_fd = 0, unsigned int sock_seq = 0
        , unsigned int msg_seq = 0, unsigned int prot = 0, unsigned int time = 0
        , unsigned int sock_ip = 0, unsigned short sock_port = 0);
    void set_cmd(unsigned cmd) { cmd_ = cmd; }
    #endif

    //�������ȵ�qmode�������ı�
    QModeMsg(const char* body, unsigned int sock_fd = 0, unsigned int sock_seq = 0
        , unsigned int msg_seq = 0, unsigned int prot = 0, unsigned int time = 0
        , unsigned int sock_ip = 0, unsigned short sock_port = 0);
    QModeMsg(void);
    ~QModeMsg();
    QMSG_MAP*  get_map();
    const char* get_body() const;
    void set_body(char* body);
    void set(char* body, unsigned int sock_fd, unsigned int sock_seq
        , unsigned int msg_seq, unsigned int prot, unsigned int time
        , unsigned int sock_ip, unsigned short sock_port);
    unsigned int get_cmd();
    unsigned int get_uin();
    unsigned int get_rflag();
    int get_result();
    unsigned int get_handle();
    unsigned int get_sock_seq();
    unsigned int get_msg_seq();
    unsigned int get_prot();
    unsigned int get_time();
    
    void set_tvtime(struct timeval *tv_time)
    {
        tvtime_ = *tv_time;
    }
    struct timeval * get_tvtime()
    {
        return &tvtime_;
    }
    unsigned int get_sock_ip();
    unsigned short get_sock_port();
    unsigned int get_body_size();
    int is_have(const char *field_name); //���ж��Ƿ���ĳ���ֶ� 
	
private:
	int parse_msg();

private:	
    char body_[QMSG_MAX_LEN+1];
    QMSG_MAP msg_map_;
    unsigned int sock_fd_;
    unsigned int sock_seq_;
    unsigned int msg_seq_;
    unsigned int prot_;
    unsigned int time_;
    unsigned int sock_ip_;
    unsigned short sock_port_;    
    unsigned int cmd_;
    unsigned int uin_;
    unsigned int rflag_; // ��Ϣ�Ƿ�͸���ı�־
    struct timeval tvtime_;
    unsigned int body_len_;  //���峤�ȣ����ڶ�����
};

#endif //_QMODE_MSG_H_
