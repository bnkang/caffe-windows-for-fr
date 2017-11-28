#include "PIMTypes.h"
#include "PIMArray.h"
#include "libPlatform/inc/PIMMemory.h"

//////////////////////////////////////////////////////////////
// Construction
PIM_Bool PIM_Array_Alloc (PIM_Array* pThis, PIM_Int32 nElementSize, PIM_Int32 capacity)
{
	/* initialize */
	pThis->pData = NULL;
	pThis->nSize = 0;
	pThis->nMaxSize = 0;
	pThis->nGrowBy = 0;
	pThis->nElementSize = nElementSize;

	/* initialize nMaxSize */
	PIM_Array_SetSize(pThis, capacity, 0);

	return 1;
}

void     PIM_Array_Free  (PIM_Array* pThis)
{
	PIM_Free(pThis->pData);
	pThis->pData = 0;
	pThis->nSize = 0;
	pThis->nMaxSize = 0;
	pThis->nGrowBy = 0;
}
	
//////////////////////////////////////////////////////////////
// Attributes
PIM_Int32 PIM_Array_GetSize  (PIM_Array* pThis)
{
	return pThis->nSize;
}

PIM_Int32  PIM_Array_GetCount (PIM_Array* pThis)
{
	return pThis->nSize;
}

PIM_Bool PIM_Array_IsEmpty  (PIM_Array* pThis)
{
	if(pThis->nSize == 0)
		return 1;
	return 0;
}

PIM_Int32	PIM_Array_SetSize  (PIM_Array* pThis, PIM_Int32 nNewSize, PIM_Int32 nGrowBy)
{
	/* Adjust the size of the internal array. "nSize" will be set to the value of "nNewSize" */
	if(nNewSize < 0 || pThis->nElementSize == 0)
		return PIM_ERROR_INVALID_ARGUMENT;

	if (nGrowBy >= 0)
		pThis->nGrowBy = nGrowBy;  /* set new size */

	if (nNewSize == 0)
	{
		if (pThis->pData != 0)
		{
			PIM_Free(pThis->pData);
			pThis->pData = NULL;
		}
		pThis->nSize = pThis->nMaxSize = 0;
	}
	else if (pThis->pData == NULL)
	{
		/* initialize pData array by nNewSize */
		PIM_Int32 nAllocSize = nNewSize > pThis->nGrowBy ? nNewSize : pThis->nGrowBy;
		//pThis->pData = PIM_Malloc(nAllocSize * sizeof(PIM_pVoid));
		//PIM_Memset(pThis->pData, 0, nAllocSize * sizeof(PIM_pVoid));

		pThis->pData = PIM_Malloc(nAllocSize * pThis->nElementSize);
		PIM_Memset(pThis->pData, 0x00, nAllocSize * pThis->nElementSize);

		pThis->nSize = nNewSize; /* need a validation. */
		pThis->nMaxSize = nAllocSize;
	}
	else if (nNewSize <= pThis->nMaxSize)
	{
		PIM_Int32 i = 0;
		PIM_Uint8* pData = 0;

		/* it fits */
		if (nNewSize > pThis->nSize)
		{
			pData = (PIM_Uint8*)pThis->pData;

			/* initialize the new elements */
			//PIM_Memset((void*)(pThis->pData + pThis->nSize), 0, (size_t)(nNewSize-pThis->nSize) * sizeof(PIM_pVoid));
			PIM_Memset((void*)(pData + pThis->nSize * pThis->nElementSize), 0, (nNewSize-pThis->nSize) * pThis->nElementSize);
		}
		else if (pThis->nSize > nNewSize)
		{
			/* remove the old elements */
			pData = (PIM_Uint8*)pThis->pData;

			for( i = 0; i < pThis->nSize-nNewSize; i++ )
			{
				*(pData + (nNewSize + i)*pThis->nElementSize) = 0x00;
			}
		}
		pThis->nSize = nNewSize; /* need a validation. */
	}
	else
	{
		PIM_Int32 nNewMax = 0;
		PIM_Uint8* pData = 0;
		PIM_Uint8* pNewData = 0;

		/* otherwise, grow array */
		nGrowBy = pThis->nGrowBy;

		if (nGrowBy == 0)
		{
			/* heuristically determine growth when nGrowBy == 0    *
			 * (this avoids heap fragmentation in many situations) */
			nGrowBy = pThis->nSize / 8;
			nGrowBy = (nGrowBy < 4) ? 4 : ((nGrowBy > 1024) ? 1024 : nGrowBy);
		}

		if (nNewSize < pThis->nMaxSize + nGrowBy)
			nNewMax = pThis->nMaxSize + nGrowBy;  /* granularity */
		else
			nNewMax = nNewSize;  /* no slush */

		//ASSERT(nNewMax >= pThis->nMaxSize);  /* no wrap around */
		if(nNewMax < pThis->nMaxSize)
			return PIM_ERROR_INVALID_ARGUMENT;

		//pNewData = PIM_Malloc( sizeof(PIM_pVoid) * nNewMax );
		//PIM_Memset(pNewData, 0x00, sizeof(PIM_pVoid) * nNewMax);
		//PIM_Memcpy(pNewData, pThis->pData, pThis->nSize * sizeof(PIM_pVoid)); /* copy new data from old */

		pData = (PIM_Uint8*)pThis->pData;
		pNewData = PIM_Malloc( pThis->nElementSize * nNewMax );
		PIM_Memset(pNewData, 0x00, pThis->nElementSize * nNewMax);
		PIM_Memcpy(pNewData, pThis->pData, pThis->nElementSize * pThis->nSize); /* copy new data from old */

		/* set the new attributes */
		PIM_Free(pThis->pData);
		pThis->pData = (PIM_pVoid*)pNewData;
		pThis->nSize = nNewSize; /* need a validation. */
		pThis->nMaxSize = nNewMax;
	}

	return PIM_SUCCESS;
}


