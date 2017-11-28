#include "stdafx.h"

#include "ImageProcess.h"
#include <memory.h>
#include <stdlib.h>

#include <mmintrin.h>	//MMX
#include <xmmintrin.h>	//SSE
#include <emmintrin.h>	//SSE2
#include <tmmintrin.h>	//SSSE3
#include <smmintrin.h>	//SSE4.1
#include <immintrin.h>	//AVX

#define BIT_SHIFT		15

void Resize_BI(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data, int des_w, int des_h)
{
	/* < 10 */
	int udtx, udty;
	/* < 10 */
	int uda_11;
	/* < 10 */
	int uda_22;
	int i, j;
	int utx, uty;
	int locgray4 = 0;
	int ucgray1 = 0, ucgray2 = 0, ucgray3 = 0, ucgray4 = 0;
	unsigned int aa, ab, ac, ad;
	
	unsigned char *pucsrcp = 0;
	unsigned char *pucdstp = des_data;	

	/* Affine transformation */
	
	uda_11 = ( (src_w-1) << 10 ) / des_w;
	uda_22 = ( (src_h-1) << 10 ) / des_h;

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

			*( pucdstp + i ) = ( unsigned char ) ( ( aa + ab + ac + ad ) >> 20 );
		}
		
		pucdstp += des_w;
	}
}

void Resize_NN(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data, int des_w, int des_h)
{
	int i;
	unsigned int utx, uty=0;
	unsigned int uda_11 = ((src_w-1) << BIT_SHIFT) / des_w;
	unsigned int uda_22 = ((src_h-1) << BIT_SHIFT) / des_h;
	unsigned char *psrc_data = src_data;
	unsigned char *pdes_data = des_data;	

	while(des_h--) {
		utx = 0;
		i = des_w;
		while(i--) {
			utx += uda_11;						
			*pdes_data++ = *(psrc_data + (utx>>BIT_SHIFT));		
		}		
		uty += uda_22;		
		psrc_data = src_data + (uty>>BIT_SHIFT)*src_w;
	}
}

