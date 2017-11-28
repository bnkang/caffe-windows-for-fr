#include "stdafx.h"

#include <stdlib.h>
//#include <memory.h>
#include "MVFD_Defines.h"
#include "MVFD_Memory.h"

#if !defined(USE_SSE) && !defined(USE_NEON)
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
		fd_memcpy(&feature_buf[i*aligned_width], &image[i*width], width);
	}
	int scale = 2;		//1,2,4,8
	for(int i = 1; i < 4; ++i)
	{
		unsigned char *obuffer = &feature_buf[i*aligned_width*height];
		int new_w = iimg_w-scale;
		int new_h = iimg_h-scale;
		for(int y = 0; y < new_h; ++y)
		{
			int *LT_pos = integral_img_buf+y*aligned_width;
			int *RT_pos = LT_pos+scale;
			int *LB_pos = LT_pos+scale*aligned_width;
			int *RB_pos = LB_pos+scale;
			unsigned char *optr = obuffer+y*aligned_width;
			for(int x = new_w; x; --x)
			{
				*(optr++) = (*(RB_pos++) - *(LB_pos++) - *(RT_pos++) + *(LT_pos++))>>(i+i);
			}
		}
		scale <<= 1;
	}
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
				fd_memcpy(&dst[src_w*start_y], &src[src_w*start_y], src_w*sub_h);
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
				fd_memcpy(&dst[src_w*y], &src[(src_h-y-1)*src_w], src_w);
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

FD_RECT MVFD_IntersectRect(FD_RECT &rc1, FD_RECT &rc2)
{
	FD_RECT overlap_rect = {FD_MAX(rc1.left, rc2.left), FD_MAX(rc1.top, rc2.top), FD_MIN(rc1.right, rc2.right), FD_MIN(rc1.bottom, rc2.bottom)};
	return overlap_rect;
}

void MVFD_RectMapping(FD_RECT &map_from, FD_RECT &map_to, FD_RECT &src, FD_RECT &dst)
{
	float scale_w = float(map_to.right-map_to.left)/float(map_from.right-map_from.left);
	float scale_h = float(map_to.bottom-map_to.top)/float(map_from.bottom-map_from.top);
	dst.left = (src.left-map_from.left)*scale_w+map_to.left;
	dst.top  = (src.top-map_from.top)*scale_h+map_to.top;
	dst.right = (src.right-map_from.left)*scale_w+map_to.left;
	dst.bottom = (src.bottom-map_from.top)*scale_h+map_to.top;
}

void MVFD_Make_ResizeBI_Map(unsigned char *src_data, int src_w, int src_h, unsigned char *dest)
{

	unsigned char *srcp = src_data;
	unsigned char *destp = dest;
	for(int y = 0; y < src_h; ++y)
	{
		for(int x = 0; x < src_w; ++x)
		{
			int dx = x < src_w-1;
			int dy = y < src_h-1 ? src_w : 0;
			*destp++ = *srcp;
			*destp++ = *(srcp+dx);
			*destp++ = *(srcp+dy);
			*destp++ = *(srcp+dy+dx);
			++srcp;
		}
	}
}
void MVFD_Make_ResizeBI_Map_ROI(unsigned char *src_data, int src_w, int src_h, FD_RECT roi, unsigned char *dest)
{
	unsigned char *srcp = src_data+roi.top*src_w+roi.left;
	unsigned char *destp = dest;
	int src_roi_w = roi.right-roi.left;
	for(int y = roi.top; y < roi.bottom; ++y)
	{
		for(int x = roi.left; x < roi.right; ++x)
		{
			int dx = x < src_w-1;
			int dy = y < src_h-1 ? src_w : 0;
			*destp++ = *srcp;
			*destp++ = *(srcp+dx);
			*destp++ = *(srcp+dy);
			*destp++ = *(srcp+dy+dx);
			++srcp;
		}
		srcp += src_w-src_roi_w;
	}
}

