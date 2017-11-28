#include "stdafx.h"

#include "MVFD_Thread.h"
#include "MVFD_Defines.h"
#include <sys/timeb.h>
#include <time.h> 
#include <errno.h>

void MVFD_Thread_Create(FD_THREAD *pThread, void * (*start_routine)(void *), void *arg)
{
	#if defined(WIN32) && defined(USE_WIN32_THREAD)
		unsigned int(__stdcall *entry_func)(void *) = (unsigned int(__stdcall *)(void *))start_routine;
		DWORD dwThreadID;
		pThread->handle = (HANDLE)_beginthreadex(NULL, 0, entry_func, arg, 0, (unsigned *)&dwThreadID);
		FDASSERT(pThread->handle != NULL);
	#else
		pthread_create(&pThread->handle, NULL, start_routine, arg);	
#endif
}

void MVFD_Thread_Release(FD_THREAD *pThread)
{
	#if defined(WIN32) && defined(USE_WIN32_THREAD)
		CloseHandle(pThread->handle);
		pThread->handle = NULL;
	#endif
}

void MVFD_Thread_Join(FD_THREAD *pThreads, int thread_num)
{
	#if defined(WIN32) && defined(USE_WIN32_THREAD)
		HANDLE handles[100];
		for(int i = 0; i < thread_num; ++i) handles[i] = pThreads[i].handle; 
		WaitForMultipleObjects(thread_num, handles, TRUE, INFINITE);
	#else
		for(int i = 0; i < thread_num; ++i)
		{
			void *ret_val;
			pthread_join(pThreads[i].handle, &ret_val);
		}
	#endif
}

void MVFD_Signal_Set(FD_SIGNAL *pSignal)
{
	#if defined(WIN32) && defined(USE_WIN32_THREAD)
		SetEvent(pSignal->handle);
	#else
		pthread_mutex_lock(&pSignal->mutex);
		pthread_cond_signal(&pSignal->handle);
		pthread_mutex_unlock(&pSignal->mutex);
	#endif
}

void MVFD_Signal_Wait(FD_SIGNAL *pSignal)
{
	#if defined(WIN32) && defined(USE_WIN32_THREAD)
		WaitForSingleObject(pSignal->handle, INFINITE);
	#else
		pthread_mutex_lock(&pSignal->mutex);
		pthread_cond_wait(&pSignal->handle, &pSignal->mutex);
		pthread_mutex_unlock(&pSignal->mutex);
	#endif
}

void MVFD_Signal_Wait_Time(FD_SIGNAL *pSignal, int time)
{
	#if defined(WIN32) && defined(USE_WIN32_THREAD)
		WaitForSingleObject(pSignal->handle, time);
	#else
	
	//struct __timeb64 tb;
	//_ftime64(&tb);
	
	struct timeb itb;
	ftime( &itb );

	timespec tm;
	tm.tv_sec = itb.time;
	tm.tv_nsec = (itb.millitm+(long)(time))* (long)(1000000);

	pthread_mutex_lock(&pSignal->mutex);
	int nRet = pthread_cond_timedwait(&pSignal->handle, &pSignal->mutex, &tm);
	pthread_mutex_unlock(&pSignal->mutex);
	if(nRet == ETIMEDOUT)
	{
		//LOGI("timed out!\n");
	}
	#endif
}

void MVFD_Signal_Create(FD_SIGNAL *pSignal)
{
	#if defined(WIN32) && defined(USE_WIN32_THREAD)
		pSignal->handle = CreateEvent(NULL, FALSE, FALSE, NULL);
		FDASSERT(pSignal->handle != NULL);
	#else
		pthread_mutex_init(&pSignal->mutex, NULL);
		pthread_cond_init(&pSignal->handle, NULL);
	#endif
}

void MVFD_Signal_Release(FD_SIGNAL *pSignal)
{
	#if defined(WIN32) && defined(USE_WIN32_THREAD)
		CloseHandle(pSignal->handle);
		pSignal->handle = NULL;
	#else
		pthread_cond_destroy(&pSignal->handle);
		pthread_mutex_destroy(&pSignal->mutex);
	#endif
}

unsigned int MVFD_Thread_GetID()
{
	#if defined(WIN32) && defined(USE_WIN32_THREAD)
		return GetCurrentThreadId();
	#else
		#ifdef WIN32
			return pthread_getw32threadid_np(pthread_self());
		#else
			return (unsigned int)pthread_self();
		#endif
	#endif
}

void MVFD_Mutex_Create(FD_MUTEX *pLock)
{
	#if defined(WIN32) && defined(USE_WIN32_THREAD)			
		pLock->handle = CreateSemaphore(NULL, 1, 1, NULL);
	#else
		pthread_mutex_init(&pLock->handle, NULL);
	#endif
}

void MVFD_Mutex_Release(FD_MUTEX *pLock)
{
	#if defined(WIN32) && defined(USE_WIN32_THREAD)		
		CloseHandle(&pLock->handle);
	#else
		pthread_mutex_destroy(&pLock->handle);
	#endif
}

void MVFD_Mutex_Lock(FD_MUTEX *pLock)
{
	#if defined(WIN32) && defined(USE_WIN32_THREAD)
		WaitForSingleObject(pLock->handle, INFINITE);
	#else
		pthread_mutex_lock(&pLock->handle);
	#endif
}

void MVFD_Mutex_UnLock(FD_MUTEX *pLock)
{
	#if defined(WIN32) && defined(USE_WIN32_THREAD)
		ReleaseSemaphore(pLock->handle, 1, NULL);
	#else
		pthread_mutex_unlock(&pLock->handle);
	#endif
}
