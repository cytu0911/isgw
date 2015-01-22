#include "sys_comm.h"

#define DIG2CHR(dig) (((dig) <= 0x09) ? ('0' + (dig)) : ('a' + (dig) - 0x0a))
#define CHR2DIG(chr) (((chr) >= '0' && (chr) <= '9') ? ((chr) - '0') : (((chr) >= 'a' && (chr) <= 'f') ? ((chr) - 'a' + 0x0a) : ((chr) - 'A' +0x0a)))

void EASY_UTIL::str2hex(const char* str, unsigned char* hex, int len)
{
    int i = 0;
    for (; i < len; i ++)
    {
        hex[i] = ((CHR2DIG(str[i * 2]) << 4) & 0xf0) + CHR2DIG(str[i * 2 + 1]);
    }
}

void EASY_UTIL::hex2str(unsigned char* data, int len, char* str)
{
    int i = 0;
    for ( ; i < len; i++)
    {
        str[i * 2] = DIG2CHR((data[i] >> 4) & 0x0f);
        str[i * 2 + 1] = DIG2CHR((data[i]) & 0x0f);
    }
    str[len * 2] = '\0';
}

void EASY_UTIL::str2upper(const char* src, char* dest)
{
    while (*src)
    {
        if (*src >= 'a' && *src <= 'z')
        {
            *dest++ = *src - 32;
        }
        else
        {
            *dest++ = *src;
        }

        src ++;
    }

    *dest = '\0';
}

void EASY_UTIL::str2lower(const char* src, char* dest)
{
    while (*src)
    {
        if (*src >= 'A' && *src <= 'Z')
        {
            *dest++ = *src + 32;
        }
        else
        {
            *dest++ = *src;
        }

        src ++;
    }

    *dest = '\0';
}

int EASY_UTIL::auth_oiserver_sign(const char* oisign, const char* key,
                                  unsigned int uin, int tmlimit)
{
    char oisign_h[100];
    str2hex(oisign, (unsigned char*)oisign_h, OISIGN_LEN / 2);

    char decrypt_buf[100];
    memset(decrypt_buf, 0, sizeof(decrypt_buf));
    int  decrypt_len = sizeof(decrypt_buf);

    if (!::oi_symmetry_decrypt2(oisign_h, OISIGN_LEN / 2,
                              key, decrypt_buf, &decrypt_len))
    {
        return -1;
    }

    int uin_offset = 0;
    int time_offset = 4;

    unsigned int uin_n = 0;
    memcpy(&uin_n, &decrypt_buf[uin_offset], sizeof(uin_n));
    uin_n = ntohl(uin_n);
    if (uin_n != uin)
    {
        return -1;
    }

    time_t key_time = 0;
    memcpy(&key_time, &decrypt_buf[time_offset], sizeof(key_time));
    key_time = ntohl(key_time);
    if (time(NULL) - key_time > tmlimit)
    {
        return -1;
    }

    return 0;
}

int EASY_UTIL::parse(char* str, CGI_PARAM_MAP& dest, const char minor
    , const char* major)
{
    char name[256];
    char value[4096];

    char* ptr = NULL;
    char* p = strtok_r(str, major, &ptr);
    while (p != NULL) 
    {
        memset(name, 0x0, sizeof(name));
        memset(value, 0x0, sizeof(value));

        char* s = strchr(p, minor);
        if (s != NULL)
        {
            int name_len = s - p > sizeof(name)-1 ? sizeof(name)-1 : s - p;
            strncpy(name, p, name_len);
            strcpy(value, s + 1);
            dest[name] = value;
        }
        p = strtok_r(NULL, major, &ptr);
    }

    return 0;
}

