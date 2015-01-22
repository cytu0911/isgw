//=============================================================================
/**
 *  @file    pp_prot.h
 *
 *  ���ļ�Ϊsvr��ܶ��������ӿ�Э�鶨���ļ�
 *
 *  @author awayfang
 */
//=============================================================================

#ifndef _PP_PORT_H_
#define _PP_PORT_H_
#include "isgw_config.h"
#include "sys/time.h"

#ifndef uint32 
typedef unsigned int uint32;
#endif 

#ifndef uint16 
typedef unsigned short uint16;
#endif 

//***********************************�ⲿ��ϢЭ��**********************************//
//����ײ����ϢЭ��ΪMSG_SEPARATOR �궨��ָ���ַ�����Ϣ��Ϊһ����������Ϣ
//ÿ����Ϣ�ڲ�����cgi�ĸ�ʽ�ָ��"cmd=001001&uin=88883960&...\n"
//����ҵ���̳ǵĲ�ѯָ��cmd����λ���ֱ�ʾ
//ǰ��λ��ƽ̨������������Ʒ������λ��Ʒ�ڲ��Լ�����

//��Ϣ����ָ���
typedef enum _PP_CMD
{
    CMD_PDB = 0, //1000 ����Ϊ pdb �ڲ�����    
    CMD_FLASH_DLL = 100,

    CMD_DNF = 1000, //DNFҵ��ʼ

    CMD_FXD = 2000, //���е�ҵ��ʼ

    CMD_WEBGAME = 3000, 

    CMD_DANCER = 4000, //QQ����ҵ��ʼ
    
    CMD_FC = 5000, //QQ�ɳ�
    
    CMD_XX = 6000, //QQѰ��

    CMD_FO = 7000, 
    
    CMD_FFO = 8000, 
    
    CMD_SG = 9000, 

    CMD_WEBDEV = 10000, // web ������ʹ�ÿ�ʼ

    CMD_QQTANG = 11000, // 

    CMD_CF = 12000, // 

    CMD_R2 = 13000, // 

    CMD_QQGAME = 14000, // 

    CMD_PM = 15000, // 
    
    CMD_AVA = 16000, // 

    CMD_DMLQ = 17000, // 

    CMD_TMXY = 18000, // 

    CMD_HXSJ = 19000, // �������� �����

    CMD_OTOT = 20000, 

    CMD_XXZ = 21000,

    CMD_YKSK = 22000,  //���ս�� 

    CMD_PET_WORLD = 23000, //����

    CMD_XY = 24000,     //��ԯ

    CMD_LOL = 25000,        //Ӣ������

    CMD_YLZT = 31000,        // ��������
    
    CMD_END //����ָ��
}PP_CMD;

/*
//ָ����ʽ:
typedef enum _pay_WAY
{
    PAY_Q_DOT=1, //Q��֧��
    PAY_QQ_CARD, //QQ��
    PAY_NET_BANK, //����
    PAY_OTHER        //����
}PP_WAY;
*/

///���ջ�������С
#ifndef MAX_INNER_MSG_LEN
#define MAX_INNER_MSG_LEN 8400
#endif 

///�ڲ�Ԥ���ĳ��ȣ�Ϊ͸����Ϣʹ�� 
#define INNER_RES_MSG_LEN 192

///Э��ָ��
#ifndef MSG_SEPARATOR
#define MSG_SEPARATOR "\n"
#endif 

///�������ֶ���
#ifndef FIELD_NAME_CMD 
#define FIELD_NAME_CMD "cmd"
#endif 

///ģ��汾��
#ifndef SERVER_VERSION 
#define SERVER_VERSION "isgw_v1.0"
#endif 

///�����ֶ���(��ѡ)
#define FIELD_NAME_RESULT "result"
#define FIELD_NAME_UIN "uin"
#define FIELD_NAME_USER "user"
#define FIELD_NAME_PASSWD "passwd"
#define FIELD_NAME_INFO "info" //��Ӧ��Ϣ��˵����Ϣ

//����Ȩ�޶���
#define OP_GROUP_ROOT 1 
#define OP_GROUP_ADMIN 2 
#define OP_GROUP_USER 3 

//Э������:
typedef enum _PROTOCOL_TYPE
{
    PROT_TCP_IP=0, //TCP IP Э��
    PROT_UDP_IP, //UDP IP Э��
}PROTOCOL_TYPE;