////////////////////////////////////////////////////////////////
//// Operations
//// Clean up
void PIM_Array_FreeExtra (PIM_Array* pThis)
{
	if(pThis == NULL)
		return;

	if (pThis->nSize != pThis->nSize)
	{
		/* shrink to desired size */
		PIM_pVoid* pNewData = NULL;

		if (pThis->nSize != 0)
		{
			PIM_Int32 len = pThis->nElementSize * pThis->nSize;

			pNewData = (PIM_pVoid*)PIM_Malloc(len);

			// copy new data from old
			PIM_Memcpy(pNewData, pThis->pData, len); 
		}

		/* get rid of old stuff (note: no destructors called) */
		PIM_Free(pThis->pData);
		pThis->pData = pNewData;
		pThis->nMaxSize = pThis->nSize;
	}
}

void PIM_Array_RemoveAll (PIM_Array* pThis)
{
	PIM_Array_SetSize(pThis, 0, -1);
}

/* Accessing elements */
const PIM_pVoid PIM_Array_GetAt(PIM_Array* pThis, PIM_Int32 nIndex)
{
	PIM_Uint8* pData = (PIM_Uint8*)pThis->pData;
	//ASSERT(nIndex >= 0 && nIndex < pThis->nSize);
	if(nIndex >= 0 && nIndex < pThis->nSize)
		return pData + nIndex * pThis->nElementSize;  // pThis->pData[nIndex];
	else
		return 0;
}

PIM_Int32 PIM_Array_SetAt(PIM_Array* pThis, PIM_Int32 nIndex, PIM_pVoid newElement)
{
	PIM_Uint8* pData = (PIM_Uint8*)pThis->pData;

	//ASSERT(nIndex >= 0 && nIndex < pThis->nSize);
	if(nIndex >= 0 && nIndex < pThis->nMaxSize)
	{
		PIM_Memcpy(pData + nIndex * pThis->nElementSize, newElement, pThis->nElementSize); //pThis->pData[nIndex] = newElement; 
		return PIM_SUCCESS;
	}
	else
		return PIM_ERROR_INVALID_ARGUMENT;
}

// Potentially growing the array
PIM_Int32 PIM_Array_Add(PIM_Array* pThis, PIM_pVoid newElement)
{
	PIM_Int32 nIndex = pThis->nSize;
	
	PIM_Array_SetAtGrow(pThis, nIndex, newElement);

	return nIndex;
}

