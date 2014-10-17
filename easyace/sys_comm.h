#ifndef _SYS_COMM_H_
#define _SYS_COMM_H_

#include "ace/LOCK_SOCK_Acceptor.h"
#include "ace/Auto_Ptr.h"
#include "ace/Reactor.h"
#include "ace/Thread_Manager.h"
#include "ace/Synch.h"
#include "ace/Get_Opt.h"
#include "ace/Logging_Strategy.h"
#include "ace/Log_Msg.h"
#include "ace/Service_Object.h"
#include "ace/svc_export.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/SOCK_Connector.h"
#include "ace/Date_Time.h"
#include "ace/Time_Value.h"
#include "ace/Atomic_Op_T.h"
#include "ace/Configuration.h"
#include "ace/Configuration_Import_Export.h"
#include "ace/SOCK_Connector.h"
#include "ace/SOCK_Dgram.h"
#include "ace/Mem_Map.h"
#include "ace/Svc_Handler.h"
#include "ace/Acceptor.h"
#include "ace/Connector.h"
#include "ace/TP_Reactor.h"
#include "ace/Singleton.h"
#include "ace/Service_Config.h"
#include "ace/ARGV.h"
#include "ace/Reactor_Notification_Strategy.h"
#include "ace/OS_NS_sys_msg.h"
#include "ace/OS_NS_arpa_inet.h"
//#include "ace/OS.h"
#include "ace/Basic_Types.h"
#include "ace/Version.h"
#include "ace/OS_NS_dlfcn.h"
#include "ace/DLL.h"
#include "ace/DLL_Manager.h"

#if ((ACE_MAJOR_VERSION > 5 || (ACE_MAJOR_VERSION==5 && ACE_MINOR_VERSION>=6)) && defined (ACE_HAS_EVENT_POLL) )
#include "ace/Dev_Poll_Reactor.h"
#endif

#include <errno.h>
#include <signal.h>

#include <string>
#include <fstream>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <deque>
#include <iomanip>
using namespace std;

#ifdef WIN32
#pragma warning(disable:4503)

#define snprintf _snprintf
#endif // WIN32

#if ((ACE_MAJOR_VERSION > 5 || (ACE_MAJOR_VERSION==5 && ACE_MINOR_VERSION>=6)) && defined (ACE_HAS_EVENT_POLL) )
typedef ACE_Reactor_Token_T<ACE_Noop_Token> ACE_Select_Reactor_Noop_Token;
typedef ACE_Select_Reactor_T<ACE_Select_Reactor_Noop_Token> ACE_Select_Reactor_N;
#else
typedef ACE_Select_Reactor_Token_T<ACE_Noop_Token> ACE_Select_Reactor_Noop_Token;
typedef ACE_Select_Reactor_T<ACE_Select_Reactor_Noop_Token> ACE_Select_Reactor_N;
#endif

extern char oi_symmetry_decrypt2(const char* pInBuf, int nInBufLen,
                                 const char* pKey, char* pOutBuf, int *pOutBufLen);
extern void oi_symmetry_encrypt2(const char* pInBuf, int nInBufLen,
                                 const char* pKey, char* pOutBuf, int *pOutBufLen);

namespace EASY_UTIL
{
typedef std::map<std::string, std::string> IPAddrMap;

void str2hex(const char* str, unsigned char* hex, int len);
void hex2str(unsigned char* data, int len, char* str);

void str2upper(const char* src, char* dest);
void str2lower(const char* src, char* dest);

enum
{
    OISIGN_LEN = 64
};

int auth_oiserver_sign(const char* oisign, const char* key,
                       unsigned int uin, int tmlimit);

typedef map<string, string> CGI_PARAM_MAP;

/// 这种解析方法会对 str 造成修改(破坏) 请使用人员注意 
int parse(char* str, CGI_PARAM_MAP& dest, const char minor='='
    , const char* major="&\r\n");

int parse(const string &src, CGI_PARAM_MAP &dest
    , const char minor='=', const char major='&');

int parse(const string &src, map<int, int> &dest
    , const char minor='=', const char major='&');

/// 分割字符串src，存入vector dest 里面 
void split(const string& src, const string& delim, vector<string>& dest);
void split(const string& src, const string& delim, vector<unsigned int>& dest);

// 分割后放入set中(即去重) 并且会去掉空白的内容
void split_ign(const string& src, const string& delim, set<string>& dest);
void split_ign(const string& src, const string& delim, set<unsigned int>& dest);
void split_ign(const string& src, const string& delim, set<int>& dest);
// 此版本会忽略分隔符之间空白的内容 一般并列的内容的时候可以忽略 比如uin列表 都是 uin
void split_ign(const string& src, const string& delim, vector<string>& dest);
void split_ign(const string& src, const string& delim, vector<unsigned int>& dest);
/// 返回从 1970-01-01 到现在的天数 
int  days(unsigned int unix_time=0);

char* get_time_str(char *time_str);
string get_local_ip(const string& if_name);
IPAddrMap get_local_ip_map();

}

#endif // _SYS_COMM_H_ 

