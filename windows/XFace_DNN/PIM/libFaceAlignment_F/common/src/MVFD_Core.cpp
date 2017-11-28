// MVFD2.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//
#include "stdafx.h"

#include <stdlib.h>
#include "MVFD_Defines.h"
#include "MVFD_Init.h"
#include "MVFD_Memory.h"
#include "MVFD_Enroll.h"
#include "MVFD_Global.h"
#include "IRMB2_CodeTable.h"

#if !defined(USE_SSE) && !defined(USE_NEON)
void MVFD_GeneratePoseMapForSmall(FD_CONTEXT *pContext, FD_CLASSIFIER *pRejector, FD_CLASSIFIER *pClassifier, FEATURE *pFeature, float rejection_rate, unsigned short *output, int *pmap_ffpos[])
{
	int valid_w = pFeature->width - FD_FACE_WIDTH;
	int valid_h = pFeature->height - FD_FACE_HEIGHT;
	int aligned_width = (((pFeature->width+8)/8)*8);
	
	unsigned char *rejection_map = MVFD_MemSection_Alloc<unsigned char>(pContext, MEM_SECTION_REJECTION_MAP, valid_w);
	unsigned short *code_image_buf = MVFD_MemSection_Alloc<unsigned short>(pContext, MEM_SECTION_IRMB2_CODE_BUF, valid_w*pClassifier->feature_num[FEATURE_IDX_IRMB2]);
	
	int *ffpos = pmap_ffpos[FEATURE_IDX_IRMB2];
	fd_memset(output, 0, pFeature->width*pFeature->height*sizeof(unsigned short));

	for(int y = 0; y < valid_h; y +=2)
	{
		unsigned short *wp = code_image_buf;
		for(int f = 0; f < pClassifier->feature_num[FEATURE_IDX_IRMB2]; ++f)
		{
			int *fpos = &ffpos[f*6];
			unsigned char *p = pFeature->irmb + y*aligned_width;
			unsigned char *p0 = p+fpos[0];
			unsigned char *p1 = p+fpos[1];
			unsigned char *p2 = p+fpos[2];
			unsigned char *p3 = p+fpos[3];
			unsigned char *p4 = p+fpos[4];
			unsigned char *p5 = p+fpos[5];
			for(int x = 0; x < valid_w; x++)
			{
				int i0 = *p0++, i1 = *p1++, i2 = *p2++, i3 = *p3++, i4 = *p4++, i5 = *p5++;

				int lcode = (i0>=i1) | ((i0 >=i2)<<1) | ((i0>=i3)<<2) | ((i0>=i4)<<3) | ((i0>=i5)<<4) |
						((i1>=i2)<<5) | ((i1>=i3)<<6) | ((i1>=i4)<<7) | ((i1>=i5)<<8) |
						((i2>=i3)<<9) | ((i2>=i4)<<10) | ((i2>=i5)<<11) | 
						((i3 >=i4)<<12) | ((i3>=i5)<<13) | 
						((i4>=i5)<<14);
				*wp++ = lcode;
				p++;
			}
		}
		
		int fnum = pClassifier->feature_num[FEATURE_IDX_IRMB2];
		unsigned short *p = code_image_buf;
		unsigned short *sp = code_image_buf;
		while(fnum--)
		{
			int xcnt = valid_w>>3;
			int remain = (valid_w-(xcnt<<3)+1)>>1;
			p = sp;
			while(xcnt--)
			{
				*p = irmb2_table[*p];
				p+=2;
				*p = irmb2_table[*p];
				p+=2;
				*p = irmb2_table[*p];
				p+=2;
				*p = irmb2_table[*p];
				p+=2;
			}
			while(remain--)
			{
				*p = irmb2_table[*p];
				p+=2;
			}
			sp += valid_w;
		}

		unsigned short rej_threshold = (unsigned short)(pRejector->thresholds[0]*rejection_rate);
		unsigned short *pose_thresholds = (unsigned short *)pClassifier->thresholds;
		for(int x = 0; x < valid_w; x+=2)
		{
			int fpos = x;
			unsigned int confidence = 0;
			unsigned short *lut = pRejector->lookup_tbl[FEATURE_IDX_IRMB2];
			int f = pRejector->feature_num[FEATURE_IDX_IRMB2]>>1;
			do
			{
				int code = code_image_buf[fpos];
				confidence += lut[code];
				lut += 720;
				fpos += valid_w;
				code = code_image_buf[fpos];
				confidence += lut[code];
				lut += 720;
				fpos += valid_w;
			} while(--f);
			if(confidence > rej_threshold) rejection_map[x] = 255;
			else rejection_map[x] = 0;
		}
		
		for(int x = 0; x < valid_w; x+=2)
		{
			int oidx = y*pFeature->width+x;
			if(rejection_map[x])
			{
				int fnum = pClassifier->feature_num[FEATURE_IDX_IRMB2];
				int fpos = x;
				unsigned short confidence[8] = {0,};
				unsigned short (*lut)[720][8] = (unsigned short (*)[720][8])pClassifier->lookup_tbl[FEATURE_IDX_IRMB2];
				for(int f = 0; f < fnum; ++f)
				{
					unsigned short (*one_lut)[8] = lut[f];
					int code = code_image_buf[fpos];
					unsigned short *weak_result = one_lut[code];
					for(int i = 0; i < 8; ++i)
					{
						confidence[i] += weak_result[i];
					}
					fpos += valid_w;
				}
				int first_idx = 0, second_idx = -1;
				for(int i = 1; i < 8; ++i)
				{
					if(confidence[first_idx] < confidence[i]) 
					{
						second_idx = first_idx;
						first_idx = i;
					}
					else
					{
						if(second_idx == -1) second_idx = i;
						else if(confidence[second_idx] < confidence[i]) second_idx = i;
					}
				}
				output[oidx] = ((second_idx+1)<<8) | (first_idx+1);
				if(confidence[first_idx] < pose_thresholds[first_idx]) output[oidx] &= 0xff00;
				if(confidence[second_idx] < pose_thresholds[second_idx]) output[oidx] &= 0x00ff;
			}
			else output[oidx] = 0;
		}
	}
}

void MVFD_GeneratePoseMap(FD_CONTEXT *pContext, FD_CLASSIFIER *pRejector, FD_CLASSIFIER *pClassifier, FEATURE *pFeature, float rejection_rate, unsigned short *output, int *pmap_ffpos[])
{
	int valid_w = pFeature->width - FD_FACE_WIDTH;
	int valid_h = pFeature->height - FD_FACE_HEIGHT;
	int aligned_width = (((pFeature->width+8)/8)*8);

	if(pFeature->width <= 40 || pFeature->height <= 40) 
	{
		MVFD_GeneratePoseMapForSmall(pContext, pRejector, pClassifier, pFeature, rejection_rate, output, pmap_ffpos);
		return;
	}
	
	unsigned char *rejection_map = MVFD_MemSection_Alloc<unsigned char>(pContext, MEM_SECTION_REJECTION_MAP, valid_w);
	unsigned short *code_image_buf = MVFD_MemSection_Alloc<unsigned short>(pContext, MEM_SECTION_IRMB2_CODE_BUF, valid_w*pClassifier->feature_num[FEATURE_IDX_IRMB2]);
	
	int *ffpos = pmap_ffpos[FEATURE_IDX_IRMB2];
	fd_memset(output, 0, pFeature->width*pFeature->height*sizeof(unsigned short));

	for(int y = 0; y < valid_h; y +=2)
	{
		unsigned short *wp = code_image_buf;
		for(int f = 0; f < pClassifier->feature_num[FEATURE_IDX_IRMB2]; ++f)
		{
			int *fpos = &ffpos[f*6];
			unsigned char *p = pFeature->irmb + y*aligned_width;
			unsigned char *p0 = p+fpos[0];
			unsigned char *p1 = p+fpos[1];
			unsigned char *p2 = p+fpos[2];
			unsigned char *p3 = p+fpos[3];
			unsigned char *p4 = p+fpos[4];
			unsigned char *p5 = p+fpos[5];
			for(int x = 0; x < valid_w; x++)
			{
				int i0 = *p0++, i1 = *p1++, i2 = *p2++, i3 = *p3++, i4 = *p4++, i5 = *p5++;

				int lcode = (i0>=i1) | ((i0 >=i2)<<1) | ((i0>=i3)<<2) | ((i0>=i4)<<3) | ((i0>=i5)<<4) |
						((i1>=i2)<<5) | ((i1>=i3)<<6) | ((i1>=i4)<<7) | ((i1>=i5)<<8) |
						((i2>=i3)<<9) | ((i2>=i4)<<10) | ((i2>=i5)<<11) | 
						((i3 >=i4)<<12) | ((i3>=i5)<<13) | 
						((i4>=i5)<<14);
				*wp++ = lcode;
				p++;
			}
		}
		
		int fnum = pClassifier->feature_num[FEATURE_IDX_IRMB2];
		unsigned short *p = code_image_buf;
		unsigned short *sp = code_image_buf;
		while(fnum--)
		{
			int xcnt = valid_w>>3;
			int remain = (valid_w-(xcnt<<3)+1)>>1;
			p = sp;
			while(xcnt--)
			{
				*p = irmb2_table[*p];
				p+=2;
				*p = irmb2_table[*p];
				p+=2;
				*p = irmb2_table[*p];
				p+=2;
				*p = irmb2_table[*p];
				p+=2;
			}
			while(remain--)
			{
				*p = irmb2_table[*p];
				p+=2;
			}
			sp += valid_w;
		}

		unsigned short rej_threshold = (unsigned short)(pRejector->thresholds[0]*rejection_rate);
		unsigned short *pose_thresholds = (unsigned short *)pClassifier->thresholds;
		for(int x = 0; x < valid_w; x+=2)
		{
			int fpos = x;
			unsigned int confidence = 0;
			unsigned short *lut = pRejector->lookup_tbl[FEATURE_IDX_IRMB2];
			int f = pRejector->feature_num[FEATURE_IDX_IRMB2]>>1;
			do
			{
				int code = code_image_buf[fpos];
				confidence += lut[code];
				lut += 720;
				fpos += valid_w;
				code = code_image_buf[fpos];
				confidence += lut[code];
				lut += 720;
				fpos += valid_w;
			} while(--f);
			if(confidence > rej_threshold) rejection_map[x] = 255;
			else rejection_map[x] = 0;
		}
		
		for(int x = 0; x < valid_w; x+=2)
		{
			int oidx = y*pFeature->width+x;
			if(rejection_map[x])
			{
				int fnum = pClassifier->feature_num[FEATURE_IDX_IRMB2];
				int fpos = x;
				unsigned short confidence[8] = {0,};
				unsigned short (*lut)[720][8] = (unsigned short (*)[720][8])pClassifier->lookup_tbl[FEATURE_IDX_IRMB2];
				for(int f = 0; f < fnum; ++f)
				{
					unsigned short (*one_lut)[8] = lut[f];
					int code = code_image_buf[fpos];
					unsigned short *weak_result = one_lut[code];
					for(int i = 0; i < 8; ++i)
					{
						confidence[i] += weak_result[i];
					}
					fpos += valid_w;
				}
				int first_idx = 0, second_idx = -1;
				for(int i = 1; i < 8; ++i)
				{
					if(confidence[first_idx] < confidence[i]) 
					{
						second_idx = first_idx;
						first_idx = i;
					}
					else
					{
						if(second_idx == -1) second_idx = i;
						else if(confidence[second_idx] < confidence[i]) second_idx = i;
					}
				}
				output[oidx] = ((second_idx+1)<<8) | (first_idx+1);
				if(confidence[first_idx] < pose_thresholds[first_idx]) output[oidx] &= 0xff00;
				if(confidence[second_idx] < pose_thresholds[second_idx]) output[oidx] &= 0x00ff;
			}
			else output[oidx] = 0;
		}
	}
}

void MVFD_GeneratePoseMapForTracking(FD_CONTEXT *pContext, FD_CLASSIFIER *pRejector, FD_CLASSIFIER *pClassifier, FEATURE *pFeature, float rejection_rate, unsigned short *output, int face_x, int face_y, int *pmap_ffpos[])
{
	int valid_w = pFeature->width - FD_FACE_WIDTH;
	int valid_h = pFeature->height - FD_FACE_HEIGHT;
	int aligned_width = (((pFeature->width+8)/8)*8);
	
	unsigned char *rejection_map = MVFD_MemSection_Alloc<unsigned char>(pContext, MEM_SECTION_REJECTION_MAP, valid_w);
	unsigned short *code_image_buf = MVFD_MemSection_Alloc<unsigned short>(pContext, MEM_SECTION_IRMB2_CODE_BUF, valid_w*pClassifier->feature_num[FEATURE_IDX_IRMB2]);
	
	int *ffpos = pmap_ffpos[FEATURE_IDX_IRMB2];
	fd_memset(output, 0, pFeature->width*pFeature->height*sizeof(unsigned short));

	for(int y = 0; y < valid_h; y +=2)
	{
		unsigned short *wp = code_image_buf;
		for(int f = 0; f < pClassifier->feature_num[FEATURE_IDX_IRMB2]; ++f)
		{
			int *fpos = &ffpos[f*6];
			unsigned char *p = pFeature->irmb + y*aligned_width;
			unsigned char *p0 = p+fpos[0];
			unsigned char *p1 = p+fpos[1];
			unsigned char *p2 = p+fpos[2];
			unsigned char *p3 = p+fpos[3];
			unsigned char *p4 = p+fpos[4];
			unsigned char *p5 = p+fpos[5];
			for(int x = 0; x < valid_w; x++)
			{
				int i0 = *p0++, i1 = *p1++, i2 = *p2++, i3 = *p3++, i4 = *p4++, i5 = *p5++;

				int lcode = (i0>=i1) | ((i0 >=i2)<<1) | ((i0>=i3)<<2) | ((i0>=i4)<<3) | ((i0>=i5)<<4) |
						((i1>=i2)<<5) | ((i1>=i3)<<6) | ((i1>=i4)<<7) | ((i1>=i5)<<8) |
						((i2>=i3)<<9) | ((i2>=i4)<<10) | ((i2>=i5)<<11) | 
						((i3 >=i4)<<12) | ((i3>=i5)<<13) | 
						((i4>=i5)<<14);
				*wp++ = lcode;
				p++;
			}
		}

		int fnum = pClassifier->feature_num[FEATURE_IDX_IRMB2];
		unsigned short *p = code_image_buf;
		unsigned short *sp = code_image_buf;
		while(fnum--)
		{
			int xcnt = valid_w>>2;
			int remain = valid_w-(xcnt<<2);
			p = sp;
			while(xcnt--)
			{
				*p++ = irmb2_table[*p];
				*p++ = irmb2_table[*p];
				*p++ = irmb2_table[*p];
				*p++ = irmb2_table[*p];
			}
			while(remain--)
			{
				*p++ = irmb2_table[*p];
			}
			sp += valid_w;
		}

		unsigned short rej_threshold = (unsigned short)(pRejector->thresholds[0]*rejection_rate);
		unsigned short *pose_thresholds = (unsigned short *)pClassifier->thresholds;
		for(int x = 0; x < valid_w; x++)
		{
			int fpos = x;
			unsigned int confidence = 0;
			unsigned short *lut = pRejector->lookup_tbl[FEATURE_IDX_IRMB2];
			int f = pRejector->feature_num[FEATURE_IDX_IRMB2]>>1;
			if((x>>1)<<1 == x || (face_x-8 < x && x < face_x+8 && face_y-8 < y && y < face_y+8))
			{
				do
				{
					int code = code_image_buf[fpos];
					confidence += lut[code];
					lut += 720;
					fpos += valid_w;
					code = code_image_buf[fpos];
					confidence += lut[code];
					lut += 720;
					fpos += valid_w;
				} while(--f);
				if(confidence > rej_threshold) rejection_map[x] = 255;
				else rejection_map[x] = 0;
			}
			else rejection_map[x] = 0;
		}
		
		for(int x = 0; x < valid_w; x++)
		{
			int oidx = y*pFeature->width+x;			
			if(rejection_map[x])
			{
				int fnum = pClassifier->feature_num[FEATURE_IDX_IRMB2];
				int fpos = x;
				unsigned short confidence[8] = {0,};
				//bug fixed by postech 2014.05.13
				unsigned short (*lut)[720][8] = (unsigned short (*)[720][8])pClassifier->lookup_tbl[FEATURE_IDX_IRMB2];
				for(int f = 0; f < fnum; ++f)
				{
					unsigned short (*one_lut)[8] = lut[f];
					int code = code_image_buf[fpos];
					unsigned short *weak_result = one_lut[code];
					for(int i = 0; i < 8; ++i)
					{
						confidence[i] += weak_result[i];
					}
					fpos += valid_w;
				}
				int first_idx = 0, second_idx = -1;
				for(int i = 1; i < 8; ++i)
				{
					if(confidence[first_idx] < confidence[i]) 
					{
						second_idx = first_idx;
						first_idx = i;
					}
					else
					{
						if(second_idx == -1) second_idx = i;
						else if(confidence[second_idx] < confidence[i]) second_idx = i;
					}
				}
				output[oidx] = ((second_idx+1)<<8) | (first_idx+1);
				if(confidence[first_idx] < pose_thresholds[first_idx]) output[oidx] &= 0xff00;
				if(confidence[second_idx] < pose_thresholds[second_idx]) output[oidx] &= 0x00ff;
			}
		}
	}
}

int MVFD_IsFace(FEATURE *pFeature, int x, int y, FD_CLASSIFIER *pClassifier, int *all_ffpos[][FD_FEATURE_TYPE_NUM])
{
	FD_CLASSIFIER *pCls = pClassifier;
	int width = pFeature->width;
	int height = pFeature->height;
	unsigned int confidence;
	int *(*one_stage_ffpos)[FD_FEATURE_TYPE_NUM] = all_ffpos;
	do
	{
		confidence = 0;
		
		//MCT
		if(pCls->feature_num[FEATURE_IDX_MCT])
		{
			int *ffpos = (*one_stage_ffpos)[FEATURE_IDX_MCT];
			unsigned short *mct_buf = &pFeature->mct[y*width+x];
			unsigned short *lut = pCls->lookup_tbl[FEATURE_IDX_MCT];
			int fnum = pCls->feature_num[FEATURE_IDX_MCT]>>1;
			while(fnum--)
			{
				int mct_code = *(mct_buf + *ffpos++);
				confidence += lut[mct_code];
				lut += 512;
				mct_code = *(mct_buf + *ffpos++);	
				confidence += lut[mct_code];
				lut += 512;
			}
		}
		
		//MCT5x5
		if(pCls->feature_num[FEATURE_IDX_MCT5x5])
		{
			int *ffpos = (*one_stage_ffpos)[FEATURE_IDX_MCT5x5];
			unsigned char *mct5x5_buf = &pFeature->img[y*width+x];
			unsigned short *lut = pCls->lookup_tbl[FEATURE_IDX_MCT5x5];
			int fnum = pCls->feature_num[FEATURE_IDX_MCT5x5];
			while(fnum--)
			{				
				unsigned char *p1 = mct5x5_buf + *(ffpos++);
				unsigned char *p2 = p1 + (width<<1);
				unsigned char *p3 = p2 + (width<<1);
				unsigned int m1 = *p1, m2 = *(p1+2), m3 = *(p1+4);
				unsigned int m4 = *p2, m5 = *(p2+2), m6 = *(p2+4);
				unsigned int m7 = *p3, m8 = *(p3+2), m9 = *(p3+4);
				unsigned int m = m1 + m2 + m3 + m4 + m5 + m6 + m7 + m8 + m9;
				unsigned int mct_code;
				mct_code = ((unsigned int)(m - (m1 << 3) - m1) & 0x80000000) >> 31;
				mct_code |= ((unsigned int)(m - (m2 << 3) - m2) & 0x80000000) >> 30;
				mct_code |= ((unsigned int)(m - (m3 << 3) - m3) & 0x80000000) >> 29;
				mct_code |= ((unsigned int)(m - (m4 << 3) - m4) & 0x80000000) >> 28;
				mct_code |= ((unsigned int)(m - (m5 << 3) - m5) & 0x80000000) >> 27;
				mct_code |= ((unsigned int)(m - (m6 << 3) - m6) & 0x80000000) >> 26;
				mct_code |= ((unsigned int)(m - (m7 << 3) - m7) & 0x80000000) >> 25;
				mct_code |= ((unsigned int)(m - (m8 << 3) - m8) & 0x80000000) >> 24;
				mct_code |= ((unsigned int)(m - (m9 << 3) - m9) & 0x80000000) >> 23;
				confidence += lut[mct_code];
				lut += 512;
			}
		}
		
		//LGP
		if(pCls->feature_num[FEATURE_IDX_LGP])
		{
			int *ffpos = (*one_stage_ffpos)[FEATURE_IDX_LGP];
			unsigned char *lgp_buf = &pFeature->img[y*width+x];
			unsigned short *lut = pCls->lookup_tbl[FEATURE_IDX_LGP];
			int fnum = pCls->feature_num[FEATURE_IDX_LGP];
			while(fnum--)
			{
				unsigned char *p1 = lgp_buf + *(ffpos++);
				unsigned char *p2 = p1 + width;
				unsigned char *p3 = p2 + width;
				int c = *(p2+1);
				int g[8];
				g[0] = abs(c - *p2);
				g[1] = abs(c - *p3);
				g[2] = abs(c - *(p3+1));
				g[3] = abs(c - *(p3+2));
				g[4] = abs(c - *(p2+2));
				g[5] = abs(c - *(p1+2));
				g[6] = abs(c - *(p1+1));
				g[7] = abs(c - *p1);

				int m = (g[0]+g[1]+g[2]+g[3]+g[4]+g[5]+g[6]+g[7])>>3;
				int lgp_code = (g[7]>m) | ((g[6]>m)<<1) | ((g[5]>m)<<2) | ((g[4]>m)<<3) | ((g[3]>m)<<4) | ((g[2]>m)<<5) | ((g[1]>m)<<6) | ((g[0]>m)<<7);

				confidence += lut[lgp_code];
				lut += 256;
			}
		}

		//clock_start("irmb2");
		//IRMB2
		if(pCls->feature_num[FEATURE_IDX_IRMB2])
		{
			int aligned_width = (((width+8)/8)*8);
			int *ffpos = (*one_stage_ffpos)[FEATURE_IDX_IRMB2];
			unsigned short *lut = pCls->lookup_tbl[FEATURE_IDX_IRMB2];
			unsigned char *irmb2_buf = &pFeature->irmb[y*aligned_width+x];
			int fnum = pCls->feature_num[FEATURE_IDX_IRMB2];
			while(fnum--)
			{
				int i0 = *(irmb2_buf + *ffpos++);
				int i1 = *(irmb2_buf + *ffpos++);
				int i2 = *(irmb2_buf + *ffpos++);
				int i3 = *(irmb2_buf + *ffpos++);
				int i4 = *(irmb2_buf + *ffpos++);
				int i5 = *(irmb2_buf + *ffpos++);
				
				int lcode = (i0>=i1) | ((i0 >=i2)<<1) | ((i0>=i3)<<2) | ((i0>=i4)<<3) | ((i0>=i5)<<4) |
						((i1>=i2)<<5) | ((i1>=i3)<<6) | ((i1>=i4)<<7) | ((i1>=i5)<<8) |
						((i2>=i3)<<9) | ((i2>=i4)<<10) | ((i2>=i5)<<11) | 
						((i3 >=i4)<<12) | ((i3>=i5)<<13) | 
						((i4>=i5)<<14);
				
				confidence += lut[irmb2_table[lcode]];
				lut += 720;
			}
		}
		//clock_end("irmb2");

		//SIMD8P
		if(pCls->feature_num[FEATURE_IDX_SIMD8P])
		{
			unsigned char v2_[8];
			int *ffpos = (*one_stage_ffpos)[FEATURE_IDX_SIMD8P];	
			unsigned char *img_buf[2] = {&pFeature->img[y*width+x], &pFeature->rot[x*height+y]};
			unsigned short *lut = pCls->lookup_tbl[FEATURE_IDX_SIMD8P];
			int fnum = pCls->feature_num[FEATURE_IDX_SIMD8P];
			while(fnum--)
			{
				unsigned char *v1 = img_buf[ffpos[0]] + ffpos[1];
				unsigned char *v2 = img_buf[ffpos[2]] + ffpos[3];
				unsigned char *m  = (unsigned char *)&ffpos[4];
				ffpos += 6;

				v2_[0] = v2[m[0]];
				v2_[1] = v2[m[1]];
				v2_[2] = v2[m[2]];
				v2_[3] = v2[m[3]];
				v2_[4] = v2[m[4]];
				v2_[5] = v2[m[5]];
				v2_[6] = v2[m[6]];
				v2_[7] = v2[m[7]];
				int code = (v1[0] >= v2_[0]) | ((v1[1] >= v2_[1])<<1) | ((v1[2] >= v2_[2])<<2) | ((v1[3] >= v2_[3])<<3) | ((v1[4] >= v2_[4])<<4) | ((v1[5] >= v2_[5])<<5) | ((v1[6] >= v2_[6])<<6) | ((v1[7] >= v2_[7])<<7);

				confidence += lut[code];
				lut += 256;				
			}
		}
		if(confidence <= pCls->threshold) 
		{
			return 0;
		}
		one_stage_ffpos++;
	} while(pCls = pCls->next);
	
	return confidence;
}