PIM_Int32 PIM_Array_AddArray(PIM_Array* pThis, const PIM_Array* pNewArray)
{
	PIM_Int32 nOldSize = 0;
	PIM_Int32 result = 0;
	PIM_Uint8* pData = 0;

	if(pThis == pNewArray)
		return PIM_ERROR_INVALID_ARGUMENT;

	nOldSize = pThis->nSize;
	result = PIM_Array_SetSize(pThis, pThis->nSize + pNewArray->nSize, -1);

	if(result != PIM_SUCCESS)
		return result;
	
	pData = (PIM_Uint8*)pThis->pData;

	PIM_Memcpy((void*)(pData + nOldSize * pThis->nElementSize), (void*)(pNewArray->pData), (pNewArray->nSize * pThis->nElementSize));

	return nOldSize;
}

PIM_Int32 PIM_Array_SetAtGrow(PIM_Array* pThis, PIM_Int32 nIndex, PIM_pVoid newElement)
{
	PIM_Uint8* pData = 0;

	if(nIndex < 0)
		return PIM_ERROR_INVALID_ARGUMENT;

	if (nIndex >= pThis->nSize)
		PIM_Array_SetSize(pThis, nIndex+1, -1);

	//pThis->pData[nIndex] = newElement;

	pData = (PIM_Uint8*)pThis->pData;
	PIM_Memcpy(pData + (nIndex * pThis->nElementSize), newElement, pThis->nElementSize);

	return PIM_SUCCESS;
}


PIM_Int32 PIM_Array_SetAtGrowArray (PIM_Array* pThis, PIM_Int32 nStartIndex, PIM_Array* pNewArray) // InsertArrayAt
{
    PIM_Int32 i = 0;
    PIM_Int32 len = 0;
    PIM_Uint8* pData = 0;

    if(pNewArray == NULL)
        return  PIM_ERROR_NULL_POINTER;

    if(nStartIndex < 0 )
        return PIM_ERROR_INVALID_ARGUMENT;

    if(pThis->nElementSize != pNewArray->nElementSize)
        return PIM_ERROR_INVALID_ARGUMENT;

    if(pNewArray->nSize <= 0)
        return PIM_ERROR_INVALID_ARGUMENT;

    if (pNewArray->nSize > 0)
    {
        if (nStartIndex >= pThis->nSize)
        {
            /* adding after the end of the array */
            PIM_Array_SetSize(pThis, nStartIndex + pNewArray->nSize, -1);   /* grow so nIndex is valid */
        }
        else if(nStartIndex + pNewArray->nSize > pThis->nSize)
        {
            /* inserting in the middle of the array */
            PIM_Int32 nOldSize = pThis->nSize;

            PIM_Array_SetSize(pThis, nOldSize + pNewArray->nSize, -1);  /* grow it to new size */

            /* destroy intial data before copying over it */
            pData = (PIM_Uint8*)pThis->pData;
            len = pNewArray->nSize * pNewArray->nElementSize;

            for( i = 0; i < len; i++ )
                *(pData + nStartIndex * pThis->nElementSize + i) = 0x00;
        }

        pData = (PIM_Uint8*)pThis->pData;

        PIM_Memcpy(pData + nStartIndex * pThis->nElementSize, pNewArray->pData, pNewArray->nSize * pNewArray->nElementSize);
    }

    return PIM_SUCCESS;
}

