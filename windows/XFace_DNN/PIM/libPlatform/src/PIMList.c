#include "PIMTypes.h"
#include "PIMList.h"
#include "libPlatform/inc/PIMMemory.h"


/***************************************************************/
/* Internal method                                             */
/***************************************************************/
static PIM_List_Node* API_List_Node_Alloc(PIM_List* pThis, PIM_List_Node* pPrev, PIM_List_Node* pNext);
static PIM_Void API_List_Node_Free(PIM_List* pThis, PIM_List_Node* pNode);




/*************************************************************/
/* Construction                                              */
/*************************************************************/
PIM_Result PIM_List_Alloc(PIM_List* pThis, PIM_Int32 nElementSize)
{
	pThis->nCount = 0;
	pThis->nElementSize = nElementSize;
	pThis->pNodeHead = NULL;
	pThis->pNodeTail = NULL;

	return PIM_SUCCESS;
}

/* destruction */
PIM_Void PIM_List_Free(PIM_List* pThis)
{
	PIM_List_RemoveAll(pThis);
}



/*************************************************************/
/* Attributes (head and tail)                                */
/*************************************************************/
PIM_Int32  PIM_List_GetCount(PIM_List* pThis)
{
	return pThis->nCount;
}

PIM_Int32  PIM_List_GetSize(PIM_List* pThis)
{
	return pThis->nCount;
}

PIM_Bool PIM_List_IsEmpty(PIM_List* pThis)
{
	if(pThis->nCount == 0)
		return PIM_TRUE;
	else
		return PIM_FALSE;
}


/*************************************************************/
/* peek at head or tail                                      */
/*************************************************************/
PIM_pVoid PIM_List_GetHead(PIM_List* pThis)
{
	if(pThis->pNodeHead == NULL)
		return NULL;
	return (PIM_pVoid)pThis->pNodeHead->pData;
}

PIM_pVoid PIM_List_GetTail(PIM_List* pThis)
{
	if(pThis->pNodeTail == NULL)
		return NULL;
	return (PIM_pVoid)pThis->pNodeTail->pData;
}



/*************************************************************/
/* iteration                                                 */
/*************************************************************/
PIM_ListCursor PIM_List_GetHeadPosition(PIM_List* pThis)
{
	return (PIM_ListCursor)pThis->pNodeHead;
}

PIM_ListCursor PIM_List_GetTailPosition(PIM_List* pThis)
{
	return (PIM_ListCursor)pThis->pNodeTail;
}

PIM_pVoid PIM_List_GetNext(PIM_ListCursor* position)       // return *Position++
{
	PIM_List_Node* pNode = (PIM_List_Node*)*position;

	if(pNode == NULL)
		return NULL;

	*position = (PIM_ListCursor) pNode->pNext;
		
	return pNode->pData;
}

PIM_pVoid PIM_List_GetPrev(PIM_ListCursor* position)       // return *Position--
{
	PIM_List_Node* pNode = (PIM_List_Node*)*position;

	if(pNode == NULL)
		return NULL;

	*position = (PIM_ListCursor) pNode->pPrev;
		
	return pNode->pData;
}



/***************************************************************/
/* Operations                                                  */
/* get head or tail (and remove it) - don't call on empty list */
/***************************************************************/
PIM_Int32 PIM_List_RemoveHead(PIM_List* pThis)
{
	PIM_pVoid pRetVal = NULL;
	PIM_List_Node* pOldNode = 0;

	if(pThis->pNodeHead == NULL)
		return PIM_ERROR_NULL_POINTER;

	pOldNode = (PIM_List_Node*)pThis->pNodeHead;
	pRetVal = pOldNode->pData;

	pThis->pNodeHead = pOldNode->pNext;
	if (pThis->pNodeHead != NULL)
		((PIM_List_Node*)pThis->pNodeHead)->pPrev = NULL;
	else
		pThis->pNodeTail = NULL;
	
	API_List_Node_Free(pThis, pOldNode);

	return PIM_SUCCESS;
}

PIM_Int32 PIM_List_RemoveTail(PIM_List* pThis)
{
	PIM_pVoid pRetVal = NULL;
	PIM_List_Node* pOldNode = 0;

	if(pThis->pNodeTail == NULL)
		return PIM_ERROR_NULL_POINTER;

	pOldNode = (PIM_List_Node*)pThis->pNodeTail;
	pRetVal = pOldNode->pData;

	pThis->pNodeTail = pOldNode->pPrev;
	if (pThis->pNodeTail != NULL)
		((PIM_List_Node*)pThis->pNodeTail)->pNext = NULL;
	else
		pThis->pNodeHead = NULL;
	
	API_List_Node_Free(pThis, pOldNode);
	
	return PIM_SUCCESS;
}



