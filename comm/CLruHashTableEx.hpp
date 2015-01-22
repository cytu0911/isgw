
#ifndef __C_LRUHASHTABLEEX_HPP__
#define __C_LRUHASHTABLEEX_HPP__

#include <iostream>
#include <utility>
#include "CShmManager.hpp"

using namespace std;

enum { _INIT_TABLE = 1,_CHECK_TABLE = 0 };

#define SHT_MIN_HEAD			0x100
#define SHT_BUCKET_ALIGN		0x100
#define SHT_MIN_ALIGN			0x08
#define SHT_VERSION				0x0101

#define SHTF_NEEDFREE			0x01
typedef unsigned long ULONG_PTR;

struct tagSHitem
{
	int iPrev;
	int iNext;
    int iLruPrev;
    int iLruNext;
	unsigned uCode;
	int fValid;
	char szData[1];
};

typedef struct tagSHitem		SHITEM;
typedef struct tagSHitem		*LPSHITEM;

struct tagSHbucket
{
	int iCount;
	int iHead;
};

typedef struct tagSHbucket		SHBUCKET;
typedef struct tagSHbucket		*LPSHBUCKET;

struct tagSHtable
{
	unsigned int cbSize;		/* the size of this struct. */
	unsigned int uFlags;		/* some flags. */
	int iVersion;				/* version number. */
	int iBuff;					/* the size of the buff. */

	int iBucket;				/* bucket number used. */
	int iMax;					/* maximum items can store. */
	int iItem;					/* current item number. */
	int iHeadSize;

	int iBucketOff;
	int iBucketSize;

	int iDataOff;
	int iDataSize;
	int iDataUnitMin;			/* the data-unit's real size. */
	int iDataUnitMax;			/* the data-unit's occupy size.*/

	int iFreeHead;
    int iLruHead;
    int iLruTail;
	int iRes;					/* reserved. */
};

typedef struct tagSHtable			SHTABLE;
typedef struct tagSHtable			*LPSHTABLE;

#define SHT_ROUND(size)					( ( (size) + SHT_MIN_ALIGN - 1) /SHT_MIN_ALIGN*SHT_MIN_ALIGN )

#define SHT_HEADSIZE()					( SHT_MIN_HEAD < sizeof(SHTABLE) ? sizeof(SHTABLE) : SHT_MIN_HEAD )

#define SHT_BUCKETSIZE(buck)			( (buck) * sizeof(SHBUCKET) )

#define SHT_DATAUNIT(data)				SHT_ROUND((data) + offsetof(SHITEM, szData))

#define SHT_DATASIZE(max, unit)			( (max) * SHT_DATAUNIT(unit) )

#define SHT_SIZE(buck, max, unit)		( SHT_HEADSIZE() + SHT_BUCKETSIZE(buck) + SHT_DATASIZE(max, unit) )

#define SHT_GET_BUCKET(pstTab, i)		( (LPSHBUCKET) ( ((ULONG_PTR)(pstTab)) + pstTab->iBucketOff + i*sizeof(SHBUCKET) ) )

#define SHT_GET_ITEM(pstTab, i)			( (LPSHITEM) ( ((ULONG_PTR)(pstTab)) + pstTab->iDataOff + i*pstTab->iDataUnitMax ) )

#define SHT_DATA2ITEM(pvData)			( (SHITEM*) ( ((ULONG_PTR)(pvData)) - offsetof(SHITEM, szData)) )
#define SHT_ITEM2DATA(pvItem)			( (pvItem)->szData )

#define SHT_DATA2INDEX(pstTab, pvData)			(( ((ULONG_PTR)(pstTab)) + pstTab->iDataOff -  ( ((ULONG_PTR)(pvData)) - offsetof(SHITEM, szData)) )/pstTab->iDataUnitMax)

typedef CShmManager<SHTABLE> shmAlloc;

template <class _Val, class _ExtractKey, class _PostProcFreeNode, class _Alloc = shmAlloc >
class CLruHashTableEx;

template <class _Val,
          class _ExtractKey, class _PostProcFreeNode, class _Alloc>
struct _LruHashtable_iterator;

template <class _Val,
          class _ExtractKey, class _PostProcFreeNode, class _Alloc>
struct _LruHashtable_const_iterator;

template <class _Val,
          class _ExtractKey, class _PostProcFreeNode, class _Alloc>
struct _LruHashtable_iterator {
  typedef CLruHashTableEx<_Val,_ExtractKey,_PostProcFreeNode,_Alloc>
          _Hashtable;
  typedef _LruHashtable_iterator<_Val,   
                              _ExtractKey, _PostProcFreeNode,  _Alloc>
          iterator;
  typedef _LruHashtable_const_iterator<_Val,   
                                    _ExtractKey, _PostProcFreeNode,  _Alloc>
          const_iterator;
  typedef SHITEM _Node;

  typedef _Val value_type;
  typedef size_t size_type;
  typedef ptrdiff_t         difference_type;  
  typedef _Val& reference;
  typedef _Val* pointer;

  _Node* _M_cur;
  _Hashtable* _M_ht;

  _LruHashtable_iterator(_Node* __n, _Hashtable* __tab) 
    : _M_cur(__n), _M_ht(__tab) {}
  _LruHashtable_iterator() {}
  reference operator*() const { return *(pointer)_M_cur->szData; }
  pointer operator->() const { return &(operator*()); }
  iterator& operator++();
  iterator operator++(int);
  bool operator==(const iterator& __it) const
    { return _M_cur == __it._M_cur; }
  bool operator!=(const iterator& __it) const
    { return _M_cur != __it._M_cur; }
};


template <class _Val,
          class _ExtractKey, class _PostProcFreeNode, class _Alloc>