//***********************************�����ϢЭ��**********************************//
//������Ϣ
typedef struct stPriProReq
{
    uint32 sock_fd; //socket ���ļ�������
    uint32 protocol; //Э������
    uint32 sock_ip; //
    uint32 sock_port; //
    uint32 sock_seq; //socket�����кţ�sock_fdֵ��ͬʱ������������
    uint32 seq_no; //��Ϣ�����кţ�Ψһ��ʶһ������
    uint32 cmd;     //������
    struct timeval tv_time;   //��Ϣ���ʱ��
    unsigned int rflag; // ��Ϣ�Ƿ�͸���ı�־
    uint32 msg_len; //�������Ч��Ϣ����    
    char msg[MAX_INNER_MSG_LEN+1]; //���ܵ�����Ϣ��
    stPriProReq():sock_fd(0),protocol(0),sock_ip(0),sock_port(0),sock_seq(0)
        ,seq_no(0),cmd(0),rflag(0),msg_len(0){}; //,msg({0}) 
}PriProReq;

//��Ӧ��Ϣ��ͷ��������Э���ͬ���ֶ�ͬ�壬�����ʱ����Ҫ��������Ϣ��ͷ���������
typedef struct stPriProAck
{
    uint32 sock_fd;
    uint32 protocol; //Э������
    uint32 sock_ip; //
    uint32 sock_port; //
    uint32 sock_seq;
    uint32 seq_no;
    uint32 cmd;     //������
    struct timeval tv_time;   //��Ӧ������Ϣ���ʱ��
    uint32 time; //ԭʼ�����ʱ��
    uint32 total_span;      //��������isgw���ܵĴ���ʱ��,��λΪ100us
    uint32 procs_span;    //����������Ӧ�������еĴ���ʱ��,��λΪ100us
    int ret_value;      //������ķ���״̬
    unsigned int rflag; // ��Ϣ�Ƿ�͸���ı�־
    uint32 msg_len; //�������Ч��Ϣ����
    char msg[MAX_INNER_MSG_LEN+1]; //��Ž������������ֵ����Ӧ����Ϣ���������ʱ��Ŵ�������
    stPriProAck():sock_fd(0),protocol(0),sock_ip(0),sock_port(0),sock_seq(0)
        ,seq_no(0),cmd(0),time(0),total_span(0),procs_span(0),ret_value(0),rflag(0)
        ,msg_len(0){}; //,msg({0}) 
}PriProAck;

/******************************************************************************
// ���ô�����
// 0 ��ʾ����/�ɹ�
// ����(<0) ��ʾϵͳ�����쳣 
//          ������������ �������� ��Ҫ�˹���Ԥ�޸���
//          �����쳣ͨ���ǲ�Ӧ�ô��ڵ� ��Ҫ�߶ȹ�ע�� 
//          ��Ȼ�ᵼ��ϵͳ���ز����� ���߲�������кܴ��Ǳ�ڷ��� 
// ����(>0) ��ʾҵ���߼������쳣 
//          �����¼������ ��¼�ظ��� 
//          ��������ǿ��Դ��ڵ� ����Ӧ�þ�������
******************************************************************************/
typedef enum _PP_ERROR
{
    ERROR_NO_FIELD = -123456789, //Э���ֶβ����� ҵ����ķ��ؾ����������������ͻ

    //�� (-10001)-(-11000 )Ϊǰ�˷����ⲿ�ӿڵĴ��� 
    ERROR_OIDB = -10002, //OIDB ����
    ERROR_MP = -10001, //Ӫ��ƽ̨���� 
    
    ERROR_FORWARD = -11, //����ת��ʧ��
    ERROR_TIMEOUT_SER = -10, //ҵ��ӿڷ�����Ϣ��ʱ 
    ERROR_IBC_FAC = -8, //ibc fac �쳣
    ERROR_DECODE = -7, //�ڲ�Э��(����)�����Ƿ� 
    ERROR_NO_REJECT = -6, //�ܾ��ṩ����
    ERROR_NO_PERMIT = -5, //��Ȩ�޷���(ϵͳ����)
    ERROR_PARA_ILL = -4, //�����Ƿ�
    ERROR_TIMEOUT_FRM = -3, //����ڲ�������Ϣ��ʱ 
    ERROR_CONFIG = -2, //�����쳣 
    ERROR_NET = -1, //�����쳣
    ERROR_OK = 0, //������� 
    ERROR_NOT_FOUND = 1, //ҵ��ӿڷ��ؼ�¼������ �������ݿ����޼�¼
    ERROR_DUPLICATE = 2, //ҵ��ӿڷ��ؼ�¼�ظ� �������ݿ��м�¼�ظ�
    ERROR_MYSQL_NOAFFTD = 3, // mysql ������Ӱ�������Ϊ 0 
    ERROR_SERV_NO_PERMIT = 4, // �û�ûȨ�޲���(ҵ���߼�����)
    
}PP_ERROR;

#endif // _PP_PORT_H_
