#include "sys_cli_def.h"

//global/last socket
//static SYS_SOCKET g_sock_fd = SYS_INVALID_SOCKET;
//socket 创建的起始时间和有效间隔
//static int g_sock_time_begin = 0;
SYS_SOCKET_INFO g_socket_list[MAX_SOCKET_NUM];

//socket 默认的 有效时间,单位 s
#ifdef TRACE
    static const int SOCK_TIME_INTERVAL = 1*60;
#else
    static const int SOCK_TIME_INTERVAL = 12*60*60;
#endif

static int g_socket_timeout = SYS_SOCKET_TIMEOUT;//

//申明win 和linux下特有的东西
//#ifndef WIN32
static SYS_Thread_Mutex sock_mutex;
//deleted by jinglinma begin 20090416
//static SYS_Thread_Mutex send_mutex;
//static SYS_Thread_Mutex recv_mutex;
//deleted by jinglinma end 20090416
static SYS_Thread_Mutex sockenv_mutex;
static SYS_Thread_Mutex sockinfo_mutex;
#ifdef WIN32
WSocketInit g_socket_init; //win 下特别需要的
#endif

int SYS_get_socket_env()
{
//#ifndef WIN32
    SysGuard guard(sockenv_mutex);//
//#endif
    char* socket_timeout_env = getenv(SYS_SOCKET_TIMEOUT_ENV_NAME);
    if ( socket_timeout_env != NULL )
    {
        g_socket_timeout = atoi(socket_timeout_env);
    }
    if ( g_socket_timeout == 0 )
    {
        g_socket_timeout = SYS_SOCKET_TIMEOUT;
    }    
    return 0;
}

int SYS_socket_list_init()
{
//#ifndef WIN32
    SysGuard guard(sockinfo_mutex);//
//#endif    
    for ( int i=0; i<MAX_SOCKET_NUM; i++ )
    {
        g_socket_list[i].index= -1;
        g_socket_list[i].socket_fd = SYS_INVALID_SOCKET;        
        g_socket_list[i].sock_time_begin= 0;
    }
    return 0;
}

//save socket by g_sock_fd
int SYS_save_sock(SYS_SOCKET sock_fd)
{
//#ifndef WIN32
    SysGuard guard(sock_mutex);//
//#endif

    int index = -1;
    index = SYS_thr_self();
    int i = 0;
    for ( i=0; i<MAX_SOCKET_NUM; i++ )
    {
        if( g_socket_list[i].index == -1 //init
	    || g_socket_list[i].index == index //reconnect
        )
        {
            g_socket_list[i].index = index;
            g_socket_list[i].socket_fd = sock_fd;
            g_socket_list[i].sock_time_begin = time(0);
            break;
        }
    }
    if ( i == MAX_SOCKET_NUM )
    {
        return SYS_ERR_SOCKET_MAX;//
    }
    
#ifdef TRACE
    cout <<"SYS_save_sock, thr_id:"<<index<<" i:"<<i<< " g_sock_list:"<<g_socket_list[i].socket_fd<<endl;
#endif
    return 0;
}

//get global/last socket
int  SYS_get_sock(SYS_SOCKET &sock_fd)
{
    int index = -1;
    index = SYS_thr_self();
    int i = 0;
    for ( i=0; i<MAX_SOCKET_NUM; i++ )
    {
        if( index == g_socket_list[i].index )
        {
            break;
        }
    }
    
    if ( i == MAX_SOCKET_NUM )
    {
#ifdef TRACE
        cout <<"SYS_get_sock can't find any match socket in socket list, need a new one, so return invalid socket"<<endl;
#endif
        //get index 0,has a bug, will always get first socket
        //i=0;
        return -1; //SYS_INVALID_SOCKET
    }

    sock_fd = g_socket_list[i].socket_fd;
#ifdef TRACE
    cout <<"SYS_get_sock, thr_id:"<<index<<" i:"<<i<< " sock_fd:"<<sock_fd<<endl;
#endif
    return 0;
}

//added by jinglinma 20090417 begin
static int SYS_socket_close( SYS_SOCKET sock_fd )
{

#ifdef WIN32
    return closesocket(sock_fd);
#else
    return close(sock_fd);
#endif

}
//added by jinglinma 20090417 end

//close socket
int SYS_close(SYS_SOCKET sock_fd)
{
//#ifndef WIN32
    SysGuard guard(sock_mutex);//
//#endif

    int index = -1;
    index = SYS_thr_self();
    int i = 0;
    for ( i=0; i<MAX_SOCKET_NUM; i++ )
    {
        if( index == g_socket_list[i].index )
        {
            g_socket_list[i].index = -1;
            g_socket_list[i].socket_fd = SYS_INVALID_SOCKET;
            g_socket_list[i].sock_time_begin = 0;
            break;
        }
    }
    if ( i == MAX_SOCKET_NUM )
    {
        return SYS_ERR_SOCKET_MAX;//
    }
    
    int ret = 0;
    
#ifndef WIN32
    ret = close(sock_fd);
#else
    ret = closesocket(sock_fd);
#endif
    
    if (ret != 0)
    {
        return -1;
    }
    
    return 0;
}

int SYS_close_all_socket()
{
    for ( int i=0; i<MAX_SOCKET_NUM; i++ )
    {
        SYS_close(g_socket_list[i].socket_fd);
    }
    return 0;
}

//判断该socket g_sock_fd 是否有效
bool SYS_is_sock_valid()
{
    //socket invalid
    //away test
    //g_sock_fd = SYS_INVALID_SOCKET;
    
    int index = -1;
    index = SYS_thr_self();
    int i = 0;
    for ( i=0; i<MAX_SOCKET_NUM; i++ )
    {        
        if( index == g_socket_list[i].index )
        {
            if ( g_socket_list[i].socket_fd == SYS_INVALID_SOCKET )
            {
#ifdef TRACE
                cout << "sock invalid, thr_id:"<<index<<" g_socket_list i:"<<i<<endl;
#endif
                return false;
            }
            break;
        }
    }
    if ( i == MAX_SOCKET_NUM )
    {
#ifdef TRACE
                cout << "can't find valid sock, thr_id:"<<index<<" g_socket_list i:"<<i<<endl;
#endif
        return false;//
    }
        
    //socket time out
    int now = time(0);

#ifdef TRACE
    cout << "g_socket_list i:"<<i<<" sock_time_begin:"<<g_socket_list[i].sock_time_begin<< " now:"<<now
    << " SOCK_TIME_INTERVAL:"<<SOCK_TIME_INTERVAL<<endl;
#endif

    if ( now - g_socket_list[i].sock_time_begin > SOCK_TIME_INTERVAL )
    {
        //close socket and set it invalid
        SYS_close(g_socket_list[i].socket_fd);
        return false;
    }
    
    return true;
}

int SYS_connect(SYS_SOCKET& sock_fd, SYS_CONF_SERVER& server_conf
	, char* error)
{
    int ret = 0;
    //find and get a old useable socket 
    SYS_get_sock(sock_fd);
    if ( sock_fd != SYS_INVALID_SOCKET )
    {
        //reuse socket
#ifdef TRACE
        cout <<"pid:"<<SYS_get_pid()<< " reuse old socket fd:"<<sock_fd<<endl;
#endif
        return 0;
    }

    //time out
    struct timeval tv;
    
    int server_index = 0;
    for( server_index = 0 ; server_index < server_conf.server_num_; server_index++ )
    {
        if (server_index > 0) //
        {
        
#ifdef TRACE
            cout <<"close invalid sock "<<sock_fd<<endl;
#endif
            //关闭上次的socket
            SYS_close(sock_fd);
        }
        
#ifdef TRACE
        cout <<"in for loop index="<<server_index
        <<" ip="<<server_conf.server_addr_[server_index]
        <<" port="<<server_conf.server_port_[server_index]
        <<endl;
#endif
        if ( server_conf.server_addr_[server_index] == 0 
          || server_conf.server_port_[server_index] == 0 
        )//非法地址或者端口
        {
#ifdef TRACE
        cout <<"invalid addr, continue ..."<<endl;
#endif     
            continue;
        }

        //open new sock and set some opt ...         
        sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        SYS_get_errno();
        if ( sock_fd == SYS_INVALID_SOCKET )
        {
            strcpy(error, strerror(errno));
            return -1;
        }
    
        // set to non-block
    #ifndef WIN32
        int fd_flag = 0;
        fd_flag = fcntl(sock_fd, F_GETFL);
        if ( fd_flag == -1 )
        {
            strcpy(error, strerror(errno));
            //added by jinglinma 20090417 begin
            SYS_socket_close(sock_fd);
            //added by jinglinma 20090417 end
            return -1;
        }
        fd_flag |= O_NONBLOCK;
        ret = fcntl(sock_fd, F_SETFL, fd_flag);
        if ( ret == -1 )
        {
            strcpy(error, strerror(errno));
            //added by jinglinma 20090417 begin
            SYS_socket_close(sock_fd);
            //added by jinglinma 20090417 end
            return -1;
        }
    #else
        unsigned long ul = 1;
        ret = ioctlsocket(sock_fd, FIONBIO, &ul);
        if (ret == SYS_SOCKET_ERROR)
        {
            errno = WSAGetLastError();
            strcpy(error, strerror(errno));
            //added by jinglinma 20090417 begin
            SYS_socket_close(sock_fd);
            //added by jinglinma 20090417 end
            return -1;
        }
    #endif
    
    #ifdef TRACE
        cout <<"set socket fd non-block ok"<<endl;
    #endif
        
        // construct sockaddr
        struct sockaddr_in ipgw_server;
        memset(&ipgw_server, 0, sizeof(ipgw_server));
        ipgw_server.sin_family = AF_INET;        
        
        SYS_get_svraddr(ipgw_server.sin_addr.s_addr, ipgw_server.sin_port
        , server_conf, server_index);
#ifdef TRACE
        cout <<"SYS_get_svraddr get new addr, curr index="<<server_conf.current_index_<<endl;
#endif
    
        // connect
        ret = connect(sock_fd,  (struct sockaddr *)&ipgw_server, sizeof(ipgw_server));
        SYS_get_errno();
        if (ret == SYS_SOCKET_ERROR && errno != EINPROGRESS && errno != EWOULDBLOCK)
        {
            SYS_save_svrstatus(server_conf.current_index_, 1);
#ifdef TRACE
            cout <<"connect failed,ret="<<ret<<",errno="<<errno
            <<",errmsg="<<strerror(errno)<<", continue ..."<<endl;
#endif
            //added by jinglinma 20090417 begin
            SYS_socket_close(sock_fd);
            //added by jinglinma 20090417 end
            continue;
        }
    
#ifdef TRACE
        cout <<"after connet() function call "<<endl;
#endif
    
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(sock_fd, &fds);
    
        int fd_num = 0;
#ifndef WIN32
        fd_num = sock_fd + 1;
#endif
    
        //struct timeval tv;
        tv.tv_sec = g_socket_timeout;
        tv.tv_usec = 0;
    
        ret = select(fd_num, NULL, &fds, NULL, &tv);
        SYS_get_errno();
        if (ret == SYS_SOCKET_ERROR)
        {
#ifdef TRACE
            cout << "select failed, "<<strerror(errno)<<endl;
#endif
            SYS_save_svrstatus(server_conf.current_index_, 1);
            //added by jinglinma 20090417 begin
            SYS_socket_close(sock_fd);
            //added by jinglinma 20090417 end
            continue;
        }
        else if (ret == 0)
        {
    
#ifdef TRACE
            cout << "select time out, "<<strerror(errno)<<endl;
#endif
            
#ifndef WIN32
            errno = ETIME;
#endif      
            SYS_save_svrstatus(server_conf.current_index_, 1);
            //added by jinglinma 20090417 begin
            SYS_socket_close(sock_fd);
            //added by jinglinma 20090417 end
            continue;
        }
    
        int sock_err = 0;
        int sock_err_len = sizeof(sock_err);
        ret = getsockopt(sock_fd, SOL_SOCKET, SO_ERROR,
                       (char*)&sock_err, (socklen_t*)&sock_err_len);
        SYS_get_errno();
        if ( ret == -1)
        {
#ifdef TRACE
            cout << "getsockopt failed, "<<strerror(errno)<<endl;
#endif
            SYS_save_svrstatus(server_conf.current_index_, 1);
            //added by jinglinma 20090417 begin
            SYS_socket_close(sock_fd);
            //added by jinglinma 20090417 end
            continue;
        }
    
        if (sock_err != 0)
        {
            errno = sock_err;
#ifdef TRACE
            cout << "sock_err not 0, "<<strerror(errno)<<endl;
#endif 
            SYS_save_svrstatus(server_conf.current_index_, 1);
            //added by jinglinma 20090417 begin
            SYS_socket_close(sock_fd);
            //added by jinglinma 20090417 end
            continue;
        }

#ifdef TRACE
        cout <<"in loop, get a useful addr, start to break, ret="<<ret
        <<" index="<<server_index<<endl;
#endif
        break;
    }
    
    if ( server_index >= server_conf.server_num_ )
    {
        strcpy(error, "can't find any useful svr address");
        return -1;
    }

    //save socket id, for next use
    SYS_save_sock(sock_fd);
#ifdef TRACE
    cout <<"pid:"<<SYS_get_pid()<< " find a useful svr, get new socket fd:"<<sock_fd<<endl;
#endif

    SYS_save_svrstatus(server_conf.current_index_);
    return 0;
}

