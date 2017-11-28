
#include "PIMTypes.h"
#include "libPlatform/inc/ImageProcess.h"
#include "libPlatform/inc/PIMMemory.h"

#if 1
void Resize_BI(PIM_Uint8 *src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Uint8 *des_data, PIM_Int32 des_w, PIM_Int32 des_h)
{
	/* < 10 */
	PIM_Int32 udtx, udty;
	/* < 10 */
	PIM_Int32 uda_11;
	/* < 10 */
	PIM_Int32 uda_22;
	PIM_Int32 i, j;
	PIM_Int32 utx, uty;
	PIM_Int32 locgray4 = 0;
	PIM_Int32 ucgray1 = 0, ucgray2 = 0, ucgray3 = 0, ucgray4 = 0;
	PIM_Uint32 aa, ab, ac, ad;
	
	PIM_Uint8 *pucsrcp = 0;
	PIM_Uint8 *pucdstp = des_data;	

	/* Affine transformation */
	
	uda_11 = ( src_w << 10 ) / des_w;
	uda_22 = ( src_h << 10 ) / des_h;

	for ( j = 0; j < des_h; j++ ) {
		for ( i = 0; i < des_w; i++ ) {
			locgray4 = 0;

			udtx = uda_11 * i;
			udty = uda_22 * j;

			utx = udtx >> 10;
			uty = udty >> 10;
			udtx -= ( utx << 10 );
			udty -= ( uty << 10 );
			pucsrcp = src_data + ( uty * src_w ) + utx;

			ucgray1 = *pucsrcp;

			if ( i >= des_w - 1 ) {
				ucgray2 = *pucsrcp;
				udtx = 1 << 10;
			}
			else {
				ucgray2 = *( pucsrcp + 1 );
				locgray4 += 1;
			}

			if ( j >= des_h - 1 ) {
				ucgray3 = *pucsrcp;
				udty = 1 << 10;
			}
			else {
				ucgray3 = *( pucsrcp + src_w );
				locgray4 += src_w;
			}

			ucgray4 = *( pucsrcp + locgray4 );

			aa = ucgray1 << 20;
			ab = ( ( ucgray2 - ucgray1 ) * ( udtx << 10 ) );
			ac = ( ( ucgray3 - ucgray1 ) * ( udty << 10 ) );
			ad = ( ucgray1 + ucgray4 - ucgray3 - ucgray2 ) * udtx * udty;

			*( pucdstp + i ) = ( PIM_Uint8 ) ( ( aa + ab + ac + ad ) >> 20 );
		}
		
		pucdstp += des_w;
	}
}