PIM_Int32 PIM_Array_InsertAt(PIM_Array* pThis, PIM_Int32 nIndex, PIM_pVoid newElement, PIM_Int32 nCount /*=1*/)
{
	PIM_Int32 i = 0;
	PIM_Int32 len = 0;
	PIM_Uint8* pData = 0;

	if(nIndex < 0)
		return PIM_ERROR_INVALID_ARGUMENT;

	if(nCount <= 0)
		return PIM_ERROR_INVALID_ARGUMENT;

	if (nIndex >= pThis->nSize)
	{
		/* adding after the end of the array */
		PIM_Array_SetSize(pThis, nIndex + nCount, -1);   /* grow so nIndex is valid */
	}
	else
	{
		/* inserting in the middle of the array */
		PIM_Int32 nOldSize = pThis->nSize;
		PIM_Array_SetSize(pThis, pThis->nSize + nCount, -1);  /* grow it to new size */

		/* destroy intial data before copying over it */
		pData = (PIM_Uint8*)pThis->pData;
		len = nCount * pThis->nElementSize;

		for( i = 0; i < len; i++ )
			*(pData + nOldSize * pThis->nElementSize + i) = 0x00;
		
		/* shift old data up to fill gap */
		PIM_Memcpy(	pData + (nIndex + nCount) * pThis->nElementSize, 
				pData + nIndex * pThis->nElementSize, 
				(nOldSize-nIndex) * pThis->nElementSize	);

		/* re-init slots we copied from */
		PIM_Memset((void*)(pData + nIndex * pThis->nElementSize), 0x00, nCount * pThis->nElementSize);
	}

	/* insert new value in the gap */
	// ASSERT(nIndex + nCount <= pThis->nSize);
	if(nIndex + nCount > pThis->nSize)
		return PIM_ERROR_INVALID_ARGUMENT;

	pData = (PIM_Uint8*)pThis->pData;
	while (nCount--)
	{
		//pThis->pData[nIndex++] = newElement;
		PIM_Memcpy(pData + nIndex * pThis->nElementSize, newElement, pThis->nElementSize);
		nIndex++;
	}

	return PIM_SUCCESS;
}

PIM_Int32 PIM_Array_InsertAtArray (PIM_Array* pThis, PIM_Int32 nStartIndex, PIM_Array* pNewArray)
{
	//ASSERT(pNewArray != NULL);
	//ASSERT(nStartIndex >= 0);
	PIM_Uint8* pData = 0;

	if(pNewArray == NULL)
		return PIM_ERROR_NULL_POINTER;

	if(nStartIndex < 0)
		return PIM_ERROR_INVALID_ARGUMENT;

	if (pNewArray->nSize > 0)
	{
		PIM_Uint8* pDummy = PIM_Malloc(pThis->nElementSize);
		
		PIM_Int32 result = PIM_Array_InsertAt(pThis, nStartIndex, (void*)pDummy, pNewArray->nSize);
		
		PIM_Free(pDummy);
		
		if( PIM_SUCCESS != result)
			return result;

		pData = (PIM_Uint8*)pThis->pData;
		PIM_Memcpy(pData + nStartIndex * pThis->nElementSize, pNewArray->pData, pNewArray->nSize * pNewArray->nElementSize);
	}

	return PIM_SUCCESS;
}

PIM_Int32 PIM_Array_RemoveAt(PIM_Array* pThis, PIM_Int32 nIndex)
{
	PIM_Int32 nUpperBound = 0;
	PIM_Int32 nMoveCount = 0;
	PIM_Uint8* pData;

	// ASSERT(nIndex >= 0);
	// ASSERT(nIndex < pThis->nSize);
	if(nIndex < 0 || nIndex >= pThis->nSize)
		return PIM_ERROR_INVALID_ARGUMENT;

	nUpperBound = nIndex + 1;

	nMoveCount = pThis->nSize - nIndex - 1;

	pData = (PIM_Uint8*)pThis->pData;

	PIM_Memcpy(pData + nIndex * pThis->nElementSize, 
		   pData + (nIndex + 1) * pThis->nElementSize, 
		   pThis->nElementSize * nMoveCount );

	PIM_Memset(pData + (pThis->nSize - 1) * pThis->nElementSize, 0x00, pThis->nElementSize);

	pThis->nSize--;

	return PIM_SUCCESS;
}

//PIM_Array PIM_Array_Copy (const PIM_Array* src);
PIM_Int32 PIM_Array_Copy (PIM_Array* pThis, const PIM_Array* pSrc)
{
    if(pThis->nElementSize != pSrc->nElementSize)
        return PIM_ERROR_INVALID_ARGUMENT;

    PIM_Array_SetSize(pThis, pSrc->nSize, -1);

    PIM_Memcpy(pThis->pData, pSrc->pData, pSrc->nSize * pSrc->nElementSize);

    return PIM_SUCCESS;    
}

