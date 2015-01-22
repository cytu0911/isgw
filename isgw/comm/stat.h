/********************************************************************************
  Copyright (C), 2008-2018
  FileName: stat.h
  Author: jinglinma awayfang              Date: 2009-12-10
  Description:
    ����ͳ��ģ�飬֧�ְ�ͳ����Ϣ��¼��mem_map�У�ͳ�������˵������:
    1-10000 Ϊ��Ӧ��cmdָ��������ؽ��>=0�����ɹ������������� 
    10001-10240  ��������ҵ���Զ����ͳ��������쳣���У�
    10241-20240 Ϊ��Ӧ��ָ��������ؽ��<0��Ϊʧ�ܣ����������� 
    ������Ϊ cmd+10240��ƫ������
    20241 - 20480  Ϊ����ڲ����쳣ͳ����� 
********************************************************************************/
#ifndef _STAT_H_
#define _STAT_H_
#include <ace/Mem_Map.h> 
#include <ace/Guard_T.h>
#include <ace/Thread_Mutex.h>
#include <string>

#define USE_RW_THREAD_MUTEX 1

//const int STAT_DESC_MAX_LEN = 60; 
//const int MAX_STAT_INDEX = 20480; //����ͳ����� 10240 ��ʼ 
const int MAX_DESC_MSGLEN = 50; 
const int MAX_STAT_INDEX = 10240;   //����ͳ����� 10240 ��ʼ 
//const int STAT_INDEX_FAIL_OFFSET = 10240; //����ͳ�����ƫ����
const int MAX_ISGW_FAILED_ITEM = 1024;   //isgw��ܵ�������ͳ����

//��������ͳ�ƶ��� 
typedef enum _STAT_CODE
{
    STAT_CODE_SVC_ENQUEUE = 20241, // �ӿ��߳���ӵ������̶߳���ʧ�� 
    STAT_CODE_SVC_TIMEOUT = 20242, // �����̴߳���ʱ������Ϣ��ʱ
    STAT_CODE_SVC_NOMEM = 20243, //  �����̴߳���ʱ�����ڴ�ľ� 
    STAT_CODE_IBCSVC_ENQUEUE = 20244, //�� ibc ��Ϣ����ʧ�� 
    STAT_CODE_SVC_OVERLOAD = 20245, // �����̴߳���ʱ����ϵͳ����
    STAT_CODE_SVC_REJECT = 20246, // ָ����������ʧ�����쳣
    STAT_CODE_IBCSVC_TIMEOUT = 20247, //ibc �����̴߳���ʱ������Ϣ��ʱ
    STAT_CODE_SVC_FORWARD = 20248, //
    STAT_CODE_IBCSVC_FAC = 20249, // ibc fac �쳣
    
    STAT_CODE_PUT_ACK_TIMEOUT = 20250, // �����̷߳�����Ӧģ����г�ʱ(���)
    STAT_CODE_ACK_NOOWNER = 20251, // ��Ӧʱû���ҵ���Ӧ�Ŀͻ���
    STAT_CODE_ACK_DISCONN = 20252, // ������Ϣʱ�Զ˹ر�
    STAT_CODE_ACK_BLOCK = 20253, // ������Ϣʱ�Զ�����
    STAT_CODE_ACK_ABNOR = 20254, // ������Ϣʱ�쳣
    STAT_CODE_ACK_UNCOMP = 20255, // ������Ϣʱ����ȫ
    STAT_CODE_CONN_FAIL = 20256, // ����˵�����ʧ��
    STAT_CODE_SEND_FAIL = 20257, // ������Ϣ�����ʧ��
    STAT_CODE_RECV_FAIL = 20258, // �Ӻ�˽�����Ϣʧ��(ֻ��ͬ���ķ�ʽ֧��) 
    STAT_CODE_DB_CONN_RUNOUT = 20259, //��ǰû�п��õ�DB����
    STAT_CODE_TCP_CONN_RUNOUT = 20260, //��ǰû�п��õ�tcp����

    STAT_CODE_ISGWC_ENQUEUE = 20261, //  isgwcintf ģ����������Ϣ����ʧ��
    STAT_CODE_ASYNC_ENQUEUE = 20262, //  �첽�߳����ʧ��
    
    STAT_CODE_END
}STAT_CODE;

typedef struct ReprtInfo
{
    ReprtInfo(unsigned a=0, int b=0, int c=0, unsigned d=0, unsigned e=0):
    cmd(a),total_count(b),failed_count(c),total_span(d),procss_span(e)
        {}
    unsigned cmd;
    int total_count;
    int failed_count;
    unsigned total_span;
    unsigned procss_span;
    
}ReprtInfo;

//#define offsetof(TYPE,MEMBER)  ((int)&(((TYPE *)0)->MEMBER))

class Stat
{
public:
    ~Stat();
    static Stat* instance(); 
    int init(const char* file_name="./.stat", int flag=0);
    int init_check(const char* file_name="./.stat");
    void incre_stat(int index, int incre=1);
    void add_stat(ReprtInfo *info);
    int* get_stat();
    void reset_stat();
    void get_statstr(const char *stat_cfg, std::string &statstr);
    
private:
    Stat()
    {
        state_ = 0;
        
        //stat�ļ���ǰ4���ֽ���һ��magic����,����check�ļ���ʽ�Ƿ���ȷ
        //�����ǵ���ָ����Ϣͳ��,ÿ��ͳ������Ҫ4��int�ֶ�,�����10240��ͳ����
        //ָ��ͳ����Ϣ������1024������ͳ����,����ͳ�Ƶ�ƫ�ƴ�20241��ʼ
        //ÿ��ͳ������Ҫ1��int�ֶ�
        file_size_ = (sizeof(ReprtInfo)-sizeof(int)) * MAX_STAT_INDEX +1024*sizeof(int) + sizeof(int);
        isgw_failed_offset_ = (sizeof(ReprtInfo)-sizeof(int)) * MAX_STAT_INDEX + sizeof(int);
    }
    
private:
    
    ACE_Mem_Map stat_file_;
    int file_size_;
    int isgw_failed_offset_;
    char file_name_[128];
    static ACE_Thread_Mutex lock_;
    int state_; //�Ƿ��ʼ���ı�־ 
    static Stat* instance_; 
};

#endif /* _STAT_H_ */