struct _LruHashtable_const_iterator {
  typedef CLruHashTableEx<_Val,_ExtractKey,_PostProcFreeNode,_Alloc>
          _Hashtable;
  typedef _LruHashtable_iterator<_Val, 
                              _ExtractKey, _PostProcFreeNode, _Alloc>
          iterator;
  typedef _LruHashtable_const_iterator<_Val,   
                                    _ExtractKey, _PostProcFreeNode,  _Alloc>
          const_iterator;
  typedef SHITEM _Node;

  typedef forward_iterator_tag iterator_category;
  typedef _Val value_type;
  typedef ptrdiff_t difference_type;
  typedef size_t size_type;
  typedef const _Val& reference;
  typedef const _Val* pointer;

  const _Node* _M_cur;
  const _Hashtable* _M_ht;

  _LruHashtable_const_iterator(const _Node* __n, const _Hashtable* __tab)
    : _M_cur(__n), _M_ht(__tab) {}
  _LruHashtable_const_iterator() {}
  _LruHashtable_const_iterator(const iterator& __it) 
    : _M_cur(__it._M_cur), _M_ht(__it._M_ht) {}
  reference operator*() const { return *(pointer)_M_cur->szData;; }
  pointer operator->() const { return &(operator*()); }
  const_iterator& operator++();
  const_iterator operator++(int);
  bool operator==(const const_iterator& __it) const 
    { return _M_cur == __it._M_cur; }
  bool operator!=(const const_iterator& __it) const 
    { return _M_cur != __it._M_cur; }
};

// Forward declaration of operator==.

template <class _Val, class _Ex, class _Proc, class _All>
class CLruHashTableEx;

template <class _Val,
          class _ExtractKey, class _PostProcFreeNode, class _Alloc>
class CLruHashTableEx {
public:
  typedef _Val value_type;

  typedef size_t            size_type;
  typedef ptrdiff_t         difference_type;
  typedef value_type*       pointer;
  typedef const value_type* const_pointer;
  typedef value_type&       reference;
  typedef const value_type& const_reference;

private:
  typedef SHITEM _Node;

public:
  typedef _Alloc allocator_type;
private:
  allocator_type _M_table_allocator;
  LPSHTABLE _M_get_Table(long iTblSize) { return _M_table_allocator.allocate(iTblSize); }
  void _M_put_Table(LPSHTABLE pTbl) { return _M_table_allocator.deallocate(pTbl); }
    #define __HASH_ALLOC_INIT(__a) _M_table_allocator(__a),
private:
  _ExtractKey           _M_get_key;
  _PostProcFreeNode _M_proc_free_node;//节点清除的后处理操作

  LPSHTABLE _M_ptable;
  size_type             _M_num_elements;  
  int _M_InitStatus;

public:
  typedef _LruHashtable_iterator<_Val,_ExtractKey,_PostProcFreeNode,_Alloc>
          iterator;
  typedef _LruHashtable_const_iterator<_Val,_ExtractKey,_PostProcFreeNode,_Alloc>
          const_iterator;

  friend struct
  _LruHashtable_iterator<_Val,_ExtractKey,_PostProcFreeNode,_Alloc>;
  friend struct
  _LruHashtable_const_iterator<_Val,_ExtractKey,_PostProcFreeNode,_Alloc>;

public:
  CLruHashTableEx(size_type n,
            int inittype,
            const _ExtractKey& __ext,
            const _PostProcFreeNode& _M_proc_free_node,
            const allocator_type& __a = shmAlloc()
            )
    : __HASH_ALLOC_INIT(__a)
    _M_get_key(__ext),
    _M_ptable(NULL),
    _M_num_elements(n),
    _M_InitStatus(-1),
    _M_proc_free_node(_M_proc_free_node)
  {
    _M_InitStatus = Init(inittype);
    return;
  }
  ~CLruHashTableEx() { _M_put_Table(_M_ptable);}
    int Init(int fCreate = _CHECK_TABLE);
    int dump_all(ostream &out = cout);
    int dump_valid(ostream &out = cout);
    int dump_lru(ostream &out = cout);

  int table_status()
    {
        return _M_InitStatus;
    }
  size_type size() const 
  {
      return _M_ptable->iItem; 
  }
  size_type max_size() const { return _M_ptable->iMax; }
  bool empty() const { return size() == 0; }

  //遍历方式不更新LRU
  iterator begin()
  {
    int i;
    _Val* pvData;
    LPSHITEM pstItem;
    for(i=0; i<_M_ptable->iMax; i++)
    {
    	pvData	=	sht_pos(_M_ptable, i, NULL);

    	pstItem	=	SHT_DATA2ITEM(pvData);

    	if(pstItem && pstItem->fValid )
    	{
            return iterator(pstItem, this);
    	}
    }      
    return end();
  }

  iterator end() { return iterator(0, this); }

  const_iterator begin() const
  {
    int i;
    _Val* pvData;
    LPSHITEM pstItem;
    for(i=0; i<_M_ptable->iMax; i++)
    {
    	pvData	=	sht_pos(_M_ptable, i, NULL);

    	pstItem	=	SHT_DATA2ITEM(pvData);

    	if( pstItem && pstItem->fValid )
    	{
            return const_iterator(pstItem, this);
    	}
    }
    return end();
  }

  const_iterator end() const { return const_iterator(0, this); }

public:

  size_type max_bucket_count() const
    { return _M_ptable->iBucket; }
  
  pointer get_LRU_tail()
  {
    if( _M_ptable->iLruTail < 0 || _M_ptable->iLruTail > _M_ptable->iMax )
    {
        return NULL;
    }
    LPSHITEM pstItem;
    pstItem = SHT_GET_ITEM(_M_ptable, _M_ptable->iLruTail);
    return (_Val*)pstItem->szData;
  } 

  size_type elems_in_bucket(size_type __bucket) const
  {
    LPSHBUCKET pstBucket;
    pstBucket	=	SHT_GET_BUCKET(_M_ptable, __bucket);
    return pstBucket->iCount;
  }

  pair<iterator, bool> insert_unique(const value_type& __obj)
  {
    return insert_unique_noresize(__obj);
  }

