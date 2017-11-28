#include "stdafx.h"

#include "sdic_ImageProcess.h"
#include <stdlib.h>
#include <math.h>
#ifdef WIN32
#include <memory.h>
#endif
#ifndef WIN32
#include "Allocator.h"
#endif

#define BIT_SHIFT		15

void sdic_Resize_BI(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data, int des_w, int des_h)
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

void sdic_Resize_NN(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data, int des_w, int des_h)
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
			utx += uda_11;						
			*pdes_data++ = *(psrc_data + (utx>>BIT_SHIFT));		
		}		
		uty += uda_22;		
		psrc_data = src_data + (uty>>BIT_SHIFT)*src_w;
	}
}

void sdic_Resize_Image2LBP(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data, int des_w, int des_h)
{
	int i;
	unsigned int utx, uty=0;
	unsigned int uda_11 = (src_w << BIT_SHIFT) / des_w;
	unsigned int uda_22 = (src_h << BIT_SHIFT) / des_h;
	unsigned char* resizeData = (unsigned char*)malloc(des_w*des_h);
	unsigned char *psrc_data = src_data;
	unsigned char *pdes_data = resizeData;	

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

	unsigned int cx;
	unsigned int size = des_w*(des_h-2)-2;
	unsigned int data_lbp;		
	unsigned char *pt = resizeData;
	unsigned char *plbp = des_data+(des_w+1);
	while(size--) {
		cx = *(pt+des_w+1);

		data_lbp = ((unsigned int)(cx - *pt) & 0x80000000) >> 31;
		data_lbp |= ((unsigned int)(cx - *(pt+1)) & 0x80000000) >> 30;
		data_lbp |= ((unsigned int)(cx - *(pt+2)) & 0x80000000) >> 29;
		data_lbp |= ((unsigned int)(cx - *(pt+des_w)) & 0x80000000) >> 24;
		data_lbp |= ((unsigned int)(cx - *(pt+(des_w+2))) & 0x80000000) >> 28;
		data_lbp |= ((unsigned int)(cx - *(pt+(des_w+des_w))) & 0x80000000) >> 25;
		data_lbp |= ((unsigned int)(cx - *(pt+(des_w+des_w+1))) & 0x80000000) >> 26;
		data_lbp |= ((unsigned int)(cx - *(pt+(des_w+des_w+2))) & 0x80000000) >> 27;

		pt++;
		*plbp++ = data_lbp;	
	}
	memset(des_data, 0, des_w);
	memset(des_data+des_w*(des_h-1), 0, des_w);
	plbp = des_data;
	for(i=0; i<des_h; i++) {
		*plbp = 0;
		*(plbp+des_w-1) = 0;
		plbp += des_w;
	}
}


