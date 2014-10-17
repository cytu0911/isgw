#include "qmode_msg.h"
#include <cstdio>
#include <cstdlib>

#ifdef BINARY_PROTOCOL
// QModeMsg在二进制协议下, 只是在body里面携带了二进制内容
// cmd uin等字段未解析
QModeMsg::QModeMsg(const int len, const char* body, unsigned int sock_fd, unsigned int sock_seq
    , unsigned int msg_seq, unsigned int prot, unsigned int time, unsigned int sock_ip
    , unsigned short port)
    : sock_fd_(sock_fd), sock_seq_(sock_seq), msg_seq_(msg_seq), prot_(prot), time_(time)
    , sock_ip_(sock_ip), sock_port_(port)
{
    memcpy(body_, body, len);
    body_len_ = len;
}

#endif

QModeMsg::QModeMsg(const char* body, unsigned int sock_fd, unsigned int sock_seq
    , unsigned int msg_seq, unsigned int prot, unsigned int time, unsigned int sock_ip
    , unsigned short port)
    : sock_fd_(sock_fd), sock_seq_(sock_seq), msg_seq_(msg_seq), prot_(prot), time_(time)
    , sock_ip_(sock_ip), sock_port_(port)
{
    cmd_ = 0;
    uin_ = 0;
    rflag_ = 0;
    gettimeofday(&tvtime_, NULL);
    //memset(body_, 0x0, sizeof(body_));
    if (body != NULL)
    {
        snprintf(body_, sizeof(body_)-1, "%s", body);
        parse_msg();
    }

    body_len_ = strlen(body_);
}

QModeMsg::QModeMsg(void)
{
    body_len_ = 0;
    sock_fd_ = 0;
    sock_seq_ = 0;
    msg_seq_ = 0;
    prot_ = 0;
    time_ = 0;
    sock_ip_ = 0;
    sock_port_ = 0;
    cmd_ = 0;
    uin_ = 0;
    rflag_ = 0;
    gettimeofday(&tvtime_, NULL);
}

QModeMsg::~QModeMsg()
{
	msg_map_.clear();
}

QMSG_MAP*  QModeMsg::get_map()
{
    return &msg_map_;
}

const char* QModeMsg::get_body() const
{
    return body_;
}

void QModeMsg::set_body(char* body)
{
    //memset(body_, 0x0, sizeof(body_));
    if (body != NULL)
    {
        snprintf(body_, sizeof(body_)-1, "%s", body);
        parse_msg();
    }
}

void QModeMsg::set(char* body, unsigned int sock_fd, unsigned int sock_seq
                   , unsigned int msg_seq, unsigned int prot, unsigned int time
                   , unsigned int sock_ip, unsigned short port)
{
    //memset(body_, 0x0, sizeof(body_));
    if (body != NULL)
    {
        snprintf(body_, sizeof(body_)-1, "%s", body);
        parse_msg();
    }
    
    sock_fd_ = sock_fd;
    sock_seq_ = sock_seq;
    msg_seq_ = msg_seq;
    prot_ = prot;
    time_ = time;
    sock_ip_ = sock_ip;
    sock_port_ = port;
    body_len_ = strlen(body_);
}

unsigned int QModeMsg::get_cmd()
{
    return cmd_;
}

unsigned int QModeMsg::get_uin()
{
    return uin_;
}

unsigned int QModeMsg::get_rflag()
{
    return rflag_;
}

int QModeMsg::get_result()
{
    char* p = strstr(body_, FIELD_NAME_RESULT); //用 strcasestr 可以忽略大小写
    if (p == NULL)
    {
        return ERROR_NO_FIELD;
    }
    
	return atoi(((msg_map_)[FIELD_NAME_RESULT]).c_str());
}

// 解析消息 
int QModeMsg::parse_msg()
{
    msg_map_.clear();
    //去掉请求结尾的\r字符,避免body拼接的问题
    int len = strlen(body_);
    if(len>0&&'\r'==body_[len-1]) body_[len-1] = '\0';
    //临时内存保存body，防止body被破坏
    char tmp_body[QMSG_MAX_LEN+1];
    //memset(tmp_body, 0x0, sizeof(tmp_body));
    snprintf(tmp_body, sizeof(tmp_body)-1, "%s", body_);
    
    char name[QMSG_NAME_LEN];
    char* ptr = NULL;
    char* p = strtok_r(tmp_body, QMSG_SEP, &ptr);
	while (p != NULL) 
	{
	    char* s = strchr(p, '=');
	    if (s != NULL)
	    {
	        int name_len = s - p > sizeof(name)-1 ? sizeof(name)-1 : s - p;            
	        snprintf(name, name_len+1, "%s", p);
	        msg_map_[name] = s + 1;
	    }
	    p = strtok_r(NULL, QMSG_SEP, &ptr);
	}

    // 对内置变量赋值 
    cmd_ = atoi(((msg_map_)[FIELD_NAME_CMD]).c_str());
    uin_ = strtoul(((msg_map_)[FIELD_NAME_UIN]).c_str(), NULL, 10);//atoi(((msg_map_)[FIELD_NAME_UIN]).c_str());
    rflag_ = atoi(((msg_map_)["_rflag"]).c_str());
    
    return 0;
}

unsigned int QModeMsg::get_handle()
{
    return sock_fd_;
}

unsigned int QModeMsg::get_sock_seq()
{
    return sock_seq_;
}

unsigned int QModeMsg::get_msg_seq()
{
    return msg_seq_;
}

unsigned int QModeMsg::get_prot()
{
    return prot_;
}

unsigned int QModeMsg::get_time()
{
    return time_;
}

unsigned int QModeMsg::get_sock_ip()
{
    return sock_ip_;
}

unsigned short QModeMsg::get_sock_port()
{
    return sock_port_;
}

unsigned int QModeMsg::get_body_size()
{
	return body_len_;
}

//简单判断是否含有某个字段 : & filename = 是相对固定的，
//但是中间可能存在空格，所以简单处理，直接判断是否有字符串
int QModeMsg::is_have(const char * field_name)
{
    if (field_name == NULL)
    {
        return ERROR_PARA_ILL;
    }
    
    char* p = strstr(body_, field_name); //用 strcasestr 可以忽略大小写
    if (p==NULL)
    {
        return -1;
    }
    
    return 0;
}

