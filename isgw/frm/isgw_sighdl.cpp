/************************************************************
  Copyright (C), 2008-2018, Tencent Tech. Co., Ltd.
  FileName: isgw_sighdl.cpp
  Author: awayfang            Date: 2008-06-25
  Description:     
***********************************************************/
#include "isgw_sighdl.h"
#include "isgw_oper_base.h"

ISGWSighdl* ISGWSighdl::instance_ = NULL;

ISGWSighdl* ISGWSighdl::instance()
{
    if (NULL == instance_)
    {
        instance_ = new ISGWSighdl();
    }
    return instance_;
}

ISGWSighdl::ISGWSighdl()
{
    //����׽�ź�SIGUSR1(idΪ10)
    ACE_Reactor::instance()->register_handler(SIGUSR1, this);

    int hdl_timer = 0; // �Ƿ�ʹ�ö�ʱ�� Ĭ�ϲ�ʹ�� 
    SysConf::instance()->get_conf_int("handle", "timer", &hdl_timer);
    if (hdl_timer == 1)
    {
        int hdl_interval = 0;
        SysConf::instance()->get_conf_int("handle", "interval", &hdl_interval);
        if (hdl_interval < 1000) // ����Ҫ 1000 ΢�� 
        {
            hdl_interval = 1000;
        }
        ACE_Time_Value delay(0,0);
        ACE_Time_Value intvl(0,hdl_interval);
        ACE_Reactor::instance()->schedule_timer(this, 0, delay, intvl);
    }
        
}

ISGWSighdl::~ISGWSighdl()
{
    
}

int ISGWSighdl::handle_signal(int signum, siginfo_t * , ucontext_t *)
{
    if(SIGUSR1==signum)
    {
        return handle_reload();
    }

    return 0;
}

int ISGWSighdl::handle_timeout(const ACE_Time_Value & tv, const void * arg)
{
    //ACE_DEBUG((LM_NOTICE, "[%D] ISGWSighdl handle_timeout\n"));
    IsgwOperBase::instance()->time_out();
    
    return 0;
}

int ISGWSighdl::handle_reload()
{
    SysConf::instance()->load_conf();
    IsgwOperBase::instance()->reload_config();
    
    ACE_DEBUG((LM_INFO, "[%D] ISGWSighdl::handle_reload reload config\n"));
    return 0;
}

//int ISGWSighdl::handle_exception(ACE_HANDLE)
//{
//
//}