void image2MCT9_1(unsigned char* src_data, int src_w, int src_h, unsigned short* mct_data)
{
	int x, y, w=src_w, h=src_h-1;
	int m1, m2, m3, m4, m5, m6, m7, m8, m9, m;
	int data_lbp;	
	unsigned char *pt1, *pt2=src_data+src_w, *pt3;
	unsigned short *pmct = mct_data+src_w+1;
	memset(mct_data, 0, sizeof(unsigned short)*src_w*src_h);

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

			m = m1 + m2 + m3 + m4 + m5 + m6 + m7 + m8 + m9;

			data_lbp = (((m - (m1<<3) - m1)>>16)&1);
			data_lbp |= ((((m - (m2<<3) - m2)>>16)&1)<<1);
			data_lbp |= ((((m - (m3<<3) - m3)>>16)&1)<<2);
			data_lbp |= ((((m - (m6<<3) - m6)>>16)&1)<<3);
			data_lbp |= ((((m - (m9<<3) - m9)>>16)&1)<<4);
			data_lbp |= ((((m - (m8<<3) - m8)>>16)&1)<<5);
			data_lbp |= ((((m - (m7<<3) - m7)>>16)&1)<<6);
			data_lbp |= ((((m - (m4<<3) - m4)>>16)&1)<<7);
			data_lbp |= ((((m - (m5<<3) - m5)>>16)&1)<<8);

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

void image2MCT_FD(unsigned char* src_data, int src_w, int src_h, unsigned short* mct_data)
{
	unsigned int m;
	unsigned int size = src_w*(src_h-2)-2;
	unsigned int data_mct;		
	unsigned char *m1 = src_data + size, *m2 = m1 + 1, *m3 = m2 + 1;
	unsigned char *m4 = m1 + src_w, *m5 = m4 + 1, *m6 = m5 + 1;
	unsigned char *m7 = m4 + src_w, *m8 = m7 + 1, *m9 = m8 + 1;
	unsigned short *pmct = mct_data+size+src_w+1;

	while(size--) {
		m = *--m1 + *--m2 + *--m3 + *--m4 + *--m5 + *--m6 + *--m7 + *--m8 + *--m9;
		data_mct = ((unsigned int)(m - (*m1 << 3) - *m1) & 0x80000000) >> 31;
		data_mct |= ((unsigned int)(m - (*m2 << 3) - *m2) & 0x80000000) >> 30;
		data_mct |= ((unsigned int)(m - (*m3 << 3) - *m3) & 0x80000000) >> 29;
		data_mct |= ((unsigned int)(m - (*m4 << 3) - *m4) & 0x80000000) >> 28;
		data_mct |= ((unsigned int)(m - (*m5 << 3) - *m5) & 0x80000000) >> 27;
		data_mct |= ((unsigned int)(m - (*m6 << 3) - *m6) & 0x80000000) >> 26;
		data_mct |= ((unsigned int)(m - (*m7 << 3) - *m7) & 0x80000000) >> 25;
		data_mct |= ((unsigned int)(m - (*m8 << 3) - *m8) & 0x80000000) >> 24;
		data_mct |= ((unsigned int)(m - (*m9 << 3) - *m9) & 0x80000000) >> 23;
		*--pmct = data_mct;	
	}
}

int Region(unsigned char* src_data, int src_w, int src_h, unsigned char* des_data, SMyRect rcRegion)
{	
	int roi_x = rcRegion.left, roi_y = rcRegion.top;
	int roi_w = rcRegion.right - rcRegion.left;
	int roi_h = rcRegion.bottom - rcRegion.top;

	int i, src_index, region_index;
	if(roi_x+roi_w > src_w) return 0;
	if(roi_y+roi_h > src_h) return 0;
		
	src_index = roi_y*src_w + roi_x;
	region_index = 0;
	for(i = 0; i<roi_h; i++) {
		memcpy(des_data+region_index, src_data+src_index, roi_w);
		src_index += src_w;
		region_index += roi_w;
	}
	return 1;
}


void FlipX(unsigned char* src_data, int src_w, int src_h, unsigned char* des_data)
{
	int y, k, w;

	unsigned char* raw_data = (unsigned char*)malloc(src_w);

	
	unsigned char* praw;
	unsigned char* pdes = des_data;

	for(k=0, y=0; y<src_h; y++) {
		memcpy(raw_data, src_data+k, src_w);		
		praw = raw_data + src_w-1;
		w = src_w;
		while(w--) *pdes++ = *praw--;
		k += src_w;
	}

	free(raw_data);

}

void rgb2gray(unsigned char *rgb, int w, int h, unsigned char *gray)
{
	int i;
	int size = w*h;

	unsigned char *src = rgb;
	unsigned char *dst = gray;

	for (i = 0 ; i<size; i++)
	{
		*dst++ = (unsigned char)(((int)*src++ + (int)*src++ + (int)*src++)/3);
	}
}

void Rotate(unsigned char *src_data, int src_w, int src_h, int angle, unsigned char *des_data)
{
	int i;
	int utx=0, uty=0;
	int uda_x, uda_y;
	int des_w = src_w, des_h = src_h;
	unsigned char *pstart_data;
	unsigned char *psrc_data;
	unsigned char *pdes_data = des_data;

	if ( !angle ) {
		uda_x = (src_w << 10) / des_w;
		uda_y = (src_h << 10) / des_h;
		pstart_data = src_data;
		psrc_data = pstart_data;
		while(des_h--) {
			utx = 0;
			i = des_w;
			while(i--) {									
				*pdes_data++ = *(psrc_data + (utx>>10));		
				utx += uda_x;
			}		
			uty += uda_y;		
			psrc_data = pstart_data + (uty>>10)*src_w;
		}
	} else if(angle == 90) {
		uda_x = (src_w << 10) / des_h;
		uda_y = (src_h << 10) / des_w;
		pstart_data = src_data + (src_h * src_w);
		psrc_data = pstart_data;

		while(des_h--) {
			uty = 0;
			i = des_w;
			while(i--) {
				*pdes_data++ = *(psrc_data - (uty>>10)*src_w);		
				uty += uda_y;
			}		
			utx += uda_x;	
			psrc_data = pstart_data + (utx>>10);			
		}

	} else if(angle == -90){
		uda_x = (src_w << 10) / des_h;
		uda_y = (src_h << 10) / des_w;
		pstart_data = src_data + src_h;
		psrc_data = pstart_data;

		while(des_h--) {
			uty = 0;
			i = des_w;
			while(i--) {
				*pdes_data++ = *(psrc_data + (uty>>10)*src_w);		
				uty += uda_y;
			}		
			utx += uda_x;	
			psrc_data = pstart_data - (utx>>10);			
		}

	} else if(angle == 45){
		const int scx = src_w>>1;
		const int scy = src_h>>1;
		const int dcx = des_w>>1;
		const int dcy = des_h>>1;
		const unsigned char *psrc = src_data;
		unsigned char *pdst = des_data;
		const int ratio45_sft10 = 724;
		const int alpha_sft10 = (scx<<10) + ratio45_sft10 * (dcx + dcy);
		const int beta_sft10 =  (scy<<10) + ratio45_sft10 * (dcy - dcx);
		const int gamma = (beta_sft10 - (src_h<<10))/ratio45_sft10;
		const int etha  = (alpha_sft10 / ratio45_sft10);
		int temp;
		int interval;

		while(des_h--) 
		{
			i = des_w;
			while(i--) 
			{
				int X = (alpha_sft10 - ratio45_sft10 * (des_h + i))>>10;
				int Y = (beta_sft10  - ratio45_sft10 * (des_h - i))>>10;

				if( X >= 0 && X < src_w && Y >= 0 && Y < src_h){
					*pdst++ = psrc[Y*src_w+X];
				}
				else{
					if(Y>=src_h || X < 0){
						temp = (Y>=src_h)?(des_h - gamma):(etha - des_h);
						interval = i - temp;
						if(interval>1){
							while(interval--) *pdst++ = 0;
							i = temp;
							*pdst++ = 0;
						}
						else *pdst++ = 0;
					}
					else{
						while(i--) *pdst++ = 0;
						*pdst++ = 0;
						break;
					}
				}
			}
		}
	}
	else if(angle == -45){
		const int scx = src_w>>1;
		const int scy = src_h>>1;
		const int dcx = des_w>>1;
		const int dcy = des_h>>1;
		const unsigned char *psrc = src_data;
		unsigned char *pdst = des_data;
		const int ratio45_sft10 = 724;
		const int alpha_sft10 = (scx<<10) - ratio45_sft10 * (dcy - dcx);
		const int beta_sft10 =  (scy<<10) + ratio45_sft10 * (dcx + dcy);
		const int gamma = (alpha_sft10 / ratio45_sft10);
		const int etha  = (beta_sft10  / ratio45_sft10);
		int temp;
		int interval;

		while(des_h--) 
		{
			i = des_w;
			while(i--) 
			{
				int X = (alpha_sft10 - ratio45_sft10 * (i - des_h))>>10;
				int Y = (beta_sft10 -  ratio45_sft10 * (i + des_h))>>10;

				if( X >= 0 && X < src_w && Y >= 0 && Y < src_h){
					*pdst++ = psrc[Y*src_w+X];
				}
				else{
					if(Y < 0 || X < 0){
						temp = (X < 0) ? (gamma + des_h) :(etha - des_h);
						interval = i - temp;
						if(interval>1){
							while(interval--) *pdst++ = 0;
							i = temp;
							*pdst++ = 0;
						}
						else *pdst++ = 0;
					}
					else{
						while(i--) *pdst++ = 0;
						*pdst++ = 0;
						break;
					}
				}
			}
		}
	}
	else {		
		const int cosT = (int)(cos(-angle*0.0174532889)*1024);
		const int sinT = (int)(sin(-angle*0.0174532889)*1024);
		const int inv_ratio_sft10_x_cost = cosT<<10;
		const int inv_ratio_sft10_x_sint = sinT<<10;

		const int scx_sft20 = src_w<<19;
		const int scy_sft20 = src_h<<19;
		const int dcx = des_w>>1;
		const int dcy = des_h>>1;

		const unsigned char *psrc = src_data;
		unsigned char *pdst = des_data;

		int inv_ratio_sft10_x_sint_x_dy = -inv_ratio_sft10_x_sint*dcy;
		int inv_ratio_sft10_x_cost_x_dy = -inv_ratio_sft10_x_cost*dcy;

		//for( y = 0; y < des_h; y++ )
		while(des_h--) 
		{
			int inv_ratio_sft10_x_sint_x_dx = -inv_ratio_sft10_x_sint*dcx;
			int inv_ratio_sft10_x_cost_x_dx = -inv_ratio_sft10_x_cost*dcx;

			//for( x = 0; x < des_w; x++ )
			i = des_w;
			while(i--) 
			{
				int X = (scx_sft20 + inv_ratio_sft10_x_cost_x_dx - inv_ratio_sft10_x_sint_x_dy + 524288)>>20;
				int Y = (scy_sft20 + inv_ratio_sft10_x_sint_x_dx + inv_ratio_sft10_x_cost_x_dy + 524288)>>20;

				if( X >= 0 && X < src_w && Y >= 0 && Y < src_h )
					*pdst++ = psrc[src_w*Y+X];
				else
					*pdst++ = 0;

				inv_ratio_sft10_x_sint_x_dx += inv_ratio_sft10_x_sint;
				inv_ratio_sft10_x_cost_x_dx += inv_ratio_sft10_x_cost;
			}
			inv_ratio_sft10_x_sint_x_dy += inv_ratio_sft10_x_sint;
			inv_ratio_sft10_x_cost_x_dy += inv_ratio_sft10_x_cost;
		}
	}
}

void InvRotate(SMyRect src, int cpx, int cpy, int angle, SMyRect* dest)
{
	struct SMyPoint2 src_cp, des_cp;
	int x, y, wh;
	double radian = angle*0.0174532889;

	src_cp.x = ( (int)(src.left + src.right) ) >> 1;
	src_cp.y = ( (int)(src.top + src.bottom) ) >> 1;    

	x = src_cp.x - cpx;
	y = cpy - src_cp.y; 	
	des_cp.x = (int)( x * cos(radian) - y * sin(radian));
	des_cp.y = (int)( x * sin(radian) + y * cos(radian));
	des_cp.x += cpx;
	des_cp.y = cpy - des_cp.y;

	wh = src.right - src.left;
	dest->left = des_cp.x - (wh>>1);
	dest->right = dest->left + wh;
	dest->top = des_cp.y - (wh>>1);
	dest->bottom = dest->top + wh; 
}

void illuminationCompensation(unsigned char* src_data, int src_w, int src_h)
{
	int i;
	int m = 0;
	double ratio;
	unsigned char EAI = 140;
	unsigned char *pSrc = src_data;
	for (i = src_w * src_h; i--; )
	{
		m += *pSrc++;
	}

	m /= (src_w * src_h);
	ratio = (double)EAI / m;

	pSrc = src_data;
	for (i = src_w * src_h; i--; )
	{
		m = int(*pSrc * ratio + 0.5);
		m = m > EAI ? EAI : m;
		*pSrc++ = m;
	}
}

void flitMean(unsigned char* src_data, int src_w, int src_h)
{
	// 1 1 1
	// 1 1 1
	// 1 1 1
	int i, m;
	int size = src_w*(src_h-2)-2;
	unsigned char* des_data = (unsigned char*)malloc(src_w * src_h);
	unsigned char *m1 = src_data + size, *m2 = m1 + 1, *m3 = m2 + 1;
	unsigned char *m4 = m1 + src_w, *m5 = m4 + 1, *m6 = m5 + 1;
	unsigned char *m7 = m4 + src_w, *m8 = m7 + 1, *m9 = m8 + 1;
	unsigned char *pSrc;
	unsigned char *pDes = des_data+size+src_w+1;

	while(size--) {
		//m = *--m2 + *--m4 + 4 * *--m5 + *--m6 + *--m8;
		//*--pDes = m >> 3;
		m = *--m1 + *--m2 + *--m3 + *--m4 + *--m5 + *--m6 + *--m7 + *--m8 + *--m9;
		*--pDes = m / 9;
	}

	memcpy(des_data, src_data, sizeof(unsigned char)*(src_w+1));
	memcpy(des_data+(src_h-1)*src_w - 1, src_data+(src_h-1)*src_w - 1, sizeof(unsigned char)*(src_w+1));

	pSrc = src_data + src_w + src_w - 1;
	pDes = des_data + src_w + src_w - 1;

	for (i=0; i<src_h - 3; i++) {
		memcpy(pDes, pSrc, sizeof(unsigned char)*2);
		pSrc += src_w;
		pDes += src_w;
	}
	memcpy(src_data, des_data, src_w * src_h);
	free(des_data);
}

void Image2LBP8_1(unsigned char* src_data, int src_w, int src_h, unsigned char* lbp_data)
{
	unsigned int cx;
	unsigned int size = src_w*(src_h - 2) - 2;
	unsigned int data_lbp;
	unsigned char *pt = src_data;
	unsigned char *plbp = lbp_data + (src_w + 1);

	memset(lbp_data, 0, sizeof(unsigned char)*src_w*src_h);


	#if 1
//#ifdef USE_SSE
	__m128i xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, sum;

	int i = 0, j = 0;
	int total_width = src_w - 3;
	int total_height = src_h - 3;
	unsigned char *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8, *p9;

	while (i <= total_width)
	{
		//pmct = mctImage + nSrcW * i;
		plbp = lbp_data + src_w + i + 1;

		p1 = src_data + i;
		p2 = src_data + i + 1;
		p3 = src_data + i + 2;
		p4 = src_data + src_w + i;
		p5 = src_data + src_w + i + 1;
		p6 = src_data + src_w + i + 2;
		p7 = src_data + src_w * 2 + i;
		p8 = src_data + src_w * 2 + i + 1;
		p9 = src_data + src_w * 2 + i + 2;

		for (j = 0; j <= total_height; j++)
		{
			__m128i zero = _mm_setzero_si128();
			__m128i ones = _mm_set1_epi8(255);

			xmm1 = _mm_loadu_si128((__m128i *)p1);
			xmm2 = _mm_loadu_si128((__m128i *)p2);
			xmm3 = _mm_loadu_si128((__m128i *)p3);

			xmm4 = _mm_loadu_si128((__m128i *)(p4));
			xmm5 = _mm_loadu_si128((__m128i *)(p5));
			xmm6 = _mm_loadu_si128((__m128i *)(p6));

			xmm7 = _mm_loadu_si128((__m128i *)p7);
			xmm8 = _mm_loadu_si128((__m128i *)(p8));
			xmm9 = _mm_loadu_si128((__m128i *)(p9));


			{
				__m128i temp, mask, code;
				code = _mm_setzero_si128();
				mask = _mm_set1_epi8(1);

				temp = _mm_min_epu8(xmm1, xmm5);
				temp = _mm_cmpeq_epi8(temp, xmm1);
				temp = _mm_xor_si128(temp, ones);
				code = _mm_and_si128(temp, mask);

				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_min_epu8(xmm2, xmm5);
				temp = _mm_cmpeq_epi8(temp, xmm2);
				temp = _mm_xor_si128(temp, ones);
				code = _mm_or_si128(code, _mm_and_si128(temp, mask));

				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_min_epu8(xmm3, xmm5);
				temp = _mm_cmpeq_epi8(temp, xmm3);
				temp = _mm_xor_si128(temp, ones);
				code = _mm_or_si128(code, _mm_and_si128(temp, mask));

				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_min_epu8(xmm6, xmm5);
				temp = _mm_cmpeq_epi8(temp, xmm6);
				temp = _mm_xor_si128(temp, ones);
				code = _mm_or_si128(code, _mm_and_si128(temp, mask));

				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_min_epu8(xmm9, xmm5);
				temp = _mm_cmpeq_epi8(temp, xmm9);
				temp = _mm_xor_si128(temp, ones);
				code = _mm_or_si128(code, _mm_and_si128(temp, mask));

				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_min_epu8(xmm8, xmm5);
				temp = _mm_cmpeq_epi8(temp, xmm8);
				temp = _mm_xor_si128(temp, ones);
				code = _mm_or_si128(code, _mm_and_si128(temp, mask));

				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_min_epu8(xmm7, xmm5);
				temp = _mm_cmpeq_epi8(temp, xmm7);
				temp = _mm_xor_si128(temp, ones);
				code = _mm_or_si128(code, _mm_and_si128(temp, mask));

				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_min_epu8(xmm4, xmm5);
				temp = _mm_cmpeq_epi8(temp, xmm4);
				temp = _mm_xor_si128(temp, ones);
				code = _mm_or_si128(code, _mm_and_si128(temp, mask));

				_mm_storeu_si128((__m128i *)plbp, code);
			}

			plbp += src_w;
			p1 += src_w;
			p2 += src_w;
			p3 += src_w;
			p4 += src_w;
			p5 += src_w;
			p6 += src_w;
			p7 += src_w;
			p8 += src_w;
			p9 += src_w;
		}

		if (i + 8 > total_width) break;
		//i = i+8+8 >= total_width+1-2 ? total_width-9+1 : i+8;
		i = i + 8 + 8 > total_width ? total_width - 7 : i + 8;


	}

	memset(lbp_data, 0, sizeof(unsigned char)*(src_w + 1));
	memset(lbp_data + (src_h - 1)*src_w - 1, 0, sizeof(unsigned char)*(src_w + 1));

	plbp = lbp_data + src_w + src_w - 1;
	for (size = 0; size<src_h - 3; size++) {
		memset(plbp, 0, sizeof(unsigned char) * 2);
		plbp += src_w;
	}

#else

	while (size--) {
		cx = *(pt + src_w + 1);

		data_lbp = ((unsigned int)(cx - *pt) & 0x80000000) >> 31;
		//data_lbp |= ((unsigned int)(cx - *(pt+1)) & 0x80000000) >> 30;
		//data_lbp |= ((unsigned int)(cx - *(pt+2)) & 0x80000000) >> 29;
		//data_lbp |= ((unsigned int)(cx - *(pt+src_w)) & 0x80000000) >> 24;
		//data_lbp |= ((unsigned int)(cx - *(pt+(src_w+2))) & 0x80000000) >> 28;
		//data_lbp |= ((unsigned int)(cx - *(pt+(src_w+src_w))) & 0x80000000) >> 25;
		//data_lbp |= ((unsigned int)(cx - *(pt+(src_w+src_w+1))) & 0x80000000) >> 26;
		//data_lbp |= ((unsigned int)(cx - *(pt+(src_w+src_w+2))) & 0x80000000) >> 27;

		pt++;
		*plbp++ = data_lbp;
	}

	memset(lbp_data, 0, sizeof(unsigned char)*(src_w + 1));
	memset(lbp_data + (src_h - 1)*src_w - 1, 0, sizeof(unsigned char)*(src_w + 1));

	plbp = lbp_data + src_w + src_w - 1;
	for (size = 0; size<src_h - 3; size++) {
		memset(plbp, 0, sizeof(unsigned char) * 2);
		plbp += src_w;
	}
#endif
}



// Face Normalization
// 
// Output : 50*40 pixel 중 Mask를 제외한 pixel에 대한 intensity(0~255)를 row-wise로 반환한다.
void face_normalization_50_40(unsigned char* src_data, unsigned char* des_data, int src_w, int src_h, struct SMyPoint ptLEye, struct SMyPoint ptREye)
{
	//int c = 1;
	int des_x, des_y;
	int des_w=40, des_h=50;
	int des_x2=11, des_y2=15;
	int des_x3=29;

	int src_size = src_w*src_h;
	int x2, y2, x3, y3;	
	int x1, y1;
	double dx1, dy1, dy2, dx2;
	double src_x, src_y;
	double a11, a12, a21, a22;
	int l, k, src, src_l, src_k, src_lk, src_w_step, des_w_step = 0;
	double a, b;

	memset(des_data, 0, des_w*des_h);
	x2 = ptLEye.x, y2 = ptLEye.y;
	x3 = ptREye.x, y3 = ptREye.y;
	dx1 = (double)(x3-x2)*des_x2/(double)(des_x3-des_x2);
	dy1 = dx1*(y3-y2)/(double)(x3-x2);
	if(dy1 < 0) dy1 = -dy1;
	dy2 = dx1*des_y2/(double)des_x2;
	dx2 = dy1*dy2/dx1;
	if(y2 > y3)	{
		x1 = (int)(x2 - dx1 - dx2 + 0.5);
		y1 = (int)(y2 + dy1 - dy2 + 0.5);
	}
	else {
		x1 = (int)(x2 - dx1 + dx2 + 0.5);
		y1 = (int)(y2 - dy1 - dy2 + 0.5);
	}

	a11 = 0.0556*(x3-x2), a12 = -0.0667*x1 + 0.1074*x2 -0.0407*x3;
	a21 = 0.0556*(y3-y2), a22 = -0.0667*y1 + 0.1074*y2 -0.0407*y3;

	unsigned char *pdes = des_data;
	for(des_y=0; des_y<des_h; des_y++) {
		for(des_x=0; des_x<des_w; des_x++) {
			src_x = a11*des_x + a12*des_y + x1;
			src_y = a21*des_x + a22*des_y + y1;

			if (src_x < 0 || src_x >= src_w - 1 || src_y < 0 || src_y >= src_h - 1)
			{
				*pdes++ = 0;
			}
			else
			{
				l = (int)(src_x);
				k = (int)(src_y);
				a = src_x - l;
				b = src_y - k;
				src_w_step = k*src_w;
				src = src_data[src_w_step + l];
				src_l = src_data[src_w_step + l + 1];
				src_k = src_data[src_w_step + src_w + l];
				src_lk = src_data[src_w_step + src_w + l + 1];

				*pdes++ = (unsigned char)(src + a*(src_l - src) + b*(src_k - src) +
					a*b*(src + src_lk - src_l - src_k));
			}
		}
		des_w_step += des_w;
	}
}

void face_normalization_20_20(unsigned char* src_data, unsigned char* des_data, int src_w, int src_h, struct SMyPoint ptLEye, struct SMyPoint ptREye)
{
	//int c = 1;
	int des_x, des_y;
	int des_w = 20, des_h = 20;
	int des_x2 = 5, des_y2 = 6;
	int des_x3 = 14;

	int src_size = src_w*src_h;
	int des_size = des_w*des_h;

	double x1, y1, x2, y2, x3, y3;

	x2 = ptLEye.x; y2 = ptLEye.y;
	x3 = ptREye.x; y3 = ptREye.y;

	double dx1, dx2, dy1, dy2;
	double src_x, src_y;
	double a11, a12, a21, a22;
	int l, k, src, src_l, src_k, src_lk, src_w_step, des_w_step = 0;
	double a, b;

	memset(des_data, 0, des_size);


	dx1 = (x3 - x2)*(des_x2) / (des_x3 - des_x2);
	dy1 = dx1*(y3 - y2) / (x3 - x2);

	if (dy1 < 0) dy1 = -dy1;
	dy2 = dx1*des_y2 / des_x2;
	dx2 = dy1*dy2 / dx1;

	if (y2 > y3) {
		x1 = (x2 + (-dx1 - dx2 + 0.5));
		y1 = (y2 + (dy1 - dy2 + 0.5));
	}
	else {
		x1 = (x2 + (-dx1 + dx2 + 0.5));
		y1 = (y2 + (-dy1 - dy2 + 0.5));
	}

	a11 = (1.0 / (des_x3 - des_x2))*(x3 - x2);
	a12 = (x2 - x1 - des_x2*a11) / des_y2;
	a21 = (1.0 / (des_x3 - des_x2))*(y3 - y2);
	a22 = (y2 - y1 - des_x2*a21) / des_y2;

	unsigned char *pdes = des_data;
	for (des_y = 0; des_y<des_h; des_y++) {
		for (des_x = 0; des_x<des_w; des_x++) {
			src_x = a11*des_x + a12*des_y + x1;
			src_y = a21*des_x + a22*des_y + y1;

			if (src_x < 0 || src_x >= src_w - 1 || src_y < 0 || src_y >= src_h - 1)
			{
				*pdes++ = 0;
			}
			else
			{
				l = (int)(src_x);
				k = (int)(src_y);
				a = src_x - l;
				b = src_y - k;
				src_w_step = k*src_w;
				src = src_data[src_w_step + l];
				src_l = src_data[src_w_step + l + 1];
				src_k = src_data[src_w_step + src_w + l];
				src_lk = src_data[src_w_step + src_w + l + 1];

				*pdes++ = (unsigned char)(src + a*(src_l - src) + b*(src_k - src) +
					a*b*(src + src_lk - src_l - src_k));
			}
		}
	}
}

bool face_normalization_48_40(const unsigned char* src_data, const int src_w, const int src_h, const SMyPoint ptSrcLEye, const SMyPoint ptSrcREye, unsigned char* des_data)
{ 
	int des_x, des_y;
	int des_w=40, des_h=48;
	double des_x2=9, des_y2=14;
	double des_x3=30;

	int src_size = src_w*src_h;
	int des_size = des_w*des_h;

	double x1, y1, x2, y2, x3, y3;

	x2 = ptSrcLEye.x; y2 = ptSrcLEye.y;
	x3 = ptSrcREye.x; y3 = ptSrcREye.y;

	double dx1, dx2, dy1, dy2;
	int src_x, src_y;
	double a11, a12, a21, a22;
	memset(des_data, 0, des_size);


	dx1 = (x3-x2)*(des_x2)/(des_x3-des_x2);
	dy1 = dx1*(y3-y2)/(x3-x2);

	if(dy1 < 0) dy1 = -dy1;
	dy2 = dx1*des_y2/des_x2;
	dx2 = dy1*dy2/dx1;

	if(y2 > y3) {
		x1 = (x2 + (-dx1 - dx2 + 0.5));
		y1 = (y2 + (dy1 - dy2 + 0.5));
	}
	else {
		x1 = (x2 + (-dx1 + dx2 + 0.5));
		y1 = (y2 + (-dy1 - dy2 + 0.5));
	}

	a11 = (1.0/(des_x3-des_x2))*(x3-x2);
	a12 = (x2-x1-des_x2*a11)/des_y2;
	a21 = (1.0/(des_x3-des_x2))*(y3-y2);
	a22 = (y2-y1-des_x2*a21)/des_y2;
	unsigned char* pdes = des_data;
	for(des_y=0; des_y<des_h; des_y++) {    
		for(des_x=0; des_x<des_w; des_x++) {
			src_x = int ((a11*des_x + a12*des_y) + x1);
			src_y = int ((a21*des_x + a22*des_y) + y1);
			if(src_x < 0) { src_x = 0; return false; }
			if(src_x >= src_w) { src_x = src_w-1; return false; }
			if(src_y < 0) { src_y = 0; return false; }
			if(src_y >= src_h) { src_y = src_h-1; return false; }

			*pdes++ = src_data[src_y*src_w+src_x];
		}
	}
	return true;
}

bool face_normalization_52_44(const unsigned char* src_data, const int src_w, const int src_h, const struct SMyPoint ptSrcLEye, const struct SMyPoint ptSrcREye, unsigned char* des_data)
{ 
	int des_x, des_y;
	int des_w=44, des_h=52;
	double des_x2=12, des_y2=16;
	double des_x3=31;

	int src_size = src_w*src_h;
	int des_size = des_w*des_h;

	double x1, y1, x2, y2, x3, y3;

	x2 = ptSrcLEye.x; y2 = ptSrcLEye.y;
	x3 = ptSrcREye.x; y3 = ptSrcREye.y;

	double dx1, dx2, dy1, dy2;
	int src_x, src_y;
	double a11, a12, a21, a22;
	memset(des_data, 0, des_size);


	dx1 = (x3-x2)*(des_x2)/(des_x3-des_x2);
	dy1 = dx1*(y3-y2)/(x3-x2);

	if(dy1 < 0) dy1 = -dy1;
	dy2 = dx1*des_y2/des_x2;
	dx2 = dy1*dy2/dx1;

	if(y2 > y3) {
		x1 = (x2 + (-dx1 - dx2 + 0.5));
		y1 = (y2 + (dy1 - dy2 + 0.5));
	}
	else {
		x1 = (x2 + (-dx1 + dx2 + 0.5));
		y1 = (y2 + (-dy1 - dy2 + 0.5));
	}

	a11 = (1.0/(des_x3-des_x2))*(x3-x2);
	a12 = (x2-x1-des_x2*a11)/des_y2;
	a21 = (1.0/(des_x3-des_x2))*(y3-y2);
	a22 = (y2-y1-des_x2*a21)/des_y2;
	unsigned char* pdes = des_data;
	for(des_y=0; des_y<des_h; des_y++) {    
		for(des_x=0; des_x<des_w; des_x++) {
			src_x = int ((a11*des_x + a12*des_y) + x1);
			src_y = int ((a21*des_x + a22*des_y) + y1);
			if(src_x < 0) { src_x = 0; }
			if(src_x >= src_w) { src_x = src_w-1; }
			if(src_y < 0) { src_y = 0; return false; }
			if(src_y >= src_h) { src_y = src_h-1; }

			*pdes++ = src_data[src_y*src_w+src_x];
		}
	}
	return true;
}

bool face_normalization_52_44_bilinear(const unsigned char* src_data, const int src_w, const int src_h, const struct SMyPoint ptSrcLEye, const struct SMyPoint ptSrcREye, unsigned char* des_data)
{ 
	int des_x, des_y;
	int des_w=44, des_h=52;
	double des_x2=12, des_y2=16;
	double des_x3=31;

	int src_size = src_w*src_h;
	int des_size = des_w*des_h;

	double x1, y1, x2, y2, x3, y3;

	x2 = ptSrcLEye.x; y2 = ptSrcLEye.y;
	x3 = ptSrcREye.x; y3 = ptSrcREye.y;

	double dx1, dx2, dy1, dy2;
	double src_x, src_y;
	double a11, a12, a21, a22;
	int l, k, src, src_l, src_k, src_lk, src_w_step, des_w_step = 0;
	double a, b;

	memset(des_data, 0, des_size);


	dx1 = (x3-x2)*(des_x2)/(des_x3-des_x2);
	dy1 = dx1*(y3-y2)/(x3-x2);

	if(dy1 < 0) dy1 = -dy1;
	dy2 = dx1*des_y2/des_x2;
	dx2 = dy1*dy2/dx1;

	if(y2 > y3) {
		x1 = (x2 + (-dx1 - dx2 + 0.5));
		y1 = (y2 + (dy1 - dy2 + 0.5));
	}
	else {
		x1 = (x2 + (-dx1 + dx2 + 0.5));
		y1 = (y2 + (-dy1 - dy2 + 0.5));
	}

	a11 = (1.0/(des_x3-des_x2))*(x3-x2);
	a12 = (x2-x1-des_x2*a11)/des_y2;
	a21 = (1.0/(des_x3-des_x2))*(y3-y2);
	a22 = (y2-y1-des_x2*a21)/des_y2;
	unsigned char* pdes = des_data;
	for(des_y=0; des_y<des_h; des_y++) {    
		for(des_x=0; des_x<des_w; des_x++) {
			src_x = a11*des_x + a12*des_y + x1;
			src_y = a21*des_x + a22*des_y + y1;

			if(src_x < 0) src_x = 0;
			if(src_x >= src_w) src_x = src_w-1;
			if(src_y < 0) src_y = 0;
			if(src_y >= src_h) src_y = src_h-1;

			l = (int)(src_x);
			k = (int)(src_y);
			a = src_x - l;
			b = src_y - k;
			src_w_step = k*src_w;
			src = src_data[src_w_step+l];
			src_l = src_data[src_w_step+l+1];
			src_k = src_data[src_w_step+src_w+l];
			src_lk = src_data[src_w_step+src_w+l+1];

			*pdes++ = (unsigned char)(src + a*(src_l-src) + b*(src_k-src) +
				a*b*(src+src_lk-src_l-src_k));
		}
	}
	return true;
}

bool face_normalization_52_44_2(unsigned char* src_data, int src_w, int src_h, struct SMyPoint ptSrcLEye, struct SMyPoint ptSrcREye, unsigned char* des_data)
{ 
	int des_x, des_y;
	int des_w=44, des_h=52;
	double des_x2=13, des_y2=16;
	double des_x3=30;

	int src_size = src_w*src_h;
	int des_size = des_w*des_h;

	double x1, y1, x2, y2, x3, y3;

	x2 = ptSrcLEye.x; y2 = ptSrcLEye.y;
	x3 = ptSrcREye.x; y3 = ptSrcREye.y;

	double dx1, dx2, dy1, dy2;
	int src_x, src_y;
	double a11, a12, a21, a22;
	memset(des_data, 0, des_size);


	dx1 = (x3-x2)*(des_x2)/(des_x3-des_x2);
	dy1 = dx1*(y3-y2)/(x3-x2);

	if(dy1 < 0) dy1 = -dy1;
	dy2 = dx1*des_y2/des_x2;
	dx2 = dy1*dy2/dx1;

	if(y2 > y3) {
		x1 = (x2 + (-dx1 - dx2 + 0.5));
		y1 = (y2 + (dy1 - dy2 + 0.5));
	}
	else {
		x1 = (x2 + (-dx1 + dx2 + 0.5));
		y1 = (y2 + (-dy1 - dy2 + 0.5));
	}

	a11 = (1.0/(des_x3-des_x2))*(x3-x2);
	a12 = (x2-x1-des_x2*a11)/des_y2;
	a21 = (1.0/(des_x3-des_x2))*(y3-y2);
	a22 = (y2-y1-des_x2*a21)/des_y2;
	unsigned char* pdes = des_data;
	for(des_y=0; des_y<des_h; des_y++) {    
		for(des_x=0; des_x<des_w; des_x++) {
			src_x = int ((a11*des_x + a12*des_y) + x1);
			src_y = int ((a21*des_x + a22*des_y) + y1);
			if(src_x < 0) { src_x = 0; }
			if(src_x >= src_w) { src_x = src_w-1;}
			if(src_y < 0) { src_y = 0; }
			if(src_y >= src_h) { src_y = src_h-1; }

			*pdes++ = src_data[src_y*src_w+src_x];
		}
	}
	return true;
}

bool face_normalization_52_44_2_bilinear(unsigned char* src_data, int src_w, int src_h, struct SMyPoint ptSrcLEye, struct SMyPoint ptSrcREye, unsigned char* des_data)
{ 
	int des_x, des_y;
	int des_w=44, des_h=52;
	double des_x2=13, des_y2=16;
	double des_x3=30;

	int src_size = src_w*src_h;
	int des_size = des_w*des_h;

	double x1, y1, x2, y2, x3, y3;

	x2 = ptSrcLEye.x; y2 = ptSrcLEye.y;
	x3 = ptSrcREye.x; y3 = ptSrcREye.y;

	double dx1, dx2, dy1, dy2;
	double src_x, src_y;
	double a11, a12, a21, a22;
	int l, k, src, src_l, src_k, src_lk, src_w_step, des_w_step = 0;
	double a, b;

	memset(des_data, 0, des_size);


	dx1 = (x3-x2)*(des_x2)/(des_x3-des_x2);
	dy1 = dx1*(y3-y2)/(x3-x2);

	if(dy1 < 0) dy1 = -dy1;
	dy2 = dx1*des_y2/des_x2;
	dx2 = dy1*dy2/dx1;

	if(y2 > y3) {
		x1 = (x2 + (-dx1 - dx2 + 0.5));
		y1 = (y2 + (dy1 - dy2 + 0.5));
	}
	else {
		x1 = (x2 + (-dx1 + dx2 + 0.5));
		y1 = (y2 + (-dy1 - dy2 + 0.5));
	}

	a11 = (1.0/(des_x3-des_x2))*(x3-x2);
	a12 = (x2-x1-des_x2*a11)/des_y2;
	a21 = (1.0/(des_x3-des_x2))*(y3-y2);
	a22 = (y2-y1-des_x2*a21)/des_y2;
	unsigned char* pdes = des_data;
	for(des_y=0; des_y<des_h; des_y++) {    
		for(des_x=0; des_x<des_w; des_x++) {
			src_x = a11*des_x + a12*des_y + x1;
			src_y = a21*des_x + a22*des_y + y1;

			if(src_x < 0 || src_x >= src_w-1 || src_y < 0 || src_y >= src_h-1)
			{
				*pdes++ = 0;
			}
			else
			{
				l = (int)(src_x);
				k = (int)(src_y);
				a = src_x - l;
				b = src_y - k;
				src_w_step = k*src_w;
				src = src_data[src_w_step+l];
				src_l = src_data[src_w_step+l+1];
				src_k = src_data[src_w_step+src_w+l];
				src_lk = src_data[src_w_step+src_w+l+1];

				*pdes++ = (unsigned char)(src + a*(src_l-src) + b*(src_k-src) +
					a*b*(src+src_lk-src_l-src_k));
			}
		}
	}
	return true;
}

void integral_image(unsigned char *src, int width, int height, int *dst)
{
	int i;
	int *upper_value;
	int row_sum;
	int *dstp = dst;
	unsigned char *src2;
	int size;

	// first row
	row_sum = 0;
	src2 = src;
	size = width;
	while(size--)
	{
		row_sum += *src2++;
		*dstp++ = row_sum;
	}

	for (i = 1 ; i<height; i++)
	{
		row_sum = 0;
		src2 = src + i*width;

		upper_value = dst + (i-1)*width;
		size = width;
		while(size--)
		{
			row_sum += *src2++;
			*dstp++ = row_sum + *upper_value++;
		}
	}
}

void square_integral_image(unsigned char *src, int width, int height, int *dst)
{
	int i;
	int *upper_value;
	int row_sum;
	int *dstp = dst;
	unsigned char *src2;
	int size;

	// first row
	row_sum = 0;
	src2 = src;
	size = width;
	while(size--)
	{
		row_sum += *src2 * *src2;
		*dstp++ = row_sum;
		src2++;
	}

	for (i = 1 ; i<height; i++)
	{
		row_sum = 0;
		src2 = src + i*width;

		upper_value = dst + (i-1)*width;
		size = width;
		while(size--)
		{
			row_sum += *src2 * *src2;
			*dstp++ = row_sum + *upper_value++;
			src2++;
		}
	}
}

int Region_int(int* src_data, int src_w, int src_h, int* des_data, SMyRect rcRegion)
{	
	int roi_x = rcRegion.left, roi_y = rcRegion.top;
	int roi_w = rcRegion.right - rcRegion.left;
	int roi_h = rcRegion.bottom - rcRegion.top;

	int i, src_index, region_index;
	if(roi_x+roi_w > src_w) return 0;
	if(roi_y+roi_h > src_h) return 0;

	src_index = roi_y*src_w + roi_x;
	region_index = 0;
	for(i = 0; i<roi_h; i++) {
		memcpy(des_data+region_index, src_data+src_index, sizeof(int)*roi_w);
		src_index += src_w;
		region_index += roi_w;
	}
	return 1;
}

int Region_double(double* src_data, int src_w, int src_h, double* des_data, SMyRect rcRegion)
{	
	int roi_x = rcRegion.left, roi_y = rcRegion.top;
	int roi_w = rcRegion.right - rcRegion.left;
	int roi_h = rcRegion.bottom - rcRegion.top;

	int i, src_index, region_index;
	if(roi_x+roi_w > src_w) return 0;
	if(roi_y+roi_h > src_h) return 0;

	src_index = roi_y*src_w + roi_x;
	region_index = 0;
	for(i = 0; i<roi_h; i++) {
		memcpy(des_data+region_index, src_data+src_index, sizeof(double)*roi_w);
		src_index += src_w;
		region_index += roi_w;
	}
	return 1;
}

int Region_float(float* src_data, int src_w, int src_h, float* des_data, SMyRect rcRegion)
{	
	int roi_x = rcRegion.left, roi_y = rcRegion.top;
	int roi_w = rcRegion.right - rcRegion.left;
	int roi_h = rcRegion.bottom - rcRegion.top;

	int i, src_index, region_index;
	if(roi_x+roi_w > src_w) return 0;
	if(roi_y+roi_h > src_h) return 0;

	src_index = roi_y*src_w + roi_x;
	region_index = 0;
	for(i = 0; i<roi_h; i++) {
		memcpy(des_data+region_index, src_data+src_index, sizeof(float)*roi_w);
		src_index += src_w;
		region_index += roi_w;
	}
	return 1;
}