/* add before head or after tail */
PIM_ListCursor PIM_List_AddHead(PIM_List* pThis, PIM_pVoid newElement)
{
	PIM_List_Node* pNewNode = API_List_Node_Alloc(pThis, NULL, pThis->pNodeHead);
	PIM_Memcpy(pNewNode->pData, newElement, pThis->nElementSize);
	
	if (pThis->pNodeHead != NULL)
		((PIM_List_Node*)pThis->pNodeHead)->pPrev = pNewNode;
	else
		pThis->pNodeTail = pNewNode;
	pThis->pNodeHead = pNewNode;

	return (PIM_ListCursor) pNewNode;
}

PIM_ListCursor PIM_List_AddTail(PIM_List* pThis, PIM_pVoid newElement)
{
	PIM_List_Node* pNewNode = API_List_Node_Alloc(pThis, pThis->pNodeTail, NULL);
	PIM_Memcpy(pNewNode->pData, newElement, pThis->nElementSize);

	if (pThis->pNodeTail != NULL)
		((PIM_List_Node*)pThis->pNodeTail)->pNext = pNewNode;
	else
		pThis->pNodeHead = pNewNode;
	pThis->pNodeTail = pNewNode;

	return (PIM_ListCursor) pNewNode;
}



/* add another list of elements before head or after tail */
PIM_Int32 PIM_List_AddListToHead(PIM_List* pDst, PIM_List* pNewList)
{
	PIM_ListCursor pos = NULL;

	if(pDst == NULL || pNewList == NULL)
		return PIM_ERROR_NULL_POINTER;

	pos = PIM_List_GetTailPosition(pNewList);

	while (pos != NULL)
		PIM_List_AddHead(pDst, PIM_List_GetPrev(&pos));

	return 1;
}

PIM_Int32 PIM_List_AddListToTail(PIM_List* pDst, PIM_List* pNewList)
{
	PIM_ListCursor pos = NULL;

	if(pDst == NULL || pNewList == NULL)
		return PIM_ERROR_NULL_POINTER;

	pos = PIM_List_GetHeadPosition(pNewList);

	while (pos != NULL)
		PIM_List_AddTail(pDst, PIM_List_GetNext(&pos));

	return 1;
}



/* remove all elements */
PIM_Int32 PIM_List_RemoveAll(PIM_List* pThis)
{
	PIM_List_Node* pNode = NULL;
	PIM_List_Node* pNext = NULL;

	if(pThis == NULL)
		return PIM_ERROR_NULL_POINTER;

	for(pNode = pThis->pNodeHead ; pNode != NULL ; pNode = pNext)
	{
		pNext = pNode->pNext;
		
		if(pNode->pData != NULL)
			PIM_Free(pNode->pData);

		PIM_Free(pNode);
	}

	pThis->nCount = 0;
	pThis->pNodeHead = pThis->pNodeTail = NULL;

	return 1;
}



/* getting/modifying an element at a given position */
PIM_pVoid PIM_List_GetAt(PIM_ListCursor position)
{
	PIM_List_Node* pNode = (PIM_List_Node*)position;

	if(pNode == NULL)
		return NULL;

	return (PIM_pVoid)pNode->pData;
}

PIM_Int32 PIM_List_SetAt(PIM_List* pThis, PIM_ListCursor position, PIM_pVoid newElement)
{
	PIM_List_Node* pNode = (PIM_List_Node*)position;

	if(pNode == NULL)
		return PIM_ERROR_NULL_POINTER;

	PIM_Memcpy(pNode->pData, newElement, pThis->nElementSize);

	return 1;
}

PIM_Int32 PIM_List_RemoveAt(PIM_List* pThis, PIM_ListCursor position)
{
	PIM_List_Node* pOldNode = NULL;

	if(pThis == NULL || position == NULL)
		return PIM_ERROR_INVALID_ARGUMENT;

	pOldNode = (PIM_List_Node*)position;

	/* remove pOldNode from list */
	if (pOldNode == pThis->pNodeHead)
	{
		pThis->pNodeHead = pOldNode->pNext;
	}
	else
	{
		((PIM_List_Node*)pOldNode->pPrev)->pNext = pOldNode->pNext;
	}
	
	if (pOldNode == pThis->pNodeTail)
	{
		pThis->pNodeTail = pOldNode->pPrev;
	}
	else
	{
		((PIM_List_Node*)pOldNode->pNext)->pPrev = pOldNode->pPrev;
	}

	API_List_Node_Free(pThis, pOldNode);

	return PIM_SUCCESS;
}



/* inserting before or after a given position */
PIM_ListCursor PIM_List_InsertBefore(PIM_List* pThis, PIM_ListCursor position, PIM_pVoid newElement)
{
	PIM_List_Node* pOldNode = NULL;
	PIM_List_Node* pNewNode = NULL;

	if(pThis == NULL)
		return NULL;

	if (position == NULL)
		return PIM_List_AddHead(pThis, newElement); /* insert before nothing -> head of the list */

	/* Insert it before position */
	pOldNode = (PIM_List_Node*) position;
	
	pNewNode = API_List_Node_Alloc(pThis, pOldNode->pPrev, pOldNode);
	PIM_Memcpy(pNewNode->pData, newElement, pThis->nElementSize);

	if (pOldNode->pPrev != NULL)
	{
		((PIM_List_Node*)pOldNode->pPrev)->pNext = pNewNode;
	}
	else
	{
		pThis->pNodeHead = pNewNode;
	}

	pOldNode->pPrev = pNewNode;

	return (PIM_ListCursor) pNewNode;
}

