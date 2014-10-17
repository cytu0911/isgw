/******************************************************************************
*  @file      ibc_oper_fac.h
*  @author awayfang <awayfang@tencent.com>
*  @history 
*  isgw 框架的 批量处理模块 
*  
******************************************************************************/
#ifndef _IBC_OPER_FAC_H_
#define _IBC_OPER_FAC_H_
#include "ibc_oper_base.h"

class IBCOperFac
{
public:
    static IBCOperBase* create_oper(int cmd);
private:
};

#endif //_IBC_OPER_FAC_H_