void Resize_BI_BGR(PIM_Uint8 *src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Uint8 *des_data, PIM_Int32 des_w, PIM_Int32 des_h)
{
	/* < 10 */
	PIM_Int32 udtx, udty;
	/* < 10 */
	PIM_Int32 uda_11;
	/* < 10 */
	PIM_Int32 uda_22;
	PIM_Int32 i, j;
	PIM_Int32 utx, uty;
	PIM_Int32 locgray4 = 0;
	//PIM_Int32 ucgray1 = 0, ucgray2 = 0, ucgray3 = 0, ucgray4 = 0;

	PIM_Int32 ucgray1_b, ucgray1_g, ucgray1_r, ucgray1_i;
	PIM_Int32 ucgray2_b, ucgray2_g, ucgray2_r, ucgray2_i;
	PIM_Int32 ucgray3_b, ucgray3_g, ucgray3_r, ucgray3_i;
	PIM_Int32 ucgray4_b, ucgray4_g, ucgray4_r, ucgray4_i;

	PIM_Uint32 aa, ab, ac, ad;

	PIM_Uint8 *pucsrcp = 0;
	PIM_Uint8 *pucdstp = des_data;

	/* Affine transformation */

	uda_11 = (src_w << 10) / des_w;
	uda_22 = (src_h << 10) / des_h;

	for( j = 0; j < des_h; j++ ) {
		for( i = 0; i < des_w; i++ ) {
			locgray4 = 0;

			udtx = uda_11 * i;
			udty = uda_22 * j;

			utx = udtx >> 10;
			uty = udty >> 10;
			udtx -= (utx << 10);
			udty -= (uty << 10);
			pucsrcp = src_data + (((uty * src_w) + utx) * 3);

			ucgray1_b = *pucsrcp;
			ucgray1_g = *(pucsrcp + 1);
			ucgray1_b = *(pucsrcp + 2);

			
			if( i >= des_w - 1 ) {
				ucgray2_b = *pucsrcp;
				ucgray2_g = *(pucsrcp + 1);
				ucgray2_r = *(pucsrcp + 2);
				udtx = 1 << 10;
			}
			else {
				ucgray2_b = *(pucsrcp + 3);
				ucgray2_g = *(pucsrcp + 4);
				ucgray2_r = *(pucsrcp + 5);
				locgray4 += 3;
			}

			if( j >= des_h - 1 ) {				
				ucgray3_b = *pucsrcp;
				ucgray3_g = *(pucsrcp + 1);
				ucgray3_r = *(pucsrcp + 2);
				udty = 1 << 10;
			}
			else {
				unsigned char *p = pucsrcp + (src_w * 3);
				ucgray3_b = *p++;
				ucgray3_g = *p++;
				ucgray3_r = *p++;
				locgray4 += src_w*3;
			}

			unsigned char *p = pucsrcp + locgray4;
			ucgray4_b = *p++;
			ucgray4_g = *p++;
			ucgray4_r = *p;
						
			unsigned char *pd = pucdstp + (i * 3);
			aa = ucgray1_b << 20;
			ab = ((ucgray2_b - ucgray1_b) * (udtx << 10));
			ac = ((ucgray3_b - ucgray1_b) * (udty << 10));
			ad = (ucgray1_b + ucgray4_b - ucgray3_b - ucgray2_b) * udtx * udty;
			*pd++ = (unsigned int)((aa + ab + ac + ad) >> 20);
			aa = ucgray1_g << 20;
			ab = ((ucgray2_g - ucgray1_g) * (udtx << 10));
			ac = ((ucgray3_g - ucgray1_g) * (udty << 10));
			ad = (ucgray1_g + ucgray4_g - ucgray3_g - ucgray2_g) * udtx * udty;
			*pd++ = (unsigned int)((aa + ab + ac + ad) >> 20);
			aa = ucgray1_r << 20;
			ab = ((ucgray2_r - ucgray1_r) * (udtx << 10));
			ac = ((ucgray3_r - ucgray1_r) * (udty << 10));
			ad = (ucgray1_r + ucgray4_r - ucgray3_r - ucgray2_r) * udtx * udty;
			*pd++ = (unsigned int)((aa + ab + ac + ad) >> 20);
			aa = ucgray1_i << 20;
			ab = ((ucgray2_i - ucgray1_i) * (udtx << 10));
			ac = ((ucgray3_i - ucgray1_i) * (udty << 10));
			ad = (ucgray1_i + ucgray4_i - ucgray3_i - ucgray2_i) * udtx * udty;
			*pd++ = (unsigned int)((aa + ab + ac + ad) >> 20);
		}

		pucdstp += des_w*3;
	}
}

void resize_bi(PIM_Uint8 *srcData, PIM_Int32 nSrcW, PIM_Int32 nSrcH, PIM_Uint8 *desData, PIM_Int32 nDesW, PIM_Int32 nDesH)
{
	PIM_Int32 udtx = 0, udty = 0, udtx_ = 0, udty_ = 0;
	PIM_Int32 uda_11;
	PIM_Int32 uda_22;
	PIM_Int32 i, j;
	PIM_Int32 utx = 0, uty = 0;
	PIM_Int32 locgray4 = 0;
	PIM_Int32 ucgray1 = 0, ucgray2 = 0, ucgray3 = 0, ucgray4 = 0;
	PIM_Int32 nSrcIndex = 0;
	PIM_Uint8 *pucsrcp = 0;
	PIM_Uint8 *pucdstp = desData;
	PIM_Uint32 aa, ab, ac, ad;

	uda_11 = (nSrcW << 10) / nDesW;
	uda_22 = (nSrcH << 10) / nDesH;

	for( j = 0; j < nDesH; j++ )
	{
		udtx = 0;
		udtx_ = 0;
		utx = 0;
		for( i = 0; i < nDesW; i++ )
		{
			locgray4 = 0;
			pucsrcp = srcData + (nSrcIndex + utx);
			ucgray1 = *pucsrcp;

			if( i >= nDesW - 1 )
			{
				ucgray2 = *pucsrcp;
				udtx = 1 << 10;
			}
			else
			{
				ucgray2 = *(pucsrcp + 1);
				locgray4 += 1;
			}

			if( j >= nDesH - 1 )
			{
				ucgray3 = *pucsrcp;
				udty = 1 << 10;
			}
			else
			{
				ucgray3 = *(pucsrcp + nSrcW);
				locgray4 += nSrcW;
			}
			ucgray4 = *(pucsrcp + locgray4);

			aa = ucgray1 << 20;
			ab = ((ucgray2 - ucgray1) * (udtx << 10));
			ac = ((ucgray3 - ucgray1) * (udty << 10));
			ad = (ucgray1 + ucgray4 - ucgray3 - ucgray2) * udtx * udty;
			*(pucdstp + i) = (PIM_Uint8)((aa + ab + ac + ad) >> 20);

			udtx_ += uda_11;
			udtx = udtx_;
			utx = udtx >> 10;
			udtx -= (utx << 10);
		}

		udty_ += uda_22;
		udty = udty_;
		uty = udty >> 10;
		udty -= (uty << 10);
		nSrcIndex = uty * nSrcW;
		pucdstp += nDesW;
	}
}

