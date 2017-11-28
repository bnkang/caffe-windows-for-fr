#include "stdafx.h"

#include <stdlib.h>
#include <memory.h>
#include "MVFD_Defines.h"
#include "MVFD_Memory.h"

#ifdef USE_SSE
#include <mmintrin.h>
#include <emmintrin.h>
#include <tmmintrin.h>
#include <smmintrin.h>
void MVFD_Image2IRMB(unsigned char *image, int width, int height, FD_CONTEXT *pContext, unsigned char *feature_buf)
{
	int iimg_w = width+1;
	int iimg_h = height+1;
	int aligned_width = (((width+8)/8)*8);
	int *integral_img_buf = MVFD_MemSection_Alloc<int>(pContext, MEM_SECTION_IRMB_INTEGRAL_IMG_BUF, aligned_width*iimg_h);
	memset(integral_img_buf, 0, aligned_width*sizeof(int));

	unsigned char *ipos = image;
	int *p0 = integral_img_buf+aligned_width+1;
	int *p1 = integral_img_buf+1;
	int s;
	int delta = aligned_width-width;
	for(int y = 1; y < height+1; ++y)
	{
		integral_img_buf[y*aligned_width] = 0;
		s = 0;
		for(int x = width; x ; --x)
		{
			s = s + *(ipos++);
			*(p0++) = s + *(p1++);
		}
		p0 += delta; p1 += delta;
	}

	for(int i = 0; i < height; ++i)
	{
		memcpy(&feature_buf[i*aligned_width], &image[i*width], width);
	}
	int scale = 2;		//1,2,4,8
	for(int i = 1; i < 4; ++i)
	{
		__m64 *obuffer = (__m64 *)&feature_buf[i*aligned_width*height];
		int new_w = iimg_w-scale;	//bytes to fill
		int new_h = iimg_h-scale;
		int loop_cnt = (new_w+7)/8;
		for(int y = 0; y < new_h; ++y)
		{
			__m128i *LT_pos = (__m128i *)(integral_img_buf+y*aligned_width);
			__m128i *RT_pos = (__m128i *)(((int *)LT_pos)+scale);
			__m128i *LB_pos = (__m128i *)(((int *)LT_pos)+scale*aligned_width);
			__m128i *RB_pos = (__m128i *)(((int *)LB_pos)+scale);
			__m64 *outpos =  (__m64 *)(((unsigned char *)obuffer)+y*aligned_width);
			int lp = loop_cnt;
			while(lp--)
			{
				__m128i xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
				xmm1 = _mm_load_si128((__m128i *)LT_pos++);
				xmm2 = _mm_load_si128((__m128i *)LB_pos++);
				xmm3 = _mm_loadu_si128((__m128i *)RT_pos++);
				xmm4 = _mm_loadu_si128((__m128i *)RB_pos++);
				xmm5 = _mm_sub_epi32(xmm4, xmm2);
				xmm5 = _mm_sub_epi32(xmm5, xmm3);
				xmm5 = _mm_add_epi32(xmm5, xmm1);
				xmm5 = _mm_srli_epi32(xmm5, i+i);

				xmm1 = _mm_load_si128((__m128i *)LT_pos++);
				xmm2 = _mm_load_si128((__m128i *)LB_pos++);
				xmm3 = _mm_loadu_si128((__m128i *)RT_pos++);
				xmm4 = _mm_loadu_si128((__m128i *)RB_pos++);
				xmm6 = _mm_sub_epi32(xmm4, xmm2);
				xmm6 = _mm_sub_epi32(xmm6, xmm3);
				xmm6 = _mm_add_epi32(xmm6, xmm1);
				xmm6 = _mm_srli_epi32(xmm6, i+i);
				xmm7 = _mm_packs_epi32(xmm5, xmm6);
				xmm7 = _mm_packus_epi16(xmm7, _mm_setzero_si128());
				
				_mm_storel_epi64((__m128i *)outpos++, xmm7);
			}
		}
		scale <<= 1;
	}
	_mm_empty();
}

void MVFD_ImageRotate(unsigned char* src_data, int src_w, int src_h, unsigned char* rot_data)
{
	unsigned char *sp = src_data;
	int src_w_div4 = src_w>>2;
	int remain = src_w-(src_w_div4<<2);
	for(int y = 0; y < src_h; ++y)
	{
		unsigned char *rp = &rot_data[y];
		int x = src_w_div4;
		while(x--)
		{
			*rp = *sp++;
			rp += src_h;
			*rp = *sp++;
			rp += src_h;
			*rp = *sp++;
			rp += src_h;
			*rp = *sp++;
			rp += src_h;
		}
		x = remain;
		while(x--)
		{
			*rp = *sp++;
			rp += src_h;
		}
	}
}