  iterator insert_equal(const value_type& __obj)
  {
    return insert_equal_noresize(__obj);
  }

  pair<iterator, bool> insert_unique_noresize(const value_type& __obj)
    {
        value_type *pdata = NULL;
        LPSHITEM pstItem;

        pdata = sht_insert_unique( _M_ptable, __obj);
        if ( NULL != pdata )
        {
            pstItem	=	SHT_DATA2ITEM(pdata);
            memcpy(pdata, &__obj, sizeof(__obj));
            return pair<iterator, bool>(iterator(pstItem, this), true);
        }
        return pair<iterator, bool>(iterator(0, this), false);
    }
  iterator insert_equal_noresize(const value_type& __obj)
{

    value_type *pdata = NULL;
    LPSHITEM pstItem;
    
    sht_insert_multi(_M_ptable, __obj);
    if ( NULL != pdata )
    {
        pstItem	=	SHT_DATA2ITEM(pdata);
        memcpy(pdata, &__obj, sizeof(__obj));
        return iterator(pstItem, this);
    }
    return iterator(0, this);
}

  pointer find_or_insert(const value_type& __obj)
    {
        value_type *pdata = sht_find(_M_ptable, __obj);
        if( pdata != NULL)
        {
        	return pdata;
        }

        return sht_insert_multi(_M_ptable, __obj);
    } 
    pointer search(const value_type& __obj)
    {
        return sht_find(_M_ptable, __obj);
    }

  iterator find(const unsigned int & __key) 
  {
    int iBucket;
    LPSHBUCKET pstBucket;
    _Node* pstItem;
    int iNode;
    int n;
    iBucket	=	(int) (__key % (unsigned int)_M_ptable->iBucket);
    pstBucket	=	SHT_GET_BUCKET(_M_ptable, iBucket);

    if( pstBucket->iCount<=0 )
    {
    	return iterator(0, this);
    }

    iNode	=	pstBucket->iHead;

    n		=	0;
    while(iNode>=0 && iNode<_M_ptable->iMax && n<pstBucket->iCount )
    {
        pstItem	=	SHT_GET_ITEM(_M_ptable, iNode);
        if( pstItem->uCode ==__key)
        {
          return iterator(pstItem, this);
        }

    	iNode	=	pstItem->iNext;
    	n++;
    }
  
    return iterator(0, this);
  } 

  const_iterator find(const unsigned int & __key) const
  {
   int iBucket;
    LPSHBUCKET pstBucket;
    const _Node* pstItem;
    int iNode;
    int n;
    const _Node* __first = NULL;
    iBucket	=	(int) (__key % (unsigned int)_M_ptable->iBucket);
    pstBucket	=	SHT_GET_BUCKET(_M_ptable, iBucket);

    if( pstBucket->iCount<=0 )
    {
    	return const_iterator(__first, this);
    }

    iNode	=	pstBucket->iHead;

    n		=	0;
    while(iNode>=0 && iNode<_M_ptable->iMax && n<pstBucket->iCount )
    {
        pstItem	=	SHT_GET_ITEM(_M_ptable, iNode);
        if( pstItem->uCode ==__key)
        {
          return const_iterator(pstItem, this);
        }

    	iNode	=	pstItem->iNext;
    	n++;
    }
  
    return const_iterator(__first, this);
  } 

  size_type count(const unsigned int &__key) const
  {
   int iBucket;
    LPSHBUCKET pstBucket;
    const _Node* pstItem;
    int iNode;
    int n;
    size_type __result = 0;
    iBucket	=	(int) (__key % (unsigned int)_M_ptable->iBucket);
    pstBucket	=	SHT_GET_BUCKET(_M_ptable, iBucket);

    if( pstBucket->iCount<=0 )
    	return __result;

    iNode	=	pstBucket->iHead;

    n		=	0;
    while(iNode>=0 && iNode<_M_ptable->iMax && n<pstBucket->iCount )
    {
        pstItem	=	SHT_GET_ITEM(_M_ptable, iNode);
        if( pstItem->uCode ==__key)
        {
             ++__result;
        }

    	iNode	=	pstItem->iNext;
    	n++;
    }
  
    return __result;
  }

  pointer remove(const value_type& __obj)
    {
        return sht_remove(_M_ptable, __obj);
    }
  
  int erase(const unsigned int & __key)
    {
       int iBucket;
        LPSHBUCKET pstBucket;
        const _Node* pstItem;
        int iNode;
        int n;
        int __erased = 0;
        iBucket	=	(int) (__key % (unsigned int)_M_ptable->iBucket);
        pstBucket	=	SHT_GET_BUCKET(_M_ptable, iBucket);

        if( pstBucket->iCount<=0 )
        {
        	return __erased;
        }

        iNode	=	pstBucket->iHead;

        n		=	0;
        while(iNode>=0 && iNode<_M_ptable->iMax && n<pstBucket->iCount )
        {
            pstItem	=	SHT_GET_ITEM(_M_ptable, iNode);
            if( pstItem->uCode ==__key)
            {
                sht_remove_by_pos(_M_ptable, iNode );
                 ++__erased;
            }

        	iNode	=	pstItem->iNext;
        	n++;
        }
      return __erased;
    }
  void erase(const iterator& __it)
    {
        int iNode;
        LPSHITEM pstItem;
        iNode = SHT_DATA2INDEX(_M_ptable, __it._M_cur);
        if(iNode >=0 && iNode<_M_ptable->iMax )
        {
            sht_remove_by_pos(_M_ptable, iNode );
        }  
    }
  void erase(const const_iterator& __it)
    {
        int iNode;
        _Node* pvData = const_cast<_Node*>(__it._M_cur);
        LPSHITEM pstItem;
        iNode = SHT_DATA2INDEX(_M_ptable, pvData);
        if(iNode >=0 && iNode<_M_ptable->iMax )
        {
            sht_remove_by_pos(_M_ptable, iNode );
        }  
    }
  

private:
    int sht_make_free_chain_i(LPSHTABLE pstTab);
    int sht_alloc_node_i(LPSHTABLE pstTab);
    int sht_free_node_i(LPSHTABLE pstTab, int iNode);
    void sht_insert_i(LPSHTABLE pstTab, int iNode, unsigned int uCode);
    void sht_remove_i(LPSHTABLE pstTab, int iNode, unsigned int uCode);
    int sht_find_i(LPSHTABLE pstTab, const _Val& pvData, unsigned int uCode);
    LPSHTABLE sht_init(void* pvBuff, int iBuff, int iBucket, int iMax, int iUnit);
    int sht_check(void* pvBuff, int iBuff, int iBucket, int iMax, int iUnit);
    LPSHTABLE sht_attach(void* pvBuff, int iBuff, int iBucket, int iMax, int iUnit);
    _Val* sht_find(LPSHTABLE pstTab, const _Val& pvData);
    _Val* sht_find_new(LPSHTABLE pstTab, const _Val& pvData);
    _Val* sht_insert_unique(LPSHTABLE pstTab, const _Val& pvData);
    _Val* sht_insert_multi(LPSHTABLE pstTab, const _Val& pvData);
    _Val* sht_remove(LPSHTABLE pstTab, const _Val& pvData);
    _Val* sht_remove_by_pos(LPSHTABLE pstTab, int iPos);
    _Val* sht_pos(LPSHTABLE pstTab, int iPos, int* pfValid);
    int sht_rebuild(LPSHTABLE pstTab);
};