#define BIT_SHIFT 15
void Resize_NN(PIM_Uint8 *src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Uint8 *des_data, PIM_Int32 des_w, PIM_Int32 des_h)
{
	PIM_Int32 i;
	PIM_Uint32 utx=0, uty=0;
	PIM_Uint32 uda_11 = (src_w << BIT_SHIFT) / des_w;
	PIM_Uint32 uda_22 = (src_h << BIT_SHIFT) / des_h;
	PIM_Uint8 *psrc_data = src_data;
	PIM_Uint8 *pdes_data = des_data;	

	while( des_h-- )
	{
		utx = 0;
		i = des_w;
		while( i-- )
		{
			
			*pdes_data++ = *(psrc_data + (utx >> BIT_SHIFT));		
			utx += uda_11;
		}		
		uty += uda_22;		
		psrc_data = src_data + (uty >> BIT_SHIFT)*src_w;
	}
}
#endif


PIM_Bool Region(PIM_Uint8 *src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Uint8* des_data, PIM_Rect rcRegion)
{	
	PIM_Int32 roi_x = rcRegion.left;
	PIM_Int32 roi_y = rcRegion.top;
	PIM_Int32 roi_w = rcRegion.right - rcRegion.left;
	PIM_Int32 roi_h = rcRegion.bottom - rcRegion.top;

	PIM_Int32 i, src_index, region_index;
	if(roi_x + roi_w > src_w)		return PIM_FALSE;
	if(roi_y + roi_h > src_h)		return PIM_FALSE;
		
	src_index = roi_y * src_w + roi_x;
	region_index = 0;
	for( i = 0; i < roi_h; i++ )
	{
		PIM_Memcpy(des_data + region_index, src_data + src_index, roi_w);
		src_index += src_w;
		region_index += roi_w;
	}
	return PIM_TRUE;
}

PIM_Bool Region_RGB(PIM_Uint8 *src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Uint8* des_data, PIM_Rect rcRegion)
{
	PIM_Int32 roi_x = rcRegion.left;
	PIM_Int32 roi_y = rcRegion.top;
	PIM_Int32 roi_w = rcRegion.right - rcRegion.left;
	PIM_Int32 roi_h = rcRegion.bottom - rcRegion.top;

	PIM_Int32 i, src_index, region_index;
	if( roi_x + roi_w > src_w )		return PIM_FALSE;
	if( roi_y + roi_h > src_h )		return PIM_FALSE;

	src_index = roi_y * (src_w*3) + (roi_x*3);
	region_index = 0;
	for( i = 0; i < roi_h; i++ )
	{
		PIM_Memcpy(des_data + region_index, src_data + src_index, roi_w*3);
		src_index += (src_w*3);
		region_index += roi_w*3;
	}
	return PIM_TRUE;
}

PIM_Bool Region_INT(PIM_Int32* src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Int32* des_data, PIM_Rect rcRegion)
{	
	PIM_Int32 roi_x = rcRegion.left, roi_y = rcRegion.top;
	PIM_Int32 roi_w = rcRegion.right - rcRegion.left;
	PIM_Int32 roi_h = rcRegion.bottom - rcRegion.top;

	PIM_Int32 i, src_index, region_index;
	if(roi_x+roi_w > src_w)			return PIM_FALSE;
	if(roi_y+roi_h > src_h)			return PIM_FALSE;

	src_index = roi_y * src_w + roi_x;
	region_index = 0;
	for( i = 0; i < roi_h; i++ )
	{
		PIM_Memcpy(des_data + region_index, src_data + src_index, roi_w * sizeof(PIM_Int32));
		src_index += src_w;
		region_index += roi_w;
	}
	return PIM_TRUE;
}


#if 0//ndef __CODE_SIZE_OPTIMIZE__
extern const PIM_Int32 sin_shift10[];
extern const PIM_Int32 cos_shift10[];

/* originated from CImageProcess class */
void CImageProcessing_rotation(PIM_Uint8 *srcData, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Uint8 *desData, PIM_Int32 cx, PIM_Int32 cy, PIM_Int32 radian)
{
#define SBASE  10
	PIM_Int32 srcSize = src_w*src_h;
	PIM_Int32 x, y;
	PIM_Int32 x_, y_;
	PIM_Int32 t2;
	PIM_Int32 a, b, y1, y2;
	PIM_Int32 l, k, src, src_l, src_k, src_lk, reg, des_w_step, end_w, end_h;
	const PIM_Int32 s = sin_shift10[radian/5+36];
	const PIM_Int32 c = cos_shift10[radian/5+36];

	PIM_Memset(desData, 0, srcSize);

	des_w_step = 0;
	end_w = src_w-2; end_h = src_h-2;

	
	for(y=0; y<src_h; y++) {
		y1 = (-s*(y-cy)) + (cx<<SBASE);
		y2 = ( c*(y-cy)) + (cy<<SBASE);

		for(x=0; x<src_w; x++) {
			reg = x-cx;
			x_ = (c*reg) + y1;
			y_ = (s*reg) + y2;

			if(x_ < 0 || x_ > (end_w<<SBASE)) continue;
			if(y_ < 0 || y_ > (end_h<<SBASE)) continue;
			
			l = (x_>>SBASE);
			k = (y_>>SBASE);
			
			l = l<<SBASE;
			k = k<<SBASE;

			a = x_ - l;
			b = y_ - k;
			
			reg = ( (k*src_w+l) >> SBASE);
			src = srcData[reg];
			src_l = srcData[reg+1];
			reg += src_w; 
			src_k = srcData[reg];
			src_lk = srcData[reg+1];

			t2 = (				(src<<SBASE) + (a*(src_l-src))        + (b*(src_k-src))        + (( a*b*(src+src_lk-src_l-src_k) )>>SBASE))>>SBASE;
			desData[des_w_step+x] = (PIM_Uint8) t2;
		}
		des_w_step += src_w;
		
	}
	
}
#endif /* __CODE_SIZE_OPTIMIZE__ */


#ifndef __CODE_SIZE_OPTIMIZE__
void preprocessByteImage(PIM_Uint8* src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Uint8* lbp_data)
{

	PIM_Uint32 size = src_w*src_h;
	PIM_Uint32 m1, m2, m3, m4, m5, m6, m7, m8, m9;
	PIM_Uint32 data_lbp;		
	PIM_Uint8 *pt1, *pt2, *pt3;
	PIM_Uint8 *plbp = lbp_data+src_w+1;
	
	pt1 = src_data;
	pt2 = pt1 + src_w;
	pt3 = pt2 + src_w;
	
	m1 = *pt1++;
	m2 = *pt1++;
	m4 = *pt2++;
    m5 = *pt2++;
	m7 = *pt3++;	
	m8 = *pt3++;

	size -= src_w;
	size -= src_w;
	size -= 2;	// 080319kyom
	while(size--) {
		m3 = *pt1++;
		m6 = *pt2++;
		m9 = *pt3++;
				
		data_lbp = ((m5 - m1)&0x100);
		data_lbp |= ((m5 - m2)&0x200);
		data_lbp |= ((m5 - m3)&0x400);
		data_lbp |= ((m5 - m6)&0x800);
		data_lbp |= ((m5 - m9)&0x1000);
		data_lbp |= ((m5 - m8)&0x2000);
		data_lbp |= ((m5 - m7)&0x4000);
		data_lbp |= ((m5 - m4)&0x8000);
		data_lbp >>= 8;
		
		m1 = m2;
		m2 = m3;
		m4 = m5;
		m5 = m6;
		m7 = m8;
		m8 = m9;

		*plbp++ = data_lbp;	
	}
}
#endif /* __CODE_SIZE_OPTIMIZE__ */


