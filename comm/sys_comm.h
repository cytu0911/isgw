#ifndef _SYS_COMM_H_
#define _SYS_COMM_H_

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>

#include <string>
#include <fstream>
#include <iostream>
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

/// ���ֽ���������� str ����޸�(�ƻ�) ��ʹ����Աע�� 
int parse(char* str, CGI_PARAM_MAP& dest, const char minor='='
    , const char* major="&\r\n");

int parse(const string &src, CGI_PARAM_MAP &dest
    , const char minor='=', const char major='&');

int parse(const string &src, map<int, int> &dest
    , const char minor='=', const char major='&');

/// �ָ��ַ���src������vector dest ���� 
void split(const string& src, const string& delim, vector<string>& dest);
void split(const string& src, const string& delim, vector<unsigned int>& dest);

// �ָ�����set��(��ȥ��) ���һ�ȥ���հ׵�����
void split_ign(const string& src, const string& delim, set<string>& dest);
void split_ign(const string& src, const string& delim, set<unsigned int>& dest);
void split_ign(const string& src, const string& delim, set<int>& dest);
// �˰汾����Էָ���֮��հ׵����� һ�㲢�е����ݵ�ʱ����Ժ��� ����uin�б� ���� uin
void split_ign(const string& src, const string& delim, vector<string>& dest);
void split_ign(const string& src, const string& delim, vector<unsigned int>& dest);
/// ���ش� 1970-01-01 �����ڵ����� 
int  days(unsigned int unix_time=0);

char* get_time_str(char *time_str);
string get_local_ip(const string& if_name);
IPAddrMap get_local_ip_map();

}

#endif // _SYS_COMM_H_ 

