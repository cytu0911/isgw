#include "sys_cli_def.h"

SYS_CONF_SERVER sys_server_conf;//= {0, 0, 0, ""};

//申明win 和linux下特有的东西
//#ifndef WIN32
static SYS_Thread_Mutex svr_mutex;
//#else

//#endif

//初始化服务器列表信息
int SYS_init_svrlist_status()
{
//#ifndef WIN32
    SysGuard guard(svr_mutex);
//#endif
    
    for ( int i = 0; i < MAX_SERVER_NUM; i++ )
    {
        sys_server_conf.server_status_[i] = 0;
    }
    //sys_server_conf.current_index_ = 0;
    return 0;
}

//获取服务器的列表信息
int SYS_read_svrconf(SYS_CONF_SERVER* conf, char* error)
{
//#ifndef WIN32
    SysGuard guard(svr_mutex);
//#endif

    if ( sys_server_conf.init_flag_ != 0 )
    {
#ifdef TRACE
        cout<<"sys_server_conf already init successful"<<endl;
#endif
        if ( conf != NULL )
        {
            memcpy(conf, &sys_server_conf, sizeof(SYS_CONF_SERVER));
        }        
        return 0;
    }
    //memset(&sys_server_conf, 0x0, sizeof(sys_server_conf));

    char svr_file_path[256];
    memset(svr_file_path, 0x0, sizeof(svr_file_path));
    char* conf_path = getenv(SYS_CONF_PATH_ENV_NAME);
    if ( conf_path != NULL )
    {
        strncpy(svr_file_path, conf_path, sizeof(svr_file_path));
        strncat(svr_file_path,"/", sizeof(svr_file_path));
    }
    else
    {
#ifdef TRACE
        cout<<"You didn't define sys conf env "<<SYS_CONF_PATH_ENV_NAME
        <<" so use default path "<<SYS_CONF_SRV_PATH<<endl;
#endif
        strncpy(svr_file_path, SYS_CONF_SRV_PATH, sizeof(svr_file_path));
    }
    
    strncat(svr_file_path,SYS_CONF_SRV_FILE, sizeof(svr_file_path));
    
    // read from file
    string server_ip;
    short server_port;
    ifstream conf_file(svr_file_path);    
    if (!conf_file.good())
    {
        sprintf(error, "open file: %s failed", svr_file_path);
        conf_file.close();
        return SYS_ERR_CONF_FILE;
    }
    
    int index = 0;
    while (!conf_file.eof()) //此处判断有点问题，会导致最后一条记录重复读取
    {
        conf_file >> server_ip >> server_port;
#ifdef TRACE
        cout<<"get new svr info"<<",ip:"<<server_ip<<",port:"<<server_port<<endl;
#endif
        if ( server_ip.length() == 0 || server_port == 0 )
    	 {
            conf_file.close();
            return SYS_ERR_CONF_FILE;
    	 }

        if ( index >= MAX_SERVER_NUM )
    	 {
            break;
    	 }        
        
        sys_server_conf.server_addr_[index] = inet_addr(server_ip.c_str());
        sys_server_conf.server_port_[index] = htons(server_port);
        index++;
    }
    sys_server_conf.server_num_ = index-1; //因为每次最后一个会出现重复读取，故减去一个
    sys_server_conf.init_flag_ = 1;//set flag
    conf_file.close();

#ifdef TRACE
        cout<<"svr conf info"<<",server num:"<<sys_server_conf.server_num_<<",init flag:"<<sys_server_conf.init_flag_<<endl;
#endif

    if ( conf != NULL )
    {
        memcpy(conf, &sys_server_conf, sizeof(SYS_CONF_SERVER));
    }
    
    return 0;
}

//保存服务器状态信息
int SYS_save_svrstatus(const int index , const int count 
	, SYS_CONF_SERVER &server_conf )
{
    if ( index >= MAX_SERVER_NUM )
    {
        return 1;
    }
    
    server_conf.server_status_[index] += count;
#ifdef TRACE
        cout<<"SYS_save_svrstatus svr index="<<index
        <<" status="<<server_conf.server_status_[index]<<endl;
#endif  
    return 0;
}

//获取一个没尝试连接过的服务器信息 ip port and index 或者按照用户的index指定来选择
int SYS_get_svraddr(addr_t &addr, port_t &port, SYS_CONF_SERVER &server_conf
	, int index )
{	
    if ( index == -1 || index >= MAX_SERVER_NUM ) //-1 system select flag
    {
        int i=0;
         
        for ( i = 0; i < MAX_SERVER_NUM; i++ )
        {
#ifdef TRACE
            cout<<"SYS_get_svraddr try svr i="<<i
            <<" status="<<server_conf.server_status_[i]<<endl;
#endif
            //when index i server is useable
            if ( server_conf.server_status_[i] == 0 )
            {
                index = i;
                break;
            }          
        }
         //the last is still not useable
         //use index server
         
         //can't find any useful svr
        if ( i == MAX_SERVER_NUM )
        {
#ifdef TRACE
            cout <<"SYS_get_svraddr can't find any useful svr, "
            <<"re init the svr list and use the first one"<<endl;
#endif
            SYS_init_svrlist_status();
            //get index 0
            index = 0;
        }
    }

    port = server_conf.server_port_[index];
    addr = server_conf.server_addr_[index];
    server_conf.current_index_ = index;
    return index;
}
