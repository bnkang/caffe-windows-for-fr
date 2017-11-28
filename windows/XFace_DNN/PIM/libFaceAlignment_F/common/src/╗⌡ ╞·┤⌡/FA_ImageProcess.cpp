//#include "StdAfx.h"
#include "../../../exp/PIM_Define_op.h"
#include "FA_ImageProcess.h"
#include <stdlib.h>
#include <math.h>
#include <memory.h>

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

			*( pucdstp + i ) = ( unsigned char ) ( ( aa + ab + ac + ad ) >> 20 );
		}
		
		pucdstp += des_w;
	}
}

void Resize_NN(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data, int des_w, int des_h)
{
	int i;
	unsigned int utx, uty=0;
	unsigned int uda_11 = (src_w << BIT_SHIFT) / des_w;
	unsigned int uda_22 = (src_h << BIT_SHIFT) / des_h;
	unsigned char *psrc_data = src_data;
	unsigned char *pdes_data = des_data;	

	while(des_h--) {
		utx = 0;
		i = des_w;
		while(i--) {
			*pdes_data++ = *(psrc_data + (utx>>BIT_SHIFT));		
			utx += uda_11;
		}		
		uty += uda_22;		
		psrc_data = src_data + (uty>>BIT_SHIFT)*src_w;
	}
}



void Image2LBP8_1(unsigned char* src_data, int src_w, int src_h, unsigned char* lbp_data)
{
	unsigned int cx;
	unsigned int size = src_w*(src_h-2)-2;
	unsigned int data_lbp;		
	unsigned char *pt = src_data;
	unsigned char *plbp = lbp_data+(src_w+1);
	
	while(size--) {
		cx = *(pt+src_w+1);
		
		data_lbp = ((unsigned int)(cx - *pt) & 0x80000000) >> 31;
		data_lbp |= ((unsigned int)(cx - *(pt+1)) & 0x80000000) >> 30;
		data_lbp |= ((unsigned int)(cx - *(pt+2)) & 0x80000000) >> 29;
		data_lbp |= ((unsigned int)(cx - *(pt+src_w)) & 0x80000000) >> 24;
		data_lbp |= ((unsigned int)(cx - *(pt+(src_w+2))) & 0x80000000) >> 28;
		data_lbp |= ((unsigned int)(cx - *(pt+(src_w+src_w))) & 0x80000000) >> 25;
		data_lbp |= ((unsigned int)(cx - *(pt+(src_w+src_w+1))) & 0x80000000) >> 26;
		data_lbp |= ((unsigned int)(cx - *(pt+(src_w+src_w+2))) & 0x80000000) >> 27;
		
		pt++;
		*plbp++ = data_lbp;	
	}
}

void image2MCT9_1(unsigned char* src_data, int src_w, int src_h, unsigned short* mct_data)
{
/*	unsigned int m;
	unsigned int size = src_w*(src_h-2);
	unsigned int data_mct;		
	unsigned char *m1 = src_data, *m2 = m1 + 1, *m3 = m2 + 1;
	unsigned char *m8 = m1 + src_w, *m9 = m8 + 1, *m4 = m9 + 1;
	unsigned char *m7 = m8 + src_w, *m6 = m7 + 1, *m5 = m6 + 1;
	unsigned short *pmct = mct_data+(src_w+1);
	
	while(size--) {
		m = *m1 + *m2 + *m3 + *m4 + *m5 + *m6 + *m7 + *m8 + *m9;
		data_mct = ((unsigned int)(m - (*m1 << 3) - *m1++) & 0x80000000) >> 31;
		data_mct |= ((unsigned int)(m - (*m2 << 3) - *m2++) & 0x80000000) >> 30;
		data_mct |= ((unsigned int)(m - (*m3 << 3) - *m3++) & 0x80000000) >> 29;
		data_mct |= ((unsigned int)(m - (*m8 << 3) - *m8++) & 0x80000000) >> 24;
		data_mct |= ((unsigned int)(m - (*m9 << 3) - *m9++) & 0x80000000) >> 23;
		data_mct |= ((unsigned int)(m - (*m4 << 3) - *m4++) & 0x80000000) >> 28;
		data_mct |= ((unsigned int)(m - (*m7 << 3) - *m7++) & 0x80000000) >> 25;
		data_mct |= ((unsigned int)(m - (*m6 << 3) - *m6++) & 0x80000000) >> 26;
		data_mct |= ((unsigned int)(m - (*m5 << 3) - *m5++) & 0x80000000) >> 27;
		*pmct++ = data_mct;	
	}*/
	unsigned int m;
	unsigned int size = src_w*(src_h-2)-2;
	unsigned int data_mct;		
	unsigned char *m1 = src_data + size, *m2 = m1 + 1, *m3 = m2 + 1;
	unsigned char *m8 = m1 + src_w, *m9 = m8 + 1, *m4 = m9 + 1;
	unsigned char *m7 = m8 + src_w, *m6 = m7 + 1, *m5 = m6 + 1;
	unsigned short *pmct = mct_data+size+src_w+1;

	while(size--) {
		m = *--m1 + *--m2 + *--m3 + *--m4 + *--m5 + *--m6 + *--m7 + *--m8 + *--m9;
		data_mct = ((unsigned int)(m - (*m1 << 3) - *m1) & 0x80000000) >> 31;
		data_mct |= ((unsigned int)(m - (*m2 << 3) - *m2) & 0x80000000) >> 30;
		data_mct |= ((unsigned int)(m - (*m3 << 3) - *m3) & 0x80000000) >> 29;
		data_mct |= ((unsigned int)(m - (*m8 << 3) - *m8) & 0x80000000) >> 24;
		data_mct |= ((unsigned int)(m - (*m9 << 3) - *m9) & 0x80000000) >> 23;
		data_mct |= ((unsigned int)(m - (*m4 << 3) - *m4) & 0x80000000) >> 28;
		data_mct |= ((unsigned int)(m - (*m7 << 3) - *m7) & 0x80000000) >> 25;
		data_mct |= ((unsigned int)(m - (*m6 << 3) - *m6) & 0x80000000) >> 26;
		data_mct |= ((unsigned int)(m - (*m5 << 3) - *m5) & 0x80000000) >> 27;
		*--pmct = data_mct;	
	}
}

int Region(unsigned char* src_data, int src_w, int src_h, unsigned char* des_data, struct SMyRect rcRegion)
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

	if( dy1 < 0 ) dy1 = -dy1;
	dy2 = dx1*des_y2 / des_x2;
	dx2 = dy1*dy2 / dx1;

	if( y2 > y3 ) {
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
	for( des_y = 0; des_y < des_h; des_y++ ) {
		for( des_x = 0; des_x < des_w; des_x++ ) {
			src_x = a11*des_x + a12*des_y + x1;
			src_y = a21*des_x + a22*des_y + y1;

			if( src_x < 0 || src_x >= src_w - 1 || src_y < 0 || src_y >= src_h - 1 )
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

