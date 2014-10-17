//=============================================================================
/**
 *  @file    ibc_oper_base.h
 *
 *  此文件为内部批量处理业务逻辑的基础类
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
    //这个为结果合并的逻辑，建议要做的非常轻量级 避免对其他线程有影响 
    virtual int merge(IBCRValue& rvalue, const char* new_item); 

	//合并后的最终回调，注意这个只会在全部结果处理完的时候最后调用一次
	virtual int end(IBCRValue& rvalue); 

private:

};

#endif // _IBC_OPER_BASE_H_
