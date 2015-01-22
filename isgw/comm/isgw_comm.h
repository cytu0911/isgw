#ifndef _ISGW_COMM_H_
#define _ISGW_COMM_H_

#include "easyace_all.h"
#include "qmode_msg.h"
#include "pp_prot.h"

namespace EASY_UTIL //easy ace 的名空间
{

typedef struct stSOCKET_INFO
{
    unsigned int index; //索引
    unsigned int sock_fd; //
    unsigned int sock_seq; //
    unsigned int sock_ip;
    unsigned int creat_time; //连接产生的时间
    int status; //0 不在使用 1 正在被使用中
}SOCKET_INFO;

///编码特殊字符
char *prot_encode(char *dest, const char *src);
///过滤特殊字符
char *prot_strim(char *dest, const char *src);
// 判断是否是合法的uin，合法的uin >10000 <int
int is_valid_uin(unsigned int uin);
unsigned int get_time();
unsigned int get_span(struct timeval *tv1, struct timeval *tv2);

};

#endif //_ISGW_COMM_H_
