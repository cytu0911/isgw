#ifndef _ACE_CONF_H_
#define _ACE_CONF_H_
#include "ace_all.h"
#include <string>
#include <map>
#include <fstream>
using namespace std;

typedef map<string, string> SECTION_CONF_MAP;
typedef map<string, SECTION_CONF_MAP> CONFIG_MAP;

class ACEConf
{
public:
    string get_conf_file(){return conf_file_;};
    // 扩展支持多个文件加载配置 
    int load_conf(const string& conf_file="", int flag=0);
    int get_conf_int(const char* section, const char* name, int* value);
    int get_conf_uint(const char* section, const char* name, uint* value);
    int get_conf_str(const char* section, const char* name,
        char* value, int buf_len);

    int write_conf_str(const char* section, const char* name, char* value);
    int write_conf_int(const char* section, const char* name, int value);
    int write_svr_conf(const std::string& svr_file = "");

    static ACEConf* instance();
    ~ACEConf();
    int release();
private:
    ACEConf();
    
    CONFIG_MAP conf_items_;
    ACE_Thread_Mutex conf_lock_;

    ACE_Configuration_Heap* ace_config_;

    string conf_file_;
    string old_conf_file_;

    static ACEConf* instance_;
};

typedef ACEConf SysConf; //兼容历史名称 

#endif // _ACE_CONF_H_

