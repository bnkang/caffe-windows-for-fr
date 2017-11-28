#include <stdlib.h>
//#include <memory.h>
#include "MVFD_Defines.h"
#include "MVFD_Memory.h"
//#include "performance_checker.h"

#ifdef USE_NEON
extern "C" void MVFD_integral_image_gen_part_arm(unsigned char * ipos, int * p0, int * p1, int width);
extern "C" void MVFD_ImageTranspose_arm(unsigned char* src_data, int src_w, int src_h, unsigned char* rot_data);
extern "C" void MVFD_ImageTranspose_neon(unsigned char *pSrc, int src_w, int src_h, unsigned char *pDes);

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
#if 0
		for(int x = width; x ; --x)
		{
			s = s + *(ipos++);
			*(p0++) = s + *(p1++);
		}
		p0 += delta; p1 += delta;
#else
		MVFD_integral_image_gen_part_arm(ipos, p0, p1, width);
		ipos += width;
		p0 += aligned_width;
		p1 += aligned_width;
#endif 

	}

	for(int i = 0; i < height; ++i)
	{
		fd_memcpy(&feature_buf[i*aligned_width], &image[i*width], width);
	}
#if 0
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
#else
	int scale = 2;		//1,2,4,8
	//for(int i = 1; i < 4; ++i)
	//i = 1
	{
		unsigned char *obuffer = &feature_buf[aligned_width*height];
		int new_w = iimg_w-scale;
		int new_h = iimg_h-scale;
		int loop_cnt = (new_w+7)/8;
		for(int y = 0; y < new_h; ++y)
		{
			int *LT_pos = integral_img_buf+y*aligned_width;
			int *RT_pos = LT_pos+scale;
			int *LB_pos = LT_pos+scale*aligned_width;
			int *RB_pos = LB_pos+scale;
			unsigned char *optr = obuffer+y*aligned_width;
			int lp = loop_cnt;
			while(lp--)
			{
				int32x4_t RB_32x4,LB_32x4,RT_32x4,LT_32x4;
				int32x4_t temp1_32x4,temp2_32x4,temp3_32x4;
				uint16x4_t temp4_16x4,temp5_16x4;
				uint8x8_t result_8x8;

				RB_32x4 = vld1q_s32(RB_pos);
				LB_32x4 = vld1q_s32(LB_pos);
				RT_32x4 = vld1q_s32(RT_pos);
				LT_32x4 = vld1q_s32(LT_pos);
				temp1_32x4 = vaddq_s32(RB_32x4, LT_32x4);
				temp2_32x4 = vaddq_s32(LB_32x4, RT_32x4);
				temp3_32x4 = vsubq_s32(temp1_32x4, temp2_32x4);
				temp4_16x4 = vqshrun_n_s32(temp3_32x4, 1);

				RB_32x4 = vld1q_s32(RB_pos + 4);
				LB_32x4 = vld1q_s32(LB_pos + 4);
				RT_32x4 = vld1q_s32(RT_pos + 4);
				LT_32x4 = vld1q_s32(LT_pos + 4);
				temp1_32x4 = vaddq_s32(RB_32x4, LT_32x4);
				temp2_32x4 = vaddq_s32(LB_32x4, RT_32x4);
				temp3_32x4 = vsubq_s32(temp1_32x4, temp2_32x4);
				temp5_16x4 = vqshrun_n_s32(temp3_32x4, 1);

				result_8x8 = vshrn_n_u16(vcombine_u16(temp4_16x4, temp5_16x4), 1);
				vst1_u8(optr, result_8x8);

				RB_pos =	RB_pos + 8;
				LB_pos =	LB_pos + 8;
				RT_pos =	RT_pos + 8;
				LT_pos =	LT_pos + 8;
				optr = optr + 8;
				
				//*(optr++) = (*(RB_pos++) - *(LB_pos++) - *(RT_pos++) + *(LT_pos++))>>(1+1);
			}
			//obuffer += scale-1+delta; LT_pos += scale+delta; RT_pos += scale+delta; LB_pos += scale+delta; RB_pos += scale+delta;
		}
		scale <<= 1;
	}
	// i = 2
	{
		unsigned char *obuffer = &feature_buf[2*aligned_width*height];
		int new_w = iimg_w-scale;
		int new_h = iimg_h-scale;
		int loop_cnt = (new_w+7)/8;
		for(int y = 0; y < new_h; ++y)
		{
			int *LT_pos = integral_img_buf+y*aligned_width;
			int *RT_pos = LT_pos+scale;
			int *LB_pos = LT_pos+scale*aligned_width;
			int *RB_pos = LB_pos+scale;
			unsigned char *optr = obuffer+y*aligned_width;
			int lp = loop_cnt;
			while(lp--)
			{
				int32x4_t RB_32x4,LB_32x4,RT_32x4,LT_32x4;
				int32x4_t temp1_32x4,temp2_32x4,temp3_32x4;
				uint16x4_t temp4_16x4,temp5_16x4;
				uint8x8_t result_8x8;

				RB_32x4 = vld1q_s32(RB_pos);
				LB_32x4 = vld1q_s32(LB_pos);
				RT_32x4 = vld1q_s32(RT_pos);
				LT_32x4 = vld1q_s32(LT_pos);
				temp1_32x4 = vaddq_s32(RB_32x4, LT_32x4);
				temp2_32x4 = vaddq_s32(LB_32x4, RT_32x4);
				temp3_32x4 = vsubq_s32(temp1_32x4, temp2_32x4);
				temp4_16x4 = vqshrun_n_s32(temp3_32x4, 2);

				RB_32x4 = vld1q_s32(RB_pos + 4);
				LB_32x4 = vld1q_s32(LB_pos + 4);
				RT_32x4 = vld1q_s32(RT_pos + 4);
				LT_32x4 = vld1q_s32(LT_pos + 4);
				temp1_32x4 = vaddq_s32(RB_32x4, LT_32x4);
				temp2_32x4 = vaddq_s32(LB_32x4, RT_32x4);
				temp3_32x4 = vsubq_s32(temp1_32x4, temp2_32x4);
				temp5_16x4 = vqshrun_n_s32(temp3_32x4, 2);

				result_8x8 = vshrn_n_u16(vcombine_u16(temp4_16x4, temp5_16x4), 2);
				vst1_u8(optr, result_8x8);

				RB_pos =	RB_pos + 8;
				LB_pos =	LB_pos + 8;
				RT_pos =	RT_pos + 8;
				LT_pos =	LT_pos + 8;
				optr = optr + 8;
				
				//*(optr++) = (*(RB_pos++) - *(LB_pos++) - *(RT_pos++) + *(LT_pos++))>>(2+2);
			}
			//obuffer += scale-1+delta; LT_pos += scale+delta; RT_pos += scale+delta; LB_pos += scale+delta; RB_pos += scale+delta;
		}
		scale <<= 1;		
	}
	// i = 3
	{
		unsigned char *obuffer = &feature_buf[3*aligned_width*height];
		int new_w = iimg_w-scale;
		int new_h = iimg_h-scale;
		int loop_cnt = (new_w+7)/8;
		for(int y = 0; y < new_h; ++y)
		{
			int *LT_pos = integral_img_buf+y*aligned_width;
			int *RT_pos = LT_pos+scale;
			int *LB_pos = LT_pos+scale*aligned_width;
			int *RB_pos = LB_pos+scale;
			unsigned char *optr = obuffer+y*aligned_width;
			int lp = loop_cnt;
			while(lp--)
			{
				int32x4_t RB_32x4,LB_32x4,RT_32x4,LT_32x4;
				int32x4_t temp1_32x4,temp2_32x4,temp3_32x4;
				uint16x4_t temp4_16x4,temp5_16x4;
				uint8x8_t result_8x8;

				RB_32x4 = vld1q_s32(RB_pos);
				LB_32x4 = vld1q_s32(LB_pos);
				RT_32x4 = vld1q_s32(RT_pos);
				LT_32x4 = vld1q_s32(LT_pos);
				temp1_32x4 = vaddq_s32(RB_32x4, LT_32x4);
				temp2_32x4 = vaddq_s32(LB_32x4, RT_32x4);
				temp3_32x4 = vsubq_s32(temp1_32x4, temp2_32x4);
				temp4_16x4 = vqshrun_n_s32(temp3_32x4, 3);

				RB_32x4 = vld1q_s32(RB_pos + 4);
				LB_32x4 = vld1q_s32(LB_pos + 4);
				RT_32x4 = vld1q_s32(RT_pos + 4);
				LT_32x4 = vld1q_s32(LT_pos + 4);
				temp1_32x4 = vaddq_s32(RB_32x4, LT_32x4);
				temp2_32x4 = vaddq_s32(LB_32x4, RT_32x4);
				temp3_32x4 = vsubq_s32(temp1_32x4, temp2_32x4);
				temp5_16x4 = vqshrun_n_s32(temp3_32x4, 3);

				result_8x8 = vshrn_n_u16(vcombine_u16(temp4_16x4, temp5_16x4), 3);
				vst1_u8(optr, result_8x8);

				RB_pos =	RB_pos + 8;
				LB_pos =	LB_pos + 8;
				RT_pos =	RT_pos + 8;
				LT_pos =	LT_pos + 8;
				optr = optr + 8;
				
				//*(optr++) = (*(RB_pos++) - *(LB_pos++) - *(RT_pos++) + *(LT_pos++))>>(3+3);
			}
			//obuffer += scale-1+delta; LT_pos += scale+delta; RT_pos += scale+delta; LB_pos += scale+delta; RB_pos += scale+delta;
		}
		scale <<= 1;		
	}