template <class _Val, class _ExK,class _Proc,class _All>
_LruHashtable_iterator<_Val,_ExK,_Proc,_All>&
_LruHashtable_iterator<_Val,_ExK,_Proc,_All>::operator++()
{
    int i;
    _Val* pvData;
    LPSHITEM pstItem;
    i = SHT_DATA2INDEX(_M_ht->_M_ptable, _M_cur);
    for(; i < _M_ht->_M_ptable->iMax; i++)
    {
    	pvData	=	sht_pos(_M_ht->_M_ptable, i, NULL);

    	pstItem	=	SHT_DATA2ITEM(pvData);

    	if( pstItem && pstItem->fValid )
    	{
    	    _M_cur = pstItem;
            return *this;
    	}
    }
    _M_cur = 0;
    return *this;
}

template <class _Val, class _ExK,class _Proc,class _All>
inline _LruHashtable_iterator<_Val,_ExK,_Proc,_All>
_LruHashtable_iterator<_Val,_ExK,_Proc,_All>::operator++(int)
{
  iterator __tmp = *this;
  ++*this;
  return __tmp;
}

template <class _Val, class _ExK,class _Proc,class _All>
_LruHashtable_const_iterator<_Val,_ExK,_Proc,_All>&
_LruHashtable_const_iterator<_Val,_ExK,_Proc,_All>::operator++()
{
    int i;
    void* pvData;
    LPSHITEM pstItem;
    i = SHT_DATA2INDEX(_M_ht->_M_ptable, _M_cur);
    for(; i < _M_ht->_M_ptable->iMax; i++)
    {
        if( i < 0 || i >= _M_ht->_M_ptable->iMax)
        {
            pvData = NULL;
        }
        else
        {
            LPSHITEM pstTmpItem;
            pstTmpItem = SHT_GET_ITEM(_M_ht->_M_ptable, i);
            pvData = (_Val*)pstTmpItem->szData;
        }
    	//pvData	=	sht_pos(_M_ht->_M_ptable, i, NULL);

    	pstItem	=	SHT_DATA2ITEM(pvData);

    	if( pstItem && pstItem->fValid )
    	{
    	    _M_cur = pstItem;
            return *this;
    	}
    }
    _M_cur = 0;
    return *this;
}

template <class _Val, class _ExK,class _Proc,class _All>
inline _LruHashtable_const_iterator<_Val,_ExK,_Proc,_All>
_LruHashtable_const_iterator<_Val,_ExK,_Proc,_All>::operator++(int)
{
  const_iterator __tmp = *this;
  ++*this;
  return __tmp;
}

template <class _Val, class _Ex, class _Proc, class _All>
int CLruHashTableEx<_Val,_Ex,_Proc,_All>::sht_make_free_chain_i(LPSHTABLE pstTab)
{
	LPSHITEM pstItem;
	LPSHITEM pstNext;
	int i;
	int n;

	pstTab->iFreeHead	=	-1;
	n	=	0;

	for(i=pstTab->iMax-1; i>=0; i--)
	{
		pstItem	=	SHT_GET_ITEM(pstTab, i);

		if( pstItem && pstItem->fValid )
			continue;

		n++;

		pstItem->iNext	=	pstTab->iFreeHead;
		pstItem->iPrev	=	-1;

		if( pstTab->iFreeHead<0 )
		{
			pstTab->iFreeHead	=	i;
		}
		else
		{
			pstNext	=	SHT_GET_ITEM(pstTab, pstTab->iFreeHead);
			pstNext->iPrev	=	i;
			pstTab->iFreeHead	=	i;
		}
	}

	return n;
}

template <class _Val, class _Ex, class _Proc, class _All>
int CLruHashTableEx<_Val,_Ex,_Proc,_All>::sht_alloc_node_i(LPSHTABLE pstTab)
{
	int iNode;
	LPSHITEM pstItem;
	LPSHITEM pstHead;

	if( pstTab->iFreeHead<0 || pstTab->iFreeHead>=pstTab->iMax )
		return -1;

	iNode	=	pstTab->iFreeHead;
	pstItem	=	SHT_GET_ITEM(pstTab, iNode);
    if (pstItem == NULL)
    {
        return -1;
    }

	pstTab->iFreeHead	=	pstItem->iNext;

	pstItem->fValid	=	1;
	pstItem->uCode	=	0;
	pstItem->iNext	=	-1;
	pstItem->iPrev	=	-1;
    pstItem->iLruNext = -1;
    pstItem->iLruPrev = -1;

	if( pstTab->iFreeHead>=0 && pstTab->iFreeHead<pstTab->iMax )
	{
		pstHead	=	SHT_GET_ITEM(pstTab, pstTab->iFreeHead);
		pstHead->iPrev	=	-1;
	}

	pstTab->iItem++;

	return iNode;
}

