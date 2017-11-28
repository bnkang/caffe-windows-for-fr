#pragma once
#include "MVFD_Defines.h"
#include "MVFD_Thread.h"
#include <map>
#include <omp.h>
using namespace std;
//Enable this flag to detect memleak
//Note all the malloc and free musd be called through MVFD_mem_alloc() and MVFD_mem_free() functions
//#define MEMORY_DEBUG

#define MEM_SECTION_FEATURE_BUF					0
#define MEM_SECTION_BILINEAR_MAP				1
#define MEM_SECTION_POSE_MAP					2
#define MEM_SECTION_ROTATION_IMG				3
#define MEM_SECTION_REJECTION_MAP				4
#define MEM_SECTION_IRMB2_CODE_BUF				5
#define MEM_SECTION_RESIZE_BUF					6
#define MEM_SECTION_REGION_BUF					7
#define MEM_SECTION_ISFACE_SCRATCH_BUFFER		8
#define MEM_SECTION_IRMB_INTEGRAL_IMG_BUF		9
#define MEM_SECTION_FACE_XY_BUF					10
#define MEM_SECTION_VIDEO_KEY_FRAME				11
#define MEM_SECTION_FAST_FEATURE_POSITIONS		12
#define	MAX_TEMP_BUFFER_NUM						500

void *MVFD_mem_alloc(int size);
void MVFD_mem_free(void *ptr);

template<typename T>
T *MVFD_MemSection_Alloc(FD_CONTEXT *pContext, int section_id, size_t size)
{
	void *ptr;
	MVFD_Mutex_Lock(&pContext->mem_section_mutex);
	{
		int thread_id = MVFD_Thread_GetID();
		unsigned long long key = ((unsigned long long)(thread_id)<<32)|section_id;
		map< unsigned long long, pair<void *, size_t> >::iterator found = pContext->mem_sections.find(key);
		if(found == pContext->mem_sections.end() || found->second.second < size)
		{
			if(found != pContext->mem_sections.end()) MVFD_mem_free(found->second.first);
			ptr = MVFD_mem_alloc(size*sizeof(T));
			pContext->mem_sections[key] = make_pair(ptr, size);
		}
		else ptr = found->second.first;
	}
	MVFD_Mutex_UnLock(&pContext->mem_section_mutex);
	return (T *)ptr;
}

void MVFD_MemSection_FreeAll(FD_CONTEXT *pContext);

void fd_memcpy(void * dest, void* src, int size);
void fd_memset(void * dest, char val, int size);
int fd_memcmp(void * dest, void* src, int size);
void fd_memmove(void * dest, void* src, int size);