void image2LBP8_1_Reco(PIM_Uint8* src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Uint8* lbp_data)
{
	PIM_Int32 x, y, w=src_w, h=src_h-1;
	PIM_Int32 m1, m2, m3, m4, m5, m6, m7, m8, m9;
	PIM_Int32 data_lbp;	
	PIM_Uint8 *pt1, *pt2=src_data+src_w, *pt3;
	PIM_Uint8 *pmct = lbp_data+src_w+1;

	PIM_Memset(lbp_data, 0, src_w*src_h);
	
	pt1 = pt2-src_w;
	pt3 = pt2+src_w;

	for(y=1; y<h; y++) {
		m1 = *pt1++;
		m4 = *pt2++;
	    m7 = *pt3++;
		m2 = *pt1++;
		m5 = *pt2++;
		m8 = *pt3++;
		
		for(x=2; x<w; x++) {
			m3 = *pt1++;
			m6 = *pt2++;
			m9 = *pt3++;

			data_lbp = (((m5 - m1)>>8)&1);
			data_lbp |= ((((m5 - m2)>>8)&1)<<1);
			data_lbp |= ((((m5 - m3)>>8)&1)<<2);
			data_lbp |= ((((m5 - m6)>>8)&1)<<3);
			data_lbp |= ((((m5 - m9)>>8)&1)<<4);
			data_lbp |= ((((m5 - m8)>>8)&1)<<5);
			data_lbp |= ((((m5 - m7)>>8)&1)<<6);
			data_lbp |= ((((m5 - m4)>>8)&1)<<7);
								
			m1 = m2;
			m4 = m5;
			m7 = m8;
			m2 = m3;
			m5 = m6;
			m8 = m9;

			*pmct++ = data_lbp;			
		}
		pmct++;
		pmct++;
	}
}


void image2LBP8_1_FR(PIM_Uint8* src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Uint8* lbp_data)
{
	PIM_Int32 i;
	PIM_Uint32 cx;
	PIM_Uint32 size = src_w*(src_h-2);
	PIM_Uint32 data_lbp;		
	PIM_Uint8 *pt = src_data;
	PIM_Uint8 *plbp = lbp_data+(src_w+1);
	while(size--) {
		cx = *(pt+src_w+1);

		data_lbp = ((PIM_Uint32)(cx - *pt) & 0x80000000) >> 31;
		data_lbp |= ((PIM_Uint32)(cx - *(pt+1)) & 0x80000000) >> 30;
		data_lbp |= ((PIM_Uint32)(cx - *(pt+2)) & 0x80000000) >> 29;
		data_lbp |= ((PIM_Uint32)(cx - *(pt+src_w)) & 0x80000000) >> 24;
		data_lbp |= ((PIM_Uint32)(cx - *(pt+(src_w+2))) & 0x80000000) >> 28;
		data_lbp |= ((PIM_Uint32)(cx - *(pt+(src_w+src_w))) & 0x80000000) >> 25;
		data_lbp |= ((PIM_Uint32)(cx - *(pt+(src_w+src_w+1))) & 0x80000000) >> 26;
		data_lbp |= ((PIM_Uint32)(cx - *(pt+(src_w+src_w+2))) & 0x80000000) >> 27;

		pt++;
		*plbp++ = data_lbp;	
	}
	PIM_Memset(lbp_data, 0, src_w);
	PIM_Memset(lbp_data+src_w*(src_h-1), 0, src_w);
	plbp = lbp_data;
	for(i=0; i<src_h; i++) {
		*plbp = 0;
		*(plbp+src_w-1) = 0;
		plbp += src_w;
	}
}

