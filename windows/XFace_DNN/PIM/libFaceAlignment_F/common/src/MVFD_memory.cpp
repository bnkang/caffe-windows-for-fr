#include "stdafx.h"

#include <stdlib.h>
#include <map>
using namespace std;
#ifdef WIN32
#include <memory.h>
#include <string.h>
#include <emmintrin.h>
#include <mmintrin.h>
#endif

#include "MVFD_Defines.h"
#include "MVFD_Global.h"
#include "MVFD_Memory.h"

void *MVFD_mem_alloc(int size)
{
#ifdef WIN32
	return _mm_malloc(size, 16);
#else
	void * ptr;
	ptr = malloc(size);
	FDASSERT(ptr != NULL)
	return ptr;
#endif
}
void MVFD_mem_free(void *ptr)
{
#ifdef WIN32
	_mm_free(ptr);
#else
	free(ptr);
#endif
}

void MVFD_MemSection_FreeAll(FD_CONTEXT *pContext)
{
	MVFD_Mutex_Lock(&pContext->mem_section_mutex);
	{
		for(map< unsigned long long, pair<void *, size_t> >::iterator it = pContext->mem_sections.begin(); it != pContext->mem_sections.end(); ++it)
		{
			MVFD_mem_free(it->second.first);
		}
		pContext->mem_sections.clear();
	}
	MVFD_Mutex_UnLock(&pContext->mem_section_mutex);
}



void fd_memcpy(void * dest, void* src, int size)
{
#ifdef RVDS
	int i = 0;
	for (i = 0; i < size; i++)
	{
		*((unsigned char *)dest)++ = *((unsigned char *)src)++;
	}
#else
	memcpy(dest,src,size);
#endif
}


void fd_memset(void * dest, char val, int size)
{
#ifdef RVDS
	int i = 0;
	for (i = 0; i < size; i++)
	{
		*((unsigned char *)dest)++ = val;
	}
#else
	memset(dest,val,size);
#endif
}

int fd_memcmp(void * dest, void* src, int size)
{
#ifdef RVDS
	int i = 0;
	for (i = 0; i < size; i++)
	{
		if(*((unsigned char *)dest)++ != *((unsigned char *)src)++)
			return 1;
	}
	return 0;
#else
	return memcmp(dest,src,size);
#endif
}

void fd_memmove(void * dest, void* src, int size)
{
#ifdef RVDS
	unsigned char * ptr = (unsigned char *)MVFD_mem_alloc(size*sizeof(unsigned char));
	
	fd_memcpy(ptr,src,size);
	fd_memcpy(dest,ptr,size);
	
	MVFD_mem_free(ptr);
	
#else
	memmove(dest,src,size);
#endif
	
}