//Compute fast feature positions from original feature position.
//fast feature position depends on image size so it needs to be re-computed whenever image size is changed.
void MVFD_RescaleFeaturePositions(FD_CONTEXT *pContext, FD_CLASSIFIER *pClassifier, int width, int height, int *ffpos[])
{
	int fnum;
	if(pClassifier->feature_num[FEATURE_IDX_MCT] > 0) 
	{
		int *mct_ffpos = ffpos[FEATURE_IDX_MCT];
		POINT_FEATURE_POS *fpos = (POINT_FEATURE_POS *)pClassifier->feature_pos[FEATURE_IDX_MCT];
		fnum = pClassifier->feature_num[FEATURE_IDX_MCT];
		while(fnum--)
		{
			*mct_ffpos++ = fpos->y*width+fpos->x;
			fpos++;
		}
	}
	if(pClassifier->feature_num[FEATURE_IDX_MCT5x5] > 0)
	{
		int *mct5x5_ffpos = ffpos[FEATURE_IDX_MCT5x5];
		POINT_FEATURE_POS *fpos = (POINT_FEATURE_POS *)pClassifier->feature_pos[FEATURE_IDX_MCT5x5];
		fnum = pClassifier->feature_num[FEATURE_IDX_MCT5x5];
		while(fnum--)
		{
			*mct5x5_ffpos++ = (fpos->y-2)*width+(fpos->x-2);
			fpos++;
		}
	}

	if(pClassifier->feature_num[FEATURE_IDX_LGP] > 0)
	{
		int *lgp_ffpos = ffpos[FEATURE_IDX_LGP];
		POINT_FEATURE_POS *fpos = (POINT_FEATURE_POS *)pClassifier->feature_pos[FEATURE_IDX_LGP];
		fnum = pClassifier->feature_num[FEATURE_IDX_LGP];
		while(fnum--)
		{
			*lgp_ffpos++ = (fpos->y-1)*width+(fpos->x-1);
			fpos++;
		}
	}
	if(pClassifier->feature_num[FEATURE_IDX_IRMB2] > 0)
	{
		int *irmb2_ffpos = ffpos[FEATURE_IDX_IRMB2];
		IRMB2_FEATURE_POS *fpos = (IRMB2_FEATURE_POS *)pClassifier->feature_pos[FEATURE_IDX_IRMB2];
		fnum = pClassifier->feature_num[FEATURE_IDX_IRMB2];
		int aligned_width = (((width+8)/8)*8);
		int one_img_size = aligned_width*height;
		while(fnum--)
		{
			int *s = fpos->scale;
			FD_POINT *p = fpos->p;
			*irmb2_ffpos++ = s[0]*one_img_size + p[0].y*aligned_width+p[0].x;
			*irmb2_ffpos++ = s[1]*one_img_size + p[1].y*aligned_width+p[1].x;
			*irmb2_ffpos++ = s[2]*one_img_size + p[2].y*aligned_width+p[2].x;
			*irmb2_ffpos++ = s[3]*one_img_size + p[3].y*aligned_width+p[3].x;			
			*irmb2_ffpos++ = s[4]*one_img_size + p[4].y*aligned_width+p[4].x;
			*irmb2_ffpos++ = s[5]*one_img_size + p[5].y*aligned_width+p[5].x;
			fpos++;
		}
	}

	if(pClassifier->feature_num[FEATURE_IDX_SIMD8P] > 0)
	{
		int *simd8p_ffpos = ffpos[FEATURE_IDX_SIMD8P];
		SIMD8P_FEATURE_POS *fpos = (SIMD8P_FEATURE_POS *)pClassifier->feature_pos[FEATURE_IDX_SIMD8P];
		fnum = pClassifier->feature_num[FEATURE_IDX_SIMD8P];
		int one_img_size = width*height;
		while(fnum--)
		{
			*simd8p_ffpos++ = fpos->p[0].type;
			*simd8p_ffpos++ = fpos->p[0].type == 0 ?
					   fpos->p[0].y*width+fpos->p[0].x :
					   fpos->p[0].x*height+fpos->p[0].y;
			*simd8p_ffpos++ = fpos->p[1].type;
			*simd8p_ffpos++ = fpos->p[1].type == 0 ?
					   fpos->p[1].y*width+fpos->p[1].x :
					   fpos->p[1].x*height+fpos->p[1].y;
			memcpy(simd8p_ffpos, fpos->mask, 8);
			simd8p_ffpos += 2;
			fpos++;
		}
	}
}
#endif