void image2LBP8_2_FR(PIM_Uint8* src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Uint8* lbp_data)
{
	PIM_Int32 i, src_dw = src_w + src_w;
	PIM_Uint32 cx;
	PIM_Uint32 size = src_w * (src_h - 4);
	PIM_Uint32 data_lbp;		
	PIM_Uint8 *pt = src_data;
	PIM_Uint8 *plbp = lbp_data + (src_dw + 2);
	while(size--)
	{
		cx = *(pt + src_dw + 2);

		data_lbp = ((PIM_Uint32)(cx - *pt) & 0x80000000) >> 31;
		data_lbp |= ((PIM_Uint32)(cx - *(pt + 2)) & 0x80000000) >> 30;
		data_lbp |= ((PIM_Uint32)(cx - *(pt + 4)) & 0x80000000) >> 29;
		data_lbp |= ((PIM_Uint32)(cx - *(pt + src_dw)) & 0x80000000) >> 24;
		data_lbp |= ((PIM_Uint32)(cx - *(pt + (src_dw + 4))) & 0x80000000) >> 28;
		data_lbp |= ((PIM_Uint32)(cx - *(pt + (src_dw + src_dw))) & 0x80000000) >> 25;
		data_lbp |= ((PIM_Uint32)(cx - *(pt + (src_dw + src_dw + 2))) & 0x80000000) >> 26;
		data_lbp |= ((PIM_Uint32)(cx - *(pt + (src_dw + src_dw + 4))) & 0x80000000) >> 27;

		pt++;
		*plbp++ = data_lbp;	
	}
	PIM_Memset(lbp_data, 0, src_w + src_w);
	PIM_Memset(lbp_data + src_w * (src_h - 2), 0, src_w + src_w);
	plbp = lbp_data;
	for( i = 0; i < src_h; i++ )
	{
		*plbp = 0;
		*(plbp + 1) = 0;
		*(plbp + src_w - 1) = 0;
		*(plbp + src_w - 2) = 0;
		plbp += src_w;
	}
}

void amhe(PIM_Uint8 *image, PIM_Int32 w, PIM_Int32 h, PIM_Uint8 *amhe_data)
{
	PIM_Int32 i, size = w * h, hist_value;
	PIM_Uint8* pimage;
	PIM_Int32* pmap_table;
	PIM_Int32 map_table[256];
	PIM_Memset(map_table,0, sizeof(PIM_Int32)*256);
	PIM_Memcpy(amhe_data,image, size);

	pimage = amhe_data;
	for(i=0; i<size; i++) map_table[*pimage++]++;

	pmap_table = map_table;
	for(i=1; i<256; i++) {
		*(pmap_table +1) += *pmap_table;
		pmap_table++;
	}

	hist_value = 33423360/map_table[255]; //  2<<17 * 255 = 33423360
	pmap_table = map_table;
	for(i=0; i<256; i++) {
		*pmap_table = (*pmap_table*hist_value)>>17;
		pmap_table++ ;
	}
	pimage = amhe_data;
	for(i=0; i<size; i++) {
		*pimage= (PIM_Uint8)map_table[*pimage];
		pimage++ ;
	}
}

#ifndef __CODE_SIZE_OPTIMIZE__
void mean_filt(PIM_Uint8* src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Uint8* filt_data)
{
	PIM_Uint32 size = src_w*(src_h-2) - 2;
	PIM_Uint32 src_w2 = src_w+src_w;
	PIM_Uint8 *pSrc, *pDes;
	PIM_Memset(filt_data, 0, src_w*src_h);
	
	pSrc = src_data;
	pDes = filt_data + (src_w + 1);
	while(size--) {
		*pDes++	= (*pSrc + *(pSrc+1) + *(pSrc+2) + 
			*(pSrc+src_w) + *(pSrc+(src_w+1)) + *(pSrc+(src_w+2)) + 
			*(pSrc+src_w2) + *(pSrc+(src_w2+1)) + *(pSrc+(src_w2+2))) / 9;
		pSrc++;
	}
}
#endif /* __CODE_SIZE_OPTIMIZE__ */

void gaussian_filt(PIM_Uint8* src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Uint8* filt_data)
{
        PIM_Uint32 size = src_w*(src_h-2);
        PIM_Uint32 src_w2 = src_w+src_w;
        PIM_Uint8 *pSrc, *pDes;
        PIM_Int32 m1, m2, m3;
        PIM_Memset(filt_data,0, src_w*src_h);
 
        pSrc = src_data;
        pDes = filt_data+ (src_w + 1);
        m1 = *pSrc +((*(pSrc+src_w))<<1)+ *(pSrc+src_w2);
        m2 = *(pSrc+1)+ ((*(pSrc+(src_w+1)))<<1)+ *(pSrc+(src_w2+1));
        pSrc+=2;
        while(size--) {
               m3 = *pSrc +((*(pSrc+src_w))<<1)+ *(pSrc+src_w2);
               pSrc++;
               *pDes++ = (m1+(m2<<1)+m3)>> 4;
               m1 = m2;
               m2 = m3;               
        }
}



