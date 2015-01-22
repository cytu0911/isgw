
#ifndef __CSHMMANAGER_HPP__
#define __CSHMMANAGER_HPP__

#include <sys/shm.h>
#include <stdio.h>

const int DEFAULT_SHARE_MEMORY_KEY = 0x7D00;
#define MAC_SHM_FLAG 0666
#ifndef UNUSED_ARG
# define UNUSED_ARG(a) (a)
#endif

template <class T>
class CShmManager {
public:
    typedef T value_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    int m_iShmKey;
    CShmManager(int     a_iShmKey = DEFAULT_SHARE_MEMORY_KEY){m_iShmKey = a_iShmKey;}
    CShmManager(const CShmManager& obj)
    {
        this->m_iShmKey = obj.m_iShmKey;
    }           
    int shm_malloc_i( int     a_iShmKey, 
                  long    a_lShmSize, 
                  int*    a_piShmId, 
                  void**  a_pvAddr );
    
    pointer allocate(long    a_lShmSize) 
    { 
        int         ishmid;
        pointer pstTab;
        if (shm_malloc_i( m_iShmKey, a_lShmSize, &ishmid, (void**)&pstTab))
          {
              printf("failed to allocate shared memory(key:%d,size:%d)\n", m_iShmKey, a_lShmSize);
              return NULL;
          }
        return pstTab;
    }
    void deallocate(pointer  pAddr) 
    { 
        UNUSED_ARG(pAddr);
        return;
    }
};

template <class T>
int CShmManager<T>::shm_malloc_i( int     a_iShmKey, 
                  long    a_lShmSize, 
                  int*    a_piShmId, 
                  void**  a_pvAddr )
{
    int     iRet;
    int     iShmId;             /*共享内存ID*/
    void*   pvShmAddr = NULL;   /*共享内存首地址*/
    struct  shmid_ds stShm_Ds;  /*内存信息的数据结构*/

    
    iShmId = shmget( a_iShmKey, a_lShmSize, IPC_CREAT | MAC_SHM_FLAG );
    if ( iShmId < 0 )
    {   
        iShmId = shmget( a_iShmKey, 0, IPC_CREAT | MAC_SHM_FLAG );
        if ( iShmId < 0 )
        {
            printf("failed to get shared memory\n");
            return -1;
        }
    }
    
    /*为防止版本不对，在这里对大小进行严格的匹配 */
    iRet = shmctl( iShmId, IPC_STAT, &stShm_Ds );
    if( iRet < 0 )
    {
        printf("failed to get shared memory status\n");
        return iRet;
    }
    
    if( stShm_Ds.shm_segsz != a_lShmSize ) /*大小不对*/
    {
        printf("unknown shared memory version(%d,%d)\n", (int)stShm_Ds.shm_segsz, (int)a_lShmSize);
        return -1;
    }
    
    pvShmAddr = shmat( iShmId, NULL, 0 );
    if ( !pvShmAddr || (unsigned long)pvShmAddr == -1 )  /* operation failed. */
    {
        printf("failed to get shared memory address.\n");
        return -1;
    }

    *a_piShmId  = iShmId;
    *a_pvAddr   = pvShmAddr;
    return 0;
}   

#endif

