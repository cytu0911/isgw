//=============================================================================
/**
 *  @file    oper.h
 *
 *  ���ļ�Ϊ����ҵ���߼���ͷ�ļ�
 *  
 *  @author awayfang
 */
//=============================================================================

#ifndef _OPER_H_
#define _OPER_H_


extern "C" int oper_init();
extern "C" int oper_proc(char* req, char* ack, int& ack_len);

#endif // _OPER_H_