void sdic_image2LBP8_1(unsigned char* src_data, int src_w, int src_h, unsigned char* lbp_data)
{
	/*unsigned int cx;
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
	}*/

	/*unsigned int cx;
	unsigned int size = src_w*(src_h-2)-2;
	unsigned int data_lbp;
	unsigned char *m1 = src_data, *m2 = m1 + 1, *m3 = m2 + 1;
	unsigned char *m4 = m1 + src_w, *m5 = m4 + 1, *m6 = m5 + 1;
	unsigned char *m7 = m4 + src_w, *m8 = m7 + 1, *m9 = m8 + 1;
	unsigned char *plbp = lbp_data+(src_w+1);
	
	while(size--) {
		cx = *m5++;
		data_lbp = ((unsigned int)(cx - *m1++) & 0x80000000) >> 31;
		data_lbp |= ((unsigned int)(cx - *m2++) & 0x80000000) >> 30;
		data_lbp |= ((unsigned int)(cx - *m3++) & 0x80000000) >> 29;
		data_lbp |= ((unsigned int)(cx - *m4++) & 0x80000000) >> 24;
		data_lbp |= ((unsigned int)(cx - *m6++) & 0x80000000) >> 28;
		data_lbp |= ((unsigned int)(cx - *m7++) & 0x80000000) >> 25;
		data_lbp |= ((unsigned int)(cx - *m8++) & 0x80000000) >> 26;
		data_lbp |= ((unsigned int)(cx - *m9++) & 0x80000000) >> 27;
		*plbp++ = data_lbp;	
	}*/
	unsigned int cx;
	unsigned int size = src_w*(src_h-2)-2;
	unsigned int data_lbp;
	unsigned char *m1 = src_data + size, *m2 = m1 + 1, *m3 = m2 + 1;
	unsigned char *m4 = m1 + src_w, *m5 = m4 + 1, *m6 = m5 + 1;
	unsigned char *m7 = m4 + src_w, *m8 = m7 + 1, *m9 = m8 + 1;
	unsigned char *plbp = lbp_data+size+src_w+1;

	while(size--) {
		cx = *--m5;
		data_lbp = ((unsigned int)(cx - *--m1) & 0x80000000) >> 31;
		data_lbp |= ((unsigned int)(cx - *--m2) & 0x80000000) >> 30;
		data_lbp |= ((unsigned int)(cx - *--m3) & 0x80000000) >> 29;
		data_lbp |= ((unsigned int)(cx - *--m4) & 0x80000000) >> 24;
		data_lbp |= ((unsigned int)(cx - *--m6) & 0x80000000) >> 28;
		data_lbp |= ((unsigned int)(cx - *--m7) & 0x80000000) >> 25;
		data_lbp |= ((unsigned int)(cx - *--m8) & 0x80000000) >> 26;
		data_lbp |= ((unsigned int)(cx - *--m9) & 0x80000000) >> 27;
		*--plbp = data_lbp;	
	}


}