template <class _Val, class _Ex, class _Proc, class _All>
int CLruHashTableEx<_Val,_Ex,_Proc,_All>::sht_free_node_i(LPSHTABLE pstTab, int iNode)
{
	LPSHITEM pstItem;
	LPSHITEM pstHead;

	if( iNode<0 || iNode>=pstTab->iMax )
		return -1;

	pstItem	=	SHT_GET_ITEM(pstTab, iNode);
    if (pstItem == NULL)
    {
        return -1;
    }
	pstItem->fValid	=	0;
	pstItem->uCode	=	0;
	pstItem->iPrev	=	-1;
	pstItem->iNext	=	-1;
    pstItem->iLruNext = -1;
    pstItem->iLruPrev = -1;

	if( pstTab->iFreeHead>=0 && pstTab->iFreeHead<pstTab->iMax )
	{
		pstHead	=	SHT_GET_ITEM(pstTab, pstTab->iFreeHead);
		pstHead->iPrev	=	iNode;
	}

	pstItem->iNext		=	pstTab->iFreeHead;
	pstTab->iFreeHead	=	iNode;

	pstTab->iItem--;

	return iNode;
}

template <class _Val, class _Ex, class _Proc, class _All>
void CLruHashTableEx<_Val,_Ex,_Proc,_All>::sht_insert_i(LPSHTABLE pstTab, int iNode, unsigned int uCode)
{
	int iBucket;
	LPSHBUCKET pstBucket;
	LPSHITEM pstItem;
	LPSHITEM pstHead;
    LPSHITEM pstLruHead;

	iBucket	=	(int) (uCode % (unsigned int)pstTab->iBucket);

	pstBucket	=	SHT_GET_BUCKET(pstTab, iBucket);

	pstItem	=	SHT_GET_ITEM(pstTab, iNode);

	pstItem->uCode	=	uCode;

	if( pstBucket->iCount>0 )
		pstItem->iNext	=	pstBucket->iHead;
	else
		pstItem->iNext	=	-1;

	pstItem->iPrev	=	-1;

	if( pstBucket->iCount>0 && pstBucket->iHead>=0 && pstBucket->iHead<pstTab->iMax )
	{
		pstHead	=	SHT_GET_ITEM(pstTab, pstBucket->iHead);
		pstHead->iPrev	=	iNode;
	}

	pstBucket->iHead	=	iNode;
	pstBucket->iCount++;

    /*add LRU chain head*/
    if( pstTab->iLruHead >=0 && pstTab->iLruHead< pstTab->iMax )
    {
        pstLruHead = SHT_GET_ITEM(pstTab, pstTab->iLruHead);
        pstLruHead->iLruPrev = iNode;
    }
    pstItem->iLruNext = pstTab->iLruHead ;
    pstItem->iLruPrev = -1;
    pstTab->iLruHead = iNode;
    if ( pstTab->iLruTail <0 && pstItem->iLruNext <0) 
        pstTab->iLruTail = iNode;
}

template <class _Val, class _Ex, class _Proc, class _All>
void CLruHashTableEx<_Val,_Ex,_Proc,_All>::sht_remove_i(LPSHTABLE pstTab, int iNode, unsigned int uCode)
{
	int iBucket;
	LPSHBUCKET pstBucket;
	LPSHITEM pstItem;
	LPSHITEM pstPrev;
	LPSHITEM pstNext;

	LPSHITEM pstLruPrev;
	LPSHITEM pstLruNext;

	iBucket	=	(int) (uCode % (unsigned int)pstTab->iBucket);

	pstBucket	=	SHT_GET_BUCKET(pstTab, iBucket);

	pstItem	=	SHT_GET_ITEM(pstTab, iNode);

	if( pstItem->iPrev>=0 && pstItem->iPrev<pstTab->iMax )
	{
		pstPrev	=	SHT_GET_ITEM(pstTab, pstItem->iPrev);
		pstPrev->iNext	=	pstItem->iNext;
	}

	if( pstItem->iNext>=0 && pstItem->iNext<pstTab->iMax )
	{
		pstNext	=	SHT_GET_ITEM(pstTab, pstItem->iNext);
		pstNext->iPrev	=	pstItem->iPrev;
	}

	if( pstBucket->iHead==iNode )
		pstBucket->iHead	=	pstItem->iNext;

	pstBucket->iCount--;

    /* delete Lru Chain Item */
	if( pstItem->iLruPrev >=0 && pstItem->iLruPrev<pstTab->iMax )
	{
		pstLruPrev = SHT_GET_ITEM(pstTab, pstItem->iLruPrev);
		pstLruPrev->iLruNext=	pstItem->iLruNext;
	}

	if( pstItem->iLruNext >=0 && pstItem->iLruNext<pstTab->iMax )
	{
		pstLruNext= SHT_GET_ITEM(pstTab, pstItem->iLruNext);
		pstLruNext->iLruPrev=	pstItem->iLruPrev;
	}

    if (pstTab->iLruHead == iNode)
        pstTab->iLruHead = pstItem->iLruNext;
    
    if (pstTab->iLruTail== iNode)
        pstTab->iLruTail = pstItem->iLruPrev;
}

