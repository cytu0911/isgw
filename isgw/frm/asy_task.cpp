#include "asy_task.h"
#include "isgw_cintf.h"
#include "isgw_ack.h"

ASYTask* ASYTask::instance_ = NULL;
ASYR_MAP ASYTask::asyr_map_;
ACE_Thread_Mutex ASYTask::asyr_map_lock_;
ASY_PROC ASYTask::asy_proc_ = NULL;

int ASYTask::init()
{
    ACE_DEBUG((LM_TRACE,
		"[%D] in ASYTask::init\n"
		));

    //active num thread
    activate(THR_NEW_LWP | THR_JOINABLE, DEFAULT_THREADS);

    ACE_DEBUG((LM_INFO, "[%D] ASYTask init succ,threads=%d\n"
        , DEFAULT_THREADS
        ));
    
    ACE_DEBUG((LM_TRACE,
		"[%D] out ASYTask::init\n"
		));
    return 0;
}

int ASYTask::fini()
{
    ACE_DEBUG((LM_TRACE,
		"[%D] in ASYTask::fini\n"
		));

    this->thr_mgr()->kill_grp(this->grp_id(), SIGINT);
    this->thr_mgr()->wait_grp(this->grp_id());
    
    ACE_DEBUG((LM_TRACE,
		"[%D] out ASYTask::fini\n"
		));
    return 0;
}

int ASYTask::insert(ASYRMsg &rmsg)
{
    ACE_Guard<ACE_Thread_Mutex> guard(asyr_map_lock_);
    if(rmsg.value.time == 0)
    {
        rmsg.value.time = ISGWAck::instance()->get_time();
    }
    asyr_map_.insert(pair<ASYRKey, ASYRValue>(rmsg.key, rmsg.value));
    // �жϼ�¼���ǲ��ǹ��࣬���̫����Ҫ����һЩ�ϵļ�¼
    if (asyr_map_.size()>MAX_ASYR_RECORED)
    {
        ASYR_MAP::iterator it;
        int now = ISGWAck::instance()->get_time();
        for(it=asyr_map_.begin(); it!=asyr_map_.end(); it++)
        {
            if(now - it->second.time > DISCARD_TIME)
            {
                asyr_map_.erase(it);
            }
        }        
    }
    
    ACE_DEBUG((LM_NOTICE, "[%D] ASYTask insert succ,map size=%d\n", asyr_map_.size()));
    return 0;
}

int ASYTask::set_proc(ASY_PROC asy_proc)
{
	asy_proc_ = asy_proc;
	return 0;
}

int ASYTask::svc(void)
{
    unsigned int threadid = syscall(__NR_gettid); //ACE_OS::thr_self();
    unsigned int pid = ACE_OS::getpid();
    
    ACE_DEBUG((LM_INFO,
        "[%D] ASYTask (%u:%u) enter svc ...\n"
        , threadid , pid
        ));
    
    do
    {
        PriProAck *ack = NULL;
        int ret = ISGWCIntf::recvq(ack);// , &time_out
        if (ack != NULL)
        {
            ACE_DEBUG((LM_NOTICE,
                "[%D] ASYTask (%u:%u) recvq msg,ret=%d,msg=%s\n"
                , threadid , pid, ret, ack->msg
                ));
            // ���� ack ��Ϣ,���뱾����Ϣ���ж�����Ҫ�����ٴ����
            ASYRKey key;
            key.sock_fd = ack->sock_fd;
            key.sock_seq = ack->sock_seq;
            key.msg_seq = ack->seq_no;

            ASYR_MAP::iterator it;
            ASYRValue prvalue; //��ֵ���浽��ʱ����������map����         
            //int flag = 0; // ��ʶ�Ƿ��ҵ���Ӧ�ļ�¼
            
            {//�������ķ�Χ
                ACE_Guard<ACE_Thread_Mutex> guard(asyr_map_lock_);            
                it = asyr_map_.find(key);
                if(it != asyr_map_.end()) 
                {
                    ACE_DEBUG((LM_TRACE, "[%D] ASYTask find match record\n")); 
                    prvalue = it->second; //(ASYRValue) 
                    //flag = 1;
                    //ɾ��map��ļ�¼
                    asyr_map_.erase(it);
                }
                else
                {
                    ACE_DEBUG((LM_ERROR, "[%D] ASYTask find record failed\n"));
                    //����ack��Ϣ����ֱ�ӷ��ظ�ǰ��
                }
            }

            //����Ϣ���лص�����
            //if (flag == 1)
            //{
            int ack_len = MAX_INNER_MSG_LEN; //��Ϊ���봫��������Ĭ��Ϊbuf�Ĵ�С
            char ack_buf[MAX_INNER_MSG_LEN+1] = {0}; // ����Ե�ַ&�ָ�Ľ����Ϣ
            QModeMsg qmode_ack(ack->msg, ack->sock_fd, ack->sock_seq
                    , ack->seq_no, ack->protocol, ack->time, ack->sock_ip, ack->sock_port);
            qmode_ack.set_tvtime(&(ack->tv_time));
            if (prvalue.asy_proc!=NULL)
            {
                //����ҵ��ص�
                prvalue.asy_proc(qmode_ack, prvalue.content, ack_buf, ack_len);
            }
            else if(asy_proc_!=NULL)
            {
                asy_proc_(qmode_ack, prvalue.content, ack_buf, ack_len);
            }
            
            //���� ack_len �Ľ��������Ӧ��Ϣ
            if (ack_len < 0) //����ͻ��˷�����Ӧ��Ϣ
            {
                //������Ӧ��Ϣ��Դ
                if (ack != NULL)
                {
                    ACE_Object_Que<PriProAck>::instance()->enqueue(ack);
                    ack = NULL;
                }
            }
            else if (ack_len == 0 || ack_len == MAX_INNER_MSG_LEN) //���տ�ܶ����Э����Ӧ
            {
                //ƴװ������Ϣ��������Э�������
                snprintf(ack->msg, MAX_INNER_MSG_LEN, "%s%s", ack_buf, MSG_SEPARATOR);
            }
            else //��ȫ�����û������Э�������Ϣ �������κ����� 
            {
                if (ack_len > MAX_INNER_MSG_LEN)
                {
                    ACE_DEBUG((LM_ERROR, "[%D] warning: ack msg len is limited,ack_len=%d\n", ack_len));
                    ack_len = MAX_INNER_MSG_LEN;
                }
                
                ack->msg_len = ack_len;
                memcpy(ack->msg, ack_buf, ack->msg_len);
            }
            //}
                        
            // ������ɷ��͸��ͻ���
            if (ack!=NULL)
            {
                ISGWAck::instance()->putq(ack);
            }
            
        }
    }while(1);
    
    ACE_DEBUG((LM_INFO,
        "[%D] ASYTask (%u:%u) leave svc ...\n"
        , threadid , pid
        ));
    return 0;
}


