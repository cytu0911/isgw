#include "stat.h"
#include <assert.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

Stat* Stat::instance_ = NULL;
ACE_Thread_Mutex Stat::lock_;

Stat* Stat::instance()
{
    if (instance_ == NULL)
    {
        ACE_Guard<ACE_Thread_Mutex> guard(lock_);
        if (instance_ == NULL)
        {
            instance_ = new Stat();
        }
    }
    return instance_;
}
Stat::~Stat()
{
    stat_file_.close();
    if (instance_ != NULL)
    {
        delete instance_;
        instance_ = NULL;
    }
    return ;
}
int Stat::init(const char* file_name, int flag)
{
    // ����Ѿ���ʼ���ɹ� ���� ����ǿ�Ƴ�ʼ�� ��ֱ�ӷ��� 
    if (state_ == 1 && flag == 0)
    {
        return 0;
    }

    if (file_name != NULL && strlen(file_name) != 0)
    {
        strncpy(file_name_, file_name,sizeof(file_name_));
    }

    if (stat_file_.map(file_name_, file_size_,
        O_RDWR | O_CREAT, ACE_DEFAULT_FILE_PERMS,
        PROT_RDWR, ACE_MAP_SHARED) != 0)
    {
        return -1;
    }
    
    int* stat_array = (int*)stat_file_.addr();
    if (stat_array[0] != MAX_STAT_INDEX)
    {
        memset(stat_array, 0, file_size_);
        stat_array[0] = MAX_STAT_INDEX;
    }
    state_ = 1; //��ʼ���ɹ� 
    
    return 0;
}

int Stat::init_check(const char* file_name)
{
    if (stat_file_.map(file_name, file_size_,
        O_RDWR, ACE_DEFAULT_FILE_PERMS,
        PROT_RDWR, ACE_MAP_SHARED) != 0)
    {
        return -1;
    }
    return 0;
}

//����ͳ����,ֻ��Կ�ܴ���ͳ��
void Stat::incre_stat(int index, int incre)
{
    int pos = 0;
    if(index<MAX_STAT_INDEX&&index>=1)
    {
        pos = (index-1)*(sizeof(ReprtInfo)-sizeof(int))+sizeof(int);
        pos = pos + offsetof(ReprtInfo,total_count);
    }
    else if(index>=STAT_CODE_SVC_ENQUEUE&&
            index<STAT_CODE_SVC_ENQUEUE+MAX_ISGW_FAILED_ITEM)
    {
        pos = isgw_failed_offset_+(index-STAT_CODE_SVC_ENQUEUE)*sizeof(int);
    }
    else
        return;

    char* stat_array = (char*)stat_file_.addr();
    if (stat_array != NULL && state_ == 1)
    {
        #if 0 // def USE_RW_THREAD_MUTEX
            ACE_Guard<ACE_Thread_Mutex> guard(lock_);
        #endif
        *((int*)(stat_array+pos)) += incre;
    }
}

//�ۼ�ָ��ͳ����Ϣ,cmd�ķ�Χ��[1,10240]
void Stat::add_stat(ReprtInfo *info)
{
    if(info->cmd<1||info->cmd>MAX_STAT_INDEX) 
        return;
        
    int pos = (info->cmd-1)*(sizeof(ReprtInfo)-sizeof(int))+sizeof(int);
    
    char* stat_array = (char*)stat_file_.addr();
    if (stat_array != NULL)
    {
        #if 0 //def USE_RW_THREAD_MUTEX
            ACE_Guard<ACE_Thread_Mutex> guard(lock_);
        #endif
        ReprtInfo* cur_item = (ReprtInfo*)(stat_array+pos);
        cur_item->total_count += info->total_count;
        cur_item->failed_count += info->failed_count;
        cur_item->total_span += info->total_span;
        cur_item->procss_span += info->procss_span;
    }
}

int* Stat::get_stat()
{
    return (int*)stat_file_.addr();
}

void Stat::reset_stat()
{
    int* stat_array = (int*)stat_file_.addr();
    if (stat_array != NULL)
    {
        memset(stat_array + 1, 0,  file_size_-sizeof(int)); 
    }
}

