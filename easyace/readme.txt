v1.6.2
1 ȥ�� AceSockHdr �� 

v1.6.1 200908
1 �����˹����ڴ��bitmap�࣬�����ڹ����ڴ�������һƬ�ڴ���Ϊbitmapʹ�ã����õĳ���Ϊ
  QQ���� 0 1 ��־���жϣ��������ܵ�Ҫ��Ƚϸߣ��������ڴ��н���

v1.6   200905
1 ��sys_io_...��������Ż�������˿��ܴ��ڵľ��й©������
2 ��win��sys_io_...����̰߳�ȫ�������Ż�

v1.5.3 200807
1 ����ΪTRACE��־: ACE_Object_Que 
2 ���� epoll ģʽ֧�� ( ���Ӵ󲢷�����֧�� ) 

v1.5.2 200806 
1 Ϊ�˵��Է��㣬��ҵ���DEBUG��־���ֿ����������淶easyace�����־����ΪLM_TRACE��
  ����ʹ��������LM_TRACE��־�����-p~TRACE(ͬʱ�ر�DEBUG����-p~DEBUG|~TRACE)
  Ŀǰ��ɵ���ACESvc��SysConf��
  
2 �Ż��� ace_app �� reactor �ĳ�ʼ��˳��
  
3 �޸��� network_intf.h �ļ���һ���궨������

4 ���� ace_svc ����Ϣ���д�С���ýӿ�
  
v1.5.1 200804 �淶�˲���ģ�����־������Ϣ
  �漰 AceSockHdr sys_conf ��
  
v1.4 200803 ʵ������ͨ��ͨ�õĵײ㴦����
1 AceSockHdr ��̳��� ACE_Svc_Handler ʵ�ֶԸ�ʽΪ {sizeof(int)+��Ϣ��} ���û���Ϣ
  ���������յ��û����ݿռ䲢����Э��ֶ�ÿ����������Ϣ���û�ֻ��Ҫ����
  virtual int process(...) �������ɶ���Ϣ�����д���
  virtual int send(...) �������������ķ���ָ������Ϣ��
  �����Ҫ�ı�Э�飬�����ı�Э�飬������ virtual int handle_input(...) ��������  
  �漰�����ļ����� ace_sock_hdr.h ace_sock_hdr.cpp 
  
2 ����������(������Э����)����벿�ֵ�ͨ�ú���(�Ի����������͵ı����)
  �漰���ļ����� sys_prot.h sys_prot.cpp
  
3 ����ͨ����غ���(ͬ��ģʽ��һ�����ڿͻ���api)������������ģʽ�����ӣ�
  (���Ը����߳����Զ�����ͬ�����������ӣ��Ա��ֹ��������������֤�̰߳�ȫ)
  ���ͣ�����ָ���ĳ������ݣ������������й���ȣ������б�����
  sys_cli_def.h(�漰���ļ�����sys_thread_mutex.h sys_guard.h sys_cli_.*.cpp sys_cli_.*.h)
  
  ֧�����ö����������ַ�������ļ�·����Ĭ��ֵ��Ҳ����ͨ�� SYS_CONF_PATH ������������
  ������ο� sys_cli_def.h �Ķ���     
  
  ��Ҫ����Ϊ��
  int SYS_connect(SYS_SOCKET& sock_fd, SYS_CONF_SERVER& server_conf
	, char* error);
  int SYS_send_n(SYS_SOCKET sock_fd, const char* buf, int len, char* error);
  int SYS_recv_n(SYS_SOCKET sock_fd, char* buf, int len, char* error);  
  
v1.3 200704 �޸��� ACESvc ��������
  �Ľ��� һЩ�ӿں����Ĳ������ͣ�ʹ��ʹ�����������㣬���� process �Ⱥ���������Ӧ�õĿ�������
  
v1.2 200608 ����˵��
1 ������ ACEApp ��Ĳ��ֽӿڣ�������̵Ĺ���Ŀ¼���ýӿڵ�
2 ������xml ini�ļ�������صĿ� ͷ�ļ� nds_config.h ����ʹ��˵����ο� nds_readme.txt

v1.1 200605 ����˵��
1 �޸��� ACEApp �࣬�����˲���ģ���˳�� ������Reactor��ʼ���Ľӿڣ�
  ʹ���û���ʹ���Լ���Reactor
2 ������ ACESvc<typename IN_MSG_TYPE, typename OUT_MSG_TYPE> ģ�壬
  ����̳���ACE_Task_Base
  ʵ��������������ģʽ����Ϣ����������ϢֻҪ�� putq(IN_MSG_TYPE* ) ���룬
  ͨ�� process ����֮����send���ͳ�ȥ��ÿ����ˮ�ߴ������Ƕ����Ŀ������̣߳�
  ʹ����ֻ��������Щ�������ɡ�
  
  �麯��˵����
   int ACESvc::putq(IN_MSG_TYPE* msg); ������Ϣ���нӿ�
   OUT_MSG_TYPE* ACESvc::process(IN_MSG_TYPE* msg); ��Ϣ������
   int ACESvc::send(OUT_MSG_TYPE* ack = NULL); ��Ϣ�������֮����õ���Ϣ���ͺ���


v1.0 200503 ʹ��˵��
  easyace�ǻ���ACE��Ӧ�ó���⣬Ŀ����Ϊ�˼�ʹ��ACEʵ��server����Ĺ�����
  easyace�з�װ�˶�ȡ ini ���á�д��־���ӽ��̹�����CGI������ʽ�ı�Э��Ľ���
  �ַ�����Сдת�������ԡ�ʹ��easyace��Ӧ�ó��򿪷��ߣ�����ͨ��override ACEApp���
  �����麯��ʵ��Ӧ���߼���
  
  ע���˿���ֻ��ACE��ͷ����(�����ļ�)������ace��صģ�����������ace�޹صĿ���ͨ�á�

�����б�:
1 ͨ���� sys_comm
  ����ace������
  ��ͨ�ַ���ת16���ơ�16��չת��ͨ�ַ���
  �ַ���ת��д��Сд
  ����cgi����
2 �����ļ������� sys_conf
  �� ini �μ���ʽ���ַ������н�������
3 md5 tea���ܽ����㷨
4 aceʵ�ֺ�̨svr��ģʽ
  ��svr��̨ģʽ��ʵ�֣������Ҫ�ı�һЩ��Ϊֻ��Ҫ������Ҫ�ĺ������ɡ�


ʹ�÷�����

1�����밲װ

easyace������ACE����˿�����ԱӦ���Ȱ�װACE�⡣ACE�ⰲװ���Ժ󣬿��Խ���
easyaceĿ¼��ִ��make���easyace�ı��롣

2��Ӧ�ó��򿪷�

Ӧ�ó�������Ӧ���ṩ����Դ�ļ���һ����������һ���Ǵ�ACEApp�̳е�Ӧ�ó����ࡣ

���磺
-------------------------------------------------------------------------------
my_app.h : 

#ifndef MY_APP_H_
#define MY_APP_H_

#include "easyace/ace_app.h"

class MyApp : public ACEApp
{
    // ...
};

#endif /* MY_APP_H_ */
-------------------------------------------------------------------------------
testace.cpp : 

#include "my_app.h"

int ACE_TMAIN (int argc, ACE_TCHAR* argv[])
{
    MyApp the_app;
    if (the_app.init(argc, argv) != 0)
    {
        return -1;
    }

    return 0;
}
-------------------------------------------------------------------------------

MyApp��Ӧ��override ACEApp�ж���ļ����麯����

��Makefile.appΪģ�壬����Ӧ�ó����Լ���Makefile���Ϳ��Խ��б����ˡ�

3������������ֹͣ

����������
ʹ��easyace������Ӧ�ó��������� <program name>_PATH��������������Ӧ�ó�����Ϊ
foo�Ļ����û�����������Ϊ FOO_PATH�������ֿ��������������޸ġ�

Ŀ¼�ṹ��
$<program name>_PATH �ǳ������еĵ�ǰĿ¼��
<program name>.iniӦ�÷����ڸ�Ŀ¼��<program name>.pidҲ���ڸ�Ŀ¼����

$<program name>_PATH/log ����Ӧ�ó�����־�ļ�

�����в�����
-d ��daemon��ʽ�ں�̨���У�������ǰ̨����
-p ָ��Ӧ�ó������֣����û��ָ������ʹ��ȱʡֵargv[0]
-c ָ���ӽ��̵���Ŀ

������
#export <program name>_PATH=.....
#$<program name>_PATH/<program name>

ֹͣ��
�����ǰ̨���У�ʹ��CTRL-Cʹ֮�˳�
����Ǻ�̨���У�cat $<program name>_PATH/<program name>.pid | xargs kill

4���麯��˵��
ACEApp::init_app()�����г��������ĳ�ʼ�����������紴�������̣߳����������˿ڵȡ�
ACEApp::quit_app()�����г����˳�����������
ACEApp::child_main()�����ڶ����ģ�͵ĳ��򣬴˺������ӽ��̵���������
ACEApp::quit_child()�����ڶ����ģ�͵ĳ��򣬴˺������ӽ��̵��˳���������
ACEApp::init_reactor������reactor�ĳ�ʼ����Ĭ��ʹ�� ACE_Select_Reactor_N();

5������ģ��

ʹ��easyace��Ӧ�ó������ѡ�񵥽��̻�����ģ�ͣ����ڶ������˵�������̻Ḻ��
�ӽ��̵�����ͼ�أ�����ӽ����쳣�˳��������̽�������������

�����̺��ӽ���ȱʡ������������������ ACE_Reactor ����ѭ����

6�������ļ�

ʹ��easyace��Ӧ�ó�����������ʱ����Ѿ��������ļ���ȡ���ڴ��У������ļ�
��INI��ʽ���ڳ����������ʹ�ã�
SysConf::instance()->get_conf_str
SysConf::instance()->get_conf_int
�ֱ��ȡ�ַ��������͵������

7����־

[common]
log_mask = "-m 10240 -N 10 -f OSTREAM -p~DEBUG -s log/testace.log"

����������������־�ģ�����ָ��ѭ����־�Ĵ�С�͸�������־�����Լ�·�����ļ�����
