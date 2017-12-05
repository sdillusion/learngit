#include "myLock.h"
#include "plat_def.h"

#ifdef __unix
#include <pthread.h>
#endif

struct thread_lock
{
#ifdef WIN32

	CRITICAL_SECTION	cs;

#else

	pthread_mutex_t	lk;

#endif
};

CMyLock::CMyLock()
{
	m_pData	= new thread_lock;
}

CMyLock::~CMyLock()
{
	thread_lock* pLock = (thread_lock*)m_pData;
	delete pLock;
}

void CMyLock::init()
{
	thread_lock* pLock = (thread_lock*)m_pData;

#ifdef WIN32

	InitializeCriticalSection(&pLock->cs);

#else

	pthread_mutex_init(&pLock->lk, NULL);

#endif
}

void CMyLock::end()
{
	thread_lock* pLock = (thread_lock*)m_pData;

#ifdef WIN32

	DeleteCriticalSection(&pLock->cs);

#else

	pthread_mutex_destroy(&pLock->lk);

#endif
}

void CMyLock::lock()
{
	thread_lock* pLock = (thread_lock*)m_pData;

#ifdef WIN32

	EnterCriticalSection(&pLock->cs);

#else

	pthread_mutex_lock(&pLock->lk);

#endif
}

void CMyLock::unlock()
{
	thread_lock* pLock = (thread_lock*)m_pData;

#ifdef WIN32

	LeaveCriticalSection(&pLock->cs);

#else

	pthread_mutex_unlock(&pLock->lk);

#endif
}