void MVFD_MT_Rotate_RIP(unsigned char *src, int src_w, int src_h, unsigned char *dst, int direction, int start_y, int end_y)
{
	switch(direction)
	{
		case ROTATE_TOP_UP:
		{			
			if(start_y < end_y)
			{
				int sub_h = end_y-start_y;
				memcpy(&dst[src_w*start_y], &src[src_w*start_y], src_w*sub_h);
			}
			break;
		}
		case ROTATE_LEFT_UP:
		{
			unsigned char *pdst;
			src = &src[start_y*src_w];
			for(int y = start_y; y < end_y; ++y)
			{
				pdst = &dst[src_h-y-1];
				for(int x = 0; x < src_w; ++x)
				{
					*pdst = *src++;
					pdst += src_h;
				}
			}
			break;
		}

		case ROTATE_RIGHT_UP:
		{
			unsigned char *pdst;
			unsigned char *dst_last_line = &dst[(src_w-1)*src_h];
			src = &src[start_y*src_w];
			for(int y = start_y; y < end_y; ++y)
			{
				pdst = &dst_last_line[y];
				for(int x = 0; x < src_w; ++x)
				{
					*pdst = *src++;
					pdst -= src_h;
				}
			}
			break;
		}

		case ROTATE_BOTTOM_UP:
		{
			for(int y = start_y; y < end_y; ++y)
			{
				memcpy(&dst[src_w*y], &src[(src_h-y-1)*src_w], src_w);
			}
			break;
		}
	}
}

