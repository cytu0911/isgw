//=============================================================================
/**
 *  @file    oper.h
 *
 *  此文件为处理业务逻辑的头文件
 *  
 *  @author awayfang
 */
//=============================================================================

#ifndef _OPER_H_
#define _OPER_H_


extern "C" int oper_init();
extern "C" int oper_proc(char* req, char* ack, int& ack_len);

#endif // _OPER_H_
