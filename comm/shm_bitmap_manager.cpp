#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>

#include "ace/Basic_Types.h"
#include "ace/Log_Msg.h"

#include "shm_bitmap_manager.h"

Shm_Bitmap_Manager* Shm_Bitmap_Manager::instance_ = NULL;

Shm_Bitmap_Manager::~Shm_Bitmap_Manager()
{
    
}

Shm_Bitmap_Manager* Shm_Bitmap_Manager::instance(void)
{
    if (NULL == instance_)
    {
        ACE_DEBUG((LM_INFO, "[%D] Shm_Bitmap_Manager::instance()\n"));
        instance_ = new Shm_Bitmap_Manager();
    }
    return instance_;
}

// ����KEY��������uin ��ʼ�������ڴ棬���ѹ����ڴ��ַӰ�䵽���̿ռ�
// ��������ڴ��Ѿ��������� max_uin ��Ч����ʱ���� ipcs -m ȷ�ϴ�С
int Shm_Bitmap_Manager::open(key_t key, ACE_UINT32 max_uin)
{
    key_t  key_handle;
    size_t size_handle;

    if ((0 == key) || (0 == max_uin))
    {
        key_handle   = PASSPORT_SHM_KEY;
        max_uin_     = MAX_UIN;
        size_handle  = PASSPORT_SHM_SIZE;
    }
    else 
    {
        key_handle   = key; 
        max_uin_     = max_uin;        
        // ÿλ��־һ������״̬��ÿ���ֽڴ���8������
        size_handle  = max_uin >> 3;
    }
    
    // Req 256M share memory
    shm_addr_ = get_share_memory(key_handle, size_handle);
    if (NULL == shm_addr_)
    {
        ACE_DEBUG((LM_ERROR, 
            "@[%D][%N,%l] init_shm error key=0x%08x, size=%u\n",
            key_handle, 
            size_handle));
        
        return -1;
    }
    
    ACE_DEBUG((LM_INFO, 
        "@[%D][%N,%l] init_shm successfull."
        "shm_addr=0x%08x, "
        "key=0x%08x, "
        "size=%u\n",
        shm_addr_, 
        key_handle, 
        size_handle));
    return 0;
}

// �ѹ����ڴ��ַ�ӽ��̿ռ�ȥӰ��
int Shm_Bitmap_Manager::close(void)
{
    int ret = 0;
	
    if (NULL != shm_addr_)
    {
        ret = shmdt(shm_addr_);
        if (-1 == ret)
        {
            ACE_DEBUG((LM_ERROR, 
                "@[%D][%N,%l] shmdt() addr=0x%08x,errno=%d\n",
                shm_addr_, errno));
        }
    }
    
    shm_addr_ = NULL;
    return ret;
}

// ��ȡ key= key�Ĺ����ڴ�ĵ�ַ����ӳ�䵽���̿ռ䣻
// ����������ڴ治���ڣ�����һ����С=size,key=key�Ĺ����ڴ棬
// ����ʼ����������Ϊ0
void * Shm_Bitmap_Manager::get_share_memory(key_t key, size_t size)
{
    void *shm_addr = NULL;
    key_t sem_key;
    int sem_id;
    sem_key = key;//ftok( pathname, key );
    ACE_DEBUG((LM_DEBUG, 
        "@[%D][%N,%l] sem_key=%u\n", sem_key));

    // ���ָ�����Ĺ��������Ƿ����
    sem_id = shmget( sem_key, size, (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP));

    // ָ�����Ĺ������β�����, �����µĹ�������, ����ʼ��ΪO
    if (-1 == sem_id)
    {
        if (ENOENT == errno)
        {
            // �����ڴ�� mode=0660�� ��ǰ�û������û�����û��ɶ���д 
            sem_id = shmget( sem_key, size, ((S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP)|IPC_CREAT));
            if(-1 == sem_id)
            {
                ACE_DEBUG((LM_ERROR, 
                    "@[%D][%N,%l] shmget() key=0x%08x,errno=%d\n",
                    sem_key, errno));
                
                return (void *)NULL;
            }
        
            shm_addr = shmat( sem_id, NULL, 0 );
            if ((void *)-1 == shm_addr)
            {
                ACE_DEBUG((LM_ERROR, 
                    "@[%D][%N,%l] shmat() sem_id=0x%08x,errno=%d\n",
                    sem_id, errno));
                return NULL;
                
            }
            else
            {
                memset(shm_addr, 0, size);
                ACE_DEBUG((LM_INFO, 
                    "@[%D][%N,%l] Create key=0x%08x,sem_id=%u,shm_addr=0x%08x\n",
                    sem_key, sem_id, shm_addr));
                return shm_addr;
            }
            
        }
        else
        {
            ACE_DEBUG((LM_ERROR, 
                "@[%D][%N,%l] shmget() key=0x%08x,errno=%d\n",
                sem_key, errno));
            return NULL;
        }
        
    }
    else
    {
        shm_addr = shmat( sem_id, NULL, 0 );
        if ((void *)-1 == shm_addr)
        {
            ACE_DEBUG((LM_ERROR, 
                "@[%D][%N,%l] shmat() sem_id=0x%08x,errno=%d\n",
                sem_id, errno));
            return NULL;
            
        }
        
        ACE_DEBUG((LM_DEBUG, 
            "@[%D][%N,%l] find key=0x%08x,sem_id=%u,shmat_addr=0x%08x\n",
            sem_key, sem_id, shm_addr));
        return shm_addr;
    }
    
}


// ��ѯuin��bitmapֵ״̬
int Shm_Bitmap_Manager::get_bit(ACE_UINT32 uin)
{
    if (uin < max_uin_)
    {
        return handle_shm_bit_i(shm_addr_, uin, BITMAP_CHK);
    }
    else
    {
        return 0;
    }
}

// uin��bitmapֵ״̬��1 
// ����ֵΪ�����ڴ洦����bit ֵ
int Shm_Bitmap_Manager::set_bit(ACE_UINT32 uin)
{
    if (uin < max_uin_)
    {
        return handle_shm_bit_i(shm_addr_, uin, BITMAP_SET);
    }
    else
    {
        return 0;
    }
}

// uin��bitmapֵ״̬��0
// ����ֵΪ�����ڴ洦����bit ֵ
int Shm_Bitmap_Manager::clr_bit(ACE_UINT32 uin)
{
    if (uin < max_uin_)
    {
        return handle_shm_bit_i(shm_addr_, uin, BITMAP_CLR);
    }
    else
    {
        return 0;
    }
}

inline
int Shm_Bitmap_Manager::handle_shm_bit_i(void * shm_addr, ACE_UINT32 uin, int flag)
{
    ACE_UINT8 bit_now = 0;    
    ACE_UINT32 uin_byte_addr_offset = (uin>>3);  // uin ǰ29λ
    ACE_UINT8 uin_bit = (1<<(uin & 0x07));        // uin ��3λ�� �ֽ�λ�������Դν�ʡ�ڴ�
    ACE_UINT8 uin_byte =0;
    unsigned char* shm_map_addr = (unsigned char*)shm_addr;
    ACE_UINT8 *curr_addr = (ACE_UINT8 *)(shm_map_addr + uin_byte_addr_offset);

    if ((NULL != shm_map_addr)&& ((void *)-1 != shm_map_addr))
    {
        // uin 
        uin_byte = (*(ACE_UINT8 *)curr_addr);
        ACE_UINT8 bit_old = (uin_byte & uin_bit) ? 1:0;

        switch (flag)
        {
        case BITMAP_CHK: // check bit
            bit_now = ((*curr_addr) & uin_bit) ? 1:0;
            break;

        case BITMAP_SET: // set bit
            (*curr_addr) = uin_byte | uin_bit;
            bit_now = ((*curr_addr) & uin_bit) ? 1:0;

            break;

        case BITMAP_CLR: //clr bit
            (*curr_addr) = uin_byte  & (~uin_bit);
            bit_now = ((*curr_addr) & uin_bit) ? 1:0;

            break;

        default:
            break;
        }

        ACE_DEBUG((LM_DEBUG, 
            "uin=%u,%d=>%d\n", 
            uin,
            bit_old, 
            bit_now));


    }
    else 
    {
        ACE_DEBUG((LM_DEBUG, 
            "@[%D][%N,%l]shm_addr=%u, uin=%u,uin_byte=0x%02X,uin_bit=0x%02X\n", 
            shm_addr,
            uin,
            uin_byte, 
            uin_bit
            ));
        return -1;
    }

    return bit_now;
}

int Shm_Bitmap_Manager::count_bit()
{
    return count_shm_set_bit_num(shm_addr_);
}

int Shm_Bitmap_Manager::count_shm_set_bit_num(void * shm_addr)
{
    ACE_UINT32 iNum = 0;
    ACE_UINT32  uin = 10000;
    for(; uin < max_uin_; uin ++)
    {
        ACE_UINT32 uin_byte_addr_offset = (uin>>3);  // uin ǰ29λ
        ACE_UINT8 uin_bit = (1<<(uin & 0x07));        // uin ��3λ�� �ֽ�λ�������Դν�ʡ�ڴ�
        ACE_UINT8 uin_byte =0;
        unsigned char* shm_map_addr = (unsigned char*)shm_addr;
        ACE_UINT8 *curr_addr = (ACE_UINT8 *)(shm_map_addr + uin_byte_addr_offset);

        ACE_UINT8 bit_now = 0;
        if ((NULL != shm_map_addr)&& ((void *)-1 != shm_map_addr))
        {
            // uin 
            uin_byte = (*(ACE_UINT8 *)curr_addr);
            ACE_UINT8 bit_old = (uin_byte & uin_bit) ? 1:0;
            if (1 == bit_old)
            {
                iNum++;
            }
        }
        else 
        {
            ACE_DEBUG((LM_ERROR, 
                "@[%D][%N,%l]shm_addr=%u, uin=%u,uin_byte=0x%02X,uin_bit=0x%02X\n", 
                shm_addr,
                uin,
                uin_byte, 
                uin_bit
                ));
            return 0;
        }
    }
    return iNum;
}
