#include "ace_app.h"
#include "ace/OS_NS_sys_wait.h"
#include "ace/Reactor.h"
#include "ace/Dev_Poll_Reactor.h"
#include "sys_comm.h"

// signal function to perform shutdown
static void daemon_quit(int sig)
{
    ACE_DEBUG((LM_INFO, "[%D] signal(%d) catched pid: %d\n",
        sig, ACE_OS::getpid()));

    ACEApp::instance()->quit_main();
}

static void child_sighdr(int sig)
{
    ACE_DEBUG((LM_INFO, "[%D] signal(%d) catched pid: %d\n",
        sig, ACE_OS::getpid()));

    ACEApp::instance()->lost_child();
}

//for ACE_TP_Reactor
static ACE_THR_FUNC_RETURN event_loop (void *arg)
{
    //ACE_Reactor *reactor = ACE_static_cast(ACE_Reactor *, arg);
    ACE_Reactor *reactor = static_cast< ACE_Reactor *> (arg);

    ACE_DEBUG((LM_INFO, "[%D] ace tp reactor event loop, tid=%d,pid=%d\n"
            , ACE_OS::thr_self ()
            , ACE_OS::getpid()
            ));
    
    reactor->owner(ACE_OS::getpid());
    reactor->run_reactor_event_loop ();
    return 0;
}

ACEApp* ACEApp::aceapp_instance_ = NULL;

ACEApp* ACEApp::instance()
{
    if ( aceapp_instance_ == NULL )
    {
        aceapp_instance_ = new ACEApp();
    }
    
    return aceapp_instance_;
}

ACEApp::ACEApp()
{
    aceapp_instance_ = this;
    is_quit_ = false;
    is_child_ = false;
    tweak_child_ = false;
}

ACEApp::~ACEApp()
{
}

void ACEApp::quit_main()
{
    is_quit_ = true;
    if (is_child_)
    {
        quit_child();
    }
    else
    {
        ACE_Reactor::instance()->end_reactor_event_loop();
    }
}

void ACEApp::quit_child()
{
    ACE_Reactor::instance()->end_reactor_event_loop();
}

bool ACEApp::is_quit()
{
    return is_quit_;
}

int ACEApp::init(int argc, ACE_TCHAR* argv[])
{
    // get the program name
#ifdef WIN32
    char* ptr = ACE_OS::strrchr(argv[0], '\\');
#else
    char* ptr = ACE_OS::strrchr(argv[0], '/');
#endif    
    if (ptr)
    {
        // strip path name
        strcpy(program_, ptr + 1);
    }
    else
    {
        strcpy(program_, argv[0]);
    }
#ifdef WIN32
    // strip .exe extension on WIN32
    ptr = strchr(program_, '.');
    if (ptr)
    {
        *ptr = '\0';
    }
#endif

    int child_proc_num = 0;
    ACE_Get_Opt get_opt(argc, argv, "vdp:c:"); 
    char c;
    while ((c = get_opt()) != EOF)
    {
        switch (c)
        {
            case 'd': // run as daemon
                ACE::daemonize();
                break;
            case 'p': // specified program name
                strcpy(program_, get_opt.opt_arg());
                break;
            case 'c': // child process number
                child_proc_num = atoi(get_opt.opt_arg());
                break;
            case 'v':
                //disp version info
                disp_version();
                return 0;
            default:
                break;
        }
    }

    ACE_OS::signal (SIGPIPE, SIG_IGN);
    ACE_OS::signal (SIGTERM, SIG_IGN);
    ACE_OS::signal (SIGCHLD, SIG_IGN);
    ACE_Sig_Action sig_int((ACE_SignalHandler)daemon_quit, SIGINT);
    //ACE_Sig_Action sig_term((ACE_SignalHandler)daemon_quit, SIGTERM);
    //ACE_Sig_Action sig_chld((ACE_SignalHandler)child_sighdr, SIGCHLD);
    ACE_UNUSED_ARG(sig_int);
    //ACE_UNUSED_ARG(sig_term);
    //ACE_UNUSED_ARG(sig_chld);
    
    //�� init_log() ֮ǰ����    
    if (init_reactor() != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] init reactor failed, program exit\n"));
        return -1;
    }
    ACE_DEBUG((LM_INFO, "[%D] init reactor succ\n"));

    if (init_sys_path(program_) != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] init sys path failed, program exit\n"));
        return -1;
    }

    if (init_conf() != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] init conf failed, program exit\n"));
        return -1;
    }
    ACE_DEBUG((LM_INFO, "[%D] init conf succeed\n"));

    if (init_log() != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] init log failed, program exit\n"));
        return -1;
    }
    ACE_DEBUG((LM_INFO, "[%D] init log succ\n"));
    
    //�汾��Ϣ��¼��־
    disp_version();

    if (init_app(argc, argv) != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] init app failed, program exit\n"));
        return -1;
    }
    ACE_DEBUG((LM_INFO, "[%D] init app succ\n"));

    write_pid_file();
    ACE_DEBUG((LM_INFO, "[%D] write pid file succ\n"));

    if (child_proc_num > 0)
    {
        parent_main(child_proc_num);
    }
    else
    {
        ACE_DEBUG((LM_INFO, "[%D] enter daemon main pid: %d\n",
            ACE_OS::getpid()));

        daemon_main();

        ACE_DEBUG((LM_INFO, "[%D] leave daemon main pid: %d\n",
            ACE_OS::getpid()));
    }

    quit_app();

    return 0;
}

