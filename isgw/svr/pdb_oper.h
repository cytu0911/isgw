//=============================================================================
/**
 *  @file    pdb_oper.h
 *
 *  ���ļ�Ϊpdb�ڲ�������ҵ������ӿڶ����ļ�
 *
 *  @author awayfang
 */
//=============================================================================
#ifndef _PDB_OPER_
#define _PDB_OPER_
#include "isgw_comm.h"
#include "pdb_prot.h"
#include "isgw_oper_base.h"

#include "temp_proxy.h"

class PdbOper : public IsgwOperBase
{
public:
    PdbOper();
    ~PdbOper();

/** @name process
* 
*
* @param req ��Σ�������Ϣ
* @param ack ���Σ���ŷ��ص���Ӧ��Ϣ
* @param ack_len ��μ���Σ�����Ϊackָ������ռ䣬
*                          ����Ϊackʵ�ʴ�ŵ���Ϣ����
*
* @retval 0 �ɹ�
* @retval ����ֵ��ʾ�� �ɹ�
*/
    int process(QModeMsg& req, char* ack, int& ack_len);
    
private:
    
private:
        
};

#endif //_PDB_OPER_
