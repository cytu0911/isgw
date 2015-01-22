/******************************************************************************
*  @file      ace_app.h
*  @author rong
*  @history 
*   200503 init rong ACE��̨���̵�������ģ��,�Խ��̽�����ͣ��
*   200605 awayfang �����˲���ģ���˳�� ������reactor��ʼ��
*   �Ľӿڣ�ʹ���û���ʹ���Լ���reactor
*  
******************************************************************************/
#ifndef _ACE_APP_H_
#define _ACE_APP_H_

#include "ace_all.h"
#include "ace_conf.h"

#ifndef MAX_FD_SETSIZE 
#define MAX_FD_SETSIZE 10240
#endif

class ACEApp
{
public:
    virtual ~ACEApp();
    int init(int argc, ACE_TCHAR* argv[]);
    virtual void disp_version();
    virtual int init_app(int argc, ACE_TCHAR* argv[]);
    virtual int init_reactor();
    virtual int  init_sys_path(const char* program);
    virtual int  init_conf();
    virtual void child_main();
    virtual void quit_child();
    virtual int quit_app();
    void daemon_main();
    void quit_main();
    void lost_child();
    bool is_quit();
    static ACEApp* instance();

protected:
    ACEApp();
    char program_[256];
    bool is_quit_;
    bool is_child_;

private:
    void tweak_child(pid_t* child_pids, int child_proc_num);
    void parent_main(int child_proc_num);
    void parent_hatch(pid_t* child_pids, int child_proc_num);
    void parent_quit(pid_t* child_pids, int child_proc_num);
    int  init_log(int log_num = -1);
    void write_pid_file();

    bool tweak_child_;
    static ACEApp* aceapp_instance_;
};

#endif // _ACE_APP_H_ 