void MVFD_MT_ResizeBI_Region_UseMap(unsigned char *src_data, int src_w, int src_h, FD_RECT roi, unsigned char *des_data, int des_w, int des_h, int start_y, int end_y)
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
	unsigned char *lt_src_data = src_data+((roi.top*src_w+roi.left)<<2);

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
			pucsrcp = lt_src_data + ((( uty * src_w ) + utx)<<2);

			ucgray1 = *pucsrcp++;
			ucgray2 = *pucsrcp++;
			ucgray3 = *pucsrcp++;
			ucgray4 = *pucsrcp;

			aa = ucgray1 << 20;
			ab = ( ( ucgray2 - ucgray1 ) * ( udtx << 10 ) );
			ac = ( ( ucgray3 - ucgray1 ) * ( udty << 10 ) );
			ad = ( ucgray1 + ucgray4 - ucgray3 - ucgray2 ) * udtx * udty;

			*( pucdstp + i ) = ( unsigned char ) ( ( aa + ab + ac + ad ) >> 20 );

		}
		
		pucdstp += des_w;
	}
}

bool MVFD_MT_ResizeBI_R2R(unsigned char *src_data, int src_w, int src_h, FD_RECT src_roi, unsigned char *des_data, int des_w, int des_h, FD_RECT des_roi)
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
	
	FD_RECT entire_src = {0, 0, src_w, src_h};
	FD_RECT entire_des = {0, 0, des_w, des_h};	
	FD_RECT valid_src_roi = MVFD_IntersectRect(entire_src, src_roi);
	FD_RECT valid_des_roi = MVFD_IntersectRect(entire_des, des_roi);
	FD_RECT valid_des_roi_in_src;
	MVFD_RectMapping(des_roi, src_roi, valid_des_roi, valid_des_roi_in_src);
	FD_RECT new_src_roi = MVFD_IntersectRect(valid_src_roi, valid_des_roi_in_src);
	FD_RECT new_des_roi;
	MVFD_RectMapping(src_roi, des_roi, new_src_roi, new_des_roi);

	unsigned char *pucsrcp = 0;
	unsigned char *pucdstp = des_data+new_des_roi.top*des_w+new_des_roi.left;
	unsigned char *lt_src_data = src_data+new_src_roi.top*src_w+new_src_roi.left;

	int des_roi_h = new_des_roi.bottom-new_des_roi.top;
	int des_roi_w = new_des_roi.right-new_des_roi.left;	
	if(des_roi_w <= 0 || des_roi_h <= 0) return false;

	/* Affine transformation */
	uda_11 = ( (new_src_roi.right-new_src_roi.left) << 10 ) / des_roi_w;
	uda_22 = ( (new_src_roi.bottom-new_src_roi.top) << 10 ) / des_roi_h;
	
	for ( j = 0; j < des_roi_h; j++ ) {
		for ( i = 0; i < des_roi_w; i++ ) {
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
		
		pucdstp += des_w;
	}
	return true;
}