#endif
}

void MVFD_ImageRotate(unsigned char* src_data, int src_w, int src_h, unsigned char* rot_data)
{
#if 0
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
#else
	unsigned char *sp = src_data + src_w*src_h -1;
	unsigned char *rp_back = rot_data + src_w*src_h -1;
	for(int y = src_h; y; y--)
	{
		int x = src_w;
		unsigned char *rp = rp_back;
		while(x--)
		{	
			*rp = *sp--;
			rp -= src_h;
		}
		rp_back --;
	}

#endif
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
	int ucgray1 = 0, ucgray2 = 0, ucgray3 = 0, ucgray4 = 0;
	unsigned int aa, ab, ac, ad;
	
	unsigned char *pucsrcp = 0;
	unsigned char *pucdstp = des_data+start_y*des_w;
	unsigned char *lt_src_data = src_data+roi.top*src_w+roi.left;

	/* Affine transformation */
	uda_11 = ( (roi.right-roi.left) << 10 ) / des_w;
	uda_22 = ( (roi.bottom-roi.top) << 10 ) / des_h;
	
	int32x4_t uda_11_32x4,uda_22_32x4;
	int32x4_t lt_src_data_32x4;
	
	lt_src_data_32x4 = vdupq_n_s32((int32_t)lt_src_data);
	uda_11_32x4 = vdupq_n_s32(uda_11);
	uda_22_32x4 = vdupq_n_s32(uda_22);
	
	for ( j = start_y; j < end_y; j++ ) {
		if( j >= des_h - 1)
		{
			for ( i = 0; i < des_w - 1; i++ ) {
	
			udtx = uda_11 * i;
			udty = uda_22 * j;
	
			utx = udtx >> 10;
			uty = udty >> 10;
			udtx -= ( utx << 10 );
			udty -= ( uty << 10 );
			pucsrcp = lt_src_data + ( uty * src_w ) + utx;
	
			ucgray1 = *pucsrcp;
				ucgray2 = *( pucsrcp + 1 );
				ucgray3 = *pucsrcp;
				ucgray4 = *( pucsrcp + 1 );

				udty = 1 << 10;
	
				aa = ucgray1 << 20;
				ab = ( ( ucgray2 - ucgray1 ) * ( udtx << 10 ) );
				ac = ( ( ucgray3 - ucgray1 ) * ( udty << 10 ) );
				ad = ( ucgray1 + ucgray4 - ucgray3 - ucgray2 ) * udtx * udty;
	
				*( pucdstp + i ) = ( unsigned char ) ( ( aa + ab + ac + ad ) >> 20 );
	
			}
			{
				udtx = uda_11 * i;
				udty = uda_22 * j;
	
				utx = udtx >> 10;
				uty = udty >> 10;
				udtx -= ( utx << 10 );
				udty -= ( uty << 10 );
				pucsrcp = lt_src_data + ( uty * src_w ) + utx;
	
				ucgray1 = *pucsrcp;
				ucgray2 = *pucsrcp;
				ucgray3 = *pucsrcp;
				ucgray4 = *pucsrcp;

				udtx = 1 << 10;
				udty = 1 << 10;
	
				aa = ucgray1 << 20;
				ab = ( ( ucgray2 - ucgray1 ) * ( udtx << 10 ) );
				ac = ( ( ucgray3 - ucgray1 ) * ( udty << 10 ) );
				ad = ( ucgray1 + ucgray4 - ucgray3 - ucgray2 ) * udtx * udty;
	
				*( pucdstp + i ) = ( unsigned char ) ( ( aa + ab + ac + ad ) >> 20 );
	
			}
			
			pucdstp += des_w;
			}
		else
		{
			int32x4_t i_32x4,iconst_32x4;
			int32x4_t udtx_32x4,udty_32x4,utx_32x4,uty_32x4;
			int32x4_t temp_const_32x4;
			i_32x4 = vcombine_s32(vcreate_s32(0x100000000), vcreate_s32(0x300000002));
			iconst_32x4 = vdupq_n_s32(4);
			//j_32x4 = vdupq_n_s32(j);
			temp_const_32x4 = vdupq_n_s32(0x3FF);

			for ( i = 0; i < (((des_w - 1)>>2)<<2); i+= 4 ) {

				int32x4_t pucsrcp_32x4,dest_32x4;
				//uint8x8x4_t temp1_8x8x4,temp2_8x8x4;
				int32x4_t ucgray1_32x4,ucgray2_32x4,ucgray3_32x4,ucgray4_32x4;
				int32x4_t aa_32x4,ab_32x4,ac_32x4,ad_32x4;
				unsigned char *pucsrcp[4];
				unsigned char *pucsrcp1[4];
				unsigned char *pucsrcp2[4];
				unsigned char *pucsrcp3[4];
				int ucgray[16];
				int temp1,temp2,temp3,temp4;
				
				//udtx = uda_11 * i;
				//udty = uda_22 * j;
				udtx_32x4 = vmulq_s32(uda_11_32x4, i_32x4);
				i_32x4 = vaddq_s32(i_32x4, iconst_32x4);
				udty_32x4 = vmulq_n_s32(uda_22_32x4, j);
	
				//utx = udtx >> 10;
				//uty = udty >> 10;
				utx_32x4 = vshrq_n_s32(udtx_32x4, 10);
				uty_32x4 = vshrq_n_s32(udty_32x4, 10);
				
				//udtx -= ( utx << 10 );
				//udty -= ( uty << 10 );
				//udtx = ( udtx & 0x3FF );
				//udty = ( udty & 0x3FF );
				udtx_32x4 = vandq_s32(udtx_32x4, temp_const_32x4);
				udty_32x4 = vandq_s32(udty_32x4, temp_const_32x4);				
				
				//pucsrcp = lt_src_data + ( uty * src_w ) + utx;
				pucsrcp_32x4 = vmlaq_n_s32(utx_32x4, uty_32x4, src_w);
				pucsrcp_32x4 = vaddq_s32(lt_src_data_32x4, pucsrcp_32x4);
	
#if 0
				vst1q_s32((int32_t *)pucsrcp, pucsrcp_32x4);
				pucsrcp_32x4 = vaddq_s32(pucsrcp_32x4, vdupq_n_s32(src_w));
				vst1q_s32((int32_t *)pucsrcp1, pucsrcp_32x4);	
				
				ucgray[0] = *(pucsrcp[0]);
				ucgray[1] = *(pucsrcp[1]);
				ucgray[2] = *(pucsrcp[2]);
				ucgray[3] = *(pucsrcp[3]);
				
				ucgray[4] = *(pucsrcp[0] + 1);
				ucgray[5] = *(pucsrcp[1] + 1);
				ucgray[6] = *(pucsrcp[2] + 1);
				ucgray[7] = *(pucsrcp[3] + 1);			
				
				ucgray[8] = *(pucsrcp1[0]);
				ucgray[9] = *(pucsrcp1[1]);
				ucgray[10] = *(pucsrcp1[2]);
				ucgray[11] = *(pucsrcp1[3]);
				
				ucgray[12] = *(pucsrcp1[0] + 1);
				ucgray[13] = *(pucsrcp1[1] + 1);
				ucgray[14] = *(pucsrcp1[2] + 1);
				ucgray[15] = *(pucsrcp1[3] + 1);	
				
				ucgray1_32x4 = vld1q_s32(ucgray);
				ucgray2_32x4 = vld1q_s32(ucgray + 4);
				ucgray3_32x4 = vld1q_s32(ucgray + 8);
				ucgray4_32x4 = vld1q_s32(ucgray + 12);
				
#else
				ucgray1_32x4 = vdupq_n_s32(0);
				ucgray2_32x4 = vdupq_n_s32(0);
				ucgray3_32x4 = vdupq_n_s32(0);
				ucgray4_32x4 = vdupq_n_s32(0);
				
				vst1q_s32((int32_t *)pucsrcp, pucsrcp_32x4);
				
				ucgray1_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp[0]), vreinterpretq_u8_s32(ucgray1_32x4), 0));
				ucgray1_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp[1]), vreinterpretq_u8_s32(ucgray1_32x4), 4));
				ucgray1_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp[2]), vreinterpretq_u8_s32(ucgray1_32x4), 8));
				ucgray1_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp[3]), vreinterpretq_u8_s32(ucgray1_32x4), 12));
				
				pucsrcp_32x4 = vaddq_s32(pucsrcp_32x4, vdupq_n_s32(1));
				vst1q_s32((int32_t *)pucsrcp1, pucsrcp_32x4);
				
				ucgray2_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp1[0]), vreinterpretq_u8_s32(ucgray2_32x4), 0));
				ucgray2_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp1[1]), vreinterpretq_u8_s32(ucgray2_32x4), 4));
				ucgray2_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp1[2]), vreinterpretq_u8_s32(ucgray2_32x4), 8));
				ucgray2_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp1[3]), vreinterpretq_u8_s32(ucgray2_32x4), 12));
				
				pucsrcp_32x4 = vaddq_s32(pucsrcp_32x4, vdupq_n_s32(src_w));
				vst1q_s32((int32_t *)pucsrcp2, pucsrcp_32x4);
				
				ucgray4_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp2[0]), vreinterpretq_u8_s32(ucgray4_32x4), 0));
				ucgray4_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp2[1]), vreinterpretq_u8_s32(ucgray4_32x4), 4));
				ucgray4_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp2[2]), vreinterpretq_u8_s32(ucgray4_32x4), 8));
				ucgray4_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp2[3]), vreinterpretq_u8_s32(ucgray4_32x4), 12));

				pucsrcp_32x4 = vsubq_s32(pucsrcp_32x4, vdupq_n_s32(1));
				vst1q_s32((int32_t *)pucsrcp3, pucsrcp_32x4);

				ucgray3_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp3[0]), vreinterpretq_u8_s32(ucgray3_32x4), 0));
				ucgray3_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp3[1]), vreinterpretq_u8_s32(ucgray3_32x4), 4));
				ucgray3_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp3[2]), vreinterpretq_u8_s32(ucgray3_32x4), 8));
				ucgray3_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp3[3]), vreinterpretq_u8_s32(ucgray3_32x4), 12));
				
#endif
				//aa = ucgray1 << 20;
				aa_32x4 = vshlq_n_s32(ucgray1_32x4, 20);
				
				//ad = ( ucgray1 + ucgray4 - ucgray3 - ucgray2 ) * udtx * udty;
				ad_32x4 = vaddq_s32(ucgray1_32x4, ucgray4_32x4);
				ad_32x4 = vsubq_s32(ad_32x4, ucgray3_32x4);
				ad_32x4 = vsubq_s32(ad_32x4, ucgray2_32x4);
				ad_32x4 = vmulq_s32(ad_32x4, udtx_32x4);
				ad_32x4 = vmulq_s32(ad_32x4, udty_32x4);				
				
				udtx_32x4 = vshlq_n_s32(udtx_32x4, 10);
				udty_32x4 = vshlq_n_s32(udty_32x4, 10);

				//ab = ( ( ucgray2 - ucgray1 ) * ( udtx << 10 ) );
				ab_32x4 = vsubq_s32(ucgray2_32x4, ucgray1_32x4);
				ab_32x4 = vmulq_s32(ab_32x4, udtx_32x4);
				
				//ac = ( ( ucgray3 - ucgray1 ) * ( udty << 10 ) );
				ac_32x4 = vsubq_s32(ucgray3_32x4, ucgray1_32x4);
				ac_32x4 = vmulq_s32(ac_32x4, udty_32x4);
				
				//*( pucdstp + i ) = ( unsigned char ) ( ( aa + ab + ac + ad ) >> 20 );
				dest_32x4 = vaddq_s32(aa_32x4, ab_32x4);
				dest_32x4 = vaddq_s32(dest_32x4, ac_32x4);
				dest_32x4 = vaddq_s32(dest_32x4, ad_32x4);

				dest_32x4 = vshrq_n_s32(dest_32x4, 20);				
				if(((int)(pucdstp)) == (((int)pucdstp & 0xFFFFFFFC)))
				{
					uint8x8x2_t temp_8x8x2;
					
					temp_8x8x2.val[0] = vreinterpret_u8_s32(vget_low_s32(dest_32x4));
					temp_8x8x2.val[1] = vreinterpret_u8_s32(vget_high_s32(dest_32x4));
					
					temp_8x8x2 = vzip_u8(temp_8x8x2.val[0],temp_8x8x2.val[1]);
					temp_8x8x2 = vzip_u8(temp_8x8x2.val[0],temp_8x8x2.val[1]);
					vst1_lane_u32((uint32_t *)( pucdstp + i ), vreinterpret_u32_u8(temp_8x8x2.val[0]), 0);
				}
				else
				{
					vst1q_s32(ucgray, dest_32x4);
					*( pucdstp + i ) =  ucgray[0];
					*( pucdstp + i +1) =  ucgray[1];
					*( pucdstp + i +2) =  ucgray[2];
					*( pucdstp + i +3) =  ucgray[3];
				}

			}
			for (; i < (des_w - 1); i++ ) {
				udtx = uda_11 * i;
				udty = uda_22 * j;
	
				utx = udtx >> 10;
				uty = udty >> 10;
				udtx -= ( utx << 10 );
				udty -= ( uty << 10 );
				pucsrcp = lt_src_data + ( uty * src_w ) + utx;
	
				ucgray1 = *pucsrcp;
				ucgray2 = *( pucsrcp + 1 );
				ucgray3 = *( pucsrcp + src_w );
				ucgray4 = *( pucsrcp + src_w + 1 );
							
	
				aa = ucgray1 << 20;
				ab = ( ( ucgray2 - ucgray1 ) * ( udtx << 10 ) );
				ac = ( ( ucgray3 - ucgray1 ) * ( udty << 10 ) );
				ad = ( ucgray1 + ucgray4 - ucgray3 - ucgray2 ) * udtx * udty;
	
				*( pucdstp + i ) = ( unsigned char ) ( ( aa + ab + ac + ad ) >> 20 );
	
			}
			
			{

				udtx = uda_11 * i;
				udty = uda_22 * j;
	
				utx = udtx >> 10;
				uty = udty >> 10;
				udtx -= ( utx << 10 );
				udty -= ( uty << 10 );
				
				pucsrcp = lt_src_data + ( uty * src_w ) + utx;
	
				ucgray1 = *pucsrcp;
				ucgray2 = *pucsrcp;
				ucgray3 = *( pucsrcp + src_w );
				ucgray4 = *( pucsrcp + src_w );
							
				udtx = 1 << 10;
	
				aa = ucgray1 << 20;
				ab = ( ( ucgray2 - ucgray1 ) * ( udtx << 10 ) );
				ac = ( ( ucgray3 - ucgray1 ) * ( udty << 10 ) );
				ad = ( ucgray1 + ucgray4 - ucgray3 - ucgray2 ) * udtx * udty;
	
				*( pucdstp + i ) = ( unsigned char ) ( ( aa + ab + ac + ad ) >> 20 );
	
			}
			pucdstp += des_w;
		}
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

#if 1
void MVFD_Make_ResizeBI_Map(unsigned char *src_data, int src_w, int src_h, unsigned char *dest)
{
	unsigned char *srcp = src_data;
	unsigned char *destp = dest;
	int aligned_w = (src_w-1) & 0xFFFFFFF0;
	for(int y = 0; y < src_h-1; ++y)
	{

		for(int x = 0; x < aligned_w; x+=16)
		{
#if 1
			//LOGD("x = %d\n",x);
			uint8x16_t src1_8x16,src2_8x16,src3_8x16,src4_8x16;
			uint8x16x2_t src1_8x16x2,src2_8x16x2;
			uint8x16x2_t src3_8x16x2,src4_8x16x2;
			//int dx = (x < src_w-1);
			//int dy = (y < src_h-1) ? src_w : 0;
			src1_8x16 = vld1q_u8(srcp);
			src2_8x16 = vld1q_u8(srcp + 1);
			src3_8x16 = vld1q_u8(srcp + src_w);
			src4_8x16 = vld1q_u8(srcp + src_w + 1);
			
			
			//vst4q_u8(destp, src_8x16x4);
			src1_8x16x2 = vzipq_u8(src1_8x16, src3_8x16);
			src2_8x16x2 = vzipq_u8(src2_8x16, src4_8x16);
			
			src3_8x16x2 = vzipq_u8(src1_8x16x2.val[0],src2_8x16x2.val[0]);
			src4_8x16x2 = vzipq_u8(src1_8x16x2.val[1],src2_8x16x2.val[1]);
			
			vst1q_u8(destp, src3_8x16x2.val[0]);
			vst1q_u8(destp+16, src3_8x16x2.val[1]);
			vst1q_u8(destp+32, src4_8x16x2.val[0]);
			vst1q_u8(destp+48, src4_8x16x2.val[1]);
			
			srcp += 16;
			destp += 16*4;

#else
		
			for(int i = 0; i < 16; i++)
			{
				//LOGD("x = %d\n",x);
				//int dx = (x < src_w-1);
				//int dy = (y < src_h-1) ? src_w : 0;
				*destp++ = *srcp;
				*destp++ = *(srcp+1);
				*destp++ = *(srcp+src_w);
				*destp++ = *(srcp+src_w+1);
				++srcp;
			}

#endif			
		}
		//LOGD("===========\n");
		for(int x = aligned_w; x < src_w; ++x)
		{
			//LOGD("x = %d\n",x);
			int dx = (x < src_w-1);
			//int dy = (y < src_h-1) ? src_w : 0;
			*destp++ = *srcp;
			*destp++ = *(srcp+dx);
			*destp++ = *(srcp+src_w);
			*destp++ = *(srcp+src_w+dx);
			++srcp;
		}
	}
	for(int x = 0; x < src_w; ++x)
	{
		int dx = (x < src_w-1);
		//int dy = (y < src_h-1) ? src_w : 0;
		*destp++ = *srcp;
		*destp++ = *(srcp+dx);
		*destp++ = *(srcp);
		*destp++ = *(srcp+dx);
		++srcp;
	}
}
#else

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

#endif

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
	//int locgray4 = 0;
	int ucgray1 = 0, ucgray2 = 0, ucgray3 = 0, ucgray4 = 0;
	unsigned int aa, ab, ac, ad;

#if 1
//	unsigned char *pucsrcp = 0;
	unsigned char *pucdstp = des_data+start_y*des_w;
	unsigned char *lt_src_data = src_data+((roi.top*src_w+roi.left)<<2);

	/* Affine transformation */
	uda_11 = ( (roi.right-roi.left) << 10 ) / des_w;
	uda_22 = ( (roi.bottom-roi.top) << 10 ) / des_h;

	int des_align4_w = ((des_w-1)>>2)<<2;
	int32x4_t uda_11_32x4,uda_22_32x4;
	int32x4_t lt_src_data_32x4;
	
	lt_src_data_32x4 = vdupq_n_s32((int32_t)lt_src_data);
	uda_11_32x4 = vdupq_n_s32(uda_11);
	uda_22_32x4 = vdupq_n_s32(uda_22);

	for ( j = start_y; j < end_y; j++ ) 
	{
		int32x4_t i_32x4,iconst_32x4, j_32x4;
		int32x4_t udtx_32x4,udty_32x4,utx_32x4,uty_32x4;
		int32x4_t temp_const_32x4;
		i_32x4 = vcombine_s32(vcreate_s32(0x100000000), vcreate_s32(0x300000002));
		iconst_32x4 = vdupq_n_s32(4);
		j_32x4 = vdupq_n_s32(j);
		temp_const_32x4 = vdupq_n_s32(0x3FF);
		for ( i = 0; i < des_align4_w; i+=4 ) 
		{
			int32x4_t pucsrcp_32x4,dest_32x4;
			//uint8x8x4_t temp1_8x8x4,temp2_8x8x4;
			int32x4_t ucgray1_32x4,ucgray2_32x4,ucgray3_32x4,ucgray4_32x4;
			int32x4_t aa_32x4,ab_32x4,ac_32x4,ad_32x4;
			unsigned char *pucsrcp[4];
			unsigned char *pucsrcp1[4];
			unsigned char *pucsrcp2[4];
			unsigned char *pucsrcp3[4];
			
			int ucgray[16];
			int temp1,temp2,temp3,temp4;
			
			//udtx = uda_11 * i;
			//udty = uda_22 * j;
			udtx_32x4 = vmulq_s32(uda_11_32x4, i_32x4);
			i_32x4 = vaddq_s32(i_32x4, iconst_32x4);
			udty_32x4 = vmulq_s32(uda_22_32x4, j_32x4);

			//utx = udtx >> 10;
			//uty = udty >> 10;
			utx_32x4 = vshrq_n_s32(udtx_32x4, 10);
			uty_32x4 = vshrq_n_s32(udty_32x4, 10);
			
			//udtx -= ( utx << 10 );
			//udty -= ( uty << 10 );
			//udtx = ( udtx & 0x3FF );
			//udty = ( udty & 0x3FF );
			udtx_32x4 = vandq_s32(udtx_32x4, temp_const_32x4);
			udty_32x4 = vandq_s32(udty_32x4, temp_const_32x4);
			
			//pucsrcp = lt_src_data + ((( uty * src_w ) + utx) << 2);
			pucsrcp_32x4 = vmlaq_n_s32(utx_32x4, uty_32x4, src_w);
			pucsrcp_32x4 = vmlaq_n_s32(lt_src_data_32x4, pucsrcp_32x4, 4);
			
			
			
			//ucgray1 = *pucsrcp++;
			//ucgray2 = *pucsrcp++;
			//ucgray3 = *pucsrcp++;
			//ucgray4 = *pucsrcp;
			ucgray1_32x4 = vdupq_n_s32(0);
			ucgray2_32x4 = vdupq_n_s32(0);
			ucgray3_32x4 = vdupq_n_s32(0);
			ucgray4_32x4 = vdupq_n_s32(0);
			
			vst1q_s32((int32_t *)pucsrcp, pucsrcp_32x4);
			
			ucgray1_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp[0]), vreinterpretq_u8_s32(ucgray1_32x4), 0));
			ucgray1_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp[1]), vreinterpretq_u8_s32(ucgray1_32x4), 4));
			ucgray1_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp[2]), vreinterpretq_u8_s32(ucgray1_32x4), 8));
			ucgray1_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp[3]), vreinterpretq_u8_s32(ucgray1_32x4), 12));
			
			pucsrcp_32x4 = vaddq_s32(pucsrcp_32x4, vdupq_n_s32(1));
			vst1q_s32((int32_t *)pucsrcp1, pucsrcp_32x4);
			
			ucgray2_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp1[0]), vreinterpretq_u8_s32(ucgray2_32x4), 0));
			ucgray2_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp1[1]), vreinterpretq_u8_s32(ucgray2_32x4), 4));
			ucgray2_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp1[2]), vreinterpretq_u8_s32(ucgray2_32x4), 8));
			ucgray2_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp1[3]), vreinterpretq_u8_s32(ucgray2_32x4), 12));

			pucsrcp_32x4 = vaddq_s32(pucsrcp_32x4, vdupq_n_s32(1));
			vst1q_s32((int32_t *)pucsrcp3, pucsrcp_32x4);

			ucgray3_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp3[0]), vreinterpretq_u8_s32(ucgray3_32x4), 0));
			ucgray3_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp3[1]), vreinterpretq_u8_s32(ucgray3_32x4), 4));
			ucgray3_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp3[2]), vreinterpretq_u8_s32(ucgray3_32x4), 8));
			ucgray3_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp3[3]), vreinterpretq_u8_s32(ucgray3_32x4), 12));
			
			
			pucsrcp_32x4 = vaddq_s32(pucsrcp_32x4, vdupq_n_s32(1));
			vst1q_s32((int32_t *)pucsrcp2, pucsrcp_32x4);
			
			ucgray4_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp2[0]), vreinterpretq_u8_s32(ucgray4_32x4), 0));
			ucgray4_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp2[1]), vreinterpretq_u8_s32(ucgray4_32x4), 4));
			ucgray4_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp2[2]), vreinterpretq_u8_s32(ucgray4_32x4), 8));
			ucgray4_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp2[3]), vreinterpretq_u8_s32(ucgray4_32x4), 12));

			//aa = ucgray1 << 20;
			aa_32x4 = vshlq_n_s32(ucgray1_32x4, 20);
			
			//ad = ( ucgray1 + ucgray4 - ucgray3 - ucgray2 ) * udtx * udty;
			ad_32x4 = vaddq_s32(ucgray1_32x4, ucgray4_32x4);
			ad_32x4 = vsubq_s32(ad_32x4, ucgray3_32x4);
			ad_32x4 = vsubq_s32(ad_32x4, ucgray2_32x4);
			ad_32x4 = vmulq_s32(ad_32x4, udtx_32x4);
			ad_32x4 = vmulq_s32(ad_32x4, udty_32x4);
			
			udtx_32x4 = vshlq_n_s32(udtx_32x4, 10);
			udty_32x4 = vshlq_n_s32(udty_32x4, 10);
			
			//ab = ( ( ucgray2 - ucgray1 ) * ( udtx << 10 ) );
			ab_32x4 = vsubq_s32(ucgray2_32x4, ucgray1_32x4);
			ab_32x4 = vmulq_s32(ab_32x4, udtx_32x4);
			
			//ac = ( ( ucgray3 - ucgray1 ) * ( udty << 10 ) );
			ac_32x4 = vsubq_s32(ucgray3_32x4, ucgray1_32x4);
			ac_32x4 = vmulq_s32(ac_32x4, udty_32x4);

			//*( pucdstp + i ) = ( unsigned char ) ( ( aa + ab + ac + ad ) >> 20 );
			
			dest_32x4 = vaddq_s32(aa_32x4, ab_32x4);
			dest_32x4 = vaddq_s32(dest_32x4, ac_32x4);
			dest_32x4 = vaddq_s32(dest_32x4, ad_32x4);

			dest_32x4 = vshrq_n_s32(dest_32x4, 20);
			
			
			if(((int)(pucdstp)) == (((int)pucdstp & 0xFFFFFFFC)))
			{
				uint8x8x2_t temp_8x8x2;
				
				temp_8x8x2.val[0] = vreinterpret_u8_s32(vget_low_s32(dest_32x4));
				temp_8x8x2.val[1] = vreinterpret_u8_s32(vget_high_s32(dest_32x4));
				
				temp_8x8x2 = vzip_u8(temp_8x8x2.val[0],temp_8x8x2.val[1]);
				temp_8x8x2 = vzip_u8(temp_8x8x2.val[0],temp_8x8x2.val[1]);
				vst1_lane_u32((uint32_t *)( pucdstp + i ), vreinterpret_u32_u8(temp_8x8x2.val[0]), 0);
			}
			else
			{
				vst1q_s32(ucgray, dest_32x4);
				*( pucdstp + i ) =  ucgray[0];
				*( pucdstp + i +1) =  ucgray[1];
				*( pucdstp + i +2) =  ucgray[2];
				*( pucdstp + i +3) =  ucgray[3];
			}
		
		}
		for ( i = des_align4_w; i < des_w; i++ ) {
			unsigned char *pucsrcp = 0;
			
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
			ucgray4 = *pucsrcp;
					

			aa = ucgray1 << 20;
			ab = ( ( ucgray2 - ucgray1 ) * ( udtx << 10 ) );
			ac = ( ( ucgray3 - ucgray1 ) * ( udty << 10 ) );
			ad = ( ucgray1 + ucgray4 - ucgray3 - ucgray2 ) * udtx * udty;

			*( pucdstp + i ) = ( unsigned char ) ( ( aa + ab + ac + ad ) >> 20 );
		}
		pucdstp += des_w;
	}

