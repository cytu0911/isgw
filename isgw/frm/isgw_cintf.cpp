#include "isgw_cintf.h"
#include "stat.h"
#include "isgw_ack.h"
//��ack->msg ��������Ϣ��key��Ϣ��һ��Ϊsockfd sock_seq msg_seq
extern int isgw_cintf_parse(PriProAck *ack);

int ISGWCIntf::msg_seq_ = 0;
ISGWC_MSGQ ISGWCIntf::queue_(MSG_QUE_SIZE, MSG_QUE_SIZE); //��Ÿ������Ͻ��յ�����Ϣ 
ACE_Time_Value ISGWCIntf::zero_(0,0);

// ��Ҫ����Ľ������� (ȱʡ)
int isgw_cintf_parse_def(PriProAck * ack)
{
    QModeMsg qmode_ack(ack->msg);
    //�����Ҫֱ�ӷ��ظ��ͻ��˿��԰Ѵ��ֶ�����Ϊ1
    ack->rflag = atoi((*(qmode_ack.get_map()))["_rflag"].c_str());
    ack->sock_fd = strtoul((*(qmode_ack.get_map()))["_sockfd"].c_str(), NULL, 10);
    ack->protocol = strtoul((*(qmode_ack.get_map()))["_prot"].c_str(), NULL, 10);
    ack->sock_seq = strtoul((*(qmode_ack.get_map()))["_sock_seq"].c_str(), NULL, 10);
    ack->seq_no = strtoul((*(qmode_ack.get_map()))["_msg_seq"].c_str(), NULL, 10);
    ack->cmd = qmode_ack.get_cmd();
    ack->time = strtoul((*(qmode_ack.get_map()))["_time"].c_str(), NULL, 10);
    return 0;
}

int ISGWCIntf::init()
{
    //������Ϣ���д�С 
    int quesize = MSG_QUE_SIZE;	
    SysConf::instance()->get_conf_int("isgw_cintf", "quesize", &quesize); 
    ACE_DEBUG((LM_INFO, "[%D] ISGWCIntf set quesize=%d(byte)\n", quesize));
    queue_.open(quesize, quesize, NULL);
    
    return 0;
}

ISGWCIntf::ISGWCIntf() : recv_len_(0), msg_len_(0)
{
    memset(recv_buf_, 0x0, sizeof(recv_buf_));
}

ISGWCIntf::~ISGWCIntf()
{
    
}

int ISGWCIntf::open(void* p)
{
    if (AceSockHdrBase::open(p) != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] ISGWCIntf open failed,ip=%s\n"
            , remote_addr_.get_host_addr()));
        return -1;
    }
    
    //ipȨ�޼�����ʱȡ��
    if (is_auth() != 0)
    {
        return -1;
    }
    
    return 0;
}

