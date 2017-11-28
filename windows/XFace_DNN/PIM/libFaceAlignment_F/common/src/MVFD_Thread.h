#pragma once
//#define USE_WIN32_THREAD

#ifdef WIN32
	#ifdef USE_WIN32_THREAD
		#include <Windows.h>
		#include <process.h>
		typedef struct
		{
			HANDLE handle;
		} FD_THREAD;
		typedef struct 
		{
			HANDLE handle;
		} FD_SIGNAL;
		typedef struct 
		{
			HANDLE handle;
		} FD_MUTEX;
	#else
		//#pragma comment(lib, "pthreadVC2.lib") 
		//#include <pthread.h>
#include "pthread.h"
		typedef struct
		{
			pthread_t handle;
		} FD_THREAD;
		typedef struct 
		{
			pthread_mutex_t mutex;
			pthread_cond_t handle;
		} FD_SIGNAL;
		typedef struct 
		{
			pthread_mutex_t handle;
		} FD_MUTEX;
	#endif
#else
		#include <pthread.h>
		typedef struct
		{
			pthread_t handle;
		} FD_THREAD;
		typedef struct 
		{
			pthread_mutex_t mutex;
			pthread_cond_t handle;
		} FD_SIGNAL;
		typedef struct 
		{
			pthread_mutex_t handle;
		} FD_MUTEX;
#endif

void MVFD_Thread_Create(FD_THREAD *pThread, void * (*start_routine)(void *), void *arg);
void MVFD_Thread_Release(FD_THREAD *pThread);
void MVFD_Thread_Join(FD_THREAD *pThreads, int thread_num);

void MVFD_Signal_Set(FD_SIGNAL *pSignal);
void MVFD_Signal_Wait(FD_SIGNAL *pSignal);
void MVFD_Signal_Wait_Time(FD_SIGNAL *pSignal, int time);
void MVFD_Signal_Create(FD_SIGNAL *pSignal);
void MVFD_Signal_Release(FD_SIGNAL *pSignal);

unsigned int MVFD_Thread_GetID();

void MVFD_Mutex_Create(FD_MUTEX *pLock);
void MVFD_Mutex_Release(FD_MUTEX *pLock);
void MVFD_Mutex_Lock(FD_MUTEX *pLock);
void MVFD_Mutex_UnLock(FD_MUTEX *pLock);