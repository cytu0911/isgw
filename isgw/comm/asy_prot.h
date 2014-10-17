//=============================================================================
/**
 *  @file    asy_prot.h
 *
 *  此文件为后端异步处理模块需要的协议定义 
 *  通过 ASYRKey 来关联
 *  
 *  @author awayfang
 */
//=============================================================================

#ifndef _ASY_PROT_H_
#define _ASY_PROT_H_
#include "easyace_all.h"
#include "qmode_msg.h"

/// Э������ص�����ָ��
//typedef int (*ASY_PARSE)(PriProAck *ack);

/// �߼������ص�����ָ��
typedef int (*ASY_PROC)(QModeMsg &ack, string& content, char* ack_buf, int& ack_len);

///数据结构 key 部分 本地连接的相关信息
typedef struct stASYRKey
{
    uint32 sock_fd; //socket 的文件描述符    
    uint32 sock_seq; //socket的序列号，sock_fd值相同时可以用来区分
    uint32 msg_seq; //消息的序列号，唯一标识该连接上的一个请求
}ASYRKey;

///数据结构 value 部分 
typedef struct stASYRValue
{    
    uint32 time;  // ��¼��Ϣ��ʱ�� �����ж��Ƿ�ʱ
    // �ص�����ָ��
    ASY_PROC asy_proc;
    // �������ڱ��浱ǰ������������
    string content;

    stASYRValue()
    {
        time = 0;
        asy_proc = NULL;
        //memset(content, 0x0, sizeof(content));
    }
}ASYRValue;

typedef struct stASYRMsg
{
   ASYRKey key;
   ASYRValue value;
}ASYRMsg;

/// ASYRKey 比较函数 
class ASYR_COMP
{
public:
    bool operator() (const ASYRKey& x, const ASYRKey& y) const
    {
        // sock_fd,sock_seq,msg_seq 相同的情况返回 false 确保作为唯一主键 
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
        
        return x.msg_seq > y.msg_seq; // 从大到小，因为大的比较新，删除从尾部删除    
    }
};

///存储结果的 map 
typedef map<ASYRKey, ASYRValue, ASYR_COMP> ASYR_MAP;

#endif // _ASY_PROT_H_
