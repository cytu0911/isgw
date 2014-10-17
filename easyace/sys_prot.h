/******************************************************************************
*  @file      sys_prot.h
*  @author awayfang
*  @history 
*  通用的编解码函数，对c++ 的普通数据类型进行网络编解码
*  
******************************************************************************/
#ifndef _SYS_PROT_H_
#define _SYS_PROT_H_

#ifdef TRACE
#include "iostream"
using namespace std;
#endif

typedef unsigned long ULONG;
typedef unsigned int UINT;
typedef unsigned short USHORT;
typedef unsigned char UCHAR;

#define uint32 unsigned int

int SYS_prot_encode_char(char *buffer, int length, int &pos, char value);
int SYS_prot_encode_int(char *buffer, int length, int &pos, int value);
int SYS_prot_encode_uint32(char *buffer, int length, int &pos, uint32 value);
int SYS_prot_encode_string(char *buffer, int length, int &pos, char *value, int val_len);
int SYS_prot_encode_string_withhead(char *buffer, int length, int &pos, char *value);
int SYS_prot_decode_char(char *buffer, int length, int &pos, char &value);
int SYS_prot_decode_int(char *buffer, int length, int &pos, int &value);
int SYS_prot_decode_uint32(char *buffer, int length, int &pos, uint32 &value);
int SYS_prot_decode_string(char *buffer, int length, int &pos, char *value, int val_len);
int SYS_prot_decode_string_withhead(char *buffer, int length, int &pos, char *value, int val_len);

int SYS_prot_encode_short(char *buffer, int length, int &pos, short value);
int SYS_prot_decode_short(char *buffer, int length, int &pos, short &value);
int SYS_prot_encode_ushort(char *buffer, int length, int &pos, USHORT value);
int SYS_prot_decode_ushort(char *buffer, int length, int &pos, USHORT &value);

#endif //#_SYS_PROT_H_