void Stat::get_statstr(const char *stat_cfg, std::string &statstr)
{
    int* stat_array = (int*)stat_file_.addr();
    if (stat_array == NULL)
    {
        return;
    }
    
    int count = stat_array[0]; // ͳ���������
    
    std::vector<std::string> stat_desc_vec;
    if (stat_cfg != NULL && strlen(stat_cfg) != 0)
    {
        std::ifstream infile(stat_cfg);
        std::string line;
        while (std::getline(infile, line))
        {
            if (line.length() < 1)
            {
                break;
            }
            
            stat_desc_vec.push_back(line.substr(0,MAX_DESC_MSGLEN));
        }
    }
    char tmp_buf[256];

    std::string desc;
    if (stat_desc_vec.size() > 0)
    {
        desc = stat_desc_vec[0];
    }
    else
    {
        desc = "System Statistics";
    }

    sprintf(tmp_buf, "-------%s, total %d item   : total-count\tfailed-count\ttotal-span\tprocess-span \n"
        , desc.c_str(), count);
    statstr += tmp_buf;
    int pos =1;
    //��ӡָ���ͳ����Ϣ
    for (int i = 0; i < count; i ++,pos+=4)
    {
        memset(tmp_buf, 0x0, sizeof(tmp_buf));
        if (stat_desc_vec.size() > i+1)
        {
            desc = stat_desc_vec[i+1];
        }
        else
        {
            sprintf(tmp_buf, "Stat item %d", i+1); //���Ͼ����ȡֵ�ĸ��ֶΣ�������ά���� 
            desc = tmp_buf;
        }
        ReprtInfo* cur_item = (ReprtInfo*)(stat_array+pos);

        // ֻ��� ͳ���� ��ֵ ��Ϊ 0 �� 
        if (cur_item->total_count!= 0)
        {
            sprintf(tmp_buf, "%-45s: %d\t%d\t%u\t%u\n"
                , desc.c_str()
                , cur_item->total_count
                , cur_item->failed_count
                , cur_item->total_span
                , cur_item->procss_span
                );
            statstr += tmp_buf;
        }
    }
    
    //��ӡ��ܵĴ���ͳ��
    for(int i=0; i<MAX_ISGW_FAILED_ITEM; i++)
    {
        memset(tmp_buf, 0x0, sizeof(tmp_buf));
        sprintf(tmp_buf, "Stat item %d", i+STAT_CODE_SVC_ENQUEUE); 
        desc = tmp_buf;
        
        int pos = isgw_failed_offset_+sizeof(int)*i;
        int value = *(int*)(((char*)stat_array)+pos);

        // ֻ��� ͳ���� ��ֵ ��Ϊ 0 �� 
        if (value!= 0)
        {
            sprintf(tmp_buf, "%-45s: %d\t0\t0\t0\n"
                , desc.c_str()
                , value
                );
            statstr += tmp_buf;
        }
    }
    
    return;
}

#ifdef _STAT_DEMO_
int stat(int argc, const char* argv[])
{
    char szOperType[64];
    strcpy(szOperType,"print");
    char szStatFile[256];
    strcpy(szStatFile, "./.stat");
    char szStatDescCfgFile[256];
    memset(szStatDescCfgFile, 0x0, sizeof(szStatDescCfgFile));
	
    if (argc < 3)
    {
        std::cout<<"Usage:"<<argv[0]<<" <clear|print> <stat_file> [desc_file]"<<std::endl;
        return -1;
    }
	
    strncpy(szOperType, argv[1],sizeof(szOperType) -1);
    strncpy(szStatFile, argv[2], sizeof(szStatFile) -1);
    if (argc >= 4)
    {
        strncpy(szStatDescCfgFile, argv[3], sizeof(szStatDescCfgFile) -1);
    }
	
    //Stat stfile;
    int ret = 0;
    ret = Stat::instance()->init_check(szStatFile);
    if (ret != 0)
    {
        std::cerr<<argv[0]<<" init mmap file failed:"<<szStatFile<< std::endl;
        return -1;
    }
    
    if (0 == strcmp(szOperType,"clear"))
    {
        Stat::instance()->reset_stat();
    }
	
    if (0 == strcmp(szOperType,"print"))
    {
        std::string strout;
        Stat::instance()->get_statstr(szStatDescCfgFile, strout);
        std::cout << strout;
    }
    
    return 0;
}

int main(int argc, const char* argv[])
{
    return stat(argc,argv);
}
#endif //_STAT_DEMO_

