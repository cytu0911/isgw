/********************************************************************************
  Copyright (C), 2008-2018
  FileName: stat.h
  Author: jinglinma awayfang              Date: 2009-12-10
  Description:
    请求统计模块，支持把统计信息记录到mem_map中，统计项规则说明如下:
    1-10000 为对应的cmd指令操作返回结果>=0（即成功）的请求数量 
    10001-10240  可以留给业务自定义的统计项（正常异常都行）
    10241-20240 为对应的指令操作返回结果<0（为失败）的请求数量 
    ，规则为 cmd+10240（偏移量）
    20241 - 20480  为框架内部的异常统计项段 
********************************************************************************/
#ifndef _STAT_H_
#define _STAT_H_
#include <ace/Mem_Map.h> 
#include <ace/Guard_T.h>
#include <ace/Thread_Mutex.h>
#include <string>

#define USE_RW_THREAD_MUTEX 1

//const int STAT_DESC_MAX_LEN = 60; 
//const int MAX_STAT_INDEX = 20480; //错误统计项从 10240 开始 
const int MAX_DESC_MSGLEN = 50; 
const int MAX_STAT_INDEX = 10240;   //错误统计项从 10240 开始 
//const int STAT_INDEX_FAIL_OFFSET = 10240; //错误统计项的偏移量
const int MAX_ISGW_FAILED_ITEM = 1024;   //isgw框架的最大错误统计项

//框架类错误统计定义 
typedef enum _STAT_CODE
{
    STAT_CODE_SVC_ENQUEUE = 20241, // 接口线程入队到工作线程队列失败 
    STAT_CODE_SVC_TIMEOUT = 20242, // 工作线程处理时发现消息超时
    STAT_CODE_SVC_NOMEM = 20243, //  工作线程处理时发现内存耗尽 
    STAT_CODE_IBCSVC_ENQUEUE = 20244, //入 ibc 消息队列失败 
    STAT_CODE_SVC_OVERLOAD = 20245, // 工作线程处理时发现系统过载
    STAT_CODE_SVC_REJECT = 20246, // 指令流量或者失败率异常
    STAT_CODE_IBCSVC_TIMEOUT = 20247, //ibc 工作线程处理时发现消息超时
    STAT_CODE_SVC_FORWARD = 20248, //
    STAT_CODE_IBCSVC_FAC = 20249, // ibc fac 异常
    
    STAT_CODE_PUT_ACK_TIMEOUT = 20250, // 工作线程放入响应模块队列超时(相对)
    STAT_CODE_ACK_NOOWNER = 20251, // 响应时没有找到对应的客户端
    STAT_CODE_ACK_DISCONN = 20252, // 回送消息时对端关闭
    STAT_CODE_ACK_BLOCK = 20253, // 回送消息时对端阻塞
    STAT_CODE_ACK_ABNOR = 20254, // 回送消息时异常
    STAT_CODE_ACK_UNCOMP = 20255, // 回送消息时不完全
    STAT_CODE_CONN_FAIL = 20256, // 跟后端的连接失败
    STAT_CODE_SEND_FAIL = 20257, // 发送消息到后端失败
    STAT_CODE_RECV_FAIL = 20258, // 从后端接收消息失败(只有同步的方式支持) 
    STAT_CODE_DB_CONN_RUNOUT = 20259, //当前没有可用的DB连接
    STAT_CODE_TCP_CONN_RUNOUT = 20260, //当前没有可用的tcp连接

    STAT_CODE_ISGWC_ENQUEUE = 20261, //  isgwcintf 模块入自身消息队列失败
    STAT_CODE_ASYNC_ENQUEUE = 20262, //  异步线程入队失败
    
    STAT_CODE_END
}STAT_CODE;

typedef struct ReprtInfo
{
    ReprtInfo(unsigned a=0, int b=0, int c=0, unsigned d=0, unsigned e=0):
    cmd(a),total_count(b),failed_count(c),total_span(d),procss_span(e)
        {}
    unsigned cmd;
    int total_count;
    int failed_count;
    unsigned total_span;
    unsigned procss_span;
    
}ReprtInfo;

//#define offsetof(TYPE,MEMBER)  ((int)&(((TYPE *)0)->MEMBER))

class Stat
{
public:
    ~Stat();
    static Stat* instance(); 
    int init(const char* file_name="./.stat", int flag=0);
    int init_check(const char* file_name="./.stat");
    void incre_stat(int index, int incre=1);
    void add_stat(ReprtInfo *info);
    int* get_stat();
    void reset_stat();
    void get_statstr(const char *stat_cfg, std::string &statstr);
    
private:
    Stat()
    {
        state_ = 0;
        
        //stat文件的前4个字节是一个magic数字,用来check文件格式是否正确
        //后面是单个指令信息统计,每个统计项需要4个int字段,最多有10240个统计项
        //指令统计信息后面是1024个错误统计项,错误统计的偏移从20241开始
        //每个统计项需要1个int字段
        file_size_ = (sizeof(ReprtInfo)-sizeof(int)) * MAX_STAT_INDEX +1024*sizeof(int) + sizeof(int);
        isgw_failed_offset_ = (sizeof(ReprtInfo)-sizeof(int)) * MAX_STAT_INDEX + sizeof(int);
    }
    
private:
    
    ACE_Mem_Map stat_file_;
    int file_size_;
    int isgw_failed_offset_;
    char file_name_[128];
    static ACE_Thread_Mutex lock_;
    int state_; //是否初始化的标志 
    static Stat* instance_; 
};

#endif /* _STAT_H_ */
