#ifndef ADMIN_PROXY_SYNC_H
#define	ADMIN_PROXY_SYNC_H

#include <vector>
#include <map>
#include "plat_conn_mgr_ex.h"
#include "qmode_msg.h"

// url=#128Char#
// Hi~#NickName#:<br>�����ĵġ�#32Char#����Ϸ���������»��#64Char#���������鿴����
struct AdminTipsParam
{
    int gid;
    int delaytime;
    std::string url;
    std::string gname;
    std::string actname;
};

class AdminProxySync
{
public:
    
    int start_tips_task(const AdminTipsParam& param);
    
public:
    static int init();
    
private:
    static PlatConnMgrEx* get_conmgr(); 
    static int init_conmgr(); 

private:
    static PlatConnMgrEx* conmgr_;
    static ACE_Thread_Mutex conmgr_lock_;
};

#endif	// ADMIN_PROXY_SYNC_H