#else
	unsigned char *pucsrcp = 0;
	unsigned char *pucdstp = des_data+start_y*des_w;
	unsigned char *lt_src_data = src_data+((roi.top*src_w+roi.left)<<2);
	
	for ( j = start_y; j < end_y; j++ ) {
		for ( i = 0; i < des_w; i++ ) {
			//locgray4 = 0;

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
#endif	
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
	
	int32x4_t uda_11_32x4,uda_22_32x4;
	int32x4_t lt_src_data_32x4;
	
	lt_src_data_32x4 = vdupq_n_s32((int32_t)lt_src_data);
	uda_11_32x4 = vdupq_n_s32(uda_11);
	uda_22_32x4 = vdupq_n_s32(uda_22);
	
	for ( j = 0; j < des_roi_h; j++ ) {
		if( j >= des_roi_h - 1)
		{
			for ( i = 0; i < des_roi_w - 1; i++ ) {
	
			udtx = uda_11 * i;
			udty = uda_22 * j;
	
			utx = udtx >> 10;
			uty = udty >> 10;
			udtx -= ( utx << 10 );
			udty -= ( uty << 10 );
			pucsrcp = lt_src_data + ( uty * src_w ) + utx;
	
			ucgray1 = *pucsrcp;
				ucgray2 = *( pucsrcp + 1 );
				ucgray3 = *pucsrcp;
				ucgray4 = *( pucsrcp + 1 );

				udty = 1 << 10;
	
				aa = ucgray1 << 20;
				ab = ( ( ucgray2 - ucgray1 ) * ( udtx << 10 ) );
				ac = ( ( ucgray3 - ucgray1 ) * ( udty << 10 ) );
				ad = ( ucgray1 + ucgray4 - ucgray3 - ucgray2 ) * udtx * udty;
	
				*( pucdstp + i ) = ( unsigned char ) ( ( aa + ab + ac + ad ) >> 20 );
	
			}
			{
				udtx = uda_11 * i;
				udty = uda_22 * j;
	
				utx = udtx >> 10;
				uty = udty >> 10;
				udtx -= ( utx << 10 );
				udty -= ( uty << 10 );
				pucsrcp = lt_src_data + ( uty * src_w ) + utx;
	
				ucgray1 = *pucsrcp;
				ucgray2 = *pucsrcp;
				ucgray3 = *pucsrcp;
				ucgray4 = *pucsrcp;

				udtx = 1 << 10;
				udty = 1 << 10;
	
				aa = ucgray1 << 20;
				ab = ( ( ucgray2 - ucgray1 ) * ( udtx << 10 ) );
				ac = ( ( ucgray3 - ucgray1 ) * ( udty << 10 ) );
				ad = ( ucgray1 + ucgray4 - ucgray3 - ucgray2 ) * udtx * udty;
	
				*( pucdstp + i ) = ( unsigned char ) ( ( aa + ab + ac + ad ) >> 20 );
	
			}
			
			pucdstp += des_w;
		}
		else
		{
			int32x4_t i_32x4,iconst_32x4;
			int32x4_t udtx_32x4,udty_32x4,utx_32x4,uty_32x4;
			int32x4_t temp_const_32x4;
			i_32x4 = vcombine_s32(vcreate_s32(0x100000000), vcreate_s32(0x300000002));
			iconst_32x4 = vdupq_n_s32(4);
			//j_32x4 = vdupq_n_s32(j);
			temp_const_32x4 = vdupq_n_s32(0x3FF);

			for ( i = 0; i < (((des_roi_w - 1)>>2)<<2); i+= 4 ) {

				int32x4_t pucsrcp_32x4,dest_32x4;
				//uint8x8x4_t temp1_8x8x4,temp2_8x8x4;
				int32x4_t ucgray1_32x4,ucgray2_32x4,ucgray3_32x4,ucgray4_32x4;
				int32x4_t aa_32x4,ab_32x4,ac_32x4,ad_32x4;
				unsigned char *pucsrcp[4];
				unsigned char *pucsrcp1[4];
				unsigned char *pucsrcp2[4];
				unsigned char *pucsrcp3[4];
				int ucgray[16];
				int temp1,temp2,temp3,temp4;
				
				//udtx = uda_11 * i;
				//udty = uda_22 * j;
				udtx_32x4 = vmulq_s32(uda_11_32x4, i_32x4);
				i_32x4 = vaddq_s32(i_32x4, iconst_32x4);
				udty_32x4 = vmulq_n_s32(uda_22_32x4, j);
	
				//utx = udtx >> 10;
				//uty = udty >> 10;
				utx_32x4 = vshrq_n_s32(udtx_32x4, 10);
				uty_32x4 = vshrq_n_s32(udty_32x4, 10);
				
				//udtx -= ( utx << 10 );
				//udty -= ( uty << 10 );
				//udtx = ( udtx & 0x3FF );
				//udty = ( udty & 0x3FF );
				udtx_32x4 = vandq_s32(udtx_32x4, temp_const_32x4);
				udty_32x4 = vandq_s32(udty_32x4, temp_const_32x4);				
				
				//pucsrcp = lt_src_data + ( uty * src_w ) + utx;
				pucsrcp_32x4 = vmlaq_n_s32(utx_32x4, uty_32x4, src_w);
				pucsrcp_32x4 = vaddq_s32(lt_src_data_32x4, pucsrcp_32x4);
	
#if 0
				vst1q_s32((int32_t *)pucsrcp, pucsrcp_32x4);
				pucsrcp_32x4 = vaddq_s32(pucsrcp_32x4, vdupq_n_s32(src_w));
				vst1q_s32((int32_t *)pucsrcp1, pucsrcp_32x4);	
				
				ucgray[0] = *(pucsrcp[0]);
				ucgray[1] = *(pucsrcp[1]);
				ucgray[2] = *(pucsrcp[2]);
				ucgray[3] = *(pucsrcp[3]);
				
				ucgray[4] = *(pucsrcp[0] + 1);
				ucgray[5] = *(pucsrcp[1] + 1);
				ucgray[6] = *(pucsrcp[2] + 1);
				ucgray[7] = *(pucsrcp[3] + 1);			
				
				ucgray[8] = *(pucsrcp1[0]);
				ucgray[9] = *(pucsrcp1[1]);
				ucgray[10] = *(pucsrcp1[2]);
				ucgray[11] = *(pucsrcp1[3]);
				
				ucgray[12] = *(pucsrcp1[0] + 1);
				ucgray[13] = *(pucsrcp1[1] + 1);
				ucgray[14] = *(pucsrcp1[2] + 1);
				ucgray[15] = *(pucsrcp1[3] + 1);	
				
				ucgray1_32x4 = vld1q_s32(ucgray);
				ucgray2_32x4 = vld1q_s32(ucgray + 4);
				ucgray3_32x4 = vld1q_s32(ucgray + 8);
				ucgray4_32x4 = vld1q_s32(ucgray + 12);
				
#else
				ucgray1_32x4 = vdupq_n_s32(0);
				ucgray2_32x4 = vdupq_n_s32(0);
				ucgray3_32x4 = vdupq_n_s32(0);
				ucgray4_32x4 = vdupq_n_s32(0);
				
				vst1q_s32((int32_t *)pucsrcp, pucsrcp_32x4);
				
				ucgray1_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp[0]), vreinterpretq_u8_s32(ucgray1_32x4), 0));
				ucgray1_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp[1]), vreinterpretq_u8_s32(ucgray1_32x4), 4));
				ucgray1_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp[2]), vreinterpretq_u8_s32(ucgray1_32x4), 8));
				ucgray1_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp[3]), vreinterpretq_u8_s32(ucgray1_32x4), 12));
				
				pucsrcp_32x4 = vaddq_s32(pucsrcp_32x4, vdupq_n_s32(1));
				vst1q_s32((int32_t *)pucsrcp1, pucsrcp_32x4);
				
				ucgray2_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp1[0]), vreinterpretq_u8_s32(ucgray2_32x4), 0));
				ucgray2_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp1[1]), vreinterpretq_u8_s32(ucgray2_32x4), 4));
				ucgray2_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp1[2]), vreinterpretq_u8_s32(ucgray2_32x4), 8));
				ucgray2_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp1[3]), vreinterpretq_u8_s32(ucgray2_32x4), 12));
				
				pucsrcp_32x4 = vaddq_s32(pucsrcp_32x4, vdupq_n_s32(src_w));
				vst1q_s32((int32_t *)pucsrcp2, pucsrcp_32x4);
				
				ucgray4_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp2[0]), vreinterpretq_u8_s32(ucgray4_32x4), 0));
				ucgray4_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp2[1]), vreinterpretq_u8_s32(ucgray4_32x4), 4));
				ucgray4_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp2[2]), vreinterpretq_u8_s32(ucgray4_32x4), 8));
				ucgray4_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp2[3]), vreinterpretq_u8_s32(ucgray4_32x4), 12));

				pucsrcp_32x4 = vsubq_s32(pucsrcp_32x4, vdupq_n_s32(1));
				vst1q_s32((int32_t *)pucsrcp3, pucsrcp_32x4);

				ucgray3_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp3[0]), vreinterpretq_u8_s32(ucgray3_32x4), 0));
				ucgray3_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp3[1]), vreinterpretq_u8_s32(ucgray3_32x4), 4));
				ucgray3_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp3[2]), vreinterpretq_u8_s32(ucgray3_32x4), 8));
				ucgray3_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp3[3]), vreinterpretq_u8_s32(ucgray3_32x4), 12));
				
