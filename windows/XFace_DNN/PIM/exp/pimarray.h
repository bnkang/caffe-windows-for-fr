/**
 *	@file PIMArray.h
 *
 *  @brief  Declares PIMArray interface.
 
 *
 *  Copyright IM Lab,(http://imlab.postech.ac.kr)
 *
 *  All rights reserved.
 */

#ifndef __PIMARRAY_H__
#define __PIMARRAY_H__

#include "PIMTypes.h"

#ifdef  __cplusplus
extern "C" {
#endif


/**
 *  @struct PIM_Array PIMArray.h "PIMArray.h"
 *  @brief Array structure.
 */
typedef struct 
{
	PIM_pVoid* pData;       /**< the actual array of data       */
	PIM_Int32 nSize;        /**< # of elements (upperBound - 1) */
	PIM_Int32 nMaxSize;     /**< max allocated                  */
	PIM_Int32 nGrowBy;      /**< grow amount                    */
	PIM_Int32 nElementSize;

} PIM_Array;


/*************************************************************/
/* Construction                                              */
/*************************************************************/
/**
 *  @brief Allocate array data structure.
 *
 *  @param pThis        [OUT] :  array handle.
 *  @param nElementSize [IN] : size
 *  @param cap          [IN] : 
 *
 *  @return success : PIM_TRUE, fail : PIM_FALSE
 */
extern PIM_Bool PIM_Array_Alloc (PIM_Array* pThis, PIM_Int32 nElementSize, PIM_Int32 cap);

/**
 *  @brief Deallocate array data structure.
 *
 *  @param pThis      [IN/OUT] :  array handle.
 */
extern void     PIM_Array_Free  (PIM_Array* pThis);
	
/*************************************************************/
/* Attributes                                                */
/*************************************************************/
/**
 *  @brief Get array size.
 *
 *  @param pThis     [IN/OUT] : array handle.
 *
 *  @return ret : size of array.
 */
extern PIM_Int32 PIM_Array_GetSize  (PIM_Array* pThis);

/**
 *  @brief Get array count.
 *
 *  @param pThis     [IN/OUT] : array handle.
 *
 *  @return ret : size of array.
 */
extern PIM_Int32 PIM_Array_GetCount (PIM_Array* pThis);

/**
 *  @brief Check if array empry or not.
 *
 *  @param pThis      [IN/OUT] : array handle.
 *
 *  @return ret : 
 */
extern PIM_Bool  PIM_Array_IsEmpty  (PIM_Array* pThis);

/**
 *  @brief Set array size.
 *
 *  @param pThis    [IN/OUT] : array handle.
 *  @param nNewSize     [IN] : new size.
 *  @param nGrowBy      [IN] : ???
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Int32 PIM_Array_SetSize  (PIM_Array* pThis, PIM_Int32 nNewSize, PIM_Int32 nGrowBy);//, PIM_INT nGrowBy = -1);


/*************************************************************/
/* Operations                                                */
/* Clean up                                                  */
/*************************************************************/
/**
 *  @brief Deallocate array data structure.
 *
 *  @param pThis       [IN/OUT] : array handle.
 */
extern void PIM_Array_FreeExtra (PIM_Array* pThis);

/**
 *  @brief Remove all data of array.
 *
 *  @param pThis       [IN/OUT] : array handle.
 */
extern void PIM_Array_RemoveAll (PIM_Array* pThis);


/* Accessing elements */
/**
 *  @brief Get array element at index.
 *
 *  @param pThis       [IN/OUT] : array handle.
 *  @param nIndex          [IN] : index.
 *
 *  @return ret : element of aray at index.
 */
extern const PIM_pVoid PIM_Array_GetAt(PIM_Array* pThis, PIM_Int32 nIndex);

/**
 *  @brief Set array element at index.
 *
 *  @param pThis       [IN/OUT] : array handle.
 *  @param nIndex          [IN] : index.
 *  @param newElement      [IN] : element.
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Int32 PIM_Array_SetAt(PIM_Array* pThis, PIM_Int32 nIndex, PIM_pVoid newElement);


/* Potentially growing the array */
/**
 *  @brief Add array element after end of array.
 *
 *  @param pThis       [IN/OUT] : array handle.
 *  @param newElement      [IN] : element.
 *
 *  @return ret : index of added element.
 */
extern PIM_Int32 PIM_Array_Add            (PIM_Array* pThis, PIM_pVoid newElement);

/**
 *  @brief Append array after end of array.
 *
 *  @param pThis     [IN/OUT] : array handle.
 *  @param pNewArray      [IN] : new array.
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : size of array before appending.
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Int32 PIM_Array_AddArray       (PIM_Array* pThis, const PIM_Array* pNewArray);

/**
 *  @brief Set element at index of array.
 *
 *  @param pThis   [IN/OUT] : array handle.
 *  @param nIndex      [IN] : index.
 *	@param newElement  [IN] : 
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success.
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Int32 PIM_Array_SetAtGrow(PIM_Array* pThis, PIM_Int32 nIndex, PIM_pVoid newElement);

/**
 *  @brief Set array at index of array.
 *
 *  @param pThis      [IN/OUT] : array handle.
 *  @param nIndex         [IN] : index
 *  @param pNewArray      [IN] : array
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success.
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Int32 PIM_Array_SetAtGrowArray (PIM_Array* pThis, PIM_Int32 nIndex, PIM_Array* pNewArray);

/**
 *  @brief Insert element at index of array.
 *
 *  @param pThis     [IN/OUT] : array handle.
 *  @param nIndex        [IN] : index.
 *  @param newElement    [IN] : element.
 *  @param nCount        [IN] : 1
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success.
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Int32 PIM_Array_InsertAt(PIM_Array* pThis, PIM_Int32 nIndex, PIM_pVoid newElement, PIM_Int32 nCount);

/**
 *  @brief Insert array at index of array.
 *
 *  @param pThis     [IN/OUT] : array handle.
 *  @param nIndex        [IN] : index.
 *  @param pNewArray     [IN] : array.
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success.
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Int32 PIM_Array_InsertAtArray(PIM_Array* pThis, PIM_Int32 nIndex, PIM_Array* pNewArray);

/**
 *  @brief Remove element at index of array.
 *
 *  @param pThis  [IN/OUT] : array handle.
 *  @param nIndex     [IN] : index.
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success.
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Int32 PIM_Array_RemoveAt(PIM_Array* pThis, PIM_Int32 nIndex); //, PIM_INT nCount); //PIM_INT nCount = 1);

/**
 *  @brief Copy array to array.
 *
 *  @param pThis  [IN/OUT] : array handle.
 *  @param src        [IN] : array.
 *
 *  @return ret < PIM_SUCCESS : fail, error code.
 *  @return ret == PIM_SUCCESS : success.
 *
 *  @see PIM_SUCCESS : see PIM_Result for return value (PIMTypes.h)
 */
extern PIM_Int32 PIM_Array_Copy (PIM_Array* pThis, const PIM_Array* src);

#ifdef  __cplusplus
}
#endif



#endif /*__PIMARRAY_H__*/