PIM_ListCursor PIM_List_InsertAfter(PIM_List* pThis, PIM_ListCursor position, PIM_pVoid newElement)
{
	PIM_List_Node* pOldNode = NULL;
	PIM_List_Node* pNewNode = NULL;

	if(pThis == NULL)
		return NULL;

	if (position == NULL)
		return PIM_List_AddTail(pThis, newElement);

	// Insert it before position
	pOldNode = (PIM_List_Node*) position;
	
	pNewNode = API_List_Node_Alloc(pThis, pOldNode, pOldNode->pNext);
	PIM_Memcpy(pNewNode->pData, newElement, pThis->nElementSize);

	if (pOldNode->pNext != NULL)
	{
		((PIM_List_Node*)pOldNode->pNext)->pPrev = pNewNode;
	}
	else
	{
		pThis->pNodeTail = pNewNode;
	}

	pOldNode->pNext = pNewNode;

	return (PIM_ListCursor) pNewNode;
}



/* helper functions (note: O(n) speed) */
PIM_Int32 Compare(PIM_List* pThis, PIM_pVoid val1, PIM_pVoid val2)
{
	PIM_Int32 i = 0;
	PIM_Uint8* pData1 = (PIM_Uint8*)val1;
	PIM_Uint8* pData2 = (PIM_Uint8*)val2;

	for( ; i < pThis->nElementSize ; i++)
		if(pData1[i] != pData2[i])
			return 1;

	return 0;
}



/* helper functions (note: O(n) speed) */
PIM_Int32 PIM_List_Find(PIM_List* pThis, PIM_pVoid searchValue, PIM_ListCursor startAfter, PIM_ListCursor* position)
{
	PIM_List_Node* pNode = NULL;

	if(pThis == NULL)
		return PIM_ERROR_NULL_POINTER;

	*position = NULL;
	pNode = (PIM_List_Node*)startAfter;
	
	if (pNode == NULL)
	{
		pNode = pThis->pNodeHead;  /* start at head */
	}
	else
	{
		pNode = pNode->pNext;  /* start after the one specified */
	}

	for (; pNode != NULL; pNode = pNode->pNext)
	{
		if (Compare(pThis, pNode->pData, searchValue) == 0)
		{
			*position = (PIM_ListCursor)pNode;
			break;
		}
	}

	return PIM_SUCCESS;
}



/* defaults to starting at the HEAD, return NULL if not found */
PIM_Result PIM_List_FindIndex(PIM_List* pThis, PIM_Int32 nIndex, PIM_ListCursor* position)
{
	PIM_List_Node* pNode = NULL;

	if(pThis == NULL)
		return PIM_ERROR_NULL_POINTER;

	if( nIndex >= pThis->nCount || nIndex < 0)
		return PIM_ERROR_INVALID_ARGUMENT;

	pNode = pThis->pNodeHead;
	while (nIndex--)
	{
		pNode = pNode->pNext;
	}

	*position = (PIM_ListCursor)pNode;

	return PIM_SUCCESS;
}


PIM_Result PIM_List_Clone(PIM_List *dst, PIM_List *src)
{
    PIM_ListCursor cursor = NULL;
    PIM_pVoid data = NULL;

    dst->nElementSize = src->nElementSize;
    dst->nCount = 0;

    cursor = PIM_List_GetHeadPosition(src);
    for(data = PIM_List_GetNext(&cursor); data != NULL; data = PIM_List_GetNext(&cursor))
    {
        PIM_List_AddTail(dst, data);
    }

    return PIM_SUCCESS;
}



/***************************************************************/
/* Internal method                                             */
/***************************************************************/
PIM_List_Node* API_List_Node_Alloc(PIM_List* pThis, PIM_List_Node* pPrev, PIM_List_Node* pNext)
{
	PIM_List_Node* pNewNode = (PIM_List_Node*)PIM_Calloc(1, sizeof(PIM_List_Node));
	pNewNode->pData = (PIM_pVoid)PIM_Calloc(1, pThis->nElementSize);

	pNewNode->pPrev = pPrev;
	pNewNode->pNext = pNext;

	pThis->nCount++;

	return pNewNode;
}



PIM_Void API_List_Node_Free(PIM_List* pThis, PIM_List_Node* pNode)
{
	if(pNode->pData != NULL)
		PIM_Free(pNode->pData);

	PIM_Free(pNode);
	
	pThis->nCount--;
}
