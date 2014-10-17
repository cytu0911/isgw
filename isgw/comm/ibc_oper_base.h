//=============================================================================
/**
 *  @file    ibc_oper_base.h
 *
 *  ���ļ�Ϊ�ڲ���������ҵ���߼��Ļ�����
 *  
 *  @author awayfang
 */
//=============================================================================

#ifndef _IBC_OPER_BASE_H_
#define _IBC_OPER_BASE_H_
#include "easyace_all.h"
#include "qmode_msg.h"
#include "ibc_prot.h"

class IBCOperBase
{
public:
    IBCOperBase() {};
    virtual ~IBCOperBase() {}; 
    virtual int process(QModeMsg& req, char* ack, int& ack_len); 
    //���Ϊ����ϲ����߼�������Ҫ���ķǳ������� ����������߳���Ӱ�� 
    virtual int merge(IBCRValue& rvalue, const char* new_item); 

	//�ϲ�������ջص���ע�����ֻ����ȫ������������ʱ��������һ��
	virtual int end(IBCRValue& rvalue); 

private:

};

#endif // _IBC_OPER_BASE_H_