void image2LGP8_1(unsigned char* src_data, int src_w, int src_h, unsigned char* lbp_data)
{
	int x, y, w=src_w, h=src_h-1;
	int m1, m2, m3, m4, m5, m6, m7, m8, m9;
	int data_lbp;	
	unsigned char *pt1, *pt2=src_data+src_w, *pt3;
	unsigned char *pmct = lbp_data+src_w+1;
	memset(lbp_data, 0, src_w*src_h);
	int t = 0;

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

			t = (abs(m5-m1) + abs(m5-m2) + abs(m5-m3) + abs(m5-m6) + abs(m5-m9) + abs(m5-m8) + abs(m5-m7) + abs(m5-m4))/8;
			
			data_lbp = (((t - abs(m5-m1))>>8)&1);
			data_lbp |= ((((t - abs(m5-m2))>>8)&1)<<1);
			data_lbp |= ((((t - abs(m5-m3))>>8)&1)<<2);
			data_lbp |= ((((t - abs(m5-m6))>>8)&1)<<3);
			data_lbp |= ((((t - abs(m5-m9))>>8)&1)<<4);
			data_lbp |= ((((t - abs(m5-m8))>>8)&1)<<5);
			data_lbp |= ((((t - abs(m5-m7))>>8)&1)<<6);
			data_lbp |= ((((t - abs(m5-m4))>>8)&1)<<7);

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

void Image2LGP_Rect(unsigned char *src_data, int src_w, int src_h, int x1, int y1, int x2, int y2, unsigned char *lgp_data)
{
	int x, y, w=src_w, h=src_h-1;
	int m1, m2, m3, m4, m5, m6, m7, m8, m9;
	int data_lbp;	
	unsigned char *pt1, *pt2=src_data+(y1+1)*src_w+x1, *pt3;
	unsigned char *pmct = lgp_data+(x2-x1)+1;
	int t = 0;

	memset(lgp_data, 0, (x2-x1)*(y2-y1));

	pt1 = pt2-src_w;
	pt3 = pt2+src_w;

	int patch_w = x2-x1;
	int patch_h = y2-y1;
	int delta = src_w-patch_w;
	for(y=2; y<patch_h; y++) {
		m1 = *pt1++;
		m4 = *pt2++;
		m7 = *pt3++;
		m2 = *pt1++;
		m5 = *pt2++;
		m8 = *pt3++;

		for(x=2; x<patch_w; x++) {
			m3 = *pt1++;
			m6 = *pt2++;
			m9 = *pt3++;

			t = (abs(m5-m1) + abs(m5-m2) + abs(m5-m3) + abs(m5-m6) + abs(m5-m9) + abs(m5-m8) + abs(m5-m7) + abs(m5-m4))/8;

			data_lbp = (((t - abs(m5-m1))>>8)&1);
			data_lbp |= ((((t - abs(m5-m2))>>8)&1)<<1);
			data_lbp |= ((((t - abs(m5-m3))>>8)&1)<<2);
			data_lbp |= ((((t - abs(m5-m6))>>8)&1)<<3);
			data_lbp |= ((((t - abs(m5-m9))>>8)&1)<<4);
			data_lbp |= ((((t - abs(m5-m8))>>8)&1)<<5);
			data_lbp |= ((((t - abs(m5-m7))>>8)&1)<<6);
			data_lbp |= ((((t - abs(m5-m4))>>8)&1)<<7);

			m1 = m2;
			m4 = m5;
			m7 = m8;
			m2 = m3;
			m5 = m6;
			m8 = m9;

			*pmct++ = data_lbp;
		}
		pt1 += delta;
		pt2 += delta;
		pt3 += delta;
		pmct++;
		pmct++;
	}
}

void sdic_image2LBP8_1_FR(unsigned char* src_data, int src_w, int src_h, unsigned char* lbp_data)
{
	int i;
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
	memset(lbp_data, 0, src_w);
	memset(lbp_data+src_w*(src_h-1), 0, src_w);
	plbp = lbp_data;
	for(i=0; i<src_h; i++) {
		*plbp = 0;
		*(plbp+src_w-1) = 0;
		plbp += src_w;
	}
}

void sdic_image2LBP8_2_FR(unsigned char* src_data, int src_w, int src_h, unsigned char* lbp_data)
{
	int i, src_dw = src_w+src_w;
	unsigned int cx;
	unsigned int size = src_w*(src_h-4)-4;
	unsigned int data_lbp;		
	unsigned char *pt = src_data;
	unsigned char *plbp = lbp_data+(src_dw+2);
	while(size--) {
		cx = *(pt+src_dw+2);

		data_lbp = ((unsigned int)(cx - *pt) & 0x80000000) >> 31;
		data_lbp |= ((unsigned int)(cx - *(pt+2)) & 0x80000000) >> 30;
		data_lbp |= ((unsigned int)(cx - *(pt+4)) & 0x80000000) >> 29;
		data_lbp |= ((unsigned int)(cx - *(pt+src_dw)) & 0x80000000) >> 24;
		data_lbp |= ((unsigned int)(cx - *(pt+(src_dw+4))) & 0x80000000) >> 28;
		data_lbp |= ((unsigned int)(cx - *(pt+(src_dw+src_dw))) & 0x80000000) >> 25;
		data_lbp |= ((unsigned int)(cx - *(pt+(src_dw+src_dw+2))) & 0x80000000) >> 26;
		data_lbp |= ((unsigned int)(cx - *(pt+(src_dw+src_dw+4))) & 0x80000000) >> 27;

		pt++;
		*plbp++ = data_lbp;	
	}
	memset(lbp_data, 0, src_w+src_w);
	memset(lbp_data+src_w*(src_h-2), 0, src_w+src_w);
	plbp = lbp_data;
	for(i=0; i<src_h; i++) {
		*plbp = 0;
		*(plbp+1) = 0;
		*(plbp+src_w-1) = 0;
		*(plbp+src_w-2) = 0;
		plbp += src_w;
	}
}


void sdic_image2MCT9_1(unsigned char* src_data, int src_w, int src_h, unsigned short* mct_data)
{
	unsigned int m;
	unsigned int size = src_w*(src_h-2)-2;
	unsigned int data_mct;		
	unsigned char *m1 = src_data + size, *m2 = m1 + 1, *m3 = m2 + 1;
	unsigned char *m8 = m1 + src_w, *m9 = m8 + 1, *m4 = m9 + 1;
	unsigned char *m7 = m8 + src_w, *m6 = m7 + 1, *m5 = m6 + 1;
	unsigned short *pmct = mct_data+size+src_w+1;

	while(size--) {
		--m1; --m2; --m3; --m4; --m5; --m6; --m7; --m8; --m9;
		m = *m1 + *m2 + *m3 + *m4 + *m5 + *m6 + *m7 + *m8 + *m9;
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

int sdic_Region(unsigned char* src_data, int src_w, int src_h, unsigned char* des_data, SMyRect rcRegion)
{	
	if(rcRegion.left < 0) return 0;
	if(rcRegion.top < 0) return 0;
	if(rcRegion.right > src_w) return 0;
	if(rcRegion.bottom > src_h) return 0;

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

void sdic_FlipX(unsigned char* src_data, int src_w, int src_h, unsigned char* des_data)
{
	int y, k, w;
#ifndef WIN32	
	unsigned char* raw_data = (unsigned char*)PoolAlloc(NULL, src_w, POOL_MEMORY_CACHED);	
#else
	unsigned char* raw_data = (unsigned char*)malloc(src_w);
#endif
	
	unsigned char* praw;
	unsigned char* pdes = des_data;

	for(k=0, y=0; y<src_h; y++) {
		memcpy(raw_data, src_data+k, src_w);		
		praw = raw_data + src_w-1;
		w = src_w;
		while(w--) *pdes++ = *praw--;
		k += src_w;
	}
#ifndef WIN32
	PoolFree(NULL, raw_data);	
#else
	free(raw_data);
#endif	
}

void sdic_FlipY(unsigned char* src_data, int src_w, int src_h, unsigned char* des_data)
{
	int y;
	des_data = des_data + src_w*(src_h-1);
	for(y = src_h; y; --y) {
		memcpy(des_data, src_data, src_w);
		src_data += src_w;
		des_data -= src_w;
	}
}

// Histogram equalization for small image ( lower than 128 x 128 )
void sdic_he_small(unsigned char *src, int w, int h)
{
	int i, size = w * h;
	unsigned char *pSrc = src;
#ifndef WIN32	
	unsigned short *map_table = (unsigned short*)PoolAlloc(NULL, sizeof(unsigned short) * 256, POOL_MEMORY_CACHED);	
#else
	unsigned short *map_table = (unsigned short*)malloc( sizeof(unsigned short) * 256 );
#endif
	
	unsigned short *pMap1 = map_table+1, *pMap2 = map_table;
	memset( map_table, 0, sizeof(unsigned short) * 256 );
	for(i=size; i--; ) map_table[*pSrc++]++;
	for(i=255; i--; ) *pMap1++ += *pMap2++;
	
	unsigned int hist_value = 66846720 / map_table[255]; // 255 * 2^18
	pMap1 = map_table;
	for(i=256; i--; ) {
		*pMap1 = ( *pMap1 * hist_value ) >> 18;
		pMap1++;
	}
	pSrc = src;
	for(i=size; i--; ) {
		*pSrc = (unsigned char)map_table[*pSrc];
		pSrc++;
	}
#ifndef WIN32
	PoolFree(NULL, map_table);	
#else
	free(map_table);
#endif		
}

int sdic_he(unsigned char *image, int w, int h, unsigned char *amhe_data)
{
	int i, size = w * h, hist_value;
	unsigned char* pimage;
	int* pmap_table;
	int map_table[256];
	memset(map_table, 0, sizeof(int)*256);
	memcpy(amhe_data, image, size);

	pimage = amhe_data;
	for(i=0; i<size; i++) map_table[*pimage++]++;
	pmap_table = map_table;
	for(i=1; i<256; i++) {
		*(pmap_table +1) += *pmap_table;
		pmap_table++;
	}
	hist_value = 33423360/map_table[255];	
	pmap_table = map_table;
	for(i=0; i<256; i++) {
		*pmap_table = (*pmap_table*hist_value)>>17;
		pmap_table++;
	}
	pimage = amhe_data;
	for(i=0; i<size; i++) {
		*pimage = (unsigned char)map_table[*pimage];
		pimage++;
	}
	return 1;
}

void sdic_Rotate(unsigned char *src_data, int src_w, int src_h, int angle, unsigned char *des_data)
{
	int i;
	int utx=0, uty=0;
	int uda_x, uda_y;
	int des_w = src_w, des_h = src_h;
	unsigned char *pstart_data;
	unsigned char *psrc_data;
	unsigned char *pdes_data = des_data;



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
		return;

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

void sdic_InvRotate(SMyRect src, int cpx, int cpy, int angle, SMyRect* dest)
{
	SMyPoint src_cp, des_cp;
	int x, y, wh;
	double radian = angle*0.0174532889;
	double cos_v=cos(radian), sin_v=sin(radian);

	src_cp.x = ( src.left + src.right ) >> 1;
	src_cp.y = ( src.top + src.bottom ) >> 1;    

	x = src_cp.x - cpx;
	y = cpy - src_cp.y; 	
	des_cp.x = (int)( x * cos_v - y * sin_v);
	des_cp.y = (int)( x * sin_v + y * cos_v);
	des_cp.x += cpx;
	des_cp.y = cpy - des_cp.y;

	wh = src.right - src.left;
	dest->left = des_cp.x - (wh>>1);
	dest->right = dest->left + wh;
	dest->top = des_cp.y - (wh>>1);
	dest->bottom = dest->top + wh; 
}

void sdic_Rotate_25(unsigned char *src_data, int src_w, int src_h, int angle, unsigned char *des_data)
{
	int i;
	int utx=0, uty=0;
	int des_w = src_w, des_h = src_h;
	unsigned char *pdes_data = des_data;

	//int cos_v=928, sin_v=433;
	//if(angle < 0) sin_v=-433;

	int cosT = 928;//int(cos(-angle*0.0174532889)*1024);
	int sinT = 433;//int(sin(-angle*0.0174532889)*1024);
	if(angle > 0) sinT=-433;

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

void sdic_InvRotate_25(SMyRect src, int cpx, int cpy, int angle, SMyRect* dest)
{
	SMyPoint src_cp, des_cp;
	int x, y, wh;
	int cos_v=928, sin_v=433;
	if(angle < 0) sin_v=-433;

	src_cp.x = ( src.left + src.right ) >> 1;
	src_cp.y = ( src.top + src.bottom ) >> 1;    

	x = src_cp.x - cpx;
	y = cpy - src_cp.y; 	
	des_cp.x = (int)((x * cos_v - y * sin_v)>>10);
	des_cp.y = (int)((x * sin_v + y * cos_v)>>10);
	des_cp.x += cpx;
	des_cp.y = cpy - des_cp.y;

	wh = src.right - src.left;
	dest->left = des_cp.x - (wh>>1);
	dest->right = dest->left + wh;
	dest->top = des_cp.y - (wh>>1);
	dest->bottom = dest->top + wh; 
}

void sdic_Rotation_L(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data)
{
	int i, j;
	unsigned char *pSrc, *pDes, *pDesInit;
	pSrc = src_data;
	pDesInit = des_data+src_h*(src_w-1);

	for (i=src_h; i--;) {
		pDes = pDesInit++;
		for (j=src_w; j--; ){
			*pDes = *pSrc++;
			pDes -= src_h;
		}
	}
}

void sdic_Rotation_R(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data)
{
	int i, j;
	unsigned char *pSrc, *pDes, *pDesInit;
	pSrc = src_data;
	pDesInit = des_data+src_h-1;

	for (i=src_h; i--;) {
		pDes = pDesInit--;
		for (j=src_w; j--; ){
			*pDes = *pSrc++;
			pDes += src_h;
		}
	}
}

void sdic_Rotation_180(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data)
{
	int i, j;
	unsigned char *pSrc, *pDes, *pDesInit;
	pSrc = src_data;
	pDesInit = des_data+src_h-1;

	for (i=src_h; i--;) {
		pDes = pDesInit--;
		for (j=src_w; j--; ){
			*pDes = *pSrc++;
			pDes += src_h;
		}
	}
}

void sdic_Position_Yflip(int nWidth, int nHeight, SMyRect src_rect, SMyRect *des_rect)
{
	des_rect->top = nHeight - src_rect.bottom;
	des_rect->bottom = nHeight - src_rect.top;
	des_rect->left = src_rect.left;
	des_rect->right = src_rect.right;
}

void sdic_Position_L2Org(int nWidth, int nHeight, SMyRect src_rect, SMyRect *des_rect)
{
	des_rect->top = src_rect.left;
	des_rect->bottom = src_rect.right;
	des_rect->left = nWidth-src_rect.bottom;
	des_rect->right = nWidth-src_rect.top;
}

void sdic_Position_R2Org(int nWidth, int nHeight, SMyRect src_rect, SMyRect *des_rect)
{
	des_rect->top = nHeight-src_rect.right;
	des_rect->bottom = nHeight-src_rect.left;
	des_rect->left = src_rect.top;
	des_rect->right = src_rect.bottom;
}

void sdic_Position_L2Org2(int nWidth, int nHeight, SMyRect src_rect, SMyRect *des_rect)
{
	des_rect->top = src_rect.left;
	des_rect->bottom = src_rect.right;
	des_rect->left = nHeight-src_rect.bottom;
	des_rect->right = nHeight-src_rect.top;
}

void sdic_Position_R2Org2(int nWidth, int nHeight, SMyRect src_rect, SMyRect *des_rect)
{
	des_rect->top = nWidth-src_rect.right;
	des_rect->bottom = nWidth-src_rect.left;
	des_rect->left = src_rect.top;
	des_rect->right = src_rect.bottom;
}

void sdic_Position_Org2L25(SMyPoint src_pt, int cx, int cy, SMyPoint *des_pt)
{
	int ptLx_org = src_pt.x - cx;
	int ptLy_org = -src_pt.y + cy;
	
	des_pt->x = (long)(0.906307787036650 * ptLx_org - 0.422618261740699 * ptLy_org + cx);
	des_pt->y = (long)(-(0.906307787036650 * ptLy_org + 0.422618261740699 * ptLx_org) + cy);
}

void sdic_Position_Org2R25(SMyPoint src_pt, int cx, int cy, SMyPoint *des_pt)
{
	int ptLx_org = src_pt.x - cx;
	int ptLy_org = -src_pt.y + cy;

	des_pt->x = (long)(0.906307787036650 * ptLx_org + 0.422618261740699 * ptLy_org + cx);
	des_pt->y = (long)(-(0.906307787036650 * ptLy_org - 0.422618261740699 * ptLx_org) + cy);
}

void SDIC_SetEyeToRot(long nWidth, long nHeight, FD_INFO *pFD_Info)
{
	int i;
	SMyRect rcLEye_org, rcREye_org, rcLEye, rcREye;
	SMyPoint ptLEye_org, ptREye_org, ptLEye, ptREye;
	for (i = 0; i < pFD_Info->cnt; i++)
	{
		if (pFD_Info->rip_angle[i] == 0 || pFD_Info->eyeL[i].enable == 0 || pFD_Info->eyeR[i].enable == 0)
		{
			continue;
		}
		ptLEye_org.x = pFD_Info->eyeL[i].left + (pFD_Info->eyeL[i].width>>1);
		ptLEye_org.y = pFD_Info->eyeL[i].top + (pFD_Info->eyeL[i].height>>1);
		ptREye_org.x = pFD_Info->eyeR[i].left + (pFD_Info->eyeR[i].width>>1);
		ptREye_org.y = pFD_Info->eyeR[i].top + (pFD_Info->eyeR[i].height>>1);

		rcLEye_org.left = pFD_Info->eyeL[i].left;
		rcLEye_org.right = pFD_Info->eyeL[i].left + pFD_Info->eyeL[i].width;
		rcLEye_org.top = pFD_Info->eyeL[i].top;
		rcLEye_org.bottom = pFD_Info->eyeL[i].top + pFD_Info->eyeL[i].height;

		rcREye_org.left = pFD_Info->eyeR[i].left;
		rcREye_org.right = pFD_Info->eyeR[i].left + pFD_Info->eyeR[i].width;
		rcREye_org.top = pFD_Info->eyeR[i].top;
		rcREye_org.bottom = pFD_Info->eyeR[i].top + pFD_Info->eyeR[i].height;

		if (pFD_Info->rip_angle[i] == -25)
		{
			sdic_Position_Org2L25(ptLEye_org, nWidth>>1, nHeight>>1, &ptLEye);
			sdic_Position_Org2L25(ptREye_org, nWidth>>1, nHeight>>1, &ptREye);
		}
		else if (pFD_Info->rip_angle[i] == 25)
		{
			sdic_Position_Org2R25(ptLEye_org, nWidth>>1, nHeight>>1, &ptLEye);
			sdic_Position_Org2R25(ptREye_org, nWidth>>1, nHeight>>1, &ptREye);
		}
		else if (pFD_Info->rip_angle[i] == -90)
		{
			sdic_Position_R2Org2(nWidth, nHeight, rcLEye_org, &rcLEye);
			sdic_Position_R2Org2(nWidth, nHeight, rcREye_org, &rcREye);
		}
		else if (pFD_Info->rip_angle[i] == 90)
		{
			sdic_Position_L2Org2(nWidth, nHeight, rcLEye_org, &rcLEye);
			sdic_Position_L2Org2(nWidth, nHeight, rcREye_org, &rcREye);
		}
		//yjm 2011.08.24
		else if (pFD_Info->rip_angle[i] == 180)
		{
			sdic_Position_Yflip(nWidth, nHeight, rcLEye_org, &rcLEye);
			sdic_Position_Yflip(nWidth, nHeight, rcREye_org, &rcREye);
		}

		if (pFD_Info->rip_angle[i] == -25 || pFD_Info->rip_angle[i] == 25)
		{
			pFD_Info->eyeL[i].left = ptLEye.x - (pFD_Info->eyeL[i].width>>1);
			pFD_Info->eyeL[i].top = ptLEye.y - (pFD_Info->eyeL[i].height>>1);
			pFD_Info->eyeR[i].left = ptREye.x - (pFD_Info->eyeR[i].width>>1);
			pFD_Info->eyeR[i].top = ptREye.y - (pFD_Info->eyeR[i].height>>1);
		}
		else
		{
			pFD_Info->eyeL[i].left = rcLEye.left;
			pFD_Info->eyeL[i].top = rcLEye.top;
			pFD_Info->eyeL[i].width = rcLEye.right - rcLEye.left;
			pFD_Info->eyeL[i].height = rcLEye.bottom - rcLEye.top;

			pFD_Info->eyeR[i].left = rcREye.left;
			pFD_Info->eyeR[i].top = rcREye.top;
			pFD_Info->eyeR[i].width = rcREye.right - rcREye.left;
			pFD_Info->eyeR[i].height = rcREye.bottom - rcREye.top;
		}
	}
}

void sdic_Position_Org2Angle( SMyPoint src_pt, int cx, int cy,  SMyPoint *des_pt, int angle)
{
	int ptLx_org = src_pt.x - cx;
	int ptLy_org = -src_pt.y + cy;

	double radian = angle*0.0174532889;
	double cos_v=cos(radian), sin_v=sin(radian);

	des_pt->x = (long)(cos_v * ptLx_org + sin_v * ptLy_org + cx);
	des_pt->y = (long)(-(cos_v * ptLy_org - sin_v * ptLx_org) + cy);
}

void sdic_Position_Org2L( SMyPoint src_pt, int cx, int cy, int nAngle, SMyPoint *des_pt)
{
	if(nAngle < 0) nAngle = -nAngle;
	int ptLx_org = src_pt.x - cx;
	int ptLy_org = -src_pt.y + cy;
	double radian = nAngle*0.0174532889;
	double cos_v=cos(radian), sin_v=sin(radian);

	des_pt->x = (long)(cos_v * ptLx_org - sin_v * ptLy_org + cx);
	des_pt->y = (long)(-(cos_v * ptLy_org + sin_v * ptLx_org) + cy);
}

void sdic_Position_Org2R( SMyPoint src_pt, int cx, int cy, int nAngle, SMyPoint *des_pt)
{
	if(nAngle < 0) nAngle = -nAngle;
	int ptLx_org = src_pt.x - cx;
	int ptLy_org = -src_pt.y + cy;
	double radian = nAngle*0.0174532889;
	double cos_v=cos(radian), sin_v=sin(radian);

	des_pt->x = (long)(cos_v * ptLx_org + sin_v * ptLy_org + cx);
	des_pt->y = (long)(-(cos_v * ptLy_org - sin_v * ptLx_org) + cy);
}