#ifndef __CODE_SIZE_OPTIMIZE__
void image2MCT9_1(PIM_Uint8* src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Uint16* mct_data)
{
	PIM_Uint32 m;
	PIM_Uint32 size = src_w*(src_h-2);
	PIM_Uint32 data_mct;		
	PIM_Uint8 *m1 = src_data;
	PIM_Uint8 *m2 = m1 + 1;
	PIM_Uint8 *m3 = m2 + 1;
	PIM_Uint8 *m8 = m1 + src_w;
	PIM_Uint8 *m9 = m8 + 1;
	PIM_Uint8 *m4 = m9 + 1;
	PIM_Uint8 *m7 = m8 + src_w;
	PIM_Uint8 *m6 = m7 + 1;
	PIM_Uint8 *m5 = m6 + 1;
	PIM_Uint16 *pmct = mct_data+(src_w+1);

	while(size--) {
		m = *m1 + *m2 + *m3 + *m4 + *m5 + *m6 + *m7 + *m8 + *m9;
		data_mct = ((PIM_Uint32)(m - (*m1 << 3) - *m1) & 0x80000000) >> 31;
		data_mct |= ((PIM_Uint32)(m - (*m2 << 3) - *m2) & 0x80000000) >> 30;
		data_mct |= ((PIM_Uint32)(m - (*m3 << 3) - *m3) & 0x80000000) >> 29;
		data_mct |= ((PIM_Uint32)(m - (*m8 << 3) - *m8) & 0x80000000) >> 24;
		data_mct |= ((PIM_Uint32)(m - (*m9 << 3) - *m9) & 0x80000000) >> 23;
		data_mct |= ((PIM_Uint32)(m - (*m4 << 3) - *m4) & 0x80000000) >> 28;
		data_mct |= ((PIM_Uint32)(m - (*m7 << 3) - *m7) & 0x80000000) >> 25;
		data_mct |= ((PIM_Uint32)(m - (*m6 << 3) - *m6) & 0x80000000) >> 26;
		data_mct |= ((PIM_Uint32)(m - (*m5 << 3) - *m5) & 0x80000000) >> 27;
		m1++;	m2++;	m3++;	
		m4++;	m5++;	m6++;	
		m7++;	m8++;	m9++;	
		*pmct++ = data_mct;	
	}
}
#endif /* __CODE_SIZE_OPTIMIZE__ */


