/******************************************************************************
*  @file      sys_prot.h
*  @author awayfang
*  @history 
*  通用的线程锁
*  
******************************************************************************/
#ifndef _SYS_THREAD_MUTEX_H_
#define _SYS_THREAD_MUTEX_H_
#ifndef WIN32
#include <pthread.h>
#else
#include <Windows.h>
#endif


/* PA Method Start */

#ifdef WIN32
	#define TMUTEX							CRITICAL_SECTION
	#define TMUTEX_INIT(pmutex)				(InitializeCriticalSection(pmutex), 0)
	#define TMUTEX_RELEASE(pmutex)			(DeleteCriticalSection(pmutex), 0)
	#define TMUTEX_LOCK(pmutex)				(EnterCriticalSection(pmutex),0)
	#define TMUTEX_TRYLOCK(pmutex)			(TryEnterCriticalSection(pmutex)?0:-1)
	#define TMUTEX_UNLOCK(pmutex)			(LeaveCriticalSection(pmutex),0)
#else
	#define TMUTEX							pthread_mutex_t
	#define TMUTEX_INIT(pmutex)				pthread_mutex_init(pmutex, NULL)
	#define TMUTEX_RELEASE(pmutex)			pthread_mutex_destroy(pmutex)
	#define TMUTEX_LOCK(pmutex)				pthread_mutex_lock(pmutex)
	#define TMUTEX_TRYLOCK(pmutex)			pthread_mutex_trylock(pmutex)
	#define TMUTEX_UNLOCK(pmutex)			pthread_mutex_unlock(pmutex)
#endif

/* PA Method End */

/* PS Method Start */

#ifdef WIN32
	#define PTHREAD_MUTEX_INITIALIZER				{0}
	#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP	{0}
	#define PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP	{0}

    #define pthread_mutexattr_t	void

	#define pthread_mutex_t						CRITICAL_SECTION
	#define pthread_mutex_init(pmutex, init)	(InitializeCriticalSection(pmutex),0)
	#define pthread_mutex_destroy(pmutex)		(DeleteCriticalSection(pmutex), 0)
	#define pthread_mutex_lock(pmutex)			(EnterCriticalSection(pmutex), 0)
	#define pthread_mutex_trylock(pmutex)		(TryEnterCriticalSection(pmutex)?0:-1)
	#define pthread_mutex_unlock(pmutex)		(LeaveCriticalSection(pmutex), 0)
#else
	#define CRITICAL_SECTION					pthread_mutex_t	
	#define InitializeCriticalSection(pmutex)	pthread_mutex_init(pmutex, NULL)
	#define DeleteCriticalSection(pmutex)		pthread_mutex_destroy(pmutex)
	#define EnterCriticalSection(pmutex)		pthread_mutex_lock(pmutex)
	#define TryEnterCriticalSection(pmutex)		((0==pthread_mutex_trylock(pmutex))?TRUE:FALSE)
	#define LeaveCriticalSection(pmutex)		pthread_mutex_unlock(pmutex)
#endif

/* PS Method End */


class SYS_Thread_Mutex
{
public:
    SYS_Thread_Mutex (const char *name = 0, pthread_mutexattr_t *attrs = 0);
    ~SYS_Thread_Mutex ();
    int acquire ();
    int release ();
private:
    pthread_mutex_t lock_; // Pthreads mutex mechanism.
};
#endif