#endif
				//aa = ucgray1 << 20;
				aa_32x4 = vshlq_n_s32(ucgray1_32x4, 20);
				
				//ad = ( ucgray1 + ucgray4 - ucgray3 - ucgray2 ) * udtx * udty;
				ad_32x4 = vaddq_s32(ucgray1_32x4, ucgray4_32x4);
				ad_32x4 = vsubq_s32(ad_32x4, ucgray3_32x4);
				ad_32x4 = vsubq_s32(ad_32x4, ucgray2_32x4);
				ad_32x4 = vmulq_s32(ad_32x4, udtx_32x4);
				ad_32x4 = vmulq_s32(ad_32x4, udty_32x4);				
				
				udtx_32x4 = vshlq_n_s32(udtx_32x4, 10);
				udty_32x4 = vshlq_n_s32(udty_32x4, 10);

				//ab = ( ( ucgray2 - ucgray1 ) * ( udtx << 10 ) );
				ab_32x4 = vsubq_s32(ucgray2_32x4, ucgray1_32x4);
				ab_32x4 = vmulq_s32(ab_32x4, udtx_32x4);
				
				//ac = ( ( ucgray3 - ucgray1 ) * ( udty << 10 ) );
				ac_32x4 = vsubq_s32(ucgray3_32x4, ucgray1_32x4);
				ac_32x4 = vmulq_s32(ac_32x4, udty_32x4);
				
				//*( pucdstp + i ) = ( unsigned char ) ( ( aa + ab + ac + ad ) >> 20 );
				dest_32x4 = vaddq_s32(aa_32x4, ab_32x4);
				dest_32x4 = vaddq_s32(dest_32x4, ac_32x4);
				dest_32x4 = vaddq_s32(dest_32x4, ad_32x4);

				dest_32x4 = vshrq_n_s32(dest_32x4, 20);				
				if(((int)(pucdstp)) == (((int)pucdstp & 0xFFFFFFFC)))
				{
					uint8x8x2_t temp_8x8x2;
					
					temp_8x8x2.val[0] = vreinterpret_u8_s32(vget_low_s32(dest_32x4));
					temp_8x8x2.val[1] = vreinterpret_u8_s32(vget_high_s32(dest_32x4));
					
					temp_8x8x2 = vzip_u8(temp_8x8x2.val[0],temp_8x8x2.val[1]);
					temp_8x8x2 = vzip_u8(temp_8x8x2.val[0],temp_8x8x2.val[1]);
					vst1_lane_u32((uint32_t *)( pucdstp + i ), vreinterpret_u32_u8(temp_8x8x2.val[0]), 0);
				}
				else
				{
					vst1q_s32(ucgray, dest_32x4);
					*( pucdstp + i ) =  ucgray[0];
					*( pucdstp + i +1) =  ucgray[1];
					*( pucdstp + i +2) =  ucgray[2];
					*( pucdstp + i +3) =  ucgray[3];
				}

			}
			for (; i < (des_roi_w - 1); i++ ) {
				udtx = uda_11 * i;
				udty = uda_22 * j;
	
				utx = udtx >> 10;
				uty = udty >> 10;
				udtx -= ( utx << 10 );
				udty -= ( uty << 10 );
				pucsrcp = lt_src_data + ( uty * src_w ) + utx;
	
				ucgray1 = *pucsrcp;
				ucgray2 = *( pucsrcp + 1 );
				ucgray3 = *( pucsrcp + src_w );
				ucgray4 = *( pucsrcp + src_w + 1 );
							
	
				aa = ucgray1 << 20;
				ab = ( ( ucgray2 - ucgray1 ) * ( udtx << 10 ) );
				ac = ( ( ucgray3 - ucgray1 ) * ( udty << 10 ) );
				ad = ( ucgray1 + ucgray4 - ucgray3 - ucgray2 ) * udtx * udty;
	
				*( pucdstp + i ) = ( unsigned char ) ( ( aa + ab + ac + ad ) >> 20 );
	
			}
			
			{

				udtx = uda_11 * i;
				udty = uda_22 * j;
	
				utx = udtx >> 10;
				uty = udty >> 10;
				udtx -= ( utx << 10 );
				udty -= ( uty << 10 );
				
				pucsrcp = lt_src_data + ( uty * src_w ) + utx;
	
				ucgray1 = *pucsrcp;
				ucgray2 = *pucsrcp;
				ucgray3 = *( pucsrcp + src_w );
				ucgray4 = *( pucsrcp + src_w );
							
				udtx = 1 << 10;
	
				aa = ucgray1 << 20;
				ab = ( ( ucgray2 - ucgray1 ) * ( udtx << 10 ) );
				ac = ( ( ucgray3 - ucgray1 ) * ( udty << 10 ) );
				ad = ( ucgray1 + ucgray4 - ucgray3 - ucgray2 ) * udtx * udty;
	
				*( pucdstp + i ) = ( unsigned char ) ( ( aa + ab + ac + ad ) >> 20 );
	
			}
			pucdstp += des_w;
		}
	}
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
	//int locgray4 = 0;
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
	
	unsigned char *pucdstp = des_data+new_des_roi.top*des_w+new_des_roi.left;
	unsigned char *lt_src_data = src_data+((new_src_roi.top*src_w+new_src_roi.left)<<2);
	
	int des_roi_h = new_des_roi.bottom-new_des_roi.top;
	int des_roi_w = new_des_roi.right-new_des_roi.left;
	if(des_roi_w <= 0 || des_roi_h <= 0) return false;

	/* Affine transformation */
	uda_11 = ( (new_src_roi.right-new_src_roi.left) << 10 ) / des_roi_w;
	uda_22 = ( (new_src_roi.bottom-new_src_roi.top) << 10 ) / des_roi_h;

	int des_align4_w = ((des_roi_w-1)>>2)<<2;
	int32x4_t uda_11_32x4,uda_22_32x4;
	int32x4_t lt_src_data_32x4;
	
	lt_src_data_32x4 = vdupq_n_s32((int32_t)lt_src_data);
	uda_11_32x4 = vdupq_n_s32(uda_11);
	uda_22_32x4 = vdupq_n_s32(uda_22);
	
	for ( j = 0; j < des_roi_h; j++ )
	{
		int32x4_t i_32x4,iconst_32x4, j_32x4;
		int32x4_t udtx_32x4,udty_32x4,utx_32x4,uty_32x4;
		int32x4_t temp_const_32x4;
		i_32x4 = vcombine_s32(vcreate_s32(0x100000000), vcreate_s32(0x300000002));
		iconst_32x4 = vdupq_n_s32(4);
		j_32x4 = vdupq_n_s32(j);
		temp_const_32x4 = vdupq_n_s32(0x3FF);
		for ( i = 0; i < des_align4_w; i+=4 ) 
		{
			int32x4_t pucsrcp_32x4,dest_32x4;
			//uint8x8x4_t temp1_8x8x4,temp2_8x8x4;
			int32x4_t ucgray1_32x4,ucgray2_32x4,ucgray3_32x4,ucgray4_32x4;
			int32x4_t aa_32x4,ab_32x4,ac_32x4,ad_32x4;
			unsigned char *pucsrcp[4];
			unsigned char *pucsrcp1[4];
			unsigned char *pucsrcp2[4];
			unsigned char *pucsrcp3[4];
			
			int ucgray[16];
			int temp1,temp2,temp3,temp4;
			
			//udtx = uda_11 * i;
			//udty = uda_22 * j;
			udtx_32x4 = vmulq_s32(uda_11_32x4, i_32x4);
			i_32x4 = vaddq_s32(i_32x4, iconst_32x4);
			udty_32x4 = vmulq_s32(uda_22_32x4, j_32x4);

			//utx = udtx >> 10;
			//uty = udty >> 10;
			utx_32x4 = vshrq_n_s32(udtx_32x4, 10);
			uty_32x4 = vshrq_n_s32(udty_32x4, 10);
			
			//udtx -= ( utx << 10 );
			//udty -= ( uty << 10 );
			//udtx = ( udtx & 0x3FF );
			//udty = ( udty & 0x3FF );
			udtx_32x4 = vandq_s32(udtx_32x4, temp_const_32x4);
			udty_32x4 = vandq_s32(udty_32x4, temp_const_32x4);
			
			//pucsrcp = lt_src_data + ((( uty * src_w ) + utx) << 2);
			pucsrcp_32x4 = vmlaq_n_s32(utx_32x4, uty_32x4, src_w);
			pucsrcp_32x4 = vmlaq_n_s32(lt_src_data_32x4, pucsrcp_32x4, 4);
			
			
			
			//ucgray1 = *pucsrcp++;
			//ucgray2 = *pucsrcp++;
			//ucgray3 = *pucsrcp++;
			//ucgray4 = *pucsrcp;
			ucgray1_32x4 = vdupq_n_s32(0);
			ucgray2_32x4 = vdupq_n_s32(0);
			ucgray3_32x4 = vdupq_n_s32(0);
			ucgray4_32x4 = vdupq_n_s32(0);
			
			vst1q_s32((int32_t *)pucsrcp, pucsrcp_32x4);
			
			ucgray1_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp[0]), vreinterpretq_u8_s32(ucgray1_32x4), 0));
			ucgray1_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp[1]), vreinterpretq_u8_s32(ucgray1_32x4), 4));
			ucgray1_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp[2]), vreinterpretq_u8_s32(ucgray1_32x4), 8));
			ucgray1_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp[3]), vreinterpretq_u8_s32(ucgray1_32x4), 12));
			
			pucsrcp_32x4 = vaddq_s32(pucsrcp_32x4, vdupq_n_s32(1));
			vst1q_s32((int32_t *)pucsrcp1, pucsrcp_32x4);
			
			ucgray2_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp1[0]), vreinterpretq_u8_s32(ucgray2_32x4), 0));
			ucgray2_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp1[1]), vreinterpretq_u8_s32(ucgray2_32x4), 4));
			ucgray2_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp1[2]), vreinterpretq_u8_s32(ucgray2_32x4), 8));
			ucgray2_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp1[3]), vreinterpretq_u8_s32(ucgray2_32x4), 12));

			pucsrcp_32x4 = vaddq_s32(pucsrcp_32x4, vdupq_n_s32(1));
			vst1q_s32((int32_t *)pucsrcp3, pucsrcp_32x4);

			ucgray3_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp3[0]), vreinterpretq_u8_s32(ucgray3_32x4), 0));
			ucgray3_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp3[1]), vreinterpretq_u8_s32(ucgray3_32x4), 4));
			ucgray3_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp3[2]), vreinterpretq_u8_s32(ucgray3_32x4), 8));
			ucgray3_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp3[3]), vreinterpretq_u8_s32(ucgray3_32x4), 12));
			
			
			pucsrcp_32x4 = vaddq_s32(pucsrcp_32x4, vdupq_n_s32(1));
			vst1q_s32((int32_t *)pucsrcp2, pucsrcp_32x4);
			
			ucgray4_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp2[0]), vreinterpretq_u8_s32(ucgray4_32x4), 0));
			ucgray4_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp2[1]), vreinterpretq_u8_s32(ucgray4_32x4), 4));
			ucgray4_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp2[2]), vreinterpretq_u8_s32(ucgray4_32x4), 8));
			ucgray4_32x4 = vreinterpretq_s32_u8(vld1q_lane_u8((pucsrcp2[3]), vreinterpretq_u8_s32(ucgray4_32x4), 12));

			//aa = ucgray1 << 20;
			aa_32x4 = vshlq_n_s32(ucgray1_32x4, 20);
			
			//ad = ( ucgray1 + ucgray4 - ucgray3 - ucgray2 ) * udtx * udty;
			ad_32x4 = vaddq_s32(ucgray1_32x4, ucgray4_32x4);
			ad_32x4 = vsubq_s32(ad_32x4, ucgray3_32x4);
			ad_32x4 = vsubq_s32(ad_32x4, ucgray2_32x4);
			ad_32x4 = vmulq_s32(ad_32x4, udtx_32x4);
			ad_32x4 = vmulq_s32(ad_32x4, udty_32x4);
			
			udtx_32x4 = vshlq_n_s32(udtx_32x4, 10);
			udty_32x4 = vshlq_n_s32(udty_32x4, 10);
			
			//ab = ( ( ucgray2 - ucgray1 ) * ( udtx << 10 ) );
			ab_32x4 = vsubq_s32(ucgray2_32x4, ucgray1_32x4);
			ab_32x4 = vmulq_s32(ab_32x4, udtx_32x4);
			
			//ac = ( ( ucgray3 - ucgray1 ) * ( udty << 10 ) );
			ac_32x4 = vsubq_s32(ucgray3_32x4, ucgray1_32x4);
			ac_32x4 = vmulq_s32(ac_32x4, udty_32x4);

			//*( pucdstp + i ) = ( unsigned char ) ( ( aa + ab + ac + ad ) >> 20 );
			
			dest_32x4 = vaddq_s32(aa_32x4, ab_32x4);
			dest_32x4 = vaddq_s32(dest_32x4, ac_32x4);
			dest_32x4 = vaddq_s32(dest_32x4, ad_32x4);

			dest_32x4 = vshrq_n_s32(dest_32x4, 20);
			
			
			if(((int)(pucdstp)) == (((int)pucdstp & 0xFFFFFFFC)))
			{
				uint8x8x2_t temp_8x8x2;
				
				temp_8x8x2.val[0] = vreinterpret_u8_s32(vget_low_s32(dest_32x4));
				temp_8x8x2.val[1] = vreinterpret_u8_s32(vget_high_s32(dest_32x4));
				
				temp_8x8x2 = vzip_u8(temp_8x8x2.val[0],temp_8x8x2.val[1]);
				temp_8x8x2 = vzip_u8(temp_8x8x2.val[0],temp_8x8x2.val[1]);
				vst1_lane_u32((uint32_t *)( pucdstp + i ), vreinterpret_u32_u8(temp_8x8x2.val[0]), 0);
			}
			else
			{
				vst1q_s32(ucgray, dest_32x4);
				*( pucdstp + i ) =  ucgray[0];
				*( pucdstp + i +1) =  ucgray[1];
				*( pucdstp + i +2) =  ucgray[2];
				*( pucdstp + i +3) =  ucgray[3];
			}
		
		}
		for ( i = des_align4_w; i < des_roi_w; i++ ) {
			unsigned char *pucsrcp = 0;
			
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

unsigned const char mask[] = {0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,
0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F};

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

#if 0
	while(size--) {
		m = (*--m1 + *--m2 + *--m3 + *--m4 + *--m5 + *--m6 + *--m7 + *--m8 + *--m9)/9;
		data_mct =  (m < *m1);
		data_mct |= (m < *m2) << 1;
		data_mct |= (m < *m3) << 2;
		data_mct |= (m < *m4) << 3;
		data_mct |= (m < *m5) << 4;
		data_mct |= (m < *m6) << 5;
		data_mct |= (m < *m7) << 6;
		data_mct |= (m < *m8) << 7;
		data_mct |= (m < *m9) << 8;
		*--pmct = data_mct;
	}
#else
	uint8x16_t  mask1_8x16,mask2_8x16,mask3_8x16;
	uint16x4_t const_7281_16x4;
	uint32x4_t const_7280_32x4;

	mask1_8x16 = vld1q_u8(mask);
	mask2_8x16 = vld1q_u8(mask + 16);
	mask3_8x16 = vld1q_u8(mask + 32);
	
	while(size >= 16)
	{
		
		uint8x16_t m1_8x16,m2_8x16,m3_8x16,m4_8x16,m5_8x16,m6_8x16,m7_8x16,m8_8x16,m9_8x16;
		uint8x16_t t_8x16;
		uint16x8_t t1_16x8,t2_16x8;
		uint32x4_t t11_32x4,t12_32x4,t21_32x4,t22_32x4;
		uint8x16x2_t pmct_8x16x2;
		
		
		//Loading All the pixels in 3x3 box
		//Loading 16 elements in each load
		//each element is of 8 Bit i.e., char size
		//Load a single vector from memory
		
		m1 -= 16;
		m1_8x16 = vld1q_u8(m1);
		
		m2 = m1 + 1;
		m2_8x16 = vld1q_u8(m2);
		
		m3 = m2 + 1;
		m3_8x16 = vld1q_u8(m3);
		
		m4 -= 16;
		m4_8x16 = vld1q_u8(m4);
		
		m5 = m4 + 1;
		m5_8x16 = vld1q_u8(m5);
		
		m6 = m5 + 1;
		m6_8x16 = vld1q_u8(m6);		
		
		m7 -= 16;
		m7_8x16 = vld1q_u8(m7);
		
		m8 = m7 + 1;
		m8_8x16 = vld1q_u8(m8);
		
		m9 = m8 + 1;
		m9_8x16 = vld1q_u8(m9);
		
		size -= 16;
		pmct -= 16;
	
		//Accumulating all the pixels of 3x3 box
		
		//Splitting vectors
		//Vector long add: vaddl_<type>. Vr[i]:=Va[i]+Vb[i]
		t1_16x8 = vaddl_u8(vget_low_u8(m1_8x16), vget_low_u8(m2_8x16));
		//Vector wide add: vaddw_<type>. Vr[i]:=Va[i]+Vb[i]
		t1_16x8 = vaddw_u8(t1_16x8, vget_low_u8(m3_8x16));
		t1_16x8 = vaddw_u8(t1_16x8, vget_low_u8(m4_8x16));
		t1_16x8 = vaddw_u8(t1_16x8, vget_low_u8(m5_8x16));
		t1_16x8 = vaddw_u8(t1_16x8, vget_low_u8(m6_8x16));
		t1_16x8 = vaddw_u8(t1_16x8, vget_low_u8(m7_8x16));
		t1_16x8 = vaddw_u8(t1_16x8, vget_low_u8(m8_8x16));
		t1_16x8 = vaddw_u8(t1_16x8, vget_low_u8(m9_8x16));
		
		//Splitting vectors
		//Vector long add: vaddl_<type>. Vr[i]:=Va[i]+Vb[i]
		t2_16x8 = vaddl_u8(vget_high_u8(m1_8x16), vget_high_u8(m2_8x16));
		//Vector wide add: vaddw_<type>. Vr[i]:=Va[i]+Vb[i]
		t2_16x8 = vaddw_u8(t2_16x8, vget_high_u8(m3_8x16));
		t2_16x8 = vaddw_u8(t2_16x8, vget_high_u8(m4_8x16));
		t2_16x8 = vaddw_u8(t2_16x8, vget_high_u8(m5_8x16));
		t2_16x8 = vaddw_u8(t2_16x8, vget_high_u8(m6_8x16));
		t2_16x8 = vaddw_u8(t2_16x8, vget_high_u8(m7_8x16));
		t2_16x8 = vaddw_u8(t2_16x8, vget_high_u8(m8_8x16));
		t2_16x8 = vaddw_u8(t2_16x8, vget_high_u8(m9_8x16));	
		
		//Max value of t can be 2304(256*9)
		//Hard Code Method for Integer division by 9(Tested for values less then 2310)
		//=(B3*7281+7280)/65536   //(t*455+454)/4096 - Old;
		
		const_7281_16x4 = vdup_n_u16(7281);
		const_7280_32x4 = vdupq_n_u32(7280);
#if 1
		//Splitting vectors
		//Vector long multiply with scalar
		t11_32x4 = vmull_u16(vget_low_u16(t1_16x8), const_7281_16x4);
		t12_32x4 = vmull_u16(vget_high_u16(t1_16x8), const_7281_16x4);

		//Load all lanes of vector to the same literal value		
		//Vector add high half: vaddhn_<type>.Vr[i]:=Va[i]+Vb[i]
		//Combining vectors
		t1_16x8 = vcombine_u16(vaddhn_u32(t11_32x4, const_7280_32x4),vaddhn_u32(t12_32x4, const_7280_32x4));
		
		t21_32x4 = vmull_u16(vget_low_u16(t2_16x8), const_7281_16x4);
		t22_32x4 = vmull_u16(vget_high_u16(t2_16x8), const_7281_16x4);
		
		//Load all lanes of vector to the same literal value		
		//Vector add high half: vaddhn_<type>.Vr[i]:=Va[i]+Vb[i]
		//Combining vectors
		t2_16x8 = vcombine_u16(vaddhn_u32(t21_32x4, const_7280_32x4),vaddhn_u32(t22_32x4, const_7280_32x4));
#else
		//Not working
		t1_16x8 = vreinterpretq_u16_s16(vqrdmulhq_n_s16(vreinterpretq_s16_u16(t1_16x8), 3640));
		t2_16x8 = vreinterpretq_u16_s16(vqrdmulhq_n_s16(vreinterpretq_s16_u16(t2_16x8), 3640));
#endif
		//Vector narrow integer
		//Combining vectors
		t_8x16 = vcombine_u8(vmovn_u16(t1_16x8),vmovn_u16(t2_16x8));
		
		//Vector compare less-than
		//Vector Compare Less Than takes the value of each element in a vector, and compares it with the
		//value of the corresponding element of a second vector. If the condition is true, the corresponding
		//element in the destination vector is set to all ones. Otherwise, it is set to all zeros.
		m1_8x16 = vcltq_u8(t_8x16, m1_8x16);
		m2_8x16 = vcltq_u8(t_8x16, m2_8x16); 
		m3_8x16 = vcltq_u8(t_8x16, m3_8x16); 
		m4_8x16 = vcltq_u8(t_8x16, m4_8x16); 
		m5_8x16 = vcltq_u8(t_8x16, m5_8x16);
		m6_8x16 = vcltq_u8(t_8x16, m6_8x16); 
		m7_8x16 = vcltq_u8(t_8x16, m7_8x16); 
		m8_8x16 = vcltq_u8(t_8x16, m8_8x16); 
		m9_8x16 = vcltq_u8(t_8x16, m9_8x16);
		
		//Bitwise Select
		//VBSL (Bitwise Select) selects each bit for the destination from the first operand if the
		//corresponding bit of the destination is 1, or from the second operand if the corresponding bit of
		//the destination is 0.
		m2_8x16 = vbslq_u8(mask1_8x16, m1_8x16, m2_8x16); 
		m4_8x16 = vbslq_u8(mask1_8x16, m3_8x16, m4_8x16); 
		m6_8x16 = vbslq_u8(mask1_8x16, m5_8x16, m6_8x16); 
		m8_8x16 = vbslq_u8(mask1_8x16, m7_8x16, m8_8x16); 

		m4_8x16 = vbslq_u8(mask2_8x16, m2_8x16, m4_8x16); 
		m8_8x16 = vbslq_u8(mask2_8x16, m6_8x16, m8_8x16); 

		pmct_8x16x2.val[0] = vbslq_u8(mask3_8x16, m4_8x16, m8_8x16); 
		
		//Right Shift by a constant
		pmct_8x16x2.val[1] = vshrq_n_u8(m9_8x16,7);

#if	(DOUBLE_MCT_AND_FASTPOSE == 1)	
		pmct_8x16x2 = vzipq_u8(pmct_8x16x2.val[0] , pmct_8x16x2.val[1] );

		t1_16x8 = vshlq_n_u16(vreinterpretq_u16_u8(pmct_8x16x2.val[0]), 1);
		t2_16x8 = vshlq_n_u16(vreinterpretq_u16_u8(pmct_8x16x2.val[1]), 1);

		vst1q_u16(pmct, t1_16x8);
		vst1q_u16((pmct + 8), t2_16x8);
#else
		//Store N-element structure to memory
		vst2q_u8((uint8_t *)pmct, pmct_8x16x2);
#endif
		
	}
	m2 = m1 + 1;
	m3 = m2 + 1;
	m5 = m4 + 1;
	m6 = m5 + 1;
	m8 = m7 + 1;
	m9 = m8 + 1;

	while(size--) {
		m = (*--m1 + *--m2 + *--m3 + *--m4 + *--m5 + *--m6 + *--m7 + *--m8 + *--m9)/9;
		data_mct =  (m < *m1);
		data_mct |= (m < *m2) << 1;
		data_mct |= (m < *m3) << 2;
		data_mct |= (m < *m4) << 3;
		data_mct |= (m < *m5) << 4;
		data_mct |= (m < *m6) << 5;
		data_mct |= (m < *m7) << 6;
		data_mct |= (m < *m8) << 7;
		data_mct |= (m < *m9) << 8;
#if	(DOUBLE_MCT_AND_FASTPOSE == 1)		
		*--pmct = data_mct << 1;
#else
		*--pmct = data_mct;
#endif
	}
#endif
}