bool MVFD_MT_ResizeBI_R2R_UseMap(unsigned char *src_data, int src_w, int src_h, FD_RECT src_roi, unsigned char *des_data, int des_w, int des_h, FD_RECT des_roi)
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
	
	FD_RECT entire_src = {0, 0, src_w, src_h};
	FD_RECT entire_des = {0, 0, des_w, des_h};	
	FD_RECT valid_src_roi = MVFD_IntersectRect(entire_src, src_roi);
	FD_RECT valid_des_roi = MVFD_IntersectRect(entire_des, des_roi);
	FD_RECT valid_des_roi_in_src;
	MVFD_RectMapping(des_roi, src_roi, valid_des_roi, valid_des_roi_in_src);
	FD_RECT new_src_roi = MVFD_IntersectRect(valid_src_roi, valid_des_roi_in_src);
	FD_RECT new_des_roi;
	MVFD_RectMapping(src_roi, des_roi, new_src_roi, new_des_roi);

	unsigned char *pucsrcp = 0;	
	unsigned char *pucdstp = des_data+new_des_roi.top*des_w+new_des_roi.left;
	unsigned char *lt_src_data = src_data+((new_src_roi.top*src_w+new_src_roi.left)<<2);

	int des_roi_h = new_des_roi.bottom-new_des_roi.top;
	int des_roi_w = new_des_roi.right-new_des_roi.left;
	if(des_roi_w <= 0 || des_roi_h <= 0) return false;

	/* Affine transformation */
	uda_11 = ( (new_src_roi.right-new_src_roi.left) << 10 ) / des_roi_w;
	uda_22 = ( (new_src_roi.bottom-new_src_roi.top) << 10 ) / des_roi_h;
	
	for ( j = 0; j < des_roi_h; j++ ) {
		for ( i = 0; i < des_roi_w; i++ ) {
			locgray4 = 0;

			udtx = uda_11 * i;
			udty = uda_22 * j;

			utx = udtx >> 10;
			uty = udty >> 10;
			udtx -= ( utx << 10 );
			udty -= ( uty << 10 );
			pucsrcp = lt_src_data + ((( uty * src_w ) + utx)<<2);

			ucgray1 = *pucsrcp++;
			ucgray2 = *pucsrcp++;
			ucgray3 = *pucsrcp++;
			ucgray4 = *pucsrcp;

			aa = ucgray1 << 20;
			ab = ( ( ucgray2 - ucgray1 ) * ( udtx << 10 ) );
			ac = ( ( ucgray3 - ucgray1 ) * ( udty << 10 ) );
			ad = ( ucgray1 + ucgray4 - ucgray3 - ucgray2 ) * udtx * udty;

			*( pucdstp + i ) = ( unsigned char ) ( ( aa + ab + ac + ad ) >> 20 );
		}
		pucdstp += des_w;
	}
	return true;
}

