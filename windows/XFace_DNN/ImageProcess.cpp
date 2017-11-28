#include "stdafx.h"
#include "exp/PIM_Define_op.h"
#include "ImageProcess.h"
#include <stdlib.h>
#include <math.h>
#ifndef WIN32
#include "Allocator.h"
#endif

#define BIT_SHIFT		15



void Resize(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data, int des_w, int des_h)
//void Resize(unsigned char *srcData, int nSrcW, int nSrcH, unsigned char *desData, int nDesW, int nDesH)
{
	int i;
	unsigned int utx, uty = 0;
	unsigned int uda_11 = (src_w << BIT_SHIFT) / des_w;
	unsigned int uda_22 = (src_h << BIT_SHIFT) / des_h;
	unsigned char *psrc_data = src_data;
	unsigned char *pdes_data = des_data;

	while( des_h-- ) {
		utx = 0;
		i = des_w;
		while( i-- ) {
			utx += uda_11;
			*pdes_data++ = *(psrc_data + (utx >> BIT_SHIFT));
		}
		uty += uda_22;
		psrc_data = src_data + (uty >> BIT_SHIFT)*src_w;
	}
	/*
	int udtx=0, udty=0, udtx_=0, udty_=0;
	int uda_11;
	int uda_22;
	int i, j;
	int utx=0, uty=0;
	int locgray4 = 0;
	int ucgray1 = 0, ucgray2 = 0, ucgray3 = 0, ucgray4 = 0;
	int nSrcIndex=0;
	unsigned char *pucsrcp = 0;
	unsigned char *pucdstp = desData;
	unsigned int aa, ab, ac, ad;

	uda_11 = (nSrcW << 10) / nDesW;
	uda_22 = (nSrcH << 10) / nDesH;

	for(j = 0; j < nDesH; j++) {
	udtx = 0;
	udtx_ = 0;
	utx = 0;
	for(i = 0; i < nDesW; i++) {
	locgray4 = 0;
	pucsrcp = srcData + (nSrcIndex + utx);
	ucgray1 = *pucsrcp;

	if(i >= nDesW - 1) {
	ucgray2 = *pucsrcp;
	udtx = 1 << 10;
	}
	else {
	ucgray2 = *(pucsrcp + 1);
	locgray4 += 1;
	}

	if (j >= nDesH - 1) {
	ucgray3 = *pucsrcp;
	udty = 1 << 10;
	}
	else {
	ucgray3 = *( pucsrcp + nSrcW );
	locgray4 += nSrcW;
	}
	ucgray4 = *( pucsrcp + locgray4 );

	aa = ucgray1 << 20;
	ab = ((ucgray2 - ucgray1) * (udtx << 10));
	ac = ((ucgray3 - ucgray1) * (udty << 10));
	ad = (ucgray1 + ucgray4 - ucgray3 - ucgray2) * udtx * udty;
	*(pucdstp + i) = (unsigned char)((aa + ab + ac + ad)>> 20);

	udtx_ += uda_11;
	udtx = udtx_;
	utx = udtx >> 10;
	udtx -= ( utx << 10 );
	}

	udty_ += uda_22;
	udty = udty_;
	uty = udty >> 10;
	udty -= ( uty << 10 );
	nSrcIndex = uty * nSrcW;
	pucdstp += nDesW;
	}
	*/
}

