#include "isgw_comm.h"
#include "isgw_ack.h"
using namespace EASY_UTIL;

///
char *EASY_UTIL::prot_encode(char *dest, const char *src)
{
    unsigned int i;
	dest[0] = '\0';
	for (i = 0; i < strlen(src); i++) 
       {
            switch (src[i]) 
            {
                case '&': strcat(dest, "%26");  break;
                case '=': strcat(dest, "%3D");  break;
                case '|': strcat(dest, "%7C");  break; //�ر���� url����һ�㲻����
                case ' ': strcat(dest, "%20");  break;
                case ',': strcat(dest, "%2C");  break;
                case ':': strcat(dest, "%3A");  break;                
                case '%': strcat(dest, "%25");  break; 
                case '~': strcat(dest, "%7E");  break;
                //���������������÷ָ��
                default: sprintf(dest, "%s%c", dest, src[i]);
            }
	}
	return dest;
}

///���������ַ�
char *EASY_UTIL::prot_strim(char *dest, const char *src)
{
    unsigned int i;

    dest[0] = '\0';
    for (i = 0; i < strlen(src); i++) 
    {
        switch (src[i]) 
        {
            case '&': strcat(dest, "");    break;
            case '=': strcat(dest, "");    break;
            case '|': strcat(dest,""); 	   break; //�ر���� url����һ�㲻����
            case '%': strcat(dest, "");    break; 
            case ' ': strcat(dest, "");    break;
            case ':': strcat(dest,"");	   break;
            default: sprintf(dest, "%s%c", dest, src[i]);
        }
    }
    return dest;
}

int EASY_UTIL::is_valid_uin(unsigned int uin)
{
    if ( uin < 10000 )
    {
        return -1;
    }
    return 0;
}

unsigned int EASY_UTIL::get_time()
{
    return ISGWAck::instance()->get_time();
}
//���ص�ֵ��λ��0.1ms
unsigned int EASY_UTIL::get_span(struct timeval *tv1, struct timeval *tv2)
{
    return ((tv2->tv_sec - tv1->tv_sec)*1000000 + tv2->tv_usec - tv1->tv_usec)/100;
}