int SYS_send_n(SYS_SOCKET sock_fd, const char* buf, int len, char* error)
{
/**/
//deleted by jinglinma begin 20090416
//#ifndef WIN32
//    SysGuard guard(send_mutex);//保证发送接收的原子性
//#endif
//deleted by jinglinma end 20090416
/**/
    int ret = 0;
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(sock_fd, &fds);

    int fd_num = 0;
#ifndef WIN32
    fd_num = sock_fd + 1;
#endif

    struct timeval tv;
    tv.tv_sec = g_socket_timeout;
    tv.tv_usec = 0;

    int pos = 0;
    while (pos < len)
    {
        ret = select(fd_num, NULL, &fds, NULL, &tv);
        SYS_get_errno();
        if (ret == SYS_SOCKET_ERROR)
        {
#ifdef TRACE
            cout << "select failed in SYS_send_n() "<<strerror(errno)<<endl;
#endif
            strcpy(error, strerror(errno));
            return -1;
        }
        else if (ret == 0)
        {
#ifdef TRACE
            cout << "select time out in SYS_send_n() "<<strerror(errno)<<endl;
#endif

#ifndef WIN32
            errno = ETIME;
#endif
            strcpy(error, strerror(errno));
            return -1;
        }

        ret = send(sock_fd, buf + pos, len - pos, 0);
        SYS_get_errno();
        if (ret == SYS_SOCKET_ERROR)
        {
            if (errno != EINPROGRESS && errno != EWOULDBLOCK)
            {
                strcpy(error, strerror(errno));
                return -1;
            }
            else
            {
                continue;
            }
        }
        pos += ret;
    }

    return len;
}

int SYS_recv_n(SYS_SOCKET sock_fd, char* buf, int len, char* error)
{
/**/
//deleted by jinglinma begin 20090416
//#ifndef WIN32
//    SysGuard guard(recv_mutex);//保证发送接收的原子性
//#endif
//deleted by jinglinma end 20090416
/**/
    int ret = 0;
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(sock_fd, &fds);
    
    int fd_num = 0;
#ifndef WIN32
    fd_num = sock_fd + 1;
#endif
    
    struct timeval tv;
    tv.tv_sec = g_socket_timeout;
    tv.tv_usec = 0;

    int pos = 0;
    while (pos < len)
    {
        ret = select(fd_num, &fds, NULL, NULL, &tv);
        SYS_get_errno();
        if (ret == SYS_SOCKET_ERROR)
        {
#ifdef TRACE
            cout << "select failed in SYS_recv_n() "<<strerror(errno)<<endl;
#endif
            strcpy(error, strerror(errno));
            return -1;
        }
        else if (ret == 0)
        {
#ifdef TRACE
            cout << "select time out in SYS_recv_n() "<<strerror(errno)<<endl;
#endif

#ifndef WIN32
            errno = ETIME;
#endif
            strcpy(error, strerror(errno));
            return -1;
        }

        ret = recv(sock_fd, buf + pos, len - pos, 0);
        SYS_get_errno();
        if (ret == 0)
        {
            strcpy(error, "connection closed by peer in SYS_recv_n() ");
            return -1;
        }
        else if (ret == SYS_SOCKET_ERROR)
        {
            if (errno != EINPROGRESS && errno != EWOULDBLOCK)
            {
                strcpy(error, strerror(errno));
                return -1;
            }
            else
            {
                continue;
            }
        }
        pos += ret;
    }

    return len;
}
