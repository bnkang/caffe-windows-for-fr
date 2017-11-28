#include "PIMTypes.h"
#include "libPlatform/inc/Table_sqrt.h"
#include "libPlatform/inc/PIMEdge.h"
#include "libPlatform/inc/PIMMemory.h"

struct ETF_N
{
	PIM_Int32 nRow, nCol; // nRow: # of rows(height), nCol: # of columns(=width)
	PIM_Int32 *nGrad;
	PIM_Int32 nMaxGrad;
};



/* Internal Method */
static PIM_Int32 InitWithXYETF_N(struct ETF_N* etf, PIM_Int32 row, PIM_Int32 col);
static void      DeallocETF_N   (struct ETF_N* etf);
static void      NormalizeETF_N (struct ETF_N* etf, PIM_Int32 scale);
static void      Set2ETF_N      (struct ETF_N* etf, PIM_Uint8 *image, PIM_Int32 scale);



/**************************************************************************************
 * Public Method
 *************************************************************************************/
PIM_Result PIM_Edge_GetLineDrawing(PIM_Uint8 *dst, PIM_Uint8 *src, PIM_Int32 width, PIM_Int32 height)
{	
	PIM_Int32 i, size = width*height;
	PIM_Result ret = PIM_SUCCESS;

	struct ETF_N etf;

	ret = InitWithXYETF_N(&etf, height, width);
	if (ret<0)
		return ret;

	Set2ETF_N(&etf, src, 255);

	for(i=0; i<size; i++) 
	{
		dst[i] = (PIM_Uint8)etf.nGrad[i];
	}
	DeallocETF_N(&etf);

	return ret;
}



/**************************************************************************************
 * Internal Method
 *************************************************************************************/
static PIM_Int32 InitWithXYETF_N(struct ETF_N* etf, PIM_Int32 row, PIM_Int32 col)
{
	etf->nRow = row; etf->nCol = col;
	etf->nGrad = (PIM_Int32*)PIM_Calloc(row*col, sizeof(PIM_Int32));
	if (etf->nGrad == NULL)
		return PIM_ERROR_NOMEM;

	etf->nMaxGrad = 1;

	return PIM_SUCCESS;
}

static void DeallocETF_N(struct ETF_N* etf)
{
	PIM_Free(etf->nGrad);
}

static void NormalizeETF_N(struct ETF_N* etf, PIM_Int32 scale)
{
	PIM_Int32 i, size = etf->nRow*etf->nCol;

	if (etf->nMaxGrad > 0)
	{
		for(i=0; i<size; i++)
		{
			etf->nGrad[i] = (etf->nGrad[i] * scale) / etf->nMaxGrad;
		}
	}
}

static void Set2ETF_N(struct ETF_N* etf, PIM_Uint8 *image, PIM_Int32 scale)
{
	PIM_Int32 i,j;
	PIM_Int32 row, col, row1, col1;
	PIM_Int32 idx, idx1;
	PIM_Int32 val0, val1, val;

	row = etf->nRow;
	col = etf->nCol;
	row1 = row-1;
	col1 = col-1;

	etf->nMaxGrad = 0;

	// gradient --> sobel.
	for(i=1; i<row1; i++)
	{
		for(j=1; j<col1; j++)
		{
			val0 = ((image[(i+1)*col+(j-1)] + (image[(i+1)*col+j]<<1) + image[(i+1)*col+(j+1)] 
			- image[(i-1)*col+(j-1)] - (image[(i-1)*col+j]<<1) - image[(i-1)*col+(j+1)]));
			val1 = ((image[(i-1)*col+(j+1)] + (image[(i)*col+(j+1)]<<1) + image[(i+1)*col+(j+1)]
			- image[(i-1)*col+(j-1)] - (image[(i)*col+(j-1)]<<1) - image[(i+1)*col+(j-1)]));

			// tangent --> counter-clockwise...
			val = val0*val0 + val1*val1;	// �ִ밪:1020^2*2 = 2080800 --> (2080800>>6)=32512 < SQRT_TABLE_SIZE(64771)
			etf->nGrad[i*col+j] = (val < SQRT_TABLE_SIZE)?	SQRT_TABLE[val]:(SQRT_TABLE[(val>>6)]<<3);
			if (etf->nMaxGrad < etf->nGrad[i*col+j])
			{
				etf->nMaxGrad = etf->nGrad[i*col+j];
			}
		}
	}

	// boundaries
	idx = row1*col; // the last row.
	for (j = 0; j < col; j++)
	{	
		etf->nGrad[j] = 0;		// 'j'th column in the 1st row.
		etf->nGrad[idx+j] = 0;	// 'j'th column in the last row.
	}

	for (i = 1; i < row1; i++)
	{		
		idx1 = i*col;
		etf->nGrad[idx1] = 0;
		idx1 = idx1 + col1;	// the last column in each row.
		etf->nGrad[idx1] = 0;
	}

	NormalizeETF_N(etf, scale);
}