template <class _Val, class _Ex, class _Proc, class _All>
int CLruHashTableEx<_Val,_Ex,_Proc,_All>::sht_find_i(LPSHTABLE pstTab, const _Val& pvData, unsigned int uCode)
{
	int iBucket;
	LPSHBUCKET pstBucket;
	LPSHITEM pstItem;
	int iNode;
	int n;

	iBucket	=	(int) (uCode % (unsigned int)pstTab->iBucket);
	pstBucket	=	SHT_GET_BUCKET(pstTab, iBucket);

	if( pstBucket->iCount<=0 )
		return -1;

	iNode	=	pstBucket->iHead;
	n		=	0;

	while(iNode>=0 && iNode<pstTab->iMax && n<pstBucket->iCount )
	{
		pstItem	=	SHT_GET_ITEM(pstTab, iNode);

		if( pstItem->uCode==uCode && pvData == *(_Val*)pstItem->szData )
			return iNode;

		iNode	=	pstItem->iNext;
		n++;
	}

	return -1;
}

template <class _Val, class _Ex, class _Proc, class _All>
LPSHTABLE CLruHashTableEx<_Val,_Ex,_Proc,_All>::sht_init(void* pvBuff, int iBuff, int iBucket, int iMax, int iUnit)
{
	LPSHTABLE pstTab;
	LPSHITEM pstItem;
	int i;

	if( iBuff<(int)SHT_HEADSIZE() || iBucket<0 || iMax<0 || iUnit<=0 )
		return NULL;

	memset(pvBuff, 0, sizeof(SHTABLE));

	pstTab	=	(LPSHTABLE) pvBuff;

	pstTab->cbSize		=	sizeof(SHTABLE);
	pstTab->uFlags		=	0;

	pstTab->iVersion	=	SHT_VERSION;
	pstTab->iBuff		=	iBuff;

	pstTab->iBucket		=	iBucket;
	pstTab->iMax		=	iMax;
	pstTab->iItem		=	0;
	pstTab->iHeadSize	=	SHT_HEADSIZE();

	pstTab->iFreeHead	=	-1;
  	pstTab->iLruHead    =	-1;
  	pstTab->iLruTail    =	-1;
	if( (iBuff - pstTab->iHeadSize)/(int)sizeof(SHBUCKET) < iBucket )
		return NULL;

	pstTab->iBucketOff	=	pstTab->iHeadSize;
	pstTab->iBucketSize	=	sizeof(SHBUCKET)*iBucket;

	pstTab->iDataOff		=	pstTab->iBucketOff + pstTab->iBucketSize;
	pstTab->iDataUnitMin	=	iUnit;
	pstTab->iDataUnitMax	=	SHT_DATAUNIT(iUnit);

	if( (iBuff - pstTab->iDataOff)/pstTab->iDataUnitMax < iMax )
		return NULL;

	pstTab->iDataSize		=	pstTab->iDataUnitMax*iMax;

	//2008.06.10 by kent, init item valid flag
	for (i = 0; i<pstTab->iMax; i++)
	{
		pstItem	=	SHT_GET_ITEM(pstTab, i);
        if (pstItem == NULL)
        {
            continue;
        }
		pstItem->fValid = 0;
	}
	//by kent end

	sht_make_free_chain_i(pstTab);

	return pstTab;
}

template <class _Val, class _Ex, class _Proc, class _All>
int CLruHashTableEx<_Val,_Ex,_Proc,_All>::sht_check(void* pvBuff, int iBuff, int iBucket, int iMax, int iUnit)
{
	LPSHTABLE pstTab;
	int iSize;

	pstTab	=	(LPSHTABLE) pvBuff;

	if( iBuff<(int)SHT_HEADSIZE() || iBucket<=0 || iMax<=0 || iUnit<=0 )
		return -1;

	iSize	=	SHT_SIZE(iBucket, iMax, iUnit);
	if( iBuff<iSize )
		return -1;

	if( pstTab->iBuff!=iBuff || pstTab->iVersion!=SHT_VERSION ||
		pstTab->cbSize!=sizeof(SHTABLE) || pstTab->iBucket!=iBucket ||
		pstTab->iMax!=iMax || pstTab->iItem>pstTab->iMax ||
		pstTab->iHeadSize!=SHT_HEADSIZE() )
		return -1;

	if( pstTab->iHeadSize>pstTab->iBucketOff || 
		pstTab->iBucketSize/sizeof(SHBUCKET)!=iBucket ||
		iBuff - pstTab->iBucketOff<pstTab->iBucketSize ||
		pstTab->iBucketOff+pstTab->iBucketSize>pstTab->iDataOff || 
		pstTab->iDataUnitMin!=iUnit || 
		pstTab->iDataUnitMax!=SHT_DATAUNIT(iUnit) || 
		pstTab->iDataSize/pstTab->iDataUnitMax!=iMax ||
		iBuff - pstTab->iDataOff<pstTab->iDataSize )
		return -1;

	return 0;
}

template <class _Val, class _Ex, class _Proc, class _All>
LPSHTABLE CLruHashTableEx<_Val,_Ex,_Proc,_All>::sht_attach(void* pvBuff, int iBuff, int iBucket, int iMax, int iUnit)
{
	if( sht_check(pvBuff, iBuff, iBucket, iMax, iUnit)<0 )
		return NULL;
	else
		return (LPSHTABLE)pvBuff;
}

template <class _Val, class _Ex, class _Proc, class _All>
_Val* CLruHashTableEx<_Val,_Ex,_Proc,_All>::sht_find(LPSHTABLE pstTab, const _Val& pvData)
{
	unsigned int uCode;
	int iNode;
	LPSHITEM pstItem;
    LPSHITEM pstLruPrev;
    LPSHITEM pstLruNext;
    LPSHITEM pstLruHead;

	uCode = _M_get_key(pvData);
	iNode	=	sht_find_i(pstTab, pvData, uCode);

	if( iNode<0 )
		return NULL;

	pstItem	=	SHT_GET_ITEM(pstTab, iNode);

    /*  Update LRU Chain    (delete and add head)   */
    if (pstTab->iLruHead != iNode)    
    {
    	if( pstItem->iLruPrev >=0 && pstItem->iLruPrev<pstTab->iMax )
    	{
    		pstLruPrev = SHT_GET_ITEM(pstTab, pstItem->iLruPrev);
    		pstLruPrev->iLruNext = pstItem->iLruNext;
    	}

    	if( pstItem->iLruNext >=0 && pstItem->iLruNext<pstTab->iMax )
    	{
    		pstLruNext = SHT_GET_ITEM(pstTab, pstItem->iLruNext);
    		pstLruNext->iLruPrev = pstItem->iLruPrev;
    	}
        /*    
        if (pstTab->iLruHead == iNode)
            pstTab->iLruHead = pstItem->iLruNext;
        */    
        if (pstTab->iLruTail == iNode)
            pstTab->iLruTail = pstItem->iLruPrev;

        if( pstTab->iLruHead >=0 && pstTab->iLruHead< pstTab->iMax )
        {
            pstLruHead = SHT_GET_ITEM(pstTab, pstTab->iLruHead);
            pstLruHead->iLruPrev = iNode;
        }
        pstItem->iLruNext = pstTab->iLruHead ;
        pstItem->iLruPrev = -1;
        pstTab->iLruHead = iNode;
    }
    
	return (_Val*)pstItem->szData;
}

