#include <set>
#include <list>
#include <string>
#include <cstring>
using namespace std;

#ifdef WIN32
#include "winsock2.h"
#else
#include <netinet/in.h>
#endif

#include "sys_prot.h"

int SYS_prot_encode_char(char *buffer, int length, int &pos, char value)
{
    if (pos + (int)sizeof(char) > length)
        return -1;
    memcpy(buffer + pos, &value, sizeof(char));
    pos += sizeof(char);
    return 0;
}

int SYS_prot_encode_int(char *buffer, int length, int &pos, int value)
{
    if (pos + (int)sizeof(int) > length)
        return -1;
    int trans = htonl(value);
    memcpy(buffer + pos, &trans, sizeof(int));
    pos += sizeof(int);
    return 0;
}

int SYS_prot_encode_uint32(char *buffer, int length, int &pos, uint32 value)
{
    if (pos + (int)sizeof(uint32) > length)
        return -1;
    uint32 trans = htonl(value);
    memcpy(buffer + pos, &trans, sizeof(uint32));
    pos += sizeof(uint32);
    return 0;
}

//int val_len 信息的结构的原始长度，非实际长度
int SYS_prot_encode_string(char *buffer, int length, int &pos, char *value, int val_len)
{
    //modify by awayfang 编码前面不带stirng的长度
    int str_len = val_len;//strlen(value);
    if (pos + (int)str_len > length)
        return -1;
    memcpy(buffer + pos, value, (int)str_len);
    pos += (int)str_len;
    return 0;
}

//根据实际长度来编码，string 头含长度
int SYS_prot_encode_string_withhead(char *buffer, int length, int &pos, char *value)
{
    int str_len = strlen(value);
    if (pos + (int)str_len + (int)sizeof(unsigned char) > length)
        return -1;
    buffer[pos] = str_len;
    memcpy(buffer + pos + sizeof(unsigned char), value, (int)str_len);
    pos += (int)str_len + sizeof(unsigned char);
    return 0;
}

int SYS_prot_decode_char(char *buffer, int length, int &pos, char &value)
{
    if (pos + (int)sizeof(char) > length)
        return -1;
    memcpy(&value, buffer + pos, sizeof(char));
    pos += sizeof(char);
    return 0;
}

int SYS_prot_decode_int(char *buffer, int length, int &pos, int &value)
{
    if (pos + (int)sizeof(int) > length)
        return -1;
    int trans;
    memcpy(&trans, buffer + pos, sizeof(int));
    value = ntohl(trans);

#ifdef TRACE
    cout<<"in decode int value="<<value<<endl;
#endif

    pos += sizeof(int);
    return 0;
}

int SYS_prot_decode_uint32(char *buffer, int length, int &pos, uint32 &value)
{
    if (pos + (int)sizeof(uint32) > length)
        return -1;
    uint32 trans;
    memcpy(&trans, buffer + pos, sizeof(uint32));
    value = ntohl(trans);

#ifdef TRACE
    cout<<"in decode uint32 value="<<value<<endl;
#endif

    pos += sizeof(uint32);
    return 0;
}

int SYS_prot_decode_string(char *buffer, int length, int &pos, char *value, int val_len)
{
#ifdef TRACE
    cout<<"in decode string pos="<<pos<<" val_len="<<val_len<<endl;
#endif
    int str_len = val_len;//buffer[pos];
    if (pos + (int)str_len > length)
        return -1;
    
    memcpy(value, buffer + pos, (int)str_len);
    value[(int)str_len-1] = 0x0;
    pos += (int)str_len;
    return 0;
}

int SYS_prot_decode_string_withhead(char *buffer, int length, int &pos, char *value, int val_len)
{
    int str_len = buffer[pos];
    if (pos + (int)str_len + (int)sizeof(unsigned char) > length)
        return -1;
    if ((int)str_len >= val_len)
        return -1;
    memcpy(value, buffer + pos + sizeof(unsigned char), (int)str_len);
    value[(int)str_len] = 0x0;
    pos += sizeof(unsigned char) + (int)str_len;
    return 0;
}

int SYS_prot_encode_short(char *buffer, int length, int &pos, short value)
{
    if (pos + (int)sizeof(short) > length)
    {
        return -1;
    }
        
    int trans = htons(value);
    memcpy(buffer + pos, &trans, sizeof(short));
    pos += sizeof(short);
    
    return 0;
}

int SYS_prot_decode_short(char *buffer, int length, int &pos, short &value)
{
    if (pos + (int)sizeof(short) > length)
        return -1;
    int trans;
    memcpy(&trans, buffer + pos, sizeof(short));
    value = ntohs(trans);

#ifdef TRACE
    cout<<"in decode short value="<<value<<endl;
#endif

    pos += sizeof(short);
    return 0;
}

int SYS_prot_encode_ushort(char *buffer, int length, int &pos, USHORT value)
{
    if (pos + (int)sizeof(USHORT) > length)
    {
        return -1;
    }
        
    int trans = htons(value);
    memcpy(buffer + pos, &trans, sizeof(USHORT));
    pos += sizeof(USHORT);
    
    return 0;
}

int SYS_prot_decode_ushort(char *buffer, int length, int &pos, USHORT &value)
{
    if (pos + (int)sizeof(USHORT) > length)
        return -1;
    int trans;
    memcpy(&trans, buffer + pos, sizeof(USHORT));
    value = ntohs(trans);

#ifdef TRACE
    cout<<"in decode ushort value="<<value<<endl;
#endif

    pos += sizeof(USHORT);
    return 0;
}