void MVFD_ImageTranspose(unsigned char* src_data, int src_w, int src_h, unsigned char* rot_data)
{
	MVFD_ImageTranspose_neon(src_data,src_w,src_h,rot_data);
	
	int 	src_w_by_16 = src_w - (src_w & 0xF);
	int 	src_h_by_16 = src_h - (src_h & 0xF);
	unsigned char *sp = src_data + src_w_by_16;
	unsigned char *dp_back = rot_data + src_w_by_16*src_h;
	
	
	for(int y = src_h; y; y--)
	{
		int x = src_w & 0xF;
		unsigned char * dp = dp_back;
		while(x--)
		{
			*dp = *sp++;
			dp += src_h;
		}
		sp += src_w_by_16;
		dp_back++;
	}
	
	sp = src_data + src_w*src_h_by_16;
	dp_back = rot_data + src_h_by_16;
	
	for(int y = (src_h & 0xF); y; y--)
	{
		int x = src_w_by_16;
		unsigned char * dp = dp_back;
		while(x--)
		{
			*dp = *sp++;
			dp += src_h;
		}
		sp += src_w & 0xF;
		dp_back++;
	}
	
}

#if	(RESIZE_NN_OPT == 1)
#if 1 
//sk: Optimized code. 
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
#else
void MVFD_Resize_NN(unsigned char *__restrict src_data, int src_w, int src_h, unsigned char *__restrict des_data, int des_w, int des_h)
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
#endif
#endif //#if (RESIZE_NN_OPT== 1)

void MVFD_CropRegion(unsigned char* src_data, int src_w, int src_h, FD_RECT roi, unsigned char *dst_data)
{
	int dst_w = roi.right-roi.left;
	int dst_h = roi.bottom-roi.top;
	for(int y = 0; y < dst_h; ++y)
	{
		fd_memcpy(&dst_data[y*dst_w],&src_data[(roi.top+y)*src_w+roi.left], dst_w);
	}
}

#endif //#ifdef USE_NEON