//void image2MCT(PIM_Uint8 *src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Uint16 *mct)
void image2MCT(PIM_Bitmap *src, PIM_Uint16 *mct)
{
    PIM_Int32 src_w = src->width;
    PIM_Int32 src_h = src->height;
	PIM_Int32 x, y, w=src_w, h=src_h-1, src_size=src->stride*src_h;
	PIM_Int32 m1, m2, m3, m4, m5, m6, m7, m8, m9;
	PIM_Int32 mu1, mu2, mu3, mu4, mu5, mu6, mu7, mu8, mu9;
	PIM_Int32 data_mean;
	PIM_Uint16 data_mct;	
	PIM_Uint8 *pt1, *pt2, *pt3;
	PIM_Uint16 *pmct;

    PIM_Uint16 *pDst = mct;
    PIM_Uint8 *pSrc = src->imageData;
    PIM_Int32 src_stride = src->stride;

	PIM_Memset(mct, 0, sizeof(PIM_Uint16)*src_size);

#if 0
	for(y=1; y<h; y++) {
		m1 = *pt1++;
		m4 = *pt2++;
		m7 = *pt3++;
		m2 = *pt1++;
		m5 = *pt2++;
		m8 = *pt3++;		
		for(x=2; x<w; x++) {
			m3 = *pt1++;
			m6 = *pt2++;
			m9 = *pt3++;
			data_mean = ((m1+m2+m3+m4+m5+m6+m7+m8+m9));

			mu1 = data_mean - (( m1 << 3 ) + m1);
			mu2 = data_mean - (( m2 << 3 ) + m2);
			mu3 = data_mean - (( m3 << 3 ) + m3);
			mu4 = data_mean - (( m4 << 3 ) + m4);
			mu5 = data_mean - (( m5 << 3 ) + m5);
			mu6 = data_mean - (( m6 << 3 ) + m6);
			mu7 = data_mean - (( m7 << 3 ) + m7);
			mu8 = data_mean - (( m8 << 3 ) + m8);
			mu9 = data_mean - (( m9 << 3 ) + m9);

			data_mct = ((mu1>>31)&1) + (((mu2>>31)&1)<<1) + (((mu3>>31)&1)<<2) +
				(((mu4>>31)&1)<<3) + (((mu5>>31)&1)<<4) + (((mu6>>31)&1)<<5) +
				(((mu7>>31)&1)<<6) + (((mu8>>31)&1)<<7) + (((mu9>>31)&1)<<8);

			m1 = m2;
			m4 = m5;
			m7 = m8;
			m2 = m3;
			m5 = m6;
			m8 = m9;

			*pmct++ = data_mct;
		}
		pmct++;
		pmct++;
	}
#else
	for(y=1; y<h; y++) {
        pt1 = &pSrc[(y-1)*src_stride];
        pt2 = &pSrc[(y  )*src_stride];
        pt3 = &pSrc[(y+1)*src_stride];

        pmct = &pDst[y*src_stride + 1];

		m1 = *pt1++;
		m4 = *pt2++;
		m7 = *pt3++;
		m2 = *pt1++;
		m5 = *pt2++;
		m8 = *pt3++;

		for(x=2; x<w; x++) {
			m3 = *pt1++;
			m6 = *pt2++;
			m9 = *pt3++;
			data_mean = ((m1+m2+m3+m4+m5+m6+m7+m8+m9));

			mu1 = data_mean - (( m1 << 3 ) + m1);
			mu2 = data_mean - (( m2 << 3 ) + m2);
			mu3 = data_mean - (( m3 << 3 ) + m3);
			mu4 = data_mean - (( m4 << 3 ) + m4);
			mu5 = data_mean - (( m5 << 3 ) + m5);
			mu6 = data_mean - (( m6 << 3 ) + m6);
			mu7 = data_mean - (( m7 << 3 ) + m7);
			mu8 = data_mean - (( m8 << 3 ) + m8);
			mu9 = data_mean - (( m9 << 3 ) + m9);

			data_mct = ((mu1>>31)&1) + (((mu2>>31)&1)<<1) + (((mu3>>31)&1)<<2) +
				(((mu4>>31)&1)<<3) + (((mu5>>31)&1)<<4) + (((mu6>>31)&1)<<5) +
				(((mu7>>31)&1)<<6) + (((mu8>>31)&1)<<7) + (((mu9>>31)&1)<<8);

			m1 = m2;
			m4 = m5;
			m7 = m8;
			m2 = m3;
			m5 = m6;
			m8 = m9;

			*pmct++ = data_mct;
		}
	}
#endif
}


#ifndef __CODE_SIZE_OPTIMIZE__
void image2MCT_Final(PIM_Uint8* src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Uint16* lbp_data)
{
	PIM_Uint32 cx;
	PIM_Uint32 size = src_w*(src_h-2);
	PIM_Uint32 data_lbp;		
	PIM_Uint8 *pt = src_data;
	PIM_Uint16 *plbp = lbp_data+(src_w+1);

	while(size--) {
		cx = *pt + *(pt+1) + *(pt+2) + *(pt+src_w) + *(pt+src_w+1) + *(pt+src_w+2) + *(pt+(src_w+src_w)) + *(pt+(src_w+src_w+1)) + *(pt+(src_w+src_w+2));
		
		data_lbp = ((PIM_Uint32)(cx - *pt) & 0x80000000) >> 31;
		data_lbp |= ((PIM_Uint32)(cx - *(pt+1)) & 0x80000000) >> 30;
		data_lbp |= ((PIM_Uint32)(cx - *(pt+2)) & 0x80000000) >> 29;
		data_lbp |= ((PIM_Uint32)(cx - *(pt+src_w)) & 0x80000000) >> 24;
		data_lbp |= ((PIM_Uint32)(cx - *(pt+(src_w+1))) & 0x80000000) >> 23;
		data_lbp |= ((PIM_Uint32)(cx - *(pt+(src_w+2))) & 0x80000000) >> 28;
		data_lbp |= ((PIM_Uint32)(cx - *(pt+(src_w+src_w))) & 0x80000000) >> 25;
		data_lbp |= ((PIM_Uint32)(cx - *(pt+(src_w+src_w+1))) & 0x80000000) >> 26;
		data_lbp |= ((PIM_Uint32)(cx - *(pt+(src_w+src_w+2))) & 0x80000000) >> 27;
		pt++;
		*plbp++ = data_lbp;	
	}
}
#endif /* __CODE_SIZE_OPTIMIZE__ */

