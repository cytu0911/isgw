//=============================================================================
/**
 *  @file    ibc_prot.h
 *
 *  ���ļ�Ϊ�ڲ���������ģ����Ҫ��Э�鶨�� 
 *  ��������ָ��ͨ���ֽ�֮��ӵ�й�ͬ�� IBCRKey ͨ�����������
 *  ��ʱֻ֧�� tcp Э�� 
 *  
 *  @author awayfang
 */
//=============================================================================

#ifndef _IBC_PORT_H_
#define _IBC_PORT_H_
#include "easyace_all.h"
#include "qmode_msg.h"
#include "pp_prot.h"

///�ڲ����������� ���ݽṹ key ����
typedef struct stIBCRKey
{
    uint32 sock_fd; //socket ���ļ�������    
    uint32 sock_seq; //socket�����кţ�sock_fdֵ��ͬʱ������������
    uint32 msg_seq; //��Ϣ�����кţ�Ψһ��ʶһ������
}IBCRKey;

///�ڲ����������� ���ݽṹ value ���� 
typedef struct stIBCRValue
{
    //ACE_Thread_Mutex lock; //ֻ�л�ô������ܶԴ˽ṹ���в��� 
    uint32 time; 
    uint32 cmd; 
    uint32 uin; 
    uint32 total; // �ܵ���Ҫ����Ĺ�����¼�� 
    uint32 cnum; // ��ǰ�Ѿ�����Ĺ�����¼�� 
    uint32 snum; //  ����ɹ��Ĺ�����¼�� 
    uint32 msg_len; //�������Ч��Ϣ����
    uint32 sock_fd_; //���´� _ ����ЩΪ͸������Ϣ 
    uint32 sock_seq_;
    uint32 msg_seq_;
    uint32 prot_;
    uint32 time_;
    int rflag_;
    int endflag_; // �Ƿ����end �ص� 
    char msg[MAX_INNER_MSG_LEN+1]; //���ܵ�����Ϣ��
    std::list<std::string> msg_list; // ���ܵ���Ϣ��
    uint32 msg_num; // ��Ϣ���¼��

	stIBCRValue()
	{
		time = 0;
		cmd = 0;
		uin = 0;
		total = 0;
		cnum = 0;
		snum = 0;
		msg_len = 0;
		sock_fd_ = 0;
		sock_seq_ = 0;
		msg_seq_ = 0;
		prot_ = 0;
		rflag_ = 0;
		endflag_ = 0;
		msg_num = 0;
		memset(msg, 0x0, sizeof(msg));
		msg_list.clear();
		msg_num = 0;
	}
}IBCRValue;

///�ڲ����������� ���ݽṹ 
typedef struct stIBCRMsg
{
   IBCRKey key;
   IBCRValue value;
}IBCRMsg;

/// IBCRKey �ȽϺ��� 
class IBCR_COMP
{
public:
    bool operator() (const IBCRKey& x, const IBCRKey& y) const
    {
        // sock_fd,sock_seq,msg_seq ��ͬ��������� false ȷ����ΪΨһ���� 
        if (x.sock_fd == y.sock_fd 
        && x.sock_seq == y.sock_seq 
        && x.msg_seq == y.msg_seq 
        )
        {
            return false;
        }
    	
        if (x.msg_seq == y.msg_seq) 
        {
            return true;
        }
    	
        return x.msg_seq > y.msg_seq; // �Ӵ�С����Ϊ��ıȽ��£�ɾ����β��ɾ��    
    }
};

///�洢����� map 
typedef map<IBCRKey, IBCRValue, IBCR_COMP> IBCR_MAP;

#endif // _IBC_PORT_H_