#ifndef MSG_LEN_SIZE 
int ISGWCIntf::handle_input(ACE_HANDLE /*fd = ACE_INVALID_HANDLE*/)
{    
    //������Ϣ
    int ret = this->peer().recv((char*)recv_buf_ + recv_len_,
                                MAX_RECV_BUF_LEN - recv_len_); //, &timeout
    switch(ret)
    {
        case -1:
        {
            ACE_DEBUG((LM_ERROR, "[%D] ISGWCIntf recv failed"
				",ret=%d"
                ",errno=%d"
                ",errmsg=%s"
                ",ip=%s"
                "\n"
                , ret
                , errno
                , strerror(errno)
                , remote_addr_.get_host_addr()
                ));
			if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINPROGRESS)
            {
                return 0;
            }
            return -1;
        }
        break;
        case 0:
        {
            ACE_DEBUG((LM_TRACE, "[%D] ISGWCIntf recv failed"
                ",connection closed by foreign host"
				",ret=%d,ip=%s\n", ret, remote_addr_.get_host_addr()));
            return -1;
        }
        break;
        default: //���ճɹ�
        recv_len_ += ret;
        ACE_DEBUG((LM_TRACE, "[%D] ISGWCIntf recv succ,ret=%d,recv_buf_=%s\n"
            , ret, recv_buf_));

        //�ж���Ϣ�Ƿ����
        if (strstr(recv_buf_, MSG_SEPARATOR) == NULL) //δ��������������
        {
            if ( recv_len_ < MAX_RECV_BUF_LEN )
            {
                return 0;
            }
            else
            {
                ACE_DEBUG((LM_ERROR, "[%D] ISGWCIntf recv failed,illegal msg"
                    ",reach max buff len and have no msg end,discard it,ip=%s\n"
                    , remote_addr_.get_host_addr()));
                return -1; //��Ϣ�Ƿ�
            }
        }        
    }

    //ע�͵���ͬһ��������ж����Ϣ��������п��ܻḲ�Ǻ������Ϣͷ
    //recv_buf_[recv_len_] = '\0'; //���һλ�ÿձ�ʾ�ַ�������


    //�Ѿ������������Ϣ
    
    //�ж���Ϣ�Ƿ�Ƿ�
    if (is_legal(recv_buf_) != 0)
    {
    	ACE_DEBUG((LM_ERROR, "[%D] ISGWCIntf recv failed,illegal msg,discard it"
            ",ip=%s\n", remote_addr_.get_host_addr()));
    	recv_len_ = 0; // reset the recv pos indicator 
    	memset(recv_buf_, 0x0, sizeof(recv_buf_));
    	return 0;
    }

    //�򻯴�����ʱ�����Ƕ����Ϣ�����һ���͵ģ�����һ�η��Ͳ��ֲ���������Ϣ
    char *msg_start = recv_buf_;
    char *msg_end = NULL;
    int proced_len = 0; //�Ѿ�����ĳ���
    int pend_len = recv_len_; //ʣ�����Ϣ����
    
    while ((msg_end = strstr(msg_start, MSG_SEPARATOR)) != NULL)
    {
        msg_len_ = msg_end + strlen(MSG_SEPARATOR)-msg_start;  
        if (process(msg_start, get_handle(), get_seq(), msg_len_) != 0)
        {
            //����������ߴ������(����޴�������),������Ϣһ����,�����¶Ͽ�����    		
            return -1;
        }
        
        proced_len += msg_len_;
        pend_len -= msg_len_;
        msg_start = msg_end + strlen(MSG_SEPARATOR);  
        
        ACE_DEBUG((LM_TRACE, "[%D] ISGWCIntf process msg msg_len=%d"
			", proced_len=%d, pend_len=%d"
			"\n"
			, msg_len_
			, proced_len
			, pend_len
			));
    }
    ACE_DEBUG((LM_NOTICE, "[%D] ISGWCIntf out while,pend_len=%d,proced_len=%d\n"
		, pend_len, proced_len
		));
    
    //�ƶ�ʣ�µ�����
    memmove(recv_buf_, msg_start, pend_len);
    memset(recv_buf_+pend_len, 0x0, sizeof(recv_buf_)-pend_len);    
    recv_len_ = pend_len; // reset the recv pos indicator    
    
    return 0;
}

#else

int ISGWCIntf::handle_input(ACE_HANDLE /*fd = ACE_INVALID_HANDLE*/)
{
    //������Ϣ
    int ret = this->peer().recv((char*)recv_buf_ + recv_len_,
                                MAX_RECV_BUF_LEN - recv_len_); //, &timeout
    switch(ret)
    {
        case -1:
        {            
            ACE_DEBUG((LM_ERROR, "[%D] ISGWCIntf recv failed"
				",ret=%d"
                ",errno=%d"
                ",errmsg=%s"
                ",ip=%s"
                "\n"
                , ret
                , errno
                , strerror(errno)
                , remote_addr_.get_host_addr()
                ));
			if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINPROGRESS)
            {
                return 0;
            }
            return -1;
        }
        break;
        case 0:
        {
            ACE_DEBUG((LM_TRACE, "[%D] ISGWCIntf recv failed"
				",connection closed by foreign host"
                ",ret=%d,ip=%s\n"
                , ret, remote_addr_.get_host_addr()));
            return -1;
        }
        break;
        default: //���ճɹ�
        recv_len_ += ret;
        ACE_DEBUG((LM_TRACE, "[%D] ISGWCIntf recv succ,ret=%d"
            ",recv_buf_=%s\n"
            , ret, recv_buf_
            ));

        //�ж���Ϣ�Ƿ����
        if (msg_len_ == 0)
        {
            if (recv_len_ >= MSG_LEN_SIZE)
            {
                memcpy(&msg_len_, recv_buf_, MSG_LEN_SIZE);
                msg_len_ = ntohl(msg_len_);
                if (msg_len_ > MAX_RECV_BUF_LEN)
                {
                    ACE_DEBUG((LM_ERROR, "[%D] ISGWCIntf recv failed,illegal msg"
                        ",msg_len_=%d>MAX_RECV_BUF_LEN=%d,ip=%s\n"
                        , msg_len_, MAX_RECV_BUF_LEN, remote_addr_.get_host_addr()
                        ));
                    return -1;
                }
            }
            else
            {
                // not complete
                ACE_DEBUG((LM_TRACE, "[%D] ISGWCIntf recv succ but uncompleted msg 1\n"));
                return 0;
            }
        }
    
        if (recv_len_ - MSG_LEN_SIZE < msg_len_)
        {
            // not complete
            ACE_DEBUG((LM_TRACE, "[%D] ISGWCIntf recv succ but uncompleted msg 2"
                ",recv_len_=%d,msg_len_=%d\n"
                , recv_len_, msg_len_
                ));
            return 0;
        }
        
    }

    //ע�͵���ͬһ��������ж����Ϣ��������п��ܻḲ�Ǻ������Ϣͷ
    //recv_buf_[recv_len_] = '\0'; //���һλ�ÿձ�ʾ�ַ�������

    //�Ѿ������������Ϣ        
    unsigned int msg_len = msg_len_;
	
    unsigned int pend_len = recv_len_;//������ĳ���
    unsigned int proced_len = 0;	//�Ѿ�����ĳ���
    char* msg_buf = NULL;		//��Ϊrecv_buf_ ���α�ָ��
    int i = 0;

    //ѭ������ÿһ����������Ϣ
    while( pend_len >=  (MSG_LEN_SIZE + msg_len) )
    {			
        ACE_DEBUG((LM_TRACE, "[%D] ISGWCIntf in while,i=%d,pend_len=%d"
            ",proced_len=%d,msg_len=%d\n"
            , i++, pend_len, proced_len, msg_len));
        
        //��Ϣָ������
        msg_buf = &recv_buf_[proced_len + MSG_LEN_SIZE];
        
        //������ǰ��Ϣ����ָ������Ϣ�飬�����д���
        if (process(msg_buf, get_handle(), get_seq(), msg_len) != 0)
        {
            //����������ߴ������(����޴�������),������Ϣһ����,�����¶Ͽ�����    		
            return -1;
        }
        
        //�Ѿ�����ĳ��� ����
        proced_len += MSG_LEN_SIZE+msg_len;
        //������ĳ��ȼ���
        pend_len -= MSG_LEN_SIZE+msg_len;    	
        
        if (pend_len < MSG_LEN_SIZE)
        {
        	//���ʣ��ĳ��Ȳ����ĸ��ֽ�ֱ���˳�
        	msg_len = 0;
        	break;
        }
        
        //ȡ��һ��������Ϣ�ĳ���
        unsigned int temp_msg_len;
        //��proced_len λ��ȡ���ĸ��ֽڵ�����
        memcpy(&temp_msg_len, &recv_buf_[proced_len], sizeof(msg_len_));
        msg_len = ntohl(temp_msg_len);
        ACE_DEBUG((LM_TRACE, "[%D] ISGWCIntf end while,sizeof(msg_len_)=%d"
            ",pend_len=%d,proced_len=%d,msg_len=%d\n"
            , sizeof(msg_len_), pend_len, proced_len, msg_len
            ));
    }    
    ACE_DEBUG((LM_NOTICE, "[%D] ISGWCIntf out while,pend_len=%d,proced_len=%d,msg_len=%d\n"
		, pend_len, proced_len, msg_len
		));
    
    if ( pend_len == 0 )
    {
    	//���ô���������
    	recv_len_ = 0; // reset the recv pos indicator
    	msg_len_  = 0;		
    }	
    else
    {	
    	//ʣ��Ĳ���ֱ���ƶ�
    	ACE_DEBUG((LM_TRACE, "[%D] ISGWCIntf memmove,proced_len=%d,pend_len=%d"
    	, proced_len, pend_len));    	
    	memmove(recv_buf_, &recv_buf_[proced_len], pend_len);    	
    	recv_len_ = pend_len;//���յ�����Ϣ���ȸ�ֵΪ������ĳ���
    	msg_len_ = msg_len;//��һ����������Ϣ���ȵ���while�����һ��ȡ��������			
    }
    
    return 0;
}

#endif

int ISGWCIntf::process(char* msg, int sock_fd, int sock_seq, int msg_len)
{
    ACE_DEBUG((LM_TRACE, "[%D] in ISGWCIntf::process()\n"));
    //��Ϣ���зֶδ���
    //char ack_msg[MAX_INNER_MSG_LEN];
    char tmp_msg[MAX_INNER_MSG_LEN] = {0};
    memcpy(tmp_msg, msg, msg_len);
    
    char* ptr = NULL;
    char* p = ACE_OS::strtok_r(tmp_msg, MSG_SEPARATOR, &ptr);
    while (p != NULL) 
    { 
        PriProAck *ack = NULL;
        ACE_Object_Que<PriProAck>::instance()->dequeue(ack);
        if ( ack == NULL )
        {
            ACE_DEBUG((LM_ERROR,
                "[%D] ISGWCIntf dequeue msg failed,maybe system has no memory\n"
                ));
            return -1;
        }
        memset(ack, 0x0, sizeof(PriProAck));
        snprintf(ack->msg, sizeof(ack->msg)-1, "%s", p);
        // ��ȡ��ԭʼ�����ǰ��������Ϣ 
        ack->sock_ip = remote_addr_.get_ip_address();
        ack->sock_port = remote_addr_.get_port_number();
        ::gettimeofday(&(ack->tv_time), NULL);
#ifdef _ISGW_CINTF_PARSE_ 
        isgw_cintf_parse(ack);
#else
	isgw_cintf_parse_def(ack);
#endif 
        /*
        QModeMsg qmode_ack(ack->msg);
        ack->rflag = atoi((*(qmode_ack.get_map()))["_rflag"].c_str());
        ack->sock_fd = strtoul((*(qmode_ack.get_map()))["_sockfd"].c_str(), NULL, 10);
        ack->protocol = strtoul((*(qmode_ack.get_map()))["_prot"].c_str(), NULL, 10);
        ack->sock_seq = strtoul((*(qmode_ack.get_map()))["_sock_seq"].c_str(), NULL, 10);
        ack->seq_no = strtoul((*(qmode_ack.get_map()))["_msg_seq"].c_str(), NULL, 10);
        ack->cmd = qmode_ack.get_cmd();
        ack->time = strtoul((*(qmode_ack.get_map()))["_time"].c_str(), NULL, 10);
        */
        ACE_DEBUG((LM_NOTICE, "[%D] ISGWCIntf process msg"
            ",rflag=%d,sock_fd=%u,protocol=%u"
            ",ip=%u,port=%u,sock_seq=%u,seq_no=%u,time=%u"
            ",msg=%s\n"
            , ack->rflag
            , ack->sock_fd
            , ack->protocol
            , ack->sock_ip
            , ack->sock_port
            , ack->sock_seq
            , ack->seq_no
            , ack->time
            , ack->msg
            ));
        
        //���� rflag �ж� ��ֱ�ӷ��ؿͻ��� ���� �ŵ��������Ϣ������ 
        if (ack->rflag != 0)
        {
            strcat(ack->msg,MSG_SEPARATOR);//����Э������� 
            ISGWAck::instance()->putq(ack);
        }
        else
        {
            int ret = queue_.enqueue(ack, &zero_);
            ACE_DEBUG((LM_NOTICE, "[%D] ISGWCIntf process enqueue msg,ret=%d\n", ret));
            if (ret == -1)
            {
                //��¼��ʧ����Ϣ
                ACE_DEBUG((LM_ERROR, "[%D] ISGWCIntf process enqueue msg failed"
                    ",rflag=%d,sock_fd=%u,protocol=%u"
                    ",ip=%u,port=%u,sock_seq=%u,seq_no=%u,time=%u"
                    ",msg=%s\n"
                    , ack->rflag
                    , ack->sock_fd
                    , ack->protocol
                    , ack->sock_ip
                    , ack->sock_port
                    , ack->sock_seq
                    , ack->seq_no
                    , ack->time
                    , ack->msg
                    ));
                // ���ʧ�ܻ�����Ϣ�������ڴ�й©
                ACE_Object_Que<PriProAck>::instance()->enqueue(ack);
                // ͳ�Ƽ��� 
                Stat::instance()->incre_stat(STAT_CODE_ISGWC_ENQUEUE);
                // ���˵�����ģ�� �˴�����Ҫ�Ͽ����� 
                //return -1;
            }
        }
                
        p = ACE_OS::strtok_r(NULL, MSG_SEPARATOR, &ptr);
    }
    
    ACE_DEBUG((LM_TRACE, "[%D] out ISGWCIntf::process()\n"));	
    return 0;	
}

int ISGWCIntf::is_legal(char* msg)
{
    if (strstr(msg, MSG_SEPARATOR) == NULL || strstr(msg, FIELD_NAME_CMD) == NULL)
    {
        return -1;
    }
	
    return 0;
}
int ISGWCIntf::is_auth()
{
    int allow_flag = 0;//Ĭ�ϲ�������
    SysConf::instance()->get_conf_int("system", "allow_flag", &allow_flag);
	if (allow_flag == 0)
    {
        return 0;
    }
	
    char ip_list[1024];
    memset(ip_list, 0x0, sizeof(ip_list));    
    SysConf::instance()->get_conf_str("system", "allow_ip", ip_list, sizeof(ip_list));
    
    if (strstr(ip_list, remote_addr_.get_host_addr()) == NULL)
    {
        ACE_DEBUG((LM_ERROR, "[%D] ISGWCIntf auth failed"
			",ip %s not allowed,close connection\n"
    		, remote_addr_.get_host_addr()
    		));
        return -1;
    }
    
    return 0;
}

int ISGWCIntf::recvq(PriProAck*& msg, ACE_Time_Value* time_out)
{
    int ret = 0;
    
    ACE_DEBUG((LM_TRACE,
        "[%D] ISGWCIntf before recvq,msg count %d\n"
        , queue_.message_count()
        ));
    
    ret = queue_.dequeue(msg, time_out);
    if ( msg == NULL || ret == -1 )
    {
        ACE_DEBUG((LM_TRACE,
            "[%D] ISGWCIntf recvq,dequeue msg failed or recv a null msg\n"
            ));
        return ret;
    }
    ACE_DEBUG((LM_TRACE,
        "[%D] ISGWCIntf after recvq,msg count %d\n"
        , queue_.message_count()
        ));
    
    return ret;
}
