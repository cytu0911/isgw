
#ifndef __CMEMMANAGER_HPP__
#define __CMEMMANAGER_HPP__

#include <sys/shm.h>
#include <stdio.h>
#include <CLruHashTableEx.hpp>

#ifndef UNUSED_ARG
# define UNUSED_ARG(a) (a)
#endif

template <class T>
class CMemManager {
public:
    typedef T value_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    CMemManager(){}
    CMemManager(const CMemManager& obj)
    {
    }           
   
    pointer allocate(long    a_lShmSize) 
    { 
        pointer pstTab = (pointer)malloc( a_lShmSize);
        if (NULL == pstTab)
          {
              printf("failed to allocate memory(size:%d)\n", a_lShmSize);
              return NULL;
          }
        return pstTab;
    }
    void deallocate(pointer  pAddr) 
    { 
        //UNUSED_ARG(pAddr);
        free(pAddr);
        return;
    }
};

typedef CMemManager<SHTABLE> memAlloc;

#endif /*__CMEMMANAGER_HPP__*/