int EASY_UTIL::parse(const string &src, CGI_PARAM_MAP &dest
    , const char minor, const char major)
{
    //std::string input = contentLength;
    std::string name, value;
    std::string::size_type pos;
    std::string::size_type oldPos = 0;

    // Parse the input in one fell swoop for efficiency
    while(true) 
    {
        // Find the '=' separating the name from its value
        pos = src.find_first_of(minor, oldPos);

        // If no '=', we're finished
        if(std::string::npos == pos)
        {
            break;
        }

        // Decode the name
        name = src.substr(oldPos, pos - oldPos);
        oldPos = ++pos;
        
        // Find the '&' separating subsequent name/value pairs
        pos = src.find_first_of(major, oldPos);

        // Even if an '&' wasn't found the rest of the string is a value
        value = src.substr(oldPos, pos - oldPos);

        // Store the pair
        dest[name] = value;
        if(std::string::npos == pos)
        {
            break;
        }

        // Update parse position
        oldPos = ++pos;
    }
    
    return 0;
}

int EASY_UTIL::parse(const string &src, map<int, int> &dest
    , const char minor, const char major)
{
    //std::string input = contentLength;
    std::string name, value;
    std::string::size_type pos;
    std::string::size_type oldPos = 0;

    // Parse the input in one fell swoop for efficiency
    while(true) 
    {
        // Find the '=' separating the name from its value
        pos = src.find_first_of(minor, oldPos);

        // If no '=', we're finished
        if(std::string::npos == pos)
        {
            break;
        }

        // Decode the name
        name = src.substr(oldPos, pos - oldPos);
        oldPos = ++pos;
        
        // Find the '&' separating subsequent name/value pairs
        pos = src.find_first_of(major, oldPos);

        // Even if an '&' wasn't found the rest of the string is a value
        value = src.substr(oldPos, pos - oldPos);

        // Store the pair
        dest[atoi(name.c_str())] = atoi(value.c_str());
        if(std::string::npos == pos)
        {
            break;
        }

        // Update parse position
        oldPos = ++pos;
    }
    
    return 0;
}


///分割字符串
void EASY_UTIL::split(const string& src, const string& delim, vector<string>& dest)
{
    int pos1 = 0, pos2 = 0;
    while (true)
    {
        pos2 = src.find_first_of(delim, pos1);
        if (pos2 == -1)
        {
            dest.push_back(src.substr(pos1));
            break;
        }
        else
        {
            dest.push_back(src.substr(pos1, pos2-pos1));
        }
        pos1 = pos2 + 1;
    }
}

///分割字符串
void EASY_UTIL::split(const string& src, const string& delim, vector<unsigned int>& dest)
{
    int pos1 = 0, pos2 = 0;
    int len = src.length();
    
    while (true&&pos1<len)
    {
        pos2 = src.find_first_of(delim, pos1);
        if (pos2 == -1)
        {
            dest.push_back(atoll(src.substr(pos1).c_str()));
            break;
        }
        else if(pos2>pos1)
        {
            dest.push_back(atoll(src.substr(pos1, pos2-pos1).c_str()));
        }
        pos1 = pos2 + 1;
    }
}

/// 分割并去重 
void EASY_UTIL::split_ign(const string& src, const string& delim, set<string>& dest)
{
    int pos1 = 0, pos2 = 0;
    while (true)
    {
        pos1 = src.find_first_not_of(delim, pos2);
        if (pos1 == -1) break;
        
        pos2 = src.find_first_of(delim, pos1);
        if (pos2 == -1)
        {
            dest.insert(src.substr(pos1));
            break;
        }
        else
        {
            dest.insert(src.substr(pos1, pos2-pos1));
        }
    }
}

void EASY_UTIL::split_ign(const string& src, const string& delim, set<unsigned int>& dest)
{
    int pos1 = 0, pos2 = 0;
    while (true)
    {
        pos1 = src.find_first_not_of(delim, pos2);
        if (pos1 == -1) break;
        
        pos2 = src.find_first_of(delim, pos1);
        if (pos2 == -1)
        {
            dest.insert(atoll(src.substr(pos1).c_str()));
            break;
        }
        else
        {
            dest.insert(atoll(src.substr(pos1, pos2-pos1).c_str()));
        }
    }
}

