//=============================================================================
/**
 *  @file    pdb_oper.h
 *
 *  此文件为pdb内部保留的业务操作接口定义文件
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
* @param req 入参，请求消息
* @param ack 出参，存放返回的响应消息
* @param ack_len 入参兼出参，传入为ack指向的最大空间，
*                          传出为ack实际存放的消息长度
*
* @retval 0 成功
* @retval 其他值表示不 成功
*/
    int process(QModeMsg& req, char* ack, int& ack_len);
    
private:
    
private:
        
};

#endif //_PDB_OPER_
