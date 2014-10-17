/******************************************************************************
* �첽���ӹ����� plat_conn_mgr_asy.h 
* send ʱ ֻ�����ҵ���ص����� sockfd �� sockseq ����Ϣ�ŵ� ISGWAck ���м���
* recv ʱ ȡ����Ķ����е���Ϣ ���ظ��ϲ��� 
* Ӧ��ģ����Ҫ�Լ�ƥ����յ�����Ϣ �������ĸ� ǰ��Ӧ�õ� 
* ʹ��ʱ �� ��������� ��̬�� ��ʹ�� ȷ���ܱ������������״̬ 
******************************************************************************/
#ifndef _PLAT_CONN_MGR_ASY_H_
#define _PLAT_CONN_MGR_ASY_H_
#include "easyace_all.h"
#include "isgw_cintf.h"
#include "asy_prot.h"

#define IP_NUM_MAX 20  //��˷����� ip ������ 
#define IP_NUM_DEF 2  //��˷����� ip ȱʡ���� 
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
    // ͨ�����ó�ʼ������ 
    int init(const char *section = NULL); 
    // �ж� intf �Ƿ����� ���� ISGWAck �Ķ��м��� �ɽӿڲ��Լ���������Ϣ  
    int send(const void *buf, int len, const unsigned int uin=0);
//#ifdef ISGW_USE_ASY
    int send(const void *buf, int len, ASYRMsg &rmsg, const unsigned int uin=0);
//#endif 
    
private:
    ///ͨ��ָ��ip��port ��ʼ����index�����ӣ���ָ�������ڲ��� 
    int init_conn(int ip_idx, const char *ip=NULL, int port=0);
    inline int get_ip_idx(const unsigned int uin);
    // �ж������Ƿ����� 
    inline int is_estab(int ip_idx);
    int fini_conn(int ip_idx);
    
private:
    // stream �����������Ϣ 
    stConnInfo conn_info_[IP_NUM_MAX];
    ACE_Thread_Mutex conn_lock_[IP_NUM_MAX]; 
    
    ACE_Thread_Mutex conn_mgr_lock_; // conn mgr ������������������Ӳ���ʱ�ô���

    char section_[MAX_SECTION_NAME_LEN];
    ACE_Time_Value time_out_;
    int ip_num_; // ���õķ������� ip ���� 
 
    char ip_[IP_NUM_MAX][16]; 
    int port_; 
};

#endif //_PLAT_CONN_MGR_ASY_H_
