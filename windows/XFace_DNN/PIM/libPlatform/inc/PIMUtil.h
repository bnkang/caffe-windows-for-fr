/*
 * Copyright IM Lab,(http://imlab.postech.ac.kr)
 * 
 * All rights reserved.
 * 
 */

#ifndef __PIMUTIL_H__
#define __PIMUTIL_H__

#include "PIMTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* sort */
typedef struct __PIM_Index_Value {
    PIM_Int32 index;
    PIM_Int32 value;
} PIM_Index_Value;

/**
 *  @brief 
 *
 *  @param ori [IN] : 
 *
 *  @return : 
 */
extern PIM_Int32 PIM_Util_ConvertOrientationToAngle(PIM_ImageOrientation ori);

/**
 *  @brief set memory with a zero
 *
 *  @param p [OUT] : a destination memory block
 *  @param size [IN] : a memory size
 */
extern PIM_Void PIM_Util_BzeroMp(PIM_Void *p, PIM_Int32 size);

/**
 *  @brief swap 'a' and 'b'
 *
 *  @param a [IN/OUT] : 
 *  @param b [IN/OUT] : 
 */
extern PIM_Void PIM_Util_SwapInt(PIM_Int32 *a, PIM_Int32 *b);

/**
 *  @brief fill memory with a constant byte
 *
 *  @param p [OUT] : a destination memory block
 *  @param value [IN] : a constant byte
 *  @param size [IN] : the memory size
 */
extern PIM_Void *PIM_Util_MemsetMp(PIM_Void *p, PIM_Int32 value, PIM_Int32 size);

/**
 *  @brief copy memory area
 *
 *  @param dst [OUT] : a destination memory block
 *  @param src [IN] : a source memory block
 *  @param size [IN] : the memory size
 */
extern PIM_Void *PIM_Util_MemcpyMp(PIM_Void *dst, const PIM_Void *src, PIM_Int32 size);

/**
 *  @brief compare memory area
 *
 *  @param cs [IN] : 
 *  @param ct [IN] : 
 *  @param size [IN] : the memory size
 *
 *  @return 
 */
extern PIM_Int32 PIM_Util_MemcmpMp(const PIM_Void * cs, const PIM_Void *ct, PIM_Uint32 size);

/**
 *  @brief make random value. 
 *
 *  @return : it returns a value between 0 and RAND_MAX
 */
#define PIM_RANDOM_MAX 32767    // (0x7fff, 32767)
extern PIM_Uint32 PIM_Random();


/**
 *  @brief make random value. 
 *
 *  @param data [IN/OUT] : 
 *  @param data_size [IN] : 
 */
extern PIM_Void PIM_Util_Quicksort_Int(PIM_Index_Value* data, PIM_Int32 data_size);

/**
 *  @brief it checked usage restrictions.
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Result PIM_Util_CheckTimeLimit();

#ifdef __cplusplus
}
#endif


#endif /* __PIMUTIL_H__ */
