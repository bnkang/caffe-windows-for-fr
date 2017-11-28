/**
 *	@file PIMList.h
 *
 *  @brief  Declares PIM List interface.
 *          
 *
 *  Copyright IM Lab,(http://imlab.postech.ac.kr)
 *
 *  All rights reserved.
 */

#ifndef __PIMLIST_H__
#define __PIMLIST_H__


#include "PIMTypes.h"

#ifdef  __cplusplus
extern "C" {
#endif



/**
 *  @struct PIM_List_Node PIMList.h "PIMList.h"
 *  @brief List node structure.
 */
typedef struct PIM_List_Node PIM_List_Node;
struct PIM_List_Node
{
	PIM_List_Node* pNext;
	PIM_List_Node* pPrev;
	PIM_pVoid      pData;   /**<  */
};

/**
 *  @struct PIM_ListCursor PIMList.h "PIMList.h"
 *  @brief 
 */
typedef void* PIM_ListCursor;

/**
 *  @struct PIM_List PIMList.h "PIMList.h"
 *  @brief List structure.
 */
typedef struct PIM_List PIM_List;
struct PIM_List
{
	PIM_List_Node* pNodeHead;
	PIM_List_Node* pNodeTail;
	PIM_Int32      nCount;          /**<  */
	PIM_Int32      nElementSize;    /**<  */
};
	

/*************************************************************
 * Construction                                              *
 *************************************************************/
/**
 *  @brief  To allocate memory for the PIM_List.
 *
 *  @param pThis [OUT] : If not NULL, Pointer of the PIM_List data structure.
 *  @param nElementSize [IN] : Size of the element node in the PIM_List.
 *
 *  @return PIM_SUCCESS
 */
PIM_Int32 PIM_List_Alloc(PIM_List* pThis, PIM_Int32 nElementSize);

/**
 *  @brief To free memory for the PIM_List.
 *
 *  @param pThis [IN/OUT] : If not NULL, Pointer of the PIM_List.
 *
 *  @return none
 */
void PIM_List_Free(PIM_List* pThis);


/************************************************************* 
 * Attributes (head and tail)                                * 
 *************************************************************/
/**
 *  @brief To get the number of element nodes.
 *
 *  @param pThis [IN] : If not NULL, Pointer of the PIM_List.
 *
 *  @return The number of element nodes.
 */
PIM_Int32 PIM_List_GetCount(PIM_List* pThis);

/**
 *  @brief 
 *
 *  @param pThis [IN] : 
 *
 *  @return 
 */
PIM_Int32 PIM_List_GetSize(PIM_List* pThis);

/**
 *  @brief 
 *
 *  @param pThis [IN] : 
 *
 *  @return 
 */
PIM_Bool  PIM_List_IsEmpty(PIM_List* pThis);



/*************************************************************
 * iteration                                                 *
 *************************************************************/
/**
 *  @brief 
 *
 *  @param pThis [IN] : 
 *
 *  @return 
 */
PIM_ListCursor PIM_List_GetHeadPosition(PIM_List* pThis);

/**
 *  @brief 
 *
 *  @param pThis [IN] : 
 *
 *  @return 
 */
PIM_ListCursor PIM_List_GetTailPosition(PIM_List* pThis);

/**
 *  @brief 
 *  @note return *cursor++
 *
 *  @param cursor [IN] : 
 *
 *  @return 
 */
PIM_pVoid  PIM_List_GetNext(PIM_ListCursor* cursor);

/**
 *  @brief 
 *  @note return *cursor++
 *
 *  @param cursor [IN] : 
 *
 *  @return 
 */
PIM_pVoid  PIM_List_GetPrev(PIM_ListCursor* cursor);


/*************************************************************
 * peek at head or tail                                      *
 *************************************************************/

/**
 *  @brief 
 *
 *  @param pThis [IN] : 
 *
 *  @return 
 */
PIM_pVoid PIM_List_GetHead(PIM_List* pThis);

/**
 *  @brief 
 *
 *  @param pThis [IN] : 
 *
 *  @return 
 */
PIM_pVoid PIM_List_GetTail(PIM_List* pThis);


/***************************************************************
 * Operations                                                  *
 * get head or tail (and remove it) - don't call on empty list *
 ***************************************************************/

/**
 *  @brief 
 *
 *  @param pThis [IN/OUT] : 
 *
 *  @return 
 */
PIM_Int32 PIM_List_RemoveHead(PIM_List* pThis);

/**
 *  @brief 
 *
 *  @param pThis [IN/OUT] : 
 *
 *  @return 
 */
PIM_Int32 PIM_List_RemoveTail(PIM_List* pThis);

/**
 *  @brief add before head.
 *
 *  @param pThis [IN/OUT] : 
 *  @param newElement [IN] : 
 *
 *  @return 
 */
PIM_ListCursor PIM_List_AddHead(PIM_List* pThis, PIM_pVoid newElement);

/**
 *  @brief add after tail.
 *
 *  @param pThis [IN/OUT] : 
 *  @param newElement [IN] : 
 *
 *  @return 
 */
PIM_ListCursor PIM_List_AddTail(PIM_List* pThis, PIM_pVoid newElement);

/**
 *  @brief add another list of elements before head.
 *
 *  @param dst [IN/OUT] : 
 *  @param pNewList [IN] : 
 *
 *  @return 
 */
PIM_Int32 PIM_List_AddListToHead(PIM_List* dst, PIM_List* pNewList);

/**
 *  @brief add another list of elements after tail.
 *
 *  @param dst [IN/OUT] : 
 *  @param pNewList [IN] : 
 *
 *  @return 
 */
PIM_Int32 PIM_List_AddListToTail(PIM_List* dst, PIM_List* pNewList);

/**
 *  @brief remove all elements.
 *
 *  @param pThis [IN/OUT] : 
 *
 *  @return 
 */
PIM_Int32 PIM_List_RemoveAll(PIM_List* pThis);

/**
 *  @brief get an element at a given cursor.
 *
 *  @param cursor [IN] : 
 *
 *  @return 
 */
PIM_pVoid PIM_List_GetAt(PIM_ListCursor cursor);

/**
 *  @brief set an element at a given cursor.
 *
 *  @param pThis [IN/OUT] : 
 *  @param cursor [IN] : 
 *  @param newElement [IN] : 
 *
 *  @return 
 */
PIM_Int32 PIM_List_SetAt(PIM_List* pThis, PIM_ListCursor cursor, PIM_pVoid newElement);

/**
 *  @brief remove an element at a given cursor.
 *
 *  @param pThis [IN/OUT] : 
 *  @param cursor [IN] : 
 *
 *  @return 
 */
PIM_Int32 PIM_List_RemoveAt(PIM_List* pThis, PIM_ListCursor cursor);

/**
 *  @brief insert an element before a given cursor.
 *
 *  @param pThis [IN/OUT] : 
 *  @param cursor [IN] : 
 *  @param newElement [IN] : 
 *
 *  @return 
 */
PIM_ListCursor PIM_List_InsertBefore(PIM_List* pThis, PIM_ListCursor cursor, PIM_pVoid newElement);

/**
 *  @brief insert an element after a given cursor.
 *
 *  @param pThis [IN/OUT] : 
 *  @param cursor [IN] : 
 *  @param newElement [IN] : 
 *
 *  @return 
 */
PIM_ListCursor PIM_List_InsertAfter(PIM_List* pThis, PIM_ListCursor cursor, PIM_pVoid newElement);


/**
 *  @brief  helper functions (note: O(n) speed)
 *
 *  @param pThis [IN] : 
 *  @param searchValue [IN] : 
 *  @param startAfter [IN] : 
 *  @param cursor [IN] : 
 *
 *  @return
 */
PIM_Int32 PIM_List_Find(PIM_List* pThis, PIM_pVoid searchValue, PIM_ListCursor startAfter, PIM_ListCursor* cursor);


/**
 *  @brief defaults to starting at the HEAD, return NULL if not found.
 * 
 *  @param pThis [IN] :
 *  @param nIndex [IN] :
 *  @param cursor [IN] :
 * 
 *  @return
 */
PIM_Int32 PIM_List_FindIndex(PIM_List* pThis, PIM_Int32 nIndex, PIM_ListCursor* cursor);



#ifdef  __cplusplus
}
#endif

	//! @}
//! @}

#endif /*__PIMLIST_H__*/
