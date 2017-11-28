#include "stdafx.h"

#include <stdlib.h>
#include <memory.h>
#include "MVFD_Init.h"
#include "MVFD_Enroll.h"
#include "MVFD_Global.h"
#include "MVFD_Memory.h"
#include "IRMB2_CodeTable.h"
#include "MVFD_ImageProcessing.h"

#ifdef USE_SSE
#include <mmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
void MVFD_GeneratePoseMapForSmall(FD_CONTEXT *pContext, FD_CLASSIFIER *pRejector, FD_CLASSIFIER *pClassifier, FEATURE *pFeature, float rejection_rate, unsigned short *output, int *pmap_ffpos[])
{
	int valid_w = pFeature->width - FD_FACE_WIDTH;
	int valid_h = pFeature->height - FD_FACE_HEIGHT;
	int aligned_width = (((pFeature->width+8)/8)*8);
	unsigned char *rejection_map = MVFD_MemSection_Alloc<unsigned char>(pContext, MEM_SECTION_REJECTION_MAP, valid_w);
	unsigned short *code_image_buf = MVFD_MemSection_Alloc<unsigned short>(pContext, MEM_SECTION_IRMB2_CODE_BUF, valid_w*pClassifier->feature_num[FEATURE_IDX_IRMB2]);

	int *ffpos = pmap_ffpos[FEATURE_IDX_IRMB2];
	memset(output, 0, pFeature->width*pFeature->height*sizeof(unsigned short));

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

		unsigned short rej_threshold = pRejector->thresholds[0]*rejection_rate;
		unsigned short *pose_thresholds = pClassifier->thresholds;
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

__declspec(dllexport)  void MVFD_GeneratePoseMap(FD_CONTEXT *pContext, FD_CLASSIFIER *pRejector, FD_CLASSIFIER *pClassifier, FEATURE *pFeature, float rejection_rate, unsigned short *output, int *pmap_ffpos[])
{
	int valid_w = pFeature->width - FD_FACE_WIDTH;
	int valid_h = pFeature->height - FD_FACE_HEIGHT;
	int aligned_width = (((pFeature->width+8)/8)*8);
	__declspec(align(16)) unsigned short confidences_all[8];
	
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

			//Make 16 IRMB2 codes at a time using SIMD
			for(int x = 0; x < valid_w; x+=16)
			{
				if(x+16 > valid_w)
				{
					p = pFeature->irmb + y*aligned_width+(valid_w-16);
					wp += valid_w-(x+16);
				}
				__m128i temp;
				__m128i mask = _mm_set1_epi8(0x01);
				__m128i code1 = _mm_setzero_si128();
				__m128i val1, val2, val3, val4, val5, val6;

				val1 = _mm_lddqu_si128((__m128i *)(p+fpos[0]));
				val2 = _mm_lddqu_si128((__m128i *)(p+fpos[1]));
				val3 = _mm_lddqu_si128((__m128i *)(p+fpos[2]));
				val4 = _mm_lddqu_si128((__m128i *)(p+fpos[3]));
				val5 = _mm_lddqu_si128((__m128i *)(p+fpos[4]));
				val6 = _mm_lddqu_si128((__m128i *)(p+fpos[5]));
				
				temp = _mm_max_epu8(val1, val2);
				temp = _mm_cmpeq_epi8(val1, temp);
				temp = _mm_and_si128(temp, mask);
				code1 = _mm_or_si128(code1, temp);

				temp = _mm_max_epu8(val1, val3);
				temp = _mm_cmpeq_epi8(val1, temp);
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_and_si128(temp, mask);
				code1 = _mm_or_si128(code1, temp);
						
				temp = _mm_max_epu8(val1, val4);
				temp = _mm_cmpeq_epi8(val1, temp);
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_and_si128(temp, mask);
				code1 = _mm_or_si128(code1, temp);
						
				temp = _mm_max_epu8(val1, val5);
				temp = _mm_cmpeq_epi8(val1, temp);
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_and_si128(temp, mask);
				code1 = _mm_or_si128(code1, temp);

				temp = _mm_max_epu8(val1, val6);
				temp = _mm_cmpeq_epi8(val1, temp);
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_and_si128(temp, mask);
				code1 = _mm_or_si128(code1, temp);
						
				temp = _mm_max_epu8(val2, val3);
				temp = _mm_cmpeq_epi8(val2, temp);
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_and_si128(temp, mask);
				code1 = _mm_or_si128(code1, temp);
						
				temp = _mm_max_epu8(val2, val4);
				temp = _mm_cmpeq_epi8(val2, temp);
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_and_si128(temp, mask);
				code1 = _mm_or_si128(code1, temp);
						
				temp = _mm_max_epu8(val2, val5);
				temp = _mm_cmpeq_epi8(val2, temp);
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_and_si128(temp, mask);
				code1 = _mm_or_si128(code1, temp);

				////
				mask = _mm_set1_epi8(0x01);
				__m128i code2 = _mm_setzero_si128();
				////

				temp = _mm_max_epu8(val2, val6);
				temp = _mm_cmpeq_epi8(val2, temp);
				temp = _mm_and_si128(temp, mask);
				code2 = _mm_or_si128(code2, temp);
						
				temp = _mm_max_epu8(val3, val4);
				temp = _mm_cmpeq_epi8(val3, temp);
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_and_si128(temp, mask);
				code2 = _mm_or_si128(code2, temp);
						
				temp = _mm_max_epu8(val3, val5);
				temp = _mm_cmpeq_epi8(val3, temp);
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_and_si128(temp, mask);
				code2 = _mm_or_si128(code2, temp);
						
				temp = _mm_max_epu8(val3, val6);
				temp = _mm_cmpeq_epi8(val3, temp);
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_and_si128(temp, mask);
				code2 = _mm_or_si128(code2, temp);
						
				temp = _mm_max_epu8(val4, val5);
				temp = _mm_cmpeq_epi8(val4, temp);
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_and_si128(temp, mask);
				code2 = _mm_or_si128(code2, temp);
						
				temp = _mm_max_epu8(val4, val6);
				temp = _mm_cmpeq_epi8(val4, temp);
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_and_si128(temp, mask);
				code2 = _mm_or_si128(code2, temp);
						
				temp = _mm_max_epu8(val5, val6);
				temp = _mm_cmpeq_epi8(val5, temp);
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_and_si128(temp, mask);
				code2 = _mm_or_si128(code2, temp);

				temp = _mm_unpacklo_epi8(code1, code2);
				_mm_storeu_si128((__m128i *)wp, temp);
				wp += 8;
				temp = _mm_unpackhi_epi8(code1, code2);
				_mm_storeu_si128((__m128i *)wp, temp);
				wp += 8;
				p += 16;
			}
		}
		
		// long code -> short code
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
		
		unsigned short rej_threshold = pRejector->thresholds[0]*rejection_rate;
		unsigned short *pose_thresholds = pClassifier->thresholds;
		for(int x = 0; x < valid_w; x+=2)
		{
			int fpos = x;
			unsigned int confidence = 0;
			unsigned short *lut = pRejector->lookup_tbl[FEATURE_IDX_IRMB2];
			int f = pRejector->feature_num[FEATURE_IDX_IRMB2]>>2;
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
					
					code = code_image_buf[fpos];
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

			//if (x,y) passes rejector
			if(rejection_map[x])
			{
				int fpos = x;
				__m128i confidence = _mm_setzero_si128();
				__m128i *lut = (__m128i *)pClassifier->lookup_tbl[FEATURE_IDX_IRMB2];
				int f = pClassifier->feature_num[FEATURE_IDX_IRMB2];
				
				//Get 8 confidence values of pose classifier
				do
				{
					int code = code_image_buf[fpos];
					__m128i lut_conf1 = _mm_loadu_si128((__m128i *)&lut[code]);
					confidence = _mm_add_epi16(confidence, lut_conf1);
					fpos += valid_w;
					lut += 720;
				} while(--f);

				//Find the maximum and second one among 8 confidences
				__m128i temp = _mm_set1_epi16(0xffff);
				confidence = _mm_sub_epi16(temp, confidence);
				_mm_store_si128((__m128i *)confidences_all, confidence);

				__m128i min_pos;
				int first_val, first_idx, second_val, second_idx;
				min_pos = _mm_minpos_epu16(confidence);
				first_val = min_pos.m128i_u16[0]; first_idx = min_pos.m128i_u16[1];
				temp = _mm_set1_epi16(first_val);
				temp = _mm_cmpeq_epi16(confidence, temp);
				confidence = _mm_or_si128(confidence, temp);
				min_pos = _mm_minpos_epu16(confidence);
				second_val = min_pos.m128i_u16[0]; second_idx = min_pos.m128i_u16[1];
				
				output[oidx] = ((second_idx+1)<<8) | (first_idx+1);
				if(0xffff-first_val < pose_thresholds[first_idx]) output[oidx] &= 0xff00;
				if(0xffff-second_val < pose_thresholds[second_idx]) output[oidx] &= 0x00ff;
			}
			else output[oidx] = 0xffff;
		}
	}
}