void MVFD_MT_ResizeBI_Region(unsigned char *src_data, int src_w, int src_h, FD_RECT roi, unsigned char *des_data, int des_w, int des_h, int start_y, int end_y)
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
	unsigned char *pucdstp = des_data+start_y*des_w;
	unsigned char *lt_src_data = src_data+roi.top*src_w+roi.left;

	/* Affine transformation */
	uda_11 = ( (roi.right-roi.left) << 10 ) / des_w;
	uda_22 = ( (roi.bottom-roi.top) << 10 ) / des_h;
	
	for ( j = start_y; j < end_y; j++ ) {
		for ( i = 0; i < des_w; i++ ) {
			locgray4 = 0;

			udtx = uda_11 * i;
			udty = uda_22 * j;

			utx = udtx >> 10;
			uty = udty >> 10;
			udtx -= ( utx << 10 );
			udty -= ( uty << 10 );
			pucsrcp = lt_src_data + ( uty * src_w ) + utx;

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

void MVFD_Make_ResizeBI_Map(unsigned char *src_data, int src_w, int src_h, unsigned char *dest)
{
	unsigned char *srcp = src_data;
	unsigned char *destp = dest;
	int aligned_w = ((src_w-1)/15)*15;
	for(int y = 0; y < src_h-1; ++y)
	{
		for(int x = 0; x < aligned_w; x += 15)
		{
			__m128i xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;
			xmm1 = _mm_loadu_si128((__m128i *)srcp);
			xmm2 = _mm_srli_si128(xmm1, 1);
			xmm3 = _mm_loadu_si128((__m128i *)(srcp+src_w));
			xmm4 = _mm_srli_si128(xmm3, 1);
			xmm5 = _mm_unpacklo_epi8(xmm1, xmm3);
			xmm6 = _mm_unpackhi_epi8(xmm1, xmm3);
			xmm7 = _mm_unpacklo_epi8(xmm2, xmm4);
			xmm8 = _mm_unpackhi_epi8(xmm2, xmm4);
			xmm1 = _mm_unpacklo_epi8(xmm5, xmm7);
			xmm2 = _mm_unpackhi_epi8(xmm5, xmm7);
			xmm3 = _mm_unpacklo_epi8(xmm6, xmm8);
			xmm4 = _mm_unpackhi_epi8(xmm6, xmm8);

			_mm_storeu_si128((__m128i *)destp, xmm1); destp += 16;
			_mm_storeu_si128((__m128i *)destp, xmm2); destp += 16;
			_mm_storeu_si128((__m128i *)destp, xmm3); destp += 16;
			_mm_storeu_si128((__m128i *)destp, xmm4); destp += 12;
			srcp += 15;
		}
		for(int x = aligned_w; x < src_w; ++x)
		{
			int dx = x < src_w-1;
			*destp++ = *srcp;
			*destp++ = *(srcp+dx);
			*destp++ = *(srcp+src_w);
			*destp++ = *(srcp+src_w+dx);
			++srcp;
		}
	}
	for(int x = 0; x < src_w; ++x)
	{
		int dx = x < src_w-1;
		*destp++ = *srcp;
		*destp++ = *(srcp+dx);
		*destp++ = *srcp;
		*destp++ = *(srcp+dx);
		srcp++;
	}

}

void MVFD_Make_ResizeBI_Map_ROI(unsigned char *src_data, int src_w, int src_h, FD_RECT roi, unsigned char *dest)
{
	unsigned char *srcp = src_data+roi.top*src_w+roi.left;
	unsigned char *destp = dest;
	int src_roi_w = roi.right-roi.left;
	int src_roi_h = roi.bottom-roi.top;
	int aligned_w = ((src_roi_w-1)/15)*15;
	for(int y = 0; y < src_roi_h-1; ++y)
	{
		for(int x = 0; x < aligned_w; x += 15)
		{
			__m128i xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8;
			xmm1 = _mm_loadu_si128((__m128i *)srcp);
			xmm2 = _mm_srli_si128(xmm1, 1);
			xmm3 = _mm_loadu_si128((__m128i *)(srcp+src_w));
			xmm4 = _mm_srli_si128(xmm3, 1);
			xmm5 = _mm_unpacklo_epi8(xmm1, xmm3);
			xmm6 = _mm_unpackhi_epi8(xmm1, xmm3);
			xmm7 = _mm_unpacklo_epi8(xmm2, xmm4);
			xmm8 = _mm_unpackhi_epi8(xmm2, xmm4);
			xmm1 = _mm_unpacklo_epi8(xmm5, xmm7);
			xmm2 = _mm_unpackhi_epi8(xmm5, xmm7);
			xmm3 = _mm_unpacklo_epi8(xmm6, xmm8);
			xmm4 = _mm_unpackhi_epi8(xmm6, xmm8);

			_mm_storeu_si128((__m128i *)destp, xmm1); destp += 16;
			_mm_storeu_si128((__m128i *)destp, xmm2); destp += 16;
			_mm_storeu_si128((__m128i *)destp, xmm3); destp += 16;
			_mm_storeu_si128((__m128i *)destp, xmm4); destp += 12;
			srcp += 15;
		}
		for(int x = roi.left+aligned_w; x < roi.right; ++x)
		{
			int dx = x < src_w-1;
			*destp++ = *srcp;
			*destp++ = *(srcp+dx);
			*destp++ = *(srcp+src_w);
			*destp++ = *(srcp+src_w+dx);
			++srcp;
		}
		srcp += src_w-src_roi_w;
	}
	for(int x = roi.left; x < roi.right; ++x)
	{
		int dx = x < src_w-1;
		*destp++ = *srcp;
		*destp++ = *(srcp+dx);
		*destp++ = *srcp;
		*destp++ = *(srcp+dx);
		srcp++;
	}
}

FD_RECT IntersectRect(FD_RECT &rc1, FD_RECT &rc2)
{
	FD_RECT overlap_rect = {FD_MAX(rc1.left, rc2.left), FD_MAX(rc1.top, rc2.top), FD_MIN(rc1.right, rc2.right), FD_MIN(rc1.bottom, rc2.bottom)};
	return overlap_rect;
}

void RectMapping(FD_RECT &map_from, FD_RECT &map_to, FD_RECT &src, FD_RECT &dst)
{
	float scale_w = float(map_to.right-map_to.left)/float(map_from.right-map_from.left);
	float scale_h = float(map_to.bottom-map_to.top)/float(map_from.bottom-map_from.top);
	dst.left = (src.left-map_from.left)*scale_w+map_to.left;
	dst.top  = (src.top-map_from.top)*scale_h+map_to.top;
	dst.right = (src.right-map_from.left)*scale_w+map_to.left;
	dst.bottom = (src.bottom-map_from.top)*scale_h+map_to.top;
}

void VMT_ResizeBI_Region_UseMap(unsigned char *src_data, int src_w, int src_h, FD_RECT roi, unsigned char *des_data, int des_w, int des_h, int start_y, int end_y)
{
	int uda_11, uda_22;
	int i, j;
	
	unsigned char *pucsrcp = 0;
	unsigned char *pucdstp = des_data+start_y*des_w;
	unsigned char *lt_src_data = src_data+((roi.top*src_w+roi.left)<<2);

	// Affine transformation
	//printf("\n");
	uda_11 = ( (roi.right-roi.left) << 10 ) / des_w;
	uda_22 = ( (roi.bottom-roi.top) << 10 ) / des_h;

	__m128i xmm_uda_11, xmm_uda_22, xmm_idx;
	xmm_uda_11 = _mm_set1_epi32(uda_11);
	xmm_uda_22 = _mm_set1_epi32(uda_22);
	xmm_idx = _mm_set_epi32(3, 2, 1, 0);
	int des_align4_w = ((des_w-1)>>2)<<2;
	for ( j = start_y; j < end_y; j++ ) {
		int *ipucdstp = (int *)pucdstp;
		for ( i = 0; i < des_align4_w; i+=4 ) {
			__m128i xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10;
			xmm1 = _mm_add_epi32(_mm_set1_epi32(i), xmm_idx);
			xmm1 = _mm_mullo_epi32(xmm_uda_11, xmm1);
			xmm2 = _mm_srli_epi32(xmm1, 10);	//utx
			xmm3 = _mm_slli_epi32(xmm2, 10);
			xmm1 = _mm_sub_epi32(xmm1, xmm3);	//udtx

			xmm3 = _mm_mullo_epi32(xmm_uda_22, _mm_set1_epi32(j));
			xmm4 = _mm_srli_epi32(xmm3, 10);	//uty
			xmm5 = _mm_slli_epi32(xmm4, 10);
			xmm3 = _mm_sub_epi32(xmm3, xmm5);	//udty
			
			xmm5 = _mm_set1_epi32(src_w);
			xmm5 = _mm_mullo_epi32(xmm4, xmm5);
			xmm5 = _mm_add_epi32(xmm5, xmm2);
			xmm5 = _mm_slli_epi32(xmm5, 2);
			xmm6 = _mm_set1_epi32((int)lt_src_data);
			xmm5 = _mm_add_epi32(xmm6, xmm5);	//pucsrcp = lt_src_data + ((( uty * src_w ) + utx)<<2);
			
			xmm9 = _mm_set1_epi32(0xff);
			xmm8 = _mm_set_epi32( *((unsigned int *)xmm5.m128i_u32[3]),		//ucgray1
								  *((unsigned int *)xmm5.m128i_u32[2]),		//ucgray2
								  *((unsigned int *)xmm5.m128i_u32[1]),		//ucgray3
								  *((unsigned int *)xmm5.m128i_u32[0]));	//ucgray4
			xmm5 = _mm_and_si128(xmm8, xmm9);
			xmm8 = _mm_srli_si128(xmm8, 1);
			xmm6 = _mm_and_si128(xmm8, xmm9);
			xmm8 = _mm_srli_si128(xmm8, 1);
			xmm7 = _mm_and_si128(xmm8, xmm9);
			xmm8 = _mm_srli_si128(xmm8, 1);
			xmm8 = _mm_and_si128(xmm8, xmm9);
			
			xmm2 = _mm_slli_epi32(xmm5, 20);	//aa
			xmm4 = _mm_mullo_epi32(_mm_sub_epi32(xmm6, xmm5), _mm_slli_epi32(xmm1, 10));	//ab
			xmm9 = _mm_mullo_epi32(_mm_sub_epi32(xmm7, xmm5), _mm_slli_epi32(xmm3, 10));	//ac
			xmm10 = _mm_sub_epi32(_mm_sub_epi32(_mm_add_epi32(xmm5, xmm8), xmm7), xmm6);
			xmm10 = _mm_mullo_epi32(xmm10, xmm1); 
			xmm10 = _mm_mullo_epi32(xmm10, xmm3); //ad

			xmm5 = _mm_add_epi32(xmm2, xmm4);
			xmm6 = _mm_add_epi32(xmm9, xmm10);
			xmm7 = _mm_add_epi32(xmm5, xmm6);
			xmm8 = _mm_srli_epi32(xmm7, 20);
			xmm8 = _mm_packs_epi32(xmm8, _mm_setzero_si128());
			xmm8 = _mm_packus_epi16(xmm8, _mm_setzero_si128());
			*ipucdstp++ = xmm8.m128i_u32[0];
		}
		{
			int ucgray1 = 0, ucgray2 = 0, ucgray3 = 0, ucgray4 = 0, locgray4;
			unsigned int aa, ab, ac, ad;
			int utx, uty, udtx, udty;
			for ( i = des_align4_w; i < des_w; i++ ) {
				locgray4 = 0;

				udtx = uda_11 * i;
				udty = uda_22 * j;

				utx = udtx >> 10;
				uty = udty >> 10;
				udtx -= ( utx << 10 );
				udty -= ( uty << 10 );
				pucsrcp = lt_src_data + ((( uty * src_w ) + utx) << 2);

				ucgray1 = *pucsrcp++;
				ucgray2 = *pucsrcp++;
				ucgray3 = *pucsrcp++;
				ucgray4 = *pucsrcp++;
						

				aa = ucgray1 << 20;
				ab = ( ( ucgray2 - ucgray1 ) * ( udtx << 10 ) );
				ac = ( ( ucgray3 - ucgray1 ) * ( udty << 10 ) );
				ad = ( ucgray1 + ucgray4 - ucgray3 - ucgray2 ) * udtx * udty;

				*( pucdstp + i ) = ( unsigned char ) ( ( aa + ab + ac + ad ) >> 20 );
			}
		}
		pucdstp += des_w;
	}
}

//resize image from one region to another region
bool MVFD_MT_ResizeBI_R2R(unsigned char *src_data, int src_w, int src_h, FD_RECT src_roi, unsigned char *des_data, int des_w, int des_h, FD_RECT des_roi)
{
	int uda_11, uda_22;
	int i, j;
	
	FD_RECT entire_src = {0, 0, src_w, src_h};
	FD_RECT entire_des = {0, 0, des_w, des_h};	
	FD_RECT valid_src_roi = IntersectRect(entire_src, src_roi);
	FD_RECT valid_des_roi = IntersectRect(entire_des, des_roi);
	FD_RECT valid_des_roi_in_src;
	RectMapping(des_roi, src_roi, valid_des_roi, valid_des_roi_in_src);
	FD_RECT new_src_roi = IntersectRect(valid_src_roi, valid_des_roi_in_src);
	FD_RECT new_des_roi;
	RectMapping(src_roi, des_roi, new_src_roi, new_des_roi);

	unsigned char *pucsrcp = 0;
	unsigned char *pucdstp = des_data+new_des_roi.top*des_w+new_des_roi.left;
	unsigned char *lt_src_data = src_data+new_src_roi.top*src_w+new_src_roi.left;
	
	int des_roi_h = new_des_roi.bottom-new_des_roi.top;
	int des_roi_w = new_des_roi.right-new_des_roi.left;
	if(des_roi_w <= 0 || des_roi_h <= 0) return false;

	/* Affine transformation */
	uda_11 = ( (new_src_roi.right-new_src_roi.left) << 10 ) / des_roi_w;
	uda_22 = ( (new_src_roi.bottom-new_src_roi.top) << 10 ) / des_roi_h;

	__m128i xmm_uda_11, xmm_uda_22, xmm_idx;
	xmm_uda_11 = _mm_set1_epi32(uda_11);
	xmm_uda_22 = _mm_set1_epi32(uda_22);
	xmm_idx = _mm_set_epi32(3, 2, 1, 0);
	int des_align4_w = ((des_roi_w-1)>>2)<<2;
	for ( j = 0; j < des_roi_h; j++ ) {
		int *ipucdstp = (int *)pucdstp;
		for ( i = 0; i < des_align4_w; i+=4 ) {
			__m128i xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10;
			xmm1 = _mm_add_epi32(_mm_set1_epi32(i), xmm_idx);
			xmm1 = _mm_mullo_epi32(xmm_uda_11, xmm1);
			xmm2 = _mm_srli_epi32(xmm1, 10);	//utx
			xmm3 = _mm_slli_epi32(xmm2, 10);
			xmm1 = _mm_sub_epi32(xmm1, xmm3);	//udtx

			xmm3 = _mm_mullo_epi32(xmm_uda_22, _mm_set1_epi32(j));
			xmm4 = _mm_srli_epi32(xmm3, 10);	//uty
			xmm5 = _mm_slli_epi32(xmm4, 10);
			xmm3 = _mm_sub_epi32(xmm3, xmm5);	//udty

			xmm5 = _mm_set1_epi32(src_w);
			xmm5 = _mm_mullo_epi32(xmm4, xmm5);
			xmm5 = _mm_add_epi32(xmm5, xmm2);
			xmm6 = _mm_set1_epi32((int)lt_src_data);
			xmm5 = _mm_add_epi32(xmm6, xmm5);	//pucsrcp = lt_src_data + ( uty * src_w ) + utx;

			xmm6 = _mm_add_epi32(xmm5, _mm_set1_epi32(1));
			xmm7 = _mm_add_epi32(xmm5, _mm_set1_epi32(j < des_h-1 ? src_w : 0));
			xmm8 = _mm_add_epi32(xmm7, _mm_set1_epi32(1));

			xmm5 = _mm_set_epi32( *((unsigned char *)xmm5.m128i_u32[3]),		//ucgray1
								  *((unsigned char *)xmm5.m128i_u32[2]),
								  *((unsigned char *)xmm5.m128i_u32[1]),
								  *((unsigned char *)xmm5.m128i_u32[0]));

			xmm6 = _mm_set_epi32( *((unsigned char *)xmm6.m128i_u32[3]),		//ucgray2
								  *((unsigned char *)xmm6.m128i_u32[2]),
								  *((unsigned char *)xmm6.m128i_u32[1]),
								  *((unsigned char *)xmm6.m128i_u32[0]));

			xmm7 = _mm_set_epi32( *((unsigned char *)xmm7.m128i_u32[3]),		//ucgray3
								  *((unsigned char *)xmm7.m128i_u32[2]),
								  *((unsigned char *)xmm7.m128i_u32[1]),
								  *((unsigned char *)xmm7.m128i_u32[0]));

			xmm8 = _mm_set_epi32( *((unsigned char *)xmm8.m128i_u32[3]),		//ucgray4
								  *((unsigned char *)xmm8.m128i_u32[2]),
								  *((unsigned char *)xmm8.m128i_u32[1]),
								  *((unsigned char *)xmm8.m128i_u32[0]));

			xmm2 = _mm_slli_epi32(xmm5, 20);	//aa
			xmm4 = _mm_mullo_epi32(_mm_sub_epi32(xmm6, xmm5), _mm_slli_epi32(xmm1, 10));	//ab
			xmm9 = _mm_mullo_epi32(_mm_sub_epi32(xmm7, xmm5), _mm_slli_epi32(xmm3, 10));	//ac
			xmm10 = _mm_sub_epi32(_mm_sub_epi32(_mm_add_epi32(xmm5, xmm8), xmm7), xmm6);
			xmm10 = _mm_mullo_epi32(xmm10, xmm1); 
			xmm10 = _mm_mullo_epi32(xmm10, xmm3); //ad

			xmm5 = _mm_add_epi32(xmm2, xmm4);
			xmm6 = _mm_add_epi32(xmm9, xmm10);
			xmm7 = _mm_add_epi32(xmm5, xmm6);
			xmm8 = _mm_srli_epi32(xmm7, 20);
			xmm8 = _mm_packs_epi32(xmm8, _mm_setzero_si128());
			xmm8 = _mm_packus_epi16(xmm8, _mm_setzero_si128());
			*ipucdstp++ = xmm8.m128i_u32[0];
		}
		{
			int ucgray1 = 0, ucgray2 = 0, ucgray3 = 0, ucgray4 = 0, locgray4;
			unsigned int aa, ab, ac, ad;
			int utx, uty, udtx, udty;
			for ( i = des_align4_w; i < des_roi_w; i++ ) {
				locgray4 = 0;

				udtx = uda_11 * i;
				udty = uda_22 * j;

				utx = udtx >> 10;
				uty = udty >> 10;
				udtx -= ( utx << 10 );
				udty -= ( uty << 10 );
				pucsrcp = lt_src_data + ( uty * src_w ) + utx;

				ucgray1 = *pucsrcp;

				if ( i >= des_roi_w - 1 ) {
					ucgray2 = *pucsrcp;
					udtx = 1 << 10;
				}
				else {
					ucgray2 = *( pucsrcp + 1 );
					locgray4 += 1;
				}

				if ( j >= des_roi_h - 1 ) {
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
		}
		pucdstp += des_w;
	}
	return true;
}


//resize image from one region to another region using bi-linear map
bool MVFD_MT_ResizeBI_R2R_UseMap(unsigned char *src_data, int src_w, int src_h, FD_RECT src_roi, unsigned char *des_data, int des_w, int des_h, FD_RECT des_roi)
{
	int uda_11, uda_22;
	int i, j;

	FD_RECT entire_src = {0, 0, src_w, src_h};
	FD_RECT entire_des = {0, 0, des_w, des_h};	
	FD_RECT valid_src_roi = IntersectRect(entire_src, src_roi);
	FD_RECT valid_des_roi = IntersectRect(entire_des, des_roi);
	FD_RECT valid_des_roi_in_src;
	RectMapping(des_roi, src_roi, valid_des_roi, valid_des_roi_in_src);
	FD_RECT new_src_roi = IntersectRect(valid_src_roi, valid_des_roi_in_src);
	FD_RECT new_des_roi;
	RectMapping(src_roi, des_roi, new_src_roi, new_des_roi);

	unsigned char *pucsrcp = 0;
	unsigned char *pucdstp = des_data+new_des_roi.top*des_w+new_des_roi.left;
	unsigned char *lt_src_data = src_data+((new_src_roi.top*src_w+new_src_roi.left)<<2);

	int des_roi_h = new_des_roi.bottom-new_des_roi.top;
	int des_roi_w = new_des_roi.right-new_des_roi.left;
	if(des_roi_w <= 0 || des_roi_h <= 0) return false;

	/* Affine transformation */
	uda_11 = ( (new_src_roi.right-new_src_roi.left) << 10 ) / des_roi_w;
	uda_22 = ( (new_src_roi.bottom-new_src_roi.top) << 10 ) / des_roi_h;

	__m128i xmm_uda_11, xmm_uda_22, xmm_idx;
	xmm_uda_11 = _mm_set1_epi32(uda_11);
	xmm_uda_22 = _mm_set1_epi32(uda_22);
	xmm_idx = _mm_set_epi32(3, 2, 1, 0);
	int des_align4_w = ((des_roi_w-1)>>2)<<2;
	for ( j = 0; j < des_roi_h; j++ ) {
		int *ipucdstp = (int *)pucdstp;
		for ( i = 0; i < des_align4_w; i+=4 ) {
			__m128i xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10;
			xmm1 = _mm_add_epi32(_mm_set1_epi32(i), xmm_idx);
			xmm1 = _mm_mullo_epi32(xmm_uda_11, xmm1);
			xmm2 = _mm_srli_epi32(xmm1, 10);	//utx
			xmm3 = _mm_slli_epi32(xmm2, 10);
			xmm1 = _mm_sub_epi32(xmm1, xmm3);	//udtx

			xmm3 = _mm_mullo_epi32(xmm_uda_22, _mm_set1_epi32(j));
			xmm4 = _mm_srli_epi32(xmm3, 10);	//uty
			xmm5 = _mm_slli_epi32(xmm4, 10);
			xmm3 = _mm_sub_epi32(xmm3, xmm5);	//udty
			
			xmm5 = _mm_set1_epi32(src_w);
			xmm5 = _mm_mullo_epi32(xmm4, xmm5);
			xmm5 = _mm_add_epi32(xmm5, xmm2);
			xmm5 = _mm_slli_epi32(xmm5, 2);
			xmm6 = _mm_set1_epi32((int)lt_src_data);
			xmm5 = _mm_add_epi32(xmm6, xmm5);	//pucsrcp = lt_src_data + ((( uty * src_w ) + utx)<<2);
			
			xmm9 = _mm_set1_epi32(0xff);
			xmm8 = _mm_set_epi32( *((unsigned int *)xmm5.m128i_u32[3]),		//ucgray1
								  *((unsigned int *)xmm5.m128i_u32[2]),		//ucgray2
								  *((unsigned int *)xmm5.m128i_u32[1]),		//ucgray3
								  *((unsigned int *)xmm5.m128i_u32[0]));	//ucgray4
			xmm5 = _mm_and_si128(xmm8, xmm9);
			xmm8 = _mm_srli_si128(xmm8, 1);
			xmm6 = _mm_and_si128(xmm8, xmm9);
			xmm8 = _mm_srli_si128(xmm8, 1);
			xmm7 = _mm_and_si128(xmm8, xmm9);
			xmm8 = _mm_srli_si128(xmm8, 1);
			xmm8 = _mm_and_si128(xmm8, xmm9);
			
			xmm2 = _mm_slli_epi32(xmm5, 20);	//aa
			xmm4 = _mm_mullo_epi32(_mm_sub_epi32(xmm6, xmm5), _mm_slli_epi32(xmm1, 10));	//ab
			xmm9 = _mm_mullo_epi32(_mm_sub_epi32(xmm7, xmm5), _mm_slli_epi32(xmm3, 10));	//ac
			xmm10 = _mm_sub_epi32(_mm_sub_epi32(_mm_add_epi32(xmm5, xmm8), xmm7), xmm6);
			xmm10 = _mm_mullo_epi32(xmm10, xmm1); 
			xmm10 = _mm_mullo_epi32(xmm10, xmm3); //ad

			xmm5 = _mm_add_epi32(xmm2, xmm4);
			xmm6 = _mm_add_epi32(xmm9, xmm10);
			xmm7 = _mm_add_epi32(xmm5, xmm6);
			xmm8 = _mm_srli_epi32(xmm7, 20);
			xmm8 = _mm_packs_epi32(xmm8, _mm_setzero_si128());
			xmm8 = _mm_packus_epi16(xmm8, _mm_setzero_si128());
			*ipucdstp++ = xmm8.m128i_u32[0];
		}
		{
			int ucgray1 = 0, ucgray2 = 0, ucgray3 = 0, ucgray4 = 0, locgray4;
			unsigned int aa, ab, ac, ad;
			int utx, uty, udtx, udty;
			for ( i = des_align4_w; i < des_roi_w; i++ ) {
				locgray4 = 0;

				udtx = uda_11 * i;
				udty = uda_22 * j;

				utx = udtx >> 10;
				uty = udty >> 10;
				udtx -= ( utx << 10 );
				udty -= ( uty << 10 );
				pucsrcp = lt_src_data + ((( uty * src_w ) + utx) << 2);

				ucgray1 = *pucsrcp++;
				ucgray2 = *pucsrcp++;
				ucgray3 = *pucsrcp++;
				ucgray4 = *pucsrcp++;

				aa = ucgray1 << 20;
				ab = ( ( ucgray2 - ucgray1 ) * ( udtx << 10 ) );
				ac = ( ( ucgray3 - ucgray1 ) * ( udty << 10 ) );
				ad = ( ucgray1 + ucgray4 - ucgray3 - ucgray2 ) * udtx * udty;

				*( pucdstp + i ) = ( unsigned char ) ( ( aa + ab + ac + ad ) >> 20 );
			}
		}
		pucdstp += des_w;
	}
	return true;
}

void MVFD_MT_Image2MCT(unsigned char* src_data, int src_w, int src_h, unsigned short* mct_data, int start_y, int end_y)
{
	start_y = start_y < 1 ? 1 : start_y;
	end_y = end_y > src_h-1 ? src_h-1 : end_y;
	int x = 0;	
	while(x < src_w)
	{
		unsigned short *dest = mct_data+start_y*src_w+1+x;
		__m128i xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, sum;
		xmm1 = _mm_setzero_si128(); 
		xmm2 = _mm_setzero_si128(); 
		xmm3 = _mm_setzero_si128();
		
		unsigned char *ptr = src_data + (start_y-1)*src_w+x;
		xmm4 = _mm_loadl_epi64((__m128i *)ptr); 
		xmm5 = _mm_loadl_epi64((__m128i *)(ptr+1)); 
		xmm6 = _mm_loadl_epi64((__m128i *)(ptr+2));
		xmm4 = _mm_unpacklo_epi8(xmm4, xmm1);
		xmm5 = _mm_unpacklo_epi8(xmm5, xmm1);
		xmm6 = _mm_unpacklo_epi8(xmm6, xmm1);

		xmm7 = _mm_loadl_epi64((__m128i *)(ptr+src_w)); 
		xmm8 = _mm_loadl_epi64((__m128i *)(ptr+src_w+1)); 
		xmm9 = _mm_loadl_epi64((__m128i *)(ptr+src_w+2));
		xmm7 = _mm_unpacklo_epi8(xmm7, xmm1);
		xmm8 = _mm_unpacklo_epi8(xmm8, xmm1);
		xmm9 = _mm_unpacklo_epi8(xmm9, xmm1);

		ptr += (src_w<<1);

		sum = _mm_setzero_si128(); 
		sum = _mm_add_epi16(sum, xmm4);
		sum = _mm_add_epi16(sum, xmm5);
		sum = _mm_add_epi16(sum, xmm6);
		sum = _mm_add_epi16(sum, xmm7);
		sum = _mm_add_epi16(sum, xmm8);
		sum = _mm_add_epi16(sum, xmm9);
		for(int y = start_y; y < end_y; ++y)
		{
			__m128i zero = _mm_setzero_si128();
			sum = _mm_sub_epi16(sum, xmm1);
			sum = _mm_sub_epi16(sum, xmm2);
			sum = _mm_sub_epi16(sum, xmm3);

			xmm1 = xmm4; xmm2 = xmm5; xmm3 = xmm6;
			xmm4 = xmm7; xmm5 = xmm8; xmm6 = xmm9;

			xmm7 = _mm_loadl_epi64((__m128i *)ptr);
			xmm8 = _mm_loadl_epi64((__m128i *)(ptr+1));
			xmm9 = _mm_loadl_epi64((__m128i *)(ptr+2));
			xmm7 = _mm_unpacklo_epi8(xmm7, zero);
			xmm8 = _mm_unpacklo_epi8(xmm8, zero);
			xmm9 = _mm_unpacklo_epi8(xmm9, zero);

			sum = _mm_add_epi16(sum, xmm7);
			sum = _mm_add_epi16(sum, xmm8);
			sum = _mm_add_epi16(sum, xmm9);
			{
				__m128i temp, mask, code;
				code = _mm_setzero_si128();
				mask = _mm_set1_epi16(1); 

				temp = _mm_slli_epi16(xmm1, 3);
				temp = _mm_add_epi16(temp, xmm1);
				temp = _mm_cmplt_epi16(sum, temp);
				code = _mm_and_si128(temp, mask);

				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_slli_epi16(xmm2, 3);
				temp = _mm_add_epi16(temp, xmm2);
				temp = _mm_cmplt_epi16(sum, temp);
				code = _mm_or_si128(code, _mm_and_si128(temp, mask));

				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_slli_epi16(xmm3, 3);
				temp = _mm_add_epi16(temp, xmm3);
				temp = _mm_cmplt_epi16(sum, temp);
				code = _mm_or_si128(code, _mm_and_si128(temp, mask));
				
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_slli_epi16(xmm4, 3);
				temp = _mm_add_epi16(temp, xmm4);
				temp = _mm_cmplt_epi16(sum, temp);
				code = _mm_or_si128(code, _mm_and_si128(temp, mask));
				
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_slli_epi16(xmm5, 3);
				temp = _mm_add_epi16(temp, xmm5);
				temp = _mm_cmplt_epi16(sum, temp);
				code = _mm_or_si128(code, _mm_and_si128(temp, mask));
				
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_slli_epi16(xmm6, 3);
				temp = _mm_add_epi16(temp, xmm6);
				temp = _mm_cmplt_epi16(sum, temp);
				code = _mm_or_si128(code, _mm_and_si128(temp, mask));
				
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_slli_epi16(xmm7, 3);
				temp = _mm_add_epi16(temp, xmm7);
				temp = _mm_cmplt_epi16(sum, temp);
				code = _mm_or_si128(code, _mm_and_si128(temp, mask));
				
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_slli_epi16(xmm8, 3);
				temp = _mm_add_epi16(temp, xmm8);
				temp = _mm_cmplt_epi16(sum, temp);
				code = _mm_or_si128(code, _mm_and_si128(temp, mask));
				
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_slli_epi16(xmm9, 3);
				temp = _mm_add_epi16(temp, xmm9);
				temp = _mm_cmplt_epi16(sum, temp);
				code = _mm_or_si128(code, _mm_and_si128(temp, mask));
				_mm_storeu_si128((__m128i *)dest, code);
			}
			
			dest += src_w;
			ptr += src_w;
		}
		if(x >= src_w-10) break;
		x = x+8+8 >= src_w-2 ? src_w-10 : x+8;
	}
}

void MVFD_CropRegion(unsigned char* src_data, int src_w, int src_h, FD_RECT roi, unsigned char *dst_data)
{
	int dst_w = roi.right-roi.left;
	int dst_h = roi.bottom-roi.top;
	for(int y = 0; y < dst_h; ++y)
	{
		memcpy(&dst_data[y*dst_w], &src_data[(roi.top+y)*src_w+roi.left], dst_w);
	}
}
#endif
