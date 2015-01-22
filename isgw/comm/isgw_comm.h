#ifndef _ISGW_COMM_H_
#define _ISGW_COMM_H_

#include "easyace_all.h"
#include "qmode_msg.h"
#include "pp_prot.h"

namespace EASY_UTIL //easy ace �����ռ�
{

typedef struct stSOCKET_INFO
{
    unsigned int index; //����
    unsigned int sock_fd; //
    unsigned int sock_seq; //
    unsigned int sock_ip;
    unsigned int creat_time; //���Ӳ�����ʱ��
    int status; //0 ����ʹ�� 1 ���ڱ�ʹ����
}SOCKET_INFO;

///���������ַ�
char *prot_encode(char *dest, const char *src);
///���������ַ�
char *prot_strim(char *dest, const char *src);
// �ж��Ƿ��ǺϷ���uin���Ϸ���uin >10000 <int
int is_valid_uin(unsigned int uin);
unsigned int get_time();
unsigned int get_span(struct timeval *tv1, struct timeval *tv2);

};

#endif //_ISGW_COMM_H_