int ACEApp::init_sys_path(const char* program)//app_path_env_name
{
    //Ĭ��ϵͳ���л���Ϊ��������_PATH��ϵĻ�������ָ����Ŀ¼
    char bin_path_env_name[256];
    EASY_UTIL::str2upper(program, bin_path_env_name);
    strcat(bin_path_env_name, "_BIN");
    
    char* bin_path_env = ACE_OS::getenv(bin_path_env_name);
    if (bin_path_env == NULL)
    {
        ACE_DEBUG((LM_ERROR, "[%D] env %s not set\n", bin_path_env_name));
        return -1;
    }

    if (chdir(bin_path_env) != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] chdir(%s) failed, %d, %s\n",
                  bin_path_env, errno, strerror(errno)));
        return -1;
    }

    ACE_DEBUG((LM_INFO, "[%D] path changed to %s\n", bin_path_env));
    return 0;
}

int ACEApp::init_conf()
{
    char cfg_path_env_name[256];
    EASY_UTIL::str2upper(program_, cfg_path_env_name);
    strcat(cfg_path_env_name, "_CFG");

    string conf_path = "";
    char* cfg_path_env = ACE_OS::getenv(cfg_path_env_name);
    if (cfg_path_env == NULL)
    {
        ACE_DEBUG((LM_INFO, "[%D] env %s not set, use default working path\n", cfg_path_env_name));
        conf_path = "./";
    }
    else
    {
        conf_path = cfg_path_env;
    }

    string conf_file = program_;
    conf_file += ".ini";
    conf_file = conf_path + "/" + conf_file;
    
    return ACEConf::instance()->load_conf(conf_file);
}

int ACEApp::init_log(int log_num /* = -1 */)
{
    // init logging strategy
    char log_opt[1024];
    if (ACEConf::instance()
        ->get_conf_str("common", "log_mask", log_opt, sizeof(log_opt)) != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] get config log_mask failed\n"));
        return -1;
    }

    if (ACE_OS::strstr(log_opt, "%s") != NULL)
    {
        char log_format[256];
        strcpy(log_format, log_opt);

        char prog[10];
        if (is_child_)
        {
            sprintf(prog, "%d", log_num == -1 ? ACE_OS::getpid() : log_num);
        }
        else
        {
            sprintf(prog, "%s", "main");
        }

        snprintf(log_opt, sizeof(log_opt)-1, log_format, prog);
    }
    
    ACE_DEBUG((LM_ERROR, "[%D] init_log log_opt=%s\n", log_opt));
    
    ACE_ARGV args(log_opt);
    ACE_Logging_Strategy* logging_strategy = new ACE_Logging_Strategy;
    if (logging_strategy->init(args.argc(), args.argv()) != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] init logging_strategy failed\n"));
        return -1;
    }

    return 0;
}

void ACEApp::write_pid_file()
{
    string pid_file_path = program_;
    pid_file_path += ".pid";
    pid_file_path = "./" + pid_file_path;
    
    ACE_DEBUG((LM_INFO, "[%D] write pid file %s\n", pid_file_path.c_str()));
    
    ofstream pid_file(pid_file_path.c_str());
    pid_file << ACE_OS::getpid();
    pid_file.close();
}

int ACEApp::init_app(int argc, ACE_TCHAR* argv[])
{
    ACE_UNUSED_ARG(argc);
    ACE_UNUSED_ARG(argv);
    
    return 0;
}

void ACEApp::disp_version()
{
    ACE_DEBUG((LM_INFO, "**********************************************\n"));
    ACE_DEBUG((LM_INFO, "* Module Name : Aceapp"));
    ACE_DEBUG((LM_INFO, "* Module Version : V1.6"));
    ACE_DEBUG((LM_INFO, "**********************************************\n"));
}

int ACEApp::init_reactor()
{    

    #if ((ACE_MAJOR_VERSION > 5 || (ACE_MAJOR_VERSION==5 && ACE_MINOR_VERSION>=6)) && defined (ACE_HAS_EVENT_POLL) )
    
    ACE_DEBUG((LM_INFO, "[%D] ACEApp use dev poll reactor, ACE_FD_SETSIZE=%d\n", ACE_FD_SETSIZE));
    //ACE5.6 ֮�ϵİ汾ʹ��EPOLL 

    //���ʵ�ָ߲���,����Ͳ� ��Ҫ����FD�����ֵ
    //ACEĬ����ʹ��ϵͳ���Ƶ��������ֵ
    //dev_poll_reactor Ĭ�϶����timer�ش�С44��,��Ϊisgw������Ҫ̫��timer
    //����Ҳ���Բ�Ԥ������̫���timer��
    //ACE_Timer_Queue          *timer_queue;
    //timer_queue = new ACE_Timer_Heap(MAX_FD_SETSIZE);
    //timer_queue->gettimeofday(ACE_High_Res_Timer::gettimeofday_hr); // �滻ʱ���ȡ�ĺ��� 
    //ACE_Dev_Poll_Reactor *dp_reactor = new ACE_Dev_Poll_Reactor(MAX_FD_SETSIZE, 0, NULL, timer_queue);
    ACE_Dev_Poll_Reactor *dp_reactor = new ACE_Dev_Poll_Reactor();
    int ret = dp_reactor->initialized();
    //ACE �����������Ȼ�� true ��ʾ�ɹ�,�����ҵ��԰���(sailzeng ��¼)
    if (ret != true )
    {
        ACE_DEBUG((LM_ERROR,"ACEApp init ACE_Dev_Poll_Reactor failed, errno:%u|%m, ret=%d\n", ACE_OS::last_error(), ret));
        return ret;
    }
    ACE_Reactor::instance(new  ACE_Reactor(dp_reactor,1),1);
    
    #else
    
    ACE_DEBUG((LM_INFO, "[%D] ACEApp use select reactor, ACE_FD_SETSIZE=%d\n", ACE_FD_SETSIZE));
    ACE_Reactor::instance(new ACE_Reactor(new ACE_Select_Reactor_N()));
    
    #endif

    return 0;
}

int ACEApp::quit_app()
{
    ACE_DEBUG((LM_INFO, "[%D] quit app,main pid: %d\n",
        ACE_OS::getpid()));
    return 0;
}

void ACEApp::daemon_main()
{
    ACE_Reactor::instance()->owner(ACE_OS::thr_self());

    while (ACE_Reactor::instance()->reactor_event_loop_done () == 0)
    {
        ACE_DEBUG((LM_INFO, "[%D] within daemon main pid: %d\n",
            ACE_OS::getpid()));

        ACE_Reactor::instance()->run_reactor_event_loop();
    }
}

void ACEApp::tweak_child(pid_t* child_pids, int child_proc_num)
{
    ACE_DEBUG((LM_INFO, "[%D] parent will tweak\n"));

    for (int i = 0; i < child_proc_num; )
    {
        pid_t pid = ACE_OS::waitpid(child_pids[i], NULL, WNOHANG);
        
        if (pid == -1 && errno == EINTR)
        {
            continue;
        }
        else if (pid == -1 && errno != EINTR)
        {
            ACE_DEBUG((LM_ERROR, "[%D] wait pid %d: %d failed, (%d)%s\n",
                i, child_pids[i], errno, ACE_OS::strerror(errno)));
        }
        else if (pid == 0)
        {
            ACE_DEBUG((LM_ERROR, "[%D] waitpid %d: %d return 0\n",
                i, child_pids[i]));
        }
        else
        {
            ACE_DEBUG((LM_ERROR,
                "[%D] child lost, pid %d: %d\n", i, pid));
            child_pids[i] = 0;
        }
        
        i ++;
    }
}

void ACEApp::lost_child()
{
    tweak_child_ = true;
    ACE_Reactor::instance()->end_reactor_event_loop();
}

void ACEApp::parent_main(int child_proc_num)
{
    ACE_DEBUG((LM_INFO, "[%D] enter parent main pid: %d\n", ACE_OS::getpid()));

    ACE_Reactor::instance()->owner(ACE_OS::thr_self());

    pid_t* child_pids = new pid_t[child_proc_num];
    ACE_Auto_Basic_Array_Ptr<pid_t> children(child_pids);
    memset(child_pids, 0x0, sizeof(pid_t) * child_proc_num);

    while (true)
    {
        if (tweak_child_)
        {
            // there are children exit, need to restart them
            tweak_child_ = false;
            tweak_child(child_pids, child_proc_num);
        }
        else if (is_quit_)
        {
            // program exit
            parent_quit(child_pids, child_proc_num);
            break;
        }

        // fork child if needed
        parent_hatch(child_pids, child_proc_num);

        if (tweak_child_)
        {
            // in case more than one child lost
            continue;
        }

        ACE_Reactor::instance()->reset_reactor_event_loop();
        if (ACE_Reactor::instance()->reactor_event_loop_done() == 0)
        {
            ACE_Reactor::instance()->run_reactor_event_loop();
        }
    }

    ACE_DEBUG((LM_INFO, "[%D] leave parent main pid: %d\n", ACE_OS::getpid()));
}

void ACEApp::parent_hatch(pid_t* child_pids, int child_proc_num)
{
    for (int i = 0; i < child_proc_num; i++)
    {
        if (child_pids[i] != 0)
        {
            continue;
        }
        
        pid_t child_pid = ACE_OS::fork();
        if (child_pid == -1)
        {
            ACE_DEBUG((LM_ERROR, "[%D] fork %d child failed, (%d)%s\n",
                i, errno, ACE_OS::strerror(errno)));
            ACE_OS::sleep(10);
            continue;
        }
        else if (child_pid == 0)
        {
            // child
            is_child_ = true;
            is_quit_  = false;
            ACE_Reactor::instance()->reset_reactor_event_loop();

            ACE_OS::signal (SIGINT,  SIG_IGN);
            ACE_OS::signal (SIGCHLD, SIG_IGN);
            
            if (init_log(i) == 0)
            {
                ACE_DEBUG((LM_INFO, "[%D] enter child main pid %d: %d\n",
                    i, ACE_OS::getpid()));
                
                child_main();
                
                ACE_DEBUG((LM_INFO, "[%D] leave child main pid %d: %d\n",
                    i, ACE_OS::getpid()));
            }

            quit_app();

            ACE_OS::exit(0);
        }
        else
        {
            // parent
            ACE_DEBUG((LM_ERROR,
                "[%D] child forked, pid %d: %d\n", i, child_pid));
            
            child_pids[i] = child_pid;
        }
    }
}

void ACEApp::parent_quit(pid_t* child_pids, int child_proc_num)
{
    ACE_DEBUG((LM_INFO, "[%D] parent will quit\n"));
    
    ACE_OS::signal (SIGCHLD, SIG_DFL);
    ACE_OS::signal (SIGINT,  SIG_IGN);
    ACE_OS::signal (SIGTERM, SIG_IGN);
    
    // tell child to quit
    ACE_OS::kill(0, SIGTERM);
    
    for (int i = 0; i < child_proc_num; )
    {
        pid_t pid = ACE_OS::waitpid(child_pids[i]);
        
        if (pid == -1 && errno == EINTR)
        {
            continue;
        }
        else if (pid == -1 && errno != EINTR)
        {
            ACE_DEBUG((LM_ERROR, "[%D] wait pid %d: %d failed, (%d)%s\n",
                i, child_pids[i], errno, ACE_OS::strerror(errno)));
        }
        else
        {
            ACE_DEBUG((LM_INFO, "[%D] child waited, pid %d: %d\n", i, pid));
        }
        
        i ++;
    }
}

void ACEApp::child_main()
{
    daemon_main();
}

