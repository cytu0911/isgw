//=============================================================================
/**
 *  @file    isgw_oper_base.h
 *
 *  ���ļ�Ϊ����ҵ���߼��Ļ�����
 *  
 *  @author awayfang
 */
//=============================================================================

#ifndef _ISGW_OPER_BASE_H_
#define _ISGW_OPER_BASE_H_
#include "easyace_all.h"
#include "qmode_msg.h"
#include "cmd_amount_contrl.h"
#include "isgw_mgr_svc.h"

#ifndef FIELD_NAME_SVC
#define FIELD_NAME_SVC "service"
#endif

//��Ϣ����ָ���
typedef enum _PDB_BASE_CMD
{
    // 100 �����ڲ����� 
    CMD_TEST = 1, //�ڲ�ѹ��������ָ�� 

    CMD_SELF_TEST = 2,					// ҵ���Բ�����
    CMD_GET_SERVER_VERSION = 3,		// ��ȡ��ǰsvr�İ汾��
    CMD_SYS_LOAD_CONF = 10,             //���¼���������Ϣ 
    CMD_SYS_GET_CONTRL_STAT = 11,       //��ѯ��������״̬
    CMD_SYS_SET_CONTRL_STAT = 12,       //��������״̬
    CMD_SYS_SET_CONTRL_SWITCH = 13,       //����ʱ�����������ƹ��ܿ���
    
    CMD_PDB_BASE_END
};

class IsgwOperBase
{
protected:
    static IsgwOperBase* oper_;
    
public:
    IsgwOperBase();
    virtual ~IsgwOperBase();
    
    static IsgwOperBase* instance(IsgwOperBase* oper);
    static IsgwOperBase* instance();
    static int  init_auth_cfg();//����Ȩ�޿����������ȡ

    virtual int init();
    virtual int process(QModeMsg& req, char* ack, int& ack_len);
    virtual int reload_config();
    virtual int time_out();
    // wait task
    virtual int wait_task();

    int internal_process(QModeMsg& req, char* ack, int& ack_len);
    int is_auth(QModeMsg& req, char* ack, int& ack_len);

private:
	int self_test(int testlevel, std::string& msg);
	
private:
    // ���������Ʊ�־��Ϊ1���������ִ���
    static int cmd_auth_flag_;
    // ��֧���������б�
    static std::map<int, int> cmd_auth_map_;
    // ҵ�����Ʊ�־��Ϊ1����ҵ����
    static int svc_auth_flag_;
    // ��֧��ҵ���б�
    static std::map<int, int> svc_auth_map_;
    
};

#endif // _ISGW_OPER_BASE_H_