/// 分割并去重 
void EASY_UTIL::split_ign(const string& src, const string& delim, set<int>& dest)
{
    int pos1 = 0, pos2 = 0;
    while (true)
    {
        pos1 = src.find_first_not_of(delim, pos2);
        if (pos1 == -1) break;
        
        pos2 = src.find_first_of(delim, pos1);
        if (pos2 == -1)
        {
            dest.insert(atoi(src.substr(pos1).c_str()));
            break;
        }
        else
        {
            dest.insert(atoi(src.substr(pos1, pos2-pos1).c_str()));
        }
    }
}


///分割字符串
void EASY_UTIL::split_ign(const string& src, const string& delim, vector<string>& dest)
{
    int pos1 = 0, pos2 = 0;
    while (true)
    {
        pos1 = src.find_first_not_of(delim, pos2);
        if (pos1 == -1) break;
        
        pos2 = src.find_first_of(delim, pos1);
        if (pos2 == -1)
        {
            dest.push_back(src.substr(pos1));
            break;
        }
        else
        {
            dest.push_back(src.substr(pos1, pos2-pos1));
        }
    }
}

///分割字符串
void EASY_UTIL::split_ign(const string& src, const string& delim, vector<unsigned int>& dest)
{
    int pos1 = 0, pos2 = 0;
    while (true)
    {
        pos1 = src.find_first_not_of(delim, pos2);
        if (pos1 == -1) break;
        
        pos2 = src.find_first_of(delim, pos1);
        if (pos2 == -1)
        {
            dest.push_back(atoll(src.substr(pos1).c_str()));
            break;
        }
        else
        {
            dest.push_back(atoll(src.substr(pos1, pos2-pos1).c_str()));
        }
    }
}

int EASY_UTIL::days(unsigned int unix_time)
{
    if(unix_time == 0)
    {
        unix_time = time(0); // 没传则获取当前时间
    }
    
    return unix_time/(24*3600);
}

char* EASY_UTIL::get_time_str(char* time_str)
{
    time_t t;
    t = time(0);
    tm ltm;
    memset(&ltm, 0x0, sizeof(ltm));
    strftime(time_str, 20, "%Y-%m-%d %H:%M:%S", localtime_r(&t, &ltm));
    return time_str;
}

string EASY_UTIL::get_local_ip(const string& if_name)
{
    IPAddrMap ipaddr_map = get_local_ip_map();
    if ( ipaddr_map.count(if_name) == 0 )
    {
        return string("");
    }
    return ipaddr_map[if_name];
}

EASY_UTIL::IPAddrMap EASY_UTIL::get_local_ip_map()
{
    static IPAddrMap ipaddr_map;
    if ( ipaddr_map.empty() ) 
    {
        const int IP_ADDR_MAX_LEN = 128;
        struct ifconf ifc;
        char ifcbuf[8 * sizeof(struct ifreq)]={0};
        ifc.ifc_len = sizeof(ifcbuf);
        ifc.ifc_buf = ifcbuf;

        int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
        int ret = ioctl(sock_fd, SIOCGIFCONF, (void *)&ifc);
        close(sock_fd);
        
        if(ret == 0)
        {
            unsigned int j;
            struct ifreq * ifr;
            struct sockaddr_in * paddr;
            char ip[IP_ADDR_MAX_LEN];

            for (ifr=ifc.ifc_req, j=0; j<ifc.ifc_len/sizeof(struct ifreq); j++, ifr++) 
            {
                paddr = (struct sockaddr_in*)&(ifr->ifr_addr);
                if(inet_ntop(AF_INET,(void *)&paddr->sin_addr, ip, sizeof(ip)-1)<0)
                {
                    break;
                }
                ipaddr_map.insert(make_pair(string(ifr->ifr_name), string(ip)));
            }
        }
    }
    
    return ipaddr_map;
}


