#include "sys_comm.h"
#include "sys_conf.h"

SysConf* SysConf::instance_ = NULL;
SysConf* SysConf::instance()
{
    if (instance_ == NULL)
    {
        instance_ = new SysConf();
    }
    return instance_;
}

SysConf::SysConf()
{
    ace_config_ = NULL;
}

SysConf::~SysConf()
{
    release();
}

int SysConf::release()
{
    if (ace_config_ != NULL)
    {
        delete ace_config_;
        ace_config_ = NULL;
    }    
}

int SysConf::load_conf(const string& conf_file, int flag)
{
    ACE_Guard<ACE_Thread_Mutex> guard(conf_lock_);
    
    //�����ļ���Դ�ͷ�
    release();
    
    if (conf_file.length() > 0)
    {
        old_conf_file_ = conf_file_;
        conf_file_ = conf_file;
    }
    
    ACE_TString ace_str;    
    ace_config_ = new ACE_Configuration_Heap();
    ace_config_->open();
    ACE_Ini_ImpExp conf_imp(*ace_config_);
    int ret = conf_imp.import_config(conf_file_.c_str());
    if (ret != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] open config %s failed,ret=%d\n"
            , conf_file_.c_str(), ret));
        return -1;
    }
    
    ACE_Configuration_Section_Key section;
    ACE_Configuration::VALUETYPE value_type;
    ACE_TString section_name;
    ACE_TString config_name;
    ACE_TString config_value;
    CONFIG_MAP conf_map;    //������

    for (int j = 0; ; j++)
    {
        if (ace_config_->enumerate_sections(ace_config_->root_section(),
            j, section_name) != 0)
        {
            break;
        }

        if (ace_config_->open_section(ace_config_->root_section(),
            section_name.c_str(), 0, section) != 0)
        {
            ACE_DEBUG((LM_ERROR, "[%D] open %s section failed\n",
                       section_name.c_str()));
            return -1;
        }

        ACE_DEBUG((LM_TRACE, "[%D] reading section: %s\n",
                   section_name.c_str()));

        SECTION_CONF_MAP section_conf;
        for (int i = 0; ; i++)
        {
            if (ace_config_->enumerate_values(section,
                i, config_name, value_type) != 0)
            {
                break;
            }

            if (ace_config_->get_string_value(section, config_name.c_str(),
                config_value) != 0)
            {
                ACE_DEBUG((LM_ERROR, "[%D] read config: %d failed\n",
                           config_name.c_str()));
                return -1;
            }

            section_conf[config_name.c_str()] = config_value.c_str();

            ACE_DEBUG((LM_TRACE, "[%D] config:%s=%s\n",
                       config_name.c_str(), config_value.c_str()));
        }
        conf_map[section_name.c_str()] = section_conf;
    }

    
    if (flag == 1) // ��������
    {
        CONFIG_MAP::const_iterator iter;
        for(iter=conf_map.begin(); iter!=conf_map.end(); iter++)
        {
            conf_items_[iter->first] = iter->second;
        }
    }
    else //������ǰ������
    {
        conf_items_.clear();
        conf_items_ = conf_map;
    }
    
    ACE_DEBUG((LM_INFO, "[%D] system config load succ,conf_file=%s,flag=%d\n"
        , conf_file.c_str(), flag));
    return 0;
}

int SysConf::get_conf_int(const char* section, const char* name, int* value)
{
    ACE_Guard<ACE_Thread_Mutex> guard(conf_lock_);
    CONFIG_MAP::const_iterator iter = conf_items_.find(section);
    if (iter != conf_items_.end())
    {
        SECTION_CONF_MAP::const_iterator it = iter->second.find(name);
        if (it != iter->second.end())
        {
            *value = atoi(it->second.c_str());
            return 0;
        }
    }

    return -1;
}

int SysConf::get_conf_uint(const char* section, const char* name, uint* value)
{
    ACE_Guard<ACE_Thread_Mutex> guard(conf_lock_);
    CONFIG_MAP::const_iterator iter = conf_items_.find(section);
    if (iter != conf_items_.end())
    {
        SECTION_CONF_MAP::const_iterator it = iter->second.find(name);
        if (it != iter->second.end())
        {
            *value = atoll(it->second.c_str());
            return 0;
        }
    }

    return -1;
}

int SysConf::get_conf_str(const char* section, const char* name,
                          char* value, int buf_len)
{
    ACE_Guard<ACE_Thread_Mutex> guard(conf_lock_);
    CONFIG_MAP::const_iterator iter = conf_items_.find(section);
    if (iter != conf_items_.end())
    {
        SECTION_CONF_MAP::const_iterator it = iter->second.find(name);
        if (it != iter->second.end())
        {
            strncpy(value, it->second.c_str(), buf_len);
            value[buf_len - 1] = '\0';
            return 0;
        }
    }

    return -1;
}

int SysConf::write_conf_int(const char* section, const char* name, int value)
{
    char value_str[11];
    sprintf(value_str, "%d", value);

    return write_conf_str(section, name, value_str);
}

// ֻ��д��һ�����ص��ļ� conf_file_ 
int SysConf::write_conf_str(const char* section, const char* name, char* value)
{
    ACE_Guard<ACE_Thread_Mutex> guard(conf_lock_);
    conf_items_[section][name] = value;

/*
    ACE_Configuration_Section_Key section_key;
    ace_config_->open_section(ace_config_->root_section(),
                              section, 1, section_key);
    ace_config_->set_string_value(section_key, name, value);

    ACE_Ini_ImpExp conf_imp(*ace_config_);
    if (conf_imp.export_config(conf_file_.c_str()) != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] write config failed: [%s] %s = %s\n",
                   section, name, value));
        return -1;
    }

    ACE_DEBUG((LM_TRACE, "[%D] write config succ: [%s] %s = %s\n",
               section, name, value));
*/
    return 0;
}

int SysConf::write_svr_conf(const std::string& svr_file)
{
    ACE_Guard<ACE_Thread_Mutex> guard(conf_lock_);
    std::ofstream ofs;
    if(svr_file.empty())
    {
        // ֻ������һ������, ��д
        if(old_conf_file_.empty())
        {
            return 0;
        }
        // δָ���ļ�, ��д�뵽���һ�μ��ص������ļ�
        else
        {
            ofs.open(conf_file_.c_str(), std::ios_base::out | std::ios_base::trunc);
        }
    }
    // д�뵽ָ���ļ�
    else
    {
        ofs.open(svr_file.c_str(), std::ios_base::out | std::ios_base::trunc);
    }
    
    if(!ofs)
    {
        return -1;
    }

    for(CONFIG_MAP::const_iterator cit = conf_items_.begin(); 
        cit != conf_items_.end(); ++cit)
    {
        const std::string& section = cit->first;
        std::string::size_type pos = section.find("_svr");
        if(pos == std::string::npos)
        {
            continue;
        }
            
        ofs<<"["<<section<<"]"<<std::endl;
        for(SECTION_CONF_MAP::const_iterator cit2 = cit->second.begin();
            cit2 != cit->second.end(); ++cit2)
        {
            ofs<<cit2->first<<" = "<<cit2->second<<std::endl;
        }
        ofs<<std::endl;
    }
    ofs.close();
    
    return 0;
}