template <class _Val, class _Ex, class _Proc, class _All>
_Val* CLruHashTableEx<_Val,_Ex,_Proc,_All>::sht_find_new(LPSHTABLE pstTab, const _Val& pvData)
{
	unsigned int uCode;
	int iNode;
	LPSHITEM pstItem;

    uCode = _M_get_key(pvData);
	iNode	=	sht_find_i(pstTab, pvData, uCode);

	if( iNode<0 )
		return NULL;

	pstItem	=	SHT_GET_ITEM(pstTab, iNode);

	return (_Val*)pstItem->szData;
}

template <class _Val, class _Ex, class _Proc, class _All>
_Val* CLruHashTableEx<_Val,_Ex,_Proc,_All>::sht_insert_unique(LPSHTABLE pstTab, const _Val& pvData)
{
	if( sht_find(pstTab, pvData) )
		return NULL;

	return sht_insert_multi(pstTab, pvData);
}

template <class _Val, class _Ex, class _Proc, class _All>
_Val* CLruHashTableEx<_Val,_Ex,_Proc,_All>::sht_insert_multi(LPSHTABLE pstTab, const _Val& pvData)
{
	unsigned int uCode;
	int iNode;
	LPSHITEM pstItem;

	iNode	=	sht_alloc_node_i(pstTab);

       //modified by jinglinma 20090510 begin
	//if( iNode<0 )
	//	return NULL;
        //try to get one of the right LRU
        if (iNode < 0)
        {
            //查找该节点
            pointer pTailObj = get_LRU_tail();
            if (pTailObj != NULL)
            {
                _M_proc_free_node(*pTailObj);//对删除的节点做最后处理
            }
            
            if( pstTab->iLruTail<0 || pstTab->iLruTail > pstTab->iMax )
            {
                return NULL;
            }
            
            sht_remove_by_pos(pstTab, pstTab->iLruTail );
            iNode = sht_alloc_node_i(pstTab);
            if (iNode < 0)
            {
                return NULL;
            }
        }
       //modified by jinglinma 20090510 end

    uCode = _M_get_key(pvData);

    sht_insert_i(pstTab, iNode, uCode);

	pstItem	=	SHT_GET_ITEM(pstTab, iNode);

    return (_Val*)pstItem->szData;
}

template <class _Val, class _Ex, class _Proc, class _All>
_Val* CLruHashTableEx<_Val,_Ex,_Proc,_All>::sht_remove(LPSHTABLE pstTab, const _Val& pvData)
{
	unsigned int uCode;
	int iNode;
	LPSHITEM pstItem;

    uCode = _M_get_key(pvData);

    iNode = sht_find_i(pstTab, pvData,  uCode);
	if( iNode<0 )
		return NULL;

	sht_remove_i(pstTab, iNode, uCode);
	sht_free_node_i(pstTab, iNode);

	pstItem	=	SHT_GET_ITEM(pstTab, iNode);

	return (_Val*)pstItem->szData;
}

template <class _Val, class _Ex, class _Proc, class _All>
_Val* CLruHashTableEx<_Val,_Ex,_Proc,_All>::sht_remove_by_pos(LPSHTABLE pstTab, int iPos)
{
	LPSHITEM pstItem;

	if( iPos<0 || iPos>=pstTab->iMax )
		return NULL;

	pstItem	=	SHT_GET_ITEM(pstTab, iPos);

	sht_remove_i(pstTab, iPos, pstItem->uCode);
	sht_free_node_i(pstTab, iPos);

	return (_Val*)pstItem->szData;
}

template <class _Val, class _Ex, class _Proc, class _All>
_Val* CLruHashTableEx<_Val,_Ex,_Proc,_All>::sht_pos(LPSHTABLE pstTab, int iPos, int* pfValid)
{
	LPSHITEM pstItem;

	if( iPos<0 || iPos>=pstTab->iMax )
		return NULL;

	pstItem	=	SHT_GET_ITEM(pstTab, iPos);
    if (pstItem == NULL)
    {
        return NULL;
    }

	if( pfValid )
		*pfValid	=	pstItem->fValid;

	return (_Val*)pstItem->szData;
}

template <class _Val, class _Ex, class _Proc, class _All>
int CLruHashTableEx<_Val,_Ex,_Proc,_All>::sht_rebuild(LPSHTABLE pstTab)
{
	LPSHITEM pstItem;
	LPSHBUCKET pstBucket;
	int i;

	sht_make_free_chain_i(pstTab);

	for(i=0; i<pstTab->iBucket; i++)
	{
		pstBucket	=	SHT_GET_BUCKET(pstTab, i);
		pstBucket->iCount	=	0;
		pstBucket->iHead	=	-1;
	}

	pstTab->iItem	=	0;

	for(i=0; i<pstTab->iMax; i++)
	{
		pstItem	=	SHT_GET_ITEM(pstTab, i);
        if (pstItem == NULL)
        {
            continue;
        }

		if( !pstItem->fValid )
			continue;

		sht_insert_i(pstTab, i, pstItem->uCode);

		pstTab->iItem++;
	}

	return 0;
}


