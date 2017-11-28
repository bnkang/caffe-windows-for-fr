/*
 * Copyright IM Lab,(http://imlab.postech.ac.kr)
 * 
 * All rights reserved.
 * 
 */

#ifndef __PIMMEMORY_H__
#define __PIMMEMORY_H__

#include "PIMTypes.h"

#ifdef __cplusplus
extern "C" {
#endif


/*************************
 * select PIMMemory type *
 *************************/

/**
 *  @brief allocate dynamic memory.
 *
 *  @param size [IN] : the size of the allocated memory
 */
extern PIM_pVoid PIM_Malloc(PIM_Int32 size);

/**
 *  @brief changes the size of the memory block pointed to by ptr to size bytes.
 *
 *  @param ptr [IN] :  a memory pointer
 *  @param size [IN] : the size of the allocated memory
 */
extern PIM_pVoid PIM_Realloc(PIM_pVoid ptr, PIM_Int32 size);

/**
 *  @brief allocate dynamic memory, it was set to zero.
 *
 *  @param obj_count [IN] : 
 *  @param obj_size [IN] : 
 */
extern PIM_pVoid PIM_Calloc(PIM_Int32 obj_count, PIM_Int32 obj_size);

/**
 *  @brief free dynamic memory
 *
 *  @param ptr [IN] : memory pointer
 */
extern PIM_Void  PIM_Free(PIM_pVoid ptr);


/**
 *  @brief fill memory with a constant byte
 *
 *  @param ptr [IN] : a destination memory block
 *  @param value [IN] : a constant byte
 *  @param size [IN] : a memory size
 */
extern PIM_pVoid PIM_Memset(PIM_pVoid ptr,  PIM_Uint8 value, PIM_Int32 size);

/**
 *  @brief copy memory area
 *
 *  @param dst [IN] : a destination memory block
 *  @param src [IN] : a source memory block
 *  @param size [IN] : a memory size
 */
extern PIM_pVoid PIM_Memcpy(PIM_pVoid dst,  PIM_pVoid src, PIM_Int32 size);

/**
 *  @brief compare memory areas
 *
 *  @param cmpA [IN] : a memory block
 *  @param cmpB [IN] : a memory block
 *  @param size [IN] : a memory size
 */
extern PIM_Int32 PIM_Memcmp(PIM_pVoid cmpA, PIM_pVoid cmpB, PIM_Int32 size);


/****************************************************************************************
 * debug method
 *  : oem/PIMDeviceApi.win.debug.c
 ***************************************************************************************/
/**
 *  @brief 
 */
extern PIM_Void   MM_InitPeakMem();
/**
 *  @brief 
 */
extern PIM_Uint32 MM_GetPeakMem();
/**
 *  @brief 
 */
extern PIM_Uint32 MM_GetUseMem();
/**
 *  @brief 
 */
extern PIM_Void   MM_ShowInfo();
/**
 *  @brief 
 */
extern PIM_Void   MM_Create();
/**
 *  @brief 
 */
extern PIM_Void   MM_Destroy();



/*** Internal STD method ******************************************************************/
// The code is implemented in "libPlatform/src/PIMMalloc.c"
// but Code modification is necessary
#if (defined __MEM_INTERNAL__)
    #define PIM_Malloc(_size_)                     PIM_Memory_Malloc(_size_)
    #define PIM_Realloc(_ptr_,_size_)              PIM_Memory_Realloc(_ptr_,_size_)
    #define PIM_Calloc(_num_,_size_)               PIM_Memory_Calloc(_num_,_size_)
    #define PIM_Free(_ptr_)                        PIM_Memory_Free(_ptr_)

/*
    #define PIM_Memset(_ptr_,_c_,_size_)           PIM_Memory_Memset(_ptr_,_c_,_size_)
    #define PIM_Memcpy(_dst_,_src_,_size_)         PIM_Memory_Memcpy(_dst_,_src_,_size_)
    #define PIM_Memcmp(_cmpA_,_cmpB_,_size_)       PIM_Memory_Memcmp(_cmpA_,_cmpB_,_size_)
*/
    #include "PIMDeviceApi.h"
    #define PIM_Memset(_ptr_,_c_,_size_)           PIM_Device_Memset(_ptr_,_c_,_size_)
    #define PIM_Memcpy(_dst_,_src_,_size_)         PIM_Device_Memcpy(_dst_,_src_,_size_)
    #define PIM_Memcmp(_cmpA_,_cmpB_,_size_)       PIM_Device_Memcmp(_cmpA_,_cmpB_,_size_)

    #define MM_InitPeakMem()
    #define MM_GetPeakMem(_out_)
    #define MM_GetUsedMem(_out_)
    #define MM_PrintMemInfo(_ptr_)
    #define MM_LOG(...)

/*** External STD method (for Debug) ******************************************************/
// The code is implemented in "oem/PIMDeviceApi.*.debug.c"
#elif (defined __OEM_DEBUG__)
    #include "PIMDeviceApi.h"
    #define PIM_Malloc(_size_)                     PIM_Device_Malloc(_size_,__FUNCTION__,__LINE__)
    #define PIM_Realloc(_ptr_,_size_)              PIM_Device_Realloc(_ptr_,_size_,__FUNCTION__,__LINE__)
    #define PIM_Calloc(_num_,_size_)               PIM_Device_Calloc(_num_,_size_,__FUNCTION__,__LINE__)
    #define PIM_Free(_ptr_)                        PIM_Device_Free(_ptr_,__FUNCTION__,__LINE__)
    #define PIM_Memset(_ptr_,_c_,_size_)           PIM_Device_Memset(_ptr_,_c_,_size_)
    #define PIM_Memcpy(_dst_,_src_,_size_)         PIM_Device_Memcpy(_dst_,_src_,_size_)
    #define PIM_Memcmp(_cmpA_,_cmpB_,_size_)       PIM_Device_Memcmp(_cmpA_,_cmpB_,_size_)

    #define MM_InitPeakMem()                       PIM_Device_InitPeakMemSize()
    #define MM_GetPeakMem(_out_)                   _out_ = PIM_Device_GetPeakMemSize()
    #define MM_GetUsedMem(_out_)                   _out_ = PIM_Device_GetUsedMemSize()
    #define MM_PrintMemInfo(_ptr_)                 PIM_Device_PrintMemInfo(_ptr_)
    #define MM_LOG(...)                            PIM_Log_Print(PIM_LOG_INFO, TAG, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

/*** External STD method ******************************************************************/
#else
    #include "PIMDeviceApi.h"
    #define PIM_Malloc(_size_)                     PIM_Device_Malloc(_size_)
    #define PIM_Realloc(_ptr_,_size_)              PIM_Device_Realloc(_ptr_,_size_)
    #define PIM_Calloc(_num_,_size_)               PIM_Device_Calloc(_num_,_size_)
    #define PIM_Free(_ptr_)                        PIM_Device_Free(_ptr_)
    #define PIM_Memset(_ptr_,_c_,_size_)           PIM_Device_Memset(_ptr_,_c_,_size_)
    #define PIM_Memcpy(_dst_,_src_,_size_)         PIM_Device_Memcpy(_dst_,_src_,_size_)
    #define PIM_Memcmp(_cmpA_,_cmpB_,_size_)       PIM_Device_Memcmp(_cmpA_,_cmpB_,_size_)

    #define MM_InitPeakMem()
    #define MM_GetPeakMem(_out_)
    #define MM_GetUsedMem(_out_)
    #define MM_PrintMemInfo(_ptr_)
    #define MM_LOG(...)
#endif
/*** switch __INTERNAL__ & __EXTERNAL__ method ********************************************/




#ifdef __cplusplus
}
#endif

#endif /*__PIMMEMORY_H__*/
