#include "sys_cli_def.h"

//系统初始化
SystemInit system_init;
char sys_conf_path[256];

static int g_pid = -1;

//申明win 和linux下特有的东西
//#ifndef WIN32
static SYS_Thread_Mutex pid_mutex;
//#else

//#endif

//get call thread id
int SYS_thr_self()
{
    int thread_id = -1;
#ifndef WIN32
    thread_id = pthread_self();
#else
    thread_id = GetCurrentThreadId();//10000;
#endif
    return thread_id;
}

//get call process id
int SYS_get_pid()
{    
//#ifndef WIN32    
    SysGuard guard(pid_mutex);
//#endif
    if (g_pid == -1)
    {
#ifndef WIN32
        g_pid = getpid();
#else
        g_pid = _getpid();
#endif
        
    }
    
    return g_pid;
}

int SYS_get_errno()
{
#ifndef WIN32   

#else
    errno = WSAGetLastError();
#endif
#ifdef TRACE
    cout <<"in SYS_get_errno, errno="<<errno<<endl;
#endif
    return errno;
}
