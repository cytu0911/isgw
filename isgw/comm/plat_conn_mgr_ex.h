/******************************************************************************
* SOCKET���ӳع�����(��չ)��������Ԫ�ĸ���(һ�����������߸�С�ļ���)�������ӹ���
* �����ӳص������������������Ͼ��жԵȵ��߼����ܣ�û�в��
* Ϊ�˱�֤�����ӣ����ڵ��ýӿ���ʵ��Ϊ��̬�������ȫ�ֶ���
* 
* history:
* ....       init awayfang
* 2009-11-20 1 ���ӱ������ӹ��ܣ�ÿ�����ӹ�������֧�����ñ��� ip
*            2 �Ż�����ֱ�Ӱ�ip�Ͷ˿���Ϣ������ĳ�Ա����
*            3 �Ż����ӳع���֧���̺߳����Ӳ��󶨣����ǲ��ҿ��е����ӣ����suse�汾������ 
*            4 ����send_recv�ӿڣ�����ͬ��ģʽ�е�����recvʹ�õ�������ʹ�ô��ҵ�����
* 2009-12-29 1 �������ӹ�����ԵĿ��ƣ�֧��ָ��ʧ�ܴ���������ʱ�� 
* 2010-03-02 1 �޸� PlatConnMgr ������֮ǰ�������������ݵ�����
*            2 ���� PlatConnMgr ����� send_recv_ex ��չ�ӿ� 
* 2010-05-04 1 ��չΪ PlatConnMgrEx �� ֧�ֶ��ip����
*            2 �����ݴ���� uin���ȷ��� ���� ���·��(����uin)
*            3 ���ĳ��ip�����⣬��ʹ����������һ��ip������
* 2010-12-01 1 ����ʱ֧�ֶ�����Э������� ����ʱֻ�����ı�Э�� 
******************************************************************************/
#ifndef _PLAT_CONN_MGR_EX_H_
#define _PLAT_CONN_MGR_EX_H_
#include "easyace_all.h"

#define IP_NUM_MAX 10  //��˷����� ip ������ 
#define IP_NUM_DEF 2  //��˷����� ip ȱʡ���� 
#define POOL_CONN_DEF 10 //ȱʡÿ��ip 10������
#define POOL_CONN_MAX 50  //���ӳص� ip ��������������
#define MAX_ERROR_LEN 128 
#define MAX_SECTION_NAME_LEN 32 
#define SOCKET_TIME_OUT 150       //��λ��ms
#define DEF_USE_STRATEGY 1
#define DEF_MAX_FAIL_TIMES 5
#define DEF_RECON_INTERVAL 10

//�����Ƿ��������
#ifndef GUARD_DOG_WATCH 
#define GUARD_DOG_WATCH(latch) ACE_Guard<ACE_Thread_Mutex> guard(latch)
#endif

class PlatConnMgrEx
{
public:
    PlatConnMgrEx();
    PlatConnMgrEx(const char*host_ip, int port, int conn_num);
    ~PlatConnMgrEx();
    ///ͨ�����ó�ʼ����������
    int init(const char *section = NULL, const std::vector<std::string> *ip_vec = NULL); 
    int send (const void *buf, int len, const unsigned int uin=0);
    int send (const void *buf, int len, const std::string& ip);
    //����ָ�����ȵ����ݣ����ҽ�����Ҫ�����ݣ����ȫ���ɹ�����ֵΪ���յ��ĳ��� 
    int send_recv (const void *send_buf, int send_len, void *recv_buf
        , int recv_len, const unsigned int uin=0);
	//����ָ���������ķ��ͺͽ�������
    int send_recv_ex (const void *send_buf, int send_len, void *recv_buf
        , int recv_len, const char* separator=NULL, const unsigned int uin=0);
    // �ֲ��� recv ʹ��ֻ�������첽ģʽ����Ϊ���ݵķ��ͺͽ��տ����ڲ�ͬ������ 
    int recv (void *buf, int len, const unsigned int uin=0);
    
private:
    //�����ӳػ�ȡһ������ ע�� ip_idx ���п��ܻᱻ�޸ĵ� 
    ACE_SOCK_Stream* get_conn(int index, int & ip_idx);
    
    //�����ӳػ�ȡһ������ ֱ�ӷ��� һ��
    ACE_Thread_Mutex& get_conn(unsigned int uin, ACE_SOCK_Stream *& conn);
    ///ͨ��ָ��ip��port ��ʼ����index�����ӣ���ָ�������ڲ��� 
    int init_conn(int index, int ip_idx, const char *ip = NULL, int port = 0); 
    int fini(int index, int ip_idx); //��ֹ��index������ 
    unsigned int get_index(int ip_idx, unsigned uin=0); //������ӵ��±� 
    inline int get_ip_idx(unsigned int uin=0);
    inline int get_ip_idx(const std::string& ip);
    
private:
    // ���Ӽ�״̬�����Ϣ 
    ACE_SOCK_Stream* conn_[IP_NUM_MAX][POOL_CONN_MAX]; //����ָ�� 
    int conn_use_flag_[IP_NUM_MAX][POOL_CONN_MAX]; //���ӵ�ʹ��״̬��־
    ACE_Thread_Mutex conn_lock_[IP_NUM_MAX][POOL_CONN_MAX]; //������һһ��Ӧ��֤�����߳�ʹ������
    //ACE_Thread_Mutex init_lock_[IP_NUM_MAX][POOL_CONN_MAX]; //��֤��ʼ�������� 

    ACE_Thread_Mutex conn_mgr_lock_; // conn mgr ������������������Ӳ���ʱ�ô���
    char section_[MAX_SECTION_NAME_LEN]; 
    int conn_nums_; //ʵ�ʵ��� ip ����������
    ACE_Time_Value time_out_;
    int ip_num_; // ���õķ������� ip ���� 

    //��������״̬�����Ϣ 
    int fail_times_[IP_NUM_MAX]; //����ʵ������ʧ�ܴ������������ͣ����գ���
    int last_fail_time_[IP_NUM_MAX]; //���ʧ��ʱ��     
    char ip_[IP_NUM_MAX][INET6_ADDRSTRLEN]; 
    int port_; 
    
    //��������
    int use_strategy_; //����ʹ�ñ�־ 
    int max_fail_times_; //�����������ʧ�ܴ������������ͣ����գ���
    int recon_interval_; //�������

};

#endif //_PLAT_CONN_MGR_EX_H_
