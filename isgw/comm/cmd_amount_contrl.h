//=============================================================================
/**
 *  @file    cmd_amount_contrl.cpp
 *
 *  ���ļ�Ϊ��Ϊ��ʵ���������ƵĹ���
 *  
 *  @author ianyuan
 */
//=============================================================================

#ifndef _CMD_AMOUNT_CONTRL_H_
#define _CMD_AMOUNT_CONTRL_H_

#include "easyace_all.h"
#include "ace/RW_Thread_Mutex.h"
#include "pp_prot.h"

class CmdAmntCntrl
{
public:
    typedef struct CmdStaticsNode
    {
        CmdStaticsNode()
        {
            status_lock_ = NULL;
            statiscs_lock_ = NULL;
            cmd = 0;
            status = 0;
            total_req = 0;
            total_fail_req = 0;
            time_intvl = 0;
            time_start = 0;
            cur_req = 0;
            cur_fail_req = 0;
            cur_fail_ratio = 0;
            max_req = 0;
            max_req_limit = 0;
            max_fail_ratio = 0;
            max_fail_ratio_limit = 0;
        }
        
        ACE_Thread_Mutex *status_lock_;     //ͬ������״̬�Ļ�����
        ACE_Thread_Mutex *statiscs_lock_;       //ͬ������ͳ�����ݵĻ�����
        
        int cmd;        //������
        int status;     //����״̬��0������1��ǰ������������2ֹͣ�ṩ����
        int total_req;          //��������
        int total_fail_req;           //����ʧ�ܵĴ���

        //�������������Ϣ
        int time_intvl;         //������ڣ���λ��
        unsigned int time_start;        //��ǰ���ڵ����
        
        int cur_req;      //��ǰ���ڵ�������
        int cur_fail_req;       //��ǰ���ڵ�ʧ�ܷ�����
        int cur_fail_ratio;     //��ǰ���ڵ�����ʧ����
        
        int max_req;           //����ķ�ֵ
        int max_req_limit;           //������������ֵ
        int max_fail_ratio;         //�������ʧ���ʣ����ڸ�ֵ��ܾ�����
        int max_fail_ratio_limit;       //ʧ���ʵ����ֵ
        
    };

    CmdAmntCntrl();
    CmdAmntCntrl(char *config_item);
    ~CmdAmntCntrl();

    int init_config(char *config_item);
    int get_status(int cmd_no, unsigned int now_t);
    void set_status(int cmd_no, int status);
    void set_time(unsigned int now_t);
    void amount_inc(int cmd_no, int result);
    void get_statiscs(int cmd_no, char*out_info, int len);
    
private:
    CmdStaticsNode* nodes_;
    int cmd_no_start_;      //��ʼ������
    int cmd_no_end_;         //ĩβ��������
    unsigned int time_now_;              //��ǰ��ʱ��
    
};

#endif // _ISGW_OPER_BASE_H_