void MVFD_GeneratePoseMapForTracking(FD_CONTEXT *pContext, FD_CLASSIFIER *pRejector, FD_CLASSIFIER *pClassifier, FEATURE *pFeature, float rejection_rate, unsigned short *output, int face_x, int face_y, int *pmap_ffpos[])
{
	int valid_w = pFeature->width - FD_FACE_WIDTH;
	int valid_h = pFeature->height - FD_FACE_HEIGHT;
	int aligned_width = (((pFeature->width+8)/8)*8);
	__declspec(align(16)) unsigned short confidences_all[24];
	
	unsigned char *rejection_map = MVFD_MemSection_Alloc<unsigned char>(pContext, MEM_SECTION_REJECTION_MAP, valid_w);
	unsigned short *code_image_buf = MVFD_MemSection_Alloc<unsigned short>(pContext, MEM_SECTION_IRMB2_CODE_BUF, valid_w*pClassifier->feature_num[FEATURE_IDX_IRMB2]);

	int *ffpos = pmap_ffpos[FEATURE_IDX_IRMB2];
	memset(output, 0, pFeature->width*pFeature->height*sizeof(unsigned short));

	for(int y = 0; y < valid_h; y +=2)
	{
		unsigned short *wp = code_image_buf;
		for(int f = 0; f < pClassifier->feature_num[FEATURE_IDX_IRMB2]; ++f)
		{
			int *fpos = &ffpos[f*6];
			unsigned char *p = pFeature->irmb + y*aligned_width;
			for(int x = 0; x < valid_w; x+=16)
			{
				if(x+16 > valid_w)
				{
					p = pFeature->irmb + y*aligned_width+(valid_w-16);
					wp += valid_w-(x+16);
				}
				__m128i temp;
				__m128i mask = _mm_set1_epi8(0x01);
				__m128i code1 = _mm_setzero_si128();
				__m128i val1, val2, val3, val4, val5, val6;

				val1 = _mm_lddqu_si128((__m128i *)(p+fpos[0]));
				val2 = _mm_lddqu_si128((__m128i *)(p+fpos[1]));
				val3 = _mm_lddqu_si128((__m128i *)(p+fpos[2]));
				val4 = _mm_lddqu_si128((__m128i *)(p+fpos[3]));
				val5 = _mm_lddqu_si128((__m128i *)(p+fpos[4]));
				val6 = _mm_lddqu_si128((__m128i *)(p+fpos[5]));
				
				temp = _mm_max_epu8(val1, val2);
				temp = _mm_cmpeq_epi8(val1, temp);
				temp = _mm_and_si128(temp, mask);
				code1 = _mm_or_si128(code1, temp);

				temp = _mm_max_epu8(val1, val3);
				temp = _mm_cmpeq_epi8(val1, temp);
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_and_si128(temp, mask);
				code1 = _mm_or_si128(code1, temp);
						
				temp = _mm_max_epu8(val1, val4);
				temp = _mm_cmpeq_epi8(val1, temp);
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_and_si128(temp, mask);
				code1 = _mm_or_si128(code1, temp);
						
				temp = _mm_max_epu8(val1, val5);
				temp = _mm_cmpeq_epi8(val1, temp);
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_and_si128(temp, mask);
				code1 = _mm_or_si128(code1, temp);

				temp = _mm_max_epu8(val1, val6);
				temp = _mm_cmpeq_epi8(val1, temp);
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_and_si128(temp, mask);
				code1 = _mm_or_si128(code1, temp);
						
				temp = _mm_max_epu8(val2, val3);
				temp = _mm_cmpeq_epi8(val2, temp);
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_and_si128(temp, mask);
				code1 = _mm_or_si128(code1, temp);
						
				temp = _mm_max_epu8(val2, val4);
				temp = _mm_cmpeq_epi8(val2, temp);
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_and_si128(temp, mask);
				code1 = _mm_or_si128(code1, temp);
						
				temp = _mm_max_epu8(val2, val5);
				temp = _mm_cmpeq_epi8(val2, temp);
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_and_si128(temp, mask);
				code1 = _mm_or_si128(code1, temp);

				////
				mask = _mm_set1_epi8(0x01);
				__m128i code2 = _mm_setzero_si128();
				////

				temp = _mm_max_epu8(val2, val6);
				temp = _mm_cmpeq_epi8(val2, temp);
				temp = _mm_and_si128(temp, mask);
				code2 = _mm_or_si128(code2, temp);
						
				temp = _mm_max_epu8(val3, val4);
				temp = _mm_cmpeq_epi8(val3, temp);
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_and_si128(temp, mask);
				code2 = _mm_or_si128(code2, temp);
						
				temp = _mm_max_epu8(val3, val5);
				temp = _mm_cmpeq_epi8(val3, temp);
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_and_si128(temp, mask);
				code2 = _mm_or_si128(code2, temp);
						
				temp = _mm_max_epu8(val3, val6);
				temp = _mm_cmpeq_epi8(val3, temp);
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_and_si128(temp, mask);
				code2 = _mm_or_si128(code2, temp);
						
				temp = _mm_max_epu8(val4, val5);
				temp = _mm_cmpeq_epi8(val4, temp);
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_and_si128(temp, mask);
				code2 = _mm_or_si128(code2, temp);
						
				temp = _mm_max_epu8(val4, val6);
				temp = _mm_cmpeq_epi8(val4, temp);
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_and_si128(temp, mask);
				code2 = _mm_or_si128(code2, temp);
						
				temp = _mm_max_epu8(val5, val6);
				temp = _mm_cmpeq_epi8(val5, temp);
				mask = _mm_slli_epi16(mask, 1);
				temp = _mm_and_si128(temp, mask);
				code2 = _mm_or_si128(code2, temp);

				temp = _mm_unpacklo_epi8(code1, code2);
				_mm_storeu_si128((__m128i *)wp, temp);
				wp += 8;
				temp = _mm_unpackhi_epi8(code1, code2);
				_mm_storeu_si128((__m128i *)wp, temp);
				wp += 8;
				p += 16;
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

		unsigned short rej_threshold = pRejector->thresholds[0]*rejection_rate;
		unsigned short *pose_thresholds = pClassifier->thresholds;
		for(int x = 0; x < valid_w; x++)
		{
			int fpos = x;
			unsigned int confidence = 0;
			unsigned short *lut = pRejector->lookup_tbl[FEATURE_IDX_IRMB2];
			int f = pRejector->feature_num[FEATURE_IDX_IRMB2]>>2;
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
					
					code = code_image_buf[fpos];
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
				int fpos = x;
				__m128i confidence = _mm_setzero_si128();
				__m128i *lut = (__m128i *)pClassifier->lookup_tbl[FEATURE_IDX_IRMB2];
				int f = pClassifier->feature_num[FEATURE_IDX_IRMB2];
				
				//Get 8 confidence values of pose classifier
				do
				{
					int code = code_image_buf[fpos];
					__m128i lut_conf1 = _mm_loadu_si128((__m128i *)&lut[code]);
					confidence = _mm_add_epi16(confidence, lut_conf1);
					fpos += valid_w;
					lut += 720;
				} while(--f);

				//Find the maximum and second one among 8 confidences
				__m128i temp = _mm_set1_epi16(0xffff);
				confidence = _mm_sub_epi16(temp, confidence);
				_mm_store_si128((__m128i *)confidences_all, confidence);

				__m128i min_pos;
				int first_val, first_idx, second_val, second_idx;
				min_pos = _mm_minpos_epu16(confidence);
				first_val = min_pos.m128i_u16[0]; first_idx = min_pos.m128i_u16[1];
				temp = _mm_set1_epi16(first_val);
				temp = _mm_cmpeq_epi16(confidence, temp);
				confidence = _mm_or_si128(confidence, temp);
				min_pos = _mm_minpos_epu16(confidence);
				second_val = min_pos.m128i_u16[0]; second_idx = min_pos.m128i_u16[1];
				
				output[oidx] = ((second_idx+1)<<8) | (first_idx+1);
				if(0xffff-first_val < pose_thresholds[first_idx]) output[oidx] &= 0xff00;
				if(0xffff-second_val < pose_thresholds[second_idx]) output[oidx] &= 0x00ff;
			}
			else output[oidx] = 0;
		}
	}
}

int MVFD_IsFace(FEATURE *pFeature, int x, int y, FD_CLASSIFIER *pClassifier, int *all_ffpos[][FD_FEATURE_TYPE_NUM])
{
	FD_CLASSIFIER *pCls = pClassifier;
	int width = pFeature->width;
	int height = pFeature->height;
	int confidence;
	__declspec(align(16)) unsigned char mask1[] = {0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 3, 3, 4, 0};
	__declspec(align(16)) unsigned char mask2[] = {1, 2, 3, 4, 5, 2, 3, 4, 5, 3, 4, 5, 4, 5, 5, 0};

	int *(*one_stage_ffpos)[FD_FEATURE_TYPE_NUM] = all_ffpos;
	do
	{
		confidence = 0;
		
		//clock_start("irmb2");
		//IRMB2
		if(pCls->feature_num[FEATURE_IDX_IRMB2])
		{
			int aligned_width = (((width+8)/8)*8);
			int *ffpos = (*one_stage_ffpos)[FEATURE_IDX_IRMB2];
			unsigned short *lut = pCls->lookup_tbl[FEATURE_IDX_IRMB2];
			unsigned char *irmb2_buf = &pFeature->irmb[y*aligned_width+x];
			int fnum = pCls->feature_num[FEATURE_IDX_IRMB2];
			__m128i mm_mask1 = _mm_load_si128((__m128i *)mask1);
			__m128i mm_mask2 = _mm_load_si128((__m128i *)mask2);
			while(fnum--)
			{
				__m128i temp;
				temp.m128i_u8[0] = *(irmb2_buf + *ffpos++);
				temp.m128i_u8[1] = *(irmb2_buf + *ffpos++);
				temp.m128i_u8[2] = *(irmb2_buf + *ffpos++);
				temp.m128i_u8[3] = *(irmb2_buf + *ffpos++);
				temp.m128i_u8[4] = *(irmb2_buf + *ffpos++);
				temp.m128i_u8[5] = *(irmb2_buf + *ffpos++);
				__m128i ival1 = _mm_shuffle_epi8(temp, mm_mask1);
				__m128i ival2 = _mm_shuffle_epi8(temp, mm_mask2);
				temp = _mm_max_epu8(ival1, ival2);
				temp = _mm_cmpeq_epi8(temp, ival1);
				int lcode = _mm_movemask_epi8(temp) & 0x7fff;
				confidence += lut[irmb2_table[lcode]];
				lut += 720;
			}
		}
		//clock_end("irmb2");

		//SIMD8P
		if(pCls->feature_num[FEATURE_IDX_SIMD8P])
		{
			int *ffpos = (*one_stage_ffpos)[FEATURE_IDX_SIMD8P];			
			unsigned char *img_buf[2] = {&pFeature->img[y*width+x], &pFeature->rot[x*height+y]};
			unsigned short *lut = pCls->lookup_tbl[FEATURE_IDX_SIMD8P];
			int fnum = pCls->feature_num[FEATURE_IDX_SIMD8P];
			int i = 0;
			while(fnum--)
			{
				__m64 value1 = *((__m64 *)(img_buf[ffpos[0]] + ffpos[1]));
				__m64 value2 = *((__m64 *)(img_buf[ffpos[2]] + ffpos[3]));
				__m64 mask = *((__m64 *)&ffpos[4]);
				ffpos += 6;

				value2 = _mm_shuffle_pi8(value2, mask);
				__m64 temp = _mm_max_pu8(value1, value2);
				temp = _mm_cmpeq_pi8(value1, temp);
				int code = _mm_movemask_pi8(temp);
				
				confidence += lut[code];
				lut += 256;
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

				/*
				//it's strange that SSE is slower than C code in this case.
				unsigned char *p1 = mct5x5_buf + *(ffpos++);
				unsigned char *p2 = p1 + (width<<1);
				unsigned char *p3 = p2 + (width<<1);
				unsigned short mm9 = *(p3+4);
				__m128i xmm1, xmm2, xmm3, xmm4, xmm5, xmm6;
				xmm1 = _mm_setzero_si128();
				xmm2 = _mm_set_epi16(*(p3+2), *p3, *(p2+4), *(p2+2), *p2, *(p1+4), *(p1+2), *p1);
				xmm3 = xmm2;
				xmm3 = _mm_hadd_epi16(xmm3, xmm1);
				xmm3 = _mm_hadd_epi16(xmm3, xmm1);
				xmm3 = _mm_hadd_epi16(xmm3, xmm1);
				unsigned short mm = xmm3.m128i_u16[0]+ *(p3+4);
				xmm3 = _mm_slli_epi16(xmm2, 3);
				xmm3 = _mm_add_epi16(xmm3, xmm2);					
				xmm4 = _mm_set1_epi16(mm);
				xmm5 = _mm_cmplt_epi16(xmm4, xmm3);
				xmm6 = _mm_packs_epi16(xmm5, xmm1);
				unsigned int code8 = _mm_movemask_epi8(xmm6) & 0xff;
				unsigned int mct_code = ((((mm9<<3)+mm9)>mm) << 8) | code8;
				confidence += lut[mct_code];
				lut += 512;
				*/
			}
		}

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
				__m128i xmm1, xmm2, xmm3, xmm4, xmm5, xmm6;
				xmm1 = _mm_setzero_si128();
				xmm2 = _mm_set_epi16(*p2, *p3, *(p3+1), *(p3+2), *(p2+2), *(p1+2), *(p1+1), *p1);
				xmm3 = _mm_set1_epi16(*(p2+1));
		
				xmm3 = _mm_abs_epi16(_mm_sub_epi16(xmm2, xmm3));

				xmm4 = _mm_hadd_epi16(xmm3, xmm1);
				xmm4 = _mm_hadd_epi16(xmm4, xmm1);
				xmm4 = _mm_hadd_epi16(xmm4, xmm1);
				unsigned short m = xmm4.m128i_u16[0]>>3; //mean

				xmm4 = _mm_set1_epi16(m);
				xmm5 = _mm_cmplt_epi16(xmm4, xmm3);
				xmm6 = _mm_packs_epi16(xmm5, xmm1);

				int lgp_code = _mm_movemask_epi8(xmm6) & 0xff;
				confidence += lut[lgp_code];
				lut += 256;

			}
		}
		if(pCls->next == NULL) break;
		if(confidence <= pCls->threshold) 
		{
			_mm_empty();
			return 0;
		}
		one_stage_ffpos++;
	} while(pCls = pCls->next);
	_mm_empty();
	
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