void MVFD_MT_Image2MCT(unsigned char* src_data, int src_w, int src_h, unsigned short* mct_data, int start_y, int end_y)
{
	start_y = start_y < 1 ? 1 : start_y;
	end_y = end_y > src_h-1 ? src_h-1 : end_y;
	if(start_y >= end_y) return;
	unsigned int m;
	unsigned int size = src_w*(end_y-start_y)-2;
	unsigned int data_mct;	
	unsigned char *m1 = src_data + (end_y-1)*src_w-2, *m2 = m1 + 1, *m3 = m2 + 1;
	unsigned char *m4 = m1 + src_w, *m5 = m4 + 1, *m6 = m5 + 1;
	unsigned char *m7 = m4 + src_w, *m8 = m7 + 1, *m9 = m8 + 1;
	unsigned short *pmct = mct_data+start_y*src_w+1+size;
	
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

void MVFD_CropRegion(unsigned char* src_data, int src_w, int src_h, FD_RECT roi, unsigned char *dst_data)
{
	int dst_w = roi.right-roi.left;
	int dst_h = roi.bottom-roi.top;
	for(int y = 0; y < dst_h; ++y)
	{
		fd_memcpy(&src_data[(roi.top+y)*src_w+roi.left], &dst_data[y*dst_w], dst_w);
	}
}

#define BIT_SHIFT 15
void MVFD_Resize_NN(unsigned char * __restrict src_data, int src_w, int src_h, unsigned char *__restrict des_data, int des_w, int des_h)
{
	int i;
	unsigned int utx, uty=0;
	unsigned int uda_11 = (src_w << BIT_SHIFT) / des_w;
	unsigned int uda_22 = (src_h << BIT_SHIFT) / des_h;
	unsigned char *__restrict psrc_data = src_data;
	unsigned char *__restrict pdes_data = des_data;
	//printf("FD: resizeNN sw=%d sh=%d dw=%d dh=%d\n",src_w,src_h,des_w,des_h);

	if(des_w>4)
	{
	    int des_w_4_rem;  

		des_w_4_rem = des_w&0x3;

		if(des_w_4_rem==0)
		{
			while(des_h--)
			{
				utx = 0;

				for(i = des_w;i>0;i-=4)
				{
					utx += uda_11;
					*pdes_data++ = *(psrc_data + (utx>>BIT_SHIFT));

					utx += uda_11;
					*pdes_data++ = *(psrc_data + (utx>>BIT_SHIFT));

					utx += uda_11;
					*pdes_data++ = *(psrc_data + (utx>>BIT_SHIFT));

					utx += uda_11;
					*pdes_data++ = *(psrc_data + (utx>>BIT_SHIFT));
				}
				uty += uda_22;
				psrc_data = src_data + (uty>>BIT_SHIFT)*src_w;
			}		
		
		}
	    else if(des_w_4_rem==1)
		{
			while(des_h--)
			{
				utx = 0;

				for(i = (des_w&0xfffffffc);i>0;i-=4)
				{
					utx += uda_11;
					*pdes_data++ = *(psrc_data + (utx>>BIT_SHIFT));

					utx += uda_11;
					*pdes_data++ = *(psrc_data + (utx>>BIT_SHIFT));

					utx += uda_11;
					*pdes_data++ = *(psrc_data + (utx>>BIT_SHIFT));

					utx += uda_11;
					*pdes_data++ = *(psrc_data + (utx>>BIT_SHIFT));
				}

				utx += uda_11;
				*pdes_data++ = *(psrc_data + (utx>>BIT_SHIFT));

				uty += uda_22;
				psrc_data = src_data + (uty>>BIT_SHIFT)*src_w;
			}			
		
		
		}
	    else if(des_w_4_rem==2)
		{
			while(des_h--)
			{
				utx = 0;

				for(i = (des_w&0xfffffffc);i>0;i-=4)
				{
					utx += uda_11;
					*pdes_data++ = *(psrc_data + (utx>>BIT_SHIFT));

					utx += uda_11;
					*pdes_data++ = *(psrc_data + (utx>>BIT_SHIFT));

					utx += uda_11;
					*pdes_data++ = *(psrc_data + (utx>>BIT_SHIFT));

					utx += uda_11;
					*pdes_data++ = *(psrc_data + (utx>>BIT_SHIFT));
				}

				utx += uda_11;
				*pdes_data++ = *(psrc_data + (utx>>BIT_SHIFT));

				utx += uda_11;
				*pdes_data++ = *(psrc_data + (utx>>BIT_SHIFT));

				uty += uda_22;
				psrc_data = src_data + (uty>>BIT_SHIFT)*src_w;
			}			
		
		}
		else
		{
			while(des_h--)
			{
				utx = 0;

				for(i = (des_w&0xfffffffc);i>0;i-=4)
				{
					utx += uda_11;
					*pdes_data++ = *(psrc_data + (utx>>BIT_SHIFT));

					utx += uda_11;
					*pdes_data++ = *(psrc_data + (utx>>BIT_SHIFT));

					utx += uda_11;
					*pdes_data++ = *(psrc_data + (utx>>BIT_SHIFT));

					utx += uda_11;
					*pdes_data++ = *(psrc_data + (utx>>BIT_SHIFT));
				}

				utx += uda_11;
				*pdes_data++ = *(psrc_data + (utx>>BIT_SHIFT));

				utx += uda_11;
				*pdes_data++ = *(psrc_data + (utx>>BIT_SHIFT));

				utx += uda_11;
				*pdes_data++ = *(psrc_data + (utx>>BIT_SHIFT));

				uty += uda_22;
				psrc_data = src_data + (uty>>BIT_SHIFT)*src_w;
			}			
		
		
		}
	}
	else
	{
			while(des_h--)
			{
				utx = 0;
				i = des_w;
				while(i--) 
				{
					utx += uda_11;
					*pdes_data++ = *(psrc_data + (utx>>BIT_SHIFT));
				}
				uty += uda_22;
				psrc_data = src_data + (uty>>BIT_SHIFT)*src_w;
			}
	
	}


}

#endif