template <class _Val, class _Ex, class _Proc, class _All>
int CLruHashTableEx<_Val,_Ex,_Proc,_All>::dump_all(ostream &out)
{
    int i;
    _Val* pvData;
    LPSHITEM pstItem;

    out << "--------------Dump hash table(ALL) start----------------\n";
    out << "Bucket:" << _M_ptable->iBucket << ", Buff:" << _M_ptable->iBuff << "\n";
    out << "Max Items:" << _M_ptable->iMax << " Total Items:" << _M_ptable->iItem << "\n";

    for(i=0; i < _M_ptable->iMax; i++)
    {
    	pvData	=	sht_pos(_M_ptable, i, NULL);

    	pstItem	=	SHT_DATA2ITEM(pvData);
        if (pstItem == NULL)
        {
            continue;
        }

    	if( pstItem && pstItem->fValid )
    	{
                    out << "\t(VALID) Item pos=" << i << " code=" << pstItem->uCode << " ";
                    out << *pvData;
                    out << endl;
    	}
    	else
    	{
                    out << "\t(FREE) Item pos=" << i << " code=" << pstItem->uCode << " \n";
    	}
    }
    out << "--------------Dump hash table(ALL) end-------------------\n";

    return 0;
}

template <class _Val, class _Ex, class _Proc, class _All>
int CLruHashTableEx<_Val,_Ex,_Proc,_All>::dump_valid(ostream &out)
{
    int i;
    int n;
    LPSHBUCKET pstBucket;
    LPSHITEM pstItem;

    out << "--------------Dump hash table(VALID) start---------------\n";
    out << "Bucket:" << _M_ptable->iBucket << ", Buff:" << _M_ptable->iBuff <<"\n";
    out << "Max Items:" << _M_ptable->iMax << " Total Items:" << _M_ptable->iItem << "\n";
 
	for(i=0; i < _M_ptable->iBucket; i++)
	{
		pstBucket	=	SHT_GET_BUCKET(_M_ptable, i);

		if( pstBucket->iCount<=0 )
	    {
			continue;
		}
	    //fprintf(fp, "Bucket %4d Items:%d\n", i, pstBucket->iCount );

		if( pstBucket->iHead<0 || pstBucket->iHead>_M_ptable->iMax )
		{
                    out << "\t[ERROR] Head Pos=" << pstBucket->iHead << "\n";
			continue;
		}

		pstItem	=	SHT_GET_ITEM(_M_ptable, pstBucket->iHead);
		n =	0;

		do
		{
			//fprintf(fp, "\t(VALID) Item pos=%d code=%08x ", i, pstItem->uCode);
			//fprintf(fp, "\t Item pos=%d  ", i );
			out << "\t";
                    out << *(_Val*)pstItem->szData;
                    out << endl;

			if( pstItem->iNext<0 )
				break;
            pstItem	=	SHT_GET_ITEM(_M_ptable, pstItem->iNext);
			n++;
		}
		while( n<pstBucket->iCount );
	}

    out << "--------------Dump hash table(VALID) end-----------------\n";
	return 0;
}

template <class _Val, class _Ex, class _Proc, class _All>
int CLruHashTableEx<_Val,_Ex,_Proc,_All>::dump_lru(ostream &out)
{
	int i;
	int n;
	LPSHITEM pstItem;

    out << "--------------Dump hash table(VALID) start---------------\n";
    out << "Bucket:" << _M_ptable->iBucket << ", Buff:" << _M_ptable->iBuff <<"\n";
    out << "Max Items:" << _M_ptable->iMax << " Total Items:" << _M_ptable->iItem << "\n";


    if( _M_ptable->iLruHead<0 || _M_ptable->iLruHead>_M_ptable->iMax )
        return 0;
    out << "--------------Dump LRU Chain start---------------\n";
    //pstItem = SHT_GET_ITEM(pstTab, pstTab->iLruHead);
    pstItem = SHT_GET_ITEM(_M_ptable, _M_ptable->iLruTail);
    n = 0;
    i=  _M_ptable->iLruTail;
    do
    {
        out << "\t LRU=" << n << ",  pos=" << i << " ";
        out << *(_Val*)pstItem->szData;
        out << endl;
    	if( pstItem->iLruPrev<0 )	break;
        i =  pstItem->iLruPrev;
        pstItem	= SHT_GET_ITEM(_M_ptable, pstItem->iLruPrev);
		n++;
    }while(1);
    out << "--------------Dump LRU Chain end---------------\n";
	return 0;
}

template <class _Val, class _Ex, class _Proc, class _All>
int CLruHashTableEx<_Val,_Ex,_Proc,_All>::Init(int fCreate)
{
    LPSHTABLE    pTab = NULL;

    long iSize = SHT_SIZE( _M_num_elements/4, _M_num_elements, sizeof(_Val));

    if( iSize <= SHT_HEADSIZE() )
    {
        printf("invalid size:%d\n", iSize);
        return -2;
    }
    _M_ptable = _M_get_Table(iSize);

    if( NULL==_M_ptable)
    {
        return -1;
    }

    if ( fCreate )
    {
        pTab = sht_attach(_M_ptable, iSize, _M_num_elements/4, _M_num_elements, sizeof(_Val));
        if (NULL == pTab)
        {
            pTab = sht_init(_M_ptable, iSize, _M_num_elements/4, _M_num_elements, sizeof(_Val));
        }
    }
    else
    {
        pTab = sht_attach(_M_ptable, iSize, _M_num_elements/4, _M_num_elements, sizeof(_Val));
    }
    if( NULL== pTab)     
    {
        printf("initialize shared memory failed\n");
        return -3;
    }

    _M_ptable->uFlags   =   SHTF_NEEDFREE;

    return 0;
}


#endif /* __C_LRUHASHTABLEEX_HPP__ */

// Local Variables:
// mode:C++
// End:


