#include "isgw_intf.h"
#include "isgw_mgr_svc.h"
#include "stat.h"
#include "isgw_ack.h"

int ISGWIntf::msg_seq_ = 0;

ISGWIntf::ISGWIntf() : recv_len_(0), msg_len_(0)
{
    //memset(recv_buf_, 0x0, sizeof(recv_buf_));
}

ISGWIntf::~ISGWIntf()
{
    
}

int ISGWIntf::open(void* p)
{
    if (AceSockHdrBase::open(p) != 0)
    {
        ACE_DEBUG((LM_ERROR, "[%D] ISGWIntf open failed,ip=%s\n"
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
int ISGWIntf::handle_input(ACE_HANDLE /*fd = ACE_INVALID_HANDLE*/)
{
    //������Ϣ
    int ret = this->peer().recv((char*)recv_buf_ + recv_len_,
                                MAX_RECV_BUF_LEN - recv_len_); //, &timeout    
    switch(ret)
    {
        case -1:
        {
            ACE_DEBUG((LM_ERROR, "[%D] ISGWIntf recv failed"
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
            ACE_DEBUG((LM_TRACE, "[%D] ISGWIntf recv failed"
                ",connection closed by foreign host"
				",ret=%d,ip=%s\n"
				, ret, remote_addr_.get_host_addr()
				));
            return -1;
        }
        break;
        default: //���ճɹ�
        recv_len_ += ret;
        ACE_DEBUG((LM_TRACE, "[%D] ISGWIntf recv succ,recv_len=%d,ret=%d,recv_buf=%s\n"
            , recv_len_, ret, recv_buf_));

        //�����ж���Ϣ�Ƿ����(���Ƿ��Ѿ���һ����������Ϣ��)
        //��Ϊ�˴��жϿ��ܰ�����������������Ľ�����
        if ( strstr(recv_buf_, MSG_SEPARATOR) == NULL) //δ��������������
        {
            if ( recv_len_ < MAX_RECV_BUF_LEN )
            {
                ACE_DEBUG((LM_WARNING, "[%D] ISGWIntf recv succ,no end"
                    ",recv_len=%d,recv_buf=%s\n"
                    , recv_len_, recv_buf_));
                return 0;
            }
            else
            {
                ACE_DEBUG((LM_ERROR, "[%D] ISGWIntf recv failed,illegal msg"
                    ",reach max buff len and have no msg end,discard it,ip=%s\n"
                    , remote_addr_.get_host_addr()
                    ));
                return -1; //��Ϣ�Ƿ�
            }
        }
		
    }
    
    //�Ѿ������������Ϣ
    
    //�ж���Ϣ�Ƿ�Ƿ�
    if (is_legal(recv_buf_) != 0)
    {
    	ACE_DEBUG((LM_ERROR, "[%D] ISGWIntf recv failed,illegal msg,discard it"
            ",ip=%s\n", remote_addr_.get_host_addr()
            ));
    	recv_len_ = 0; // reset the recv pos indicator 
    	//memset(recv_buf_, 0x0, sizeof(recv_buf_));
    	return 0;
    }

    char *msg_start = recv_buf_;
    char *msg_end = NULL;
    int proced_len = 0; //�Ѿ�����ĳ���
    int pend_len = recv_len_; //pend_len δ�������Ϣ����

    //proced_len ����С��recv_len_ ���⴦����buf������������� 
    while ((msg_end = strstr(msg_start, MSG_SEPARATOR)) != NULL && proced_len < recv_len_)
    {
        msg_len_ = msg_end + strlen(MSG_SEPARATOR)-msg_start;
        // �����Ƿ�ᴦ���������������(����������Ϣ)
        if((proced_len+msg_len_) > recv_len_)
        {
            ACE_DEBUG((LM_WARNING, "[%D] ISGWIntf proc no end msg"
                ",proced_len=%d,msg_len_=%d,recv_len_=%d\n"
                , proced_len, msg_len_, recv_len_
                ));
            break;
        }
        if (process(msg_start, get_handle(), get_seq(), msg_len_) != 0)
        {
            //����������ߴ������(����޴�������),������Ϣһ����,�����¶Ͽ�����    		
            return -1;
        }
        
        proced_len += msg_len_;
        pend_len -= msg_len_;
        msg_start = msg_end + strlen(MSG_SEPARATOR);  
        
        ACE_DEBUG((LM_TRACE, "[%D] ISGWIntf proc msg"
			",msg_len=%d,proced_len=%d,pend_len=%d\n"
			, msg_len_, proced_len, pend_len));
    }
    
    //�ƶ�ʣ�µ�����
    if (msg_start!=recv_buf_)
    {
        memmove(recv_buf_, msg_start, pend_len);
        ACE_DEBUG((LM_TRACE, "[%D] ISGWIntf proc msg move,start=%d,pend_len=%d\n"
            , (msg_start-recv_buf_), pend_len));
    }
    
    //memset(recv_buf_+pend_len, 0x0, sizeof(recv_buf_)-pend_len);
    recv_len_ = pend_len; // reset the recv pos indicator    
    
    return 0;
}

#else
// ����Ϣ���ȵ���Ϣ
int ISGWIntf::handle_input(ACE_HANDLE /*fd = ACE_INVALID_HANDLE*/)
{
    // �ı�Э��,�����ֶ�����Ϣ�ж���ռMSG_LEN_SIZE�ֽ�
    unsigned int len_field_extra_size = MSG_LEN_SIZE;          
#ifdef BINARY_PROTOCOL
    // ������Э��,�����ֶα����������Ϣ��
    len_field_extra_size = 0;                                  
#endif 

    //������Ϣ
    int ret = this->peer().recv((char*)recv_buf_ + recv_len_,
                                MAX_RECV_BUF_LEN - recv_len_); //, &timeout
    switch(ret)
    {
        case -1:
        {            
            ACE_DEBUG((LM_ERROR, "[%D] ISGWIntf recv failed"
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
            ACE_DEBUG((LM_TRACE, "[%D] ISGWIntf recv failed"
				",connection closed by foreign host"
                ",ret=%d,ip=%s\n"
                , ret, remote_addr_.get_host_addr()));
            return -1;
        }
        break;
        default: //���ճɹ�
        recv_len_ += ret;
        ACE_DEBUG((LM_TRACE, "[%D] ISGWIntf recv succ,ret=%d"
            ",recv_buf_=%s\n"
            , ret, recv_buf_
            ));

        //�ж���Ϣ�Ƿ����
        if (msg_len_ == 0) //û��ȡ����Ϣ�����ȵ����
        {
            if (recv_len_ >= MSG_LEN_SIZE)
            {
                memcpy(&msg_len_, recv_buf_, MSG_LEN_SIZE);
                if(MSG_LEN_SIZE == 2)
                {
                    msg_len_ = ntohs(msg_len_);
                }
                else
                {
                    msg_len_ = ntohl(msg_len_);
                }
                
                if (msg_len_ > MAX_RECV_BUF_LEN || msg_len_ <= MSG_LEN_SIZE)
                {
                    ACE_DEBUG((LM_ERROR, "[%D] ISGWIntf recv failed,illegal msg"
                        ",msg_len_=%d,ip=%s\n"
                        , msg_len_, remote_addr_.get_host_addr()
                        ));
                    return -1;
                }
            }
            else
            {
                // not complete
                ACE_DEBUG((LM_TRACE, "[%D] ISGWIntf recv succ but uncompleted msg 1\n"));
                return 0;
            }
        }

        if (recv_len_ < (len_field_extra_size + msg_len_ ))
        {
            // not complete
            ACE_DEBUG((LM_TRACE, "[%D] ISGWIntf recv succ but uncompleted msg 2"
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
    while( pend_len >=  (len_field_extra_size + msg_len) )
    {			
        ACE_DEBUG((LM_TRACE, "[%D] ISGWIntf in while,i=%d,pend_len=%d"
            ",proced_len=%d,msg_len=%d\n"
            , i, pend_len, proced_len, msg_len));
        
        //��Ϣָ������
        msg_buf = &recv_buf_[proced_len + len_field_extra_size];
        
        //������ǰ��Ϣ����ָ������Ϣ�飬�����д���
        if (process(msg_buf, get_handle(), get_seq(), msg_len) != 0)
        {
            //����������ߴ������(����޴�������),������Ϣһ����,�����¶Ͽ�����    		
            return -1;
        }
        
        //�Ѿ�����ĳ��� ����
        proced_len += len_field_extra_size + msg_len;
        //������ĳ��ȼ���
        pend_len -= len_field_extra_size + msg_len;
        
        if (pend_len < MSG_LEN_SIZE)
        {
        	//���ʣ��ĳ��Ȳ����ĸ��ֽ�ֱ���˳�
        	break;
        }
        
        //ȡ��һ��������Ϣ�ĳ���
        //��proced_len λ��ȡ��
        msg_len = 0;
        memcpy(&msg_len , &recv_buf_[proced_len], MSG_LEN_SIZE);
        if(MSG_LEN_SIZE == 2)
        {
            msg_len = ntohs(msg_len);
        }
        else
        {
            msg_len = ntohl(msg_len);
        }
        ACE_DEBUG((LM_TRACE, "[%D] ISGWIntf end while,i=%d,MSG_LEN_SIZE=%d"
            ",pend_len=%d,proced_len=%d,msg_len=%d\n"
            , i, MSG_LEN_SIZE, pend_len, proced_len, msg_len
            ));
        
        // ���msg_len �쳣 ��Ͽ�
        if (msg_len > MAX_RECV_BUF_LEN || msg_len <= MSG_LEN_SIZE)
        {
            ACE_DEBUG((LM_ERROR, "[%D] ISGWIntf recv failed,illegal msg"
                ",msg_len=%d,ip=%s\n"
                , msg_len, remote_addr_.get_host_addr()
                ));
            return -1;
        }
        
        i++;
    }    
    
    ACE_DEBUG((LM_DEBUG, "[%D] ISGWIntf out while,pend_len=%d,proced_len=%d"
		",msg_len=%d\n"
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
    	ACE_DEBUG((LM_TRACE, "[%D] ISGWIntf memmove,proced_len=%d,pend_len=%d"
    	, proced_len, pend_len));    	
    	memmove(recv_buf_, &recv_buf_[proced_len], pend_len);    	
    	recv_len_ = pend_len;//���յ�����Ϣ���ȸ�ֵΪ������ĳ���
    	msg_len_ = msg_len;//��һ����������Ϣ���ȵ���while�����һ��ȡ��������			
    }
    
    return 0;
}

#endif

#ifndef BINARY_PROTOCOL
int ISGWIntf::process(char* msg, int sock_fd, int sock_seq, int msg_len)
{
    ACE_DEBUG((LM_TRACE, "[%D] in ISGWIntf::process()\n"));
    //��Ϣ���зֶδ���
    char tmp_msg[MAX_INNER_MSG_LEN] = {0}; //ʹ����ʱ����������ԭʼ��Ϣ���ƻ�
    //memset(tmp_msg, 0x0, sizeof(tmp_msg));
    memcpy(tmp_msg, msg, msg_len);
    
    char* ptr = NULL;
    char* p = ACE_OS::strtok_r(tmp_msg, MSG_SEPARATOR, &ptr);
    while (p != NULL) 
    {
        PriProReq *req = NULL;
        ACE_Object_Que<PriProReq>::instance()->dequeue(req);
        if ( req == NULL )
        {
            ACE_DEBUG((LM_ERROR,
                "[%D] ISGWIntf dequeue msg failed,maybe system has no memory\n"
                ));
            return -1;
        }
        
        //memset(req, 0x0, sizeof(PriProReq));
        req->seq_no = msg_seq_++;
        req->sock_fd = sock_fd;
        req->protocol = PROT_TCP_IP;
        req->sock_ip = remote_addr_.get_ip_address();
        req->sock_port = remote_addr_.get_port_number();
        req->sock_seq = sock_seq;
        //req->cmd = 0;
        ::gettimeofday(&(req->tv_time), NULL);
        req->msg_len = snprintf(req->msg, sizeof(req->msg)-1, "%s", p);
        ACE_DEBUG((LM_NOTICE, "[%D] ISGWIntf putq msg to ISGWMgrSvc"
            ",sock_fd=%u,protocol=%u,ip=%u,port=%u,sock_seq=%u,seq_no=%u,time=%u"
            ",msg=%s\n"
            , req->sock_fd
            , req->protocol
            , req->sock_ip
            , req->sock_port
            , req->sock_seq
            , req->seq_no
            , req->tv_time.tv_sec
            , req->msg
            ));
        if (ISGWMgrSvc::instance()->putq(req) == -1)
        {
            //��¼��ʧ����Ϣ
            ACE_DEBUG((LM_ERROR, "[%D] ISGWIntf putq msg to ISGWMgrSvc failed"
                ",sock_fd=%u,protocol=%u,ip=%u,port=%u,sock_seq=%u,seq_no=%u,time=%u"
                ",msg=%s\n"
                , req->sock_fd
                , req->protocol
                , req->sock_ip
                , req->sock_port
                , req->sock_seq
                , req->seq_no
                , req->tv_time.tv_sec
                , req->msg
                ));
            //���ʧ�ܻ�����Ϣ�������ڴ�й©
            ACE_Object_Que<PriProReq>::instance()->enqueue(req);
            // ͳ�Ƽ��� 
            Stat::instance()->incre_stat(STAT_CODE_SVC_ENQUEUE);
            //���ʧ�ܿ��Է���ʧ�� ���ӻ�Ͽ� 
            return -1;
        }
        
        p = ACE_OS::strtok_r(NULL, MSG_SEPARATOR, &ptr);
    }

    ACE_DEBUG((LM_TRACE, "[%D] out ISGWIntf::process()\n"));	
    return 0;	
}

#else

int ISGWIntf::process(char* msg, int sock_fd, int sock_seq, int msg_len)
{
    ACE_DEBUG((LM_TRACE, "[%D] in ISGWIntf::process()\n"));
   
    PriProReq *req = NULL;
    ACE_Object_Que<PriProReq>::instance()->dequeue(req);
    if ( req == NULL )
    {
        ACE_DEBUG((LM_ERROR,
            "[%D] ISGWIntf dequeue msg failed,maybe system has no memory\n"
            ));
        return -1;
    }
    
    //memset(req, 0x0, sizeof(PriProReq));
    req->seq_no = msg_seq_++;
    req->sock_fd = sock_fd;
    req->protocol = PROT_TCP_IP;
    req->sock_ip = remote_addr_.get_ip_address();
    req->sock_port = remote_addr_.get_port_number();
    req->sock_seq = sock_seq;
    //req->cmd = 0;
    ::gettimeofday(&(req->tv_time), NULL);
    memcpy(req->msg, msg, msg_len);
    req->msg_len = msg_len;
    ACE_DEBUG((LM_NOTICE, "[%D] ISGWIntf putq msg to ISGWMgrSvc"
        ",sock_fd=%u,protocol=%u,ip=%u,port=%u,sock_seq=%u,seq_no=%u,time=%u"
        ",msg=%s\n"
        , req->sock_fd
        , req->protocol
        , req->sock_ip
        , req->sock_port
        , req->sock_seq
        , req->seq_no
        , req->tv_time.tv_sec
        , req->msg
        ));
    if (ISGWMgrSvc::instance()->putq(req) == -1)
    {
        //��¼��ʧ����Ϣ
        ACE_DEBUG((LM_ERROR, "[%D] ISGWIntf putq msg to ISGWMgrSvc failed"
            ",sock_fd=%u,protocol=%u,ip=%u,port=%u,sock_seq=%u,seq_no=%u,time=%u"
            ",msg=%s\n"
            , req->sock_fd
            , req->protocol
            , req->sock_ip
            , req->sock_port
            , req->sock_seq
            , req->seq_no
            , req->tv_time.tv_sec
            , req->msg
            ));
        //���ʧ�ܻ�����Ϣ�������ڴ�й©
        ACE_Object_Que<PriProReq>::instance()->enqueue(req);
        // ͳ�Ƽ��� 
        Stat::instance()->incre_stat(STAT_CODE_SVC_ENQUEUE);
        //���ʧ�ܿ��Է���ʧ�� ���ӻ�Ͽ� 
        return -1;
    }

    ACE_DEBUG((LM_TRACE, "[%D] out ISGWIntf::process()\n"));	
    return 0;	
}

#endif

int ISGWIntf::is_legal(char* msg)
{
    // Ŀǰֻ�ж��Ƿ���������
    if (strstr(msg, FIELD_NAME_CMD) == NULL)
    {
        return -1;
    }
    
    return 0;
}
int ISGWIntf::is_auth()
{
    int allow_flag = 0;//Ĭ�ϲ�������
    SysConf::instance()->get_conf_int("system", "allow_flag", &allow_flag);
	if (allow_flag == 0)
    {
        return 0;
    }
	
    char ip_list[1024] = {0};
    SysConf::instance()->get_conf_str("system", "allow_ip", ip_list, sizeof(ip_list));
    
    if (strstr(ip_list, remote_addr_.get_host_addr()) == NULL)
    {
        ACE_DEBUG((LM_ERROR, "[%D] ISGWIntf auth failed"
			",ip %s not allowed,close connection\n"
    		, remote_addr_.get_host_addr()
    		));
        return -1;
    }
    
    return 0;
}
