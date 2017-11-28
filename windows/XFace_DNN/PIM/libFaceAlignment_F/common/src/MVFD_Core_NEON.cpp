// MVFD2.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include <stdlib.h>
#include "MVFD_Defines.h"
#include "MVFD_Init.h"
#include "MVFD_Memory.h"
#include "MVFD_Enroll.h"
#include "MVFD_Global.h"
#include "IRMB2_CodeTable.h"

#ifdef USE_NEON

#define USE_LCODE_GEN_ASM			1
#define USE_LCODE_2_SHORT_ASM	1
#define USE_MCT5x5_ASM				1

extern "C" void MVFD_IRMB_lcode_Gen_detect_arm(int *fpos, unsigned char *p, int valid_w, unsigned short **wp, unsigned short *wp_last);
extern "C" void MVFD_IRMB_lcode_Gen_arm(int *fpos, unsigned char *p, int valid_w, unsigned short **wp);
extern "C" void MVFD_IRMB_longtoshort_arm_new(const int *irmb2_table, unsigned short *p, int xcnt, int remain );
extern "C" int MVFD_IsFaceMCT_Conf_arm_new(unsigned short *mct_buf,int *ffpos, int fnum,unsigned short *lut);
extern "C" int MVFD_IsFaceSIMD8PConf(unsigned short *lut,unsigned char * code_image_buf,int fnum);
extern "C" int MVFD_IsFaceIRMB(unsigned char * irmb_image_buf,int *ffpos,unsigned char * code_image_buf,int fnum,unsigned short * lut,const int * irmb2_tabl);
extern "C" int MVFD_IsFaceMCT5x5(unsigned char * mct_5x5_buf,int *ffpos,unsigned char * code_image_buf, int fnum_By_16, unsigned short * lut, int width);

void MVFD_IRMB_lcode_Gen(int *fpos, unsigned char *p, int valid_w, unsigned short **wp)
{	
	int x;
	
	unsigned char *p0 = p+fpos[0];
	unsigned char *p1 = p+fpos[1];
	unsigned char *p2 = p+fpos[2];
	unsigned char *p3 = p+fpos[3];
	unsigned char *p4 = p+fpos[4];
	unsigned char *p5 = p+fpos[5];
	unsigned short *wpn = *wp;
	
	for(x = 0; x < valid_w; x+=2)
	{
		int i0 = *p0++, i1 = *p1++, i2 = *p2++, i3 = *p3++, i4 = *p4++, i5 = *p5++;
	
		int lcode = (i0>=i1) | ((i0 >=i2)<<1) | ((i0>=i3)<<2) | ((i0>=i4)<<3) | ((i0>=i5)<<4) |
				((i1>=i2)<<5) | ((i1>=i3)<<6) | ((i1>=i4)<<7) | ((i1>=i5)<<8) |
				((i2>=i3)<<9) | ((i2>=i4)<<10) | ((i2>=i5)<<11) | 
				((i3 >=i4)<<12) | ((i3>=i5)<<13) | 
				((i4>=i5)<<14);
		*wpn = lcode;
		wpn += 1;
		//p++;
		//p++;
		p0++;
		p1++;
		p2++;
		p3++;
		p4++;
		p5++;
	}
	*wp += valid_w;

}

void MVFD_GeneratePoseMap(FD_CONTEXT *pContext, FD_CLASSIFIER *pRejector, FD_CLASSIFIER *pClassifier, FEATURE *pFeature, unsigned short *output, int *pmap_ffpos[])
{
	int valid_w = pFeature->width - FD_FACE_WIDTH;
	int valid_h = pFeature->height - FD_FACE_HEIGHT;
	int aligned_width = (((pFeature->width+8)/8)*8);
	//unsigned char *rejection_map = MVFD_MemSection_Alloc<unsigned char>(pContext, MEM_SECTION_REJECTION_MAP, valid_w);
	unsigned short *code_image_buf = MVFD_MemSection_Alloc<unsigned short>(pContext, MEM_SECTION_IRMB2_CODE_BUF, valid_w*pClassifier->feature_num[FEATURE_IDX_IRMB2]);
	
	int *ffpos = pmap_ffpos[FEATURE_IDX_IRMB2];
	memset(output, 0, pFeature->width*pFeature->height*sizeof(unsigned short));

	unsigned const short rej_threshold = (unsigned short)pRejector->thresholds[0]*FD_REJECTION_RATE;
	unsigned const short *pose_thresholds = (unsigned short *)pClassifier->thresholds;
	unsigned const short *lookup_tbl = pRejector->lookup_tbl[FEATURE_IDX_IRMB2];
	for(int y = 0; y < valid_h; y +=2)
	{
		unsigned short *wp = code_image_buf;
		for(int f = 0; f < pClassifier->feature_num[FEATURE_IDX_IRMB2]; ++f)
		{
			int *fpos = &ffpos[f*6];
			unsigned char *p = pFeature->irmb + y*aligned_width;
			
#if (USE_LCODE_GEN_ASM == 1)
			if(valid_w >= 32)
			{
				MVFD_IRMB_lcode_Gen_detect_arm(fpos, p, valid_w, &wp, (wp + (valid_w >> 1) -16));
			}
			else
			{
				MVFD_IRMB_lcode_Gen(fpos, p, valid_w, &wp);
			}
#elif 1
			
			MVFD_IRMB_lcode_Gen(fpos, p, valid_w, &wp);
			
#else // C- Step size 1 - Code changes are required to enable this
#warning "Code changes are required to enable this"
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
#endif //(USE_LCODE_GEN_ASM == 1)
		}
		
		int fnum = pClassifier->feature_num[FEATURE_IDX_IRMB2];
		unsigned short *p = code_image_buf;
		unsigned short *sp = code_image_buf;
		while(fnum--)
		{
			int xcnt = valid_w>>3;
			int remain = (valid_w-(xcnt<<3)+1)>>1;
			p = sp;
#if (USE_LCODE_2_SHORT_ASM == 0)
			while(xcnt--)
			{
				*p = irmb2_table[*p];
				p+=1;
				*p = irmb2_table[*p];
				p+=1;
				*p = irmb2_table[*p];
				p+=1;
				*p = irmb2_table[*p];
				p+=1;
			}
			while(remain--)
			{
				*p = irmb2_table[*p];
				p+=1;
			}
#else
			//LOGE("fnum:%d,p:%p,xcnt:%d,remain:%d",fnum,p,xcnt,remain);
			MVFD_IRMB_longtoshort_arm_new(irmb2_table, p, xcnt,remain );
#endif
			sp += valid_w;
		}

		for(int x = 0; x < valid_w; x+=2)
		{


#if 1 //(USE_FEATURE_DEP_OPT == 0)
			int fpos = x >> 1;
			unsigned int confidence1 = 0,confidence2 = 0;
			unsigned short *lut = pRejector->lookup_tbl[FEATURE_IDX_IRMB2];
			int f = pRejector->feature_num[FEATURE_IDX_IRMB2]>>1;
			do
			{
				int code = code_image_buf[fpos];
				confidence1 += lut[code];
				lut += 720;
				fpos += valid_w;
				code = code_image_buf[fpos];
				confidence2 += lut[code];
				lut += 720;
				fpos += valid_w;
			} while(--f);
#else
#warning "Rejecter  Feature Dependent Changes"
			int fpos = x >> 1;
			unsigned int confidence1 = 0,confidence2 = 0;
			unsigned short * code_buf = (unsigned short *)(code_image_buf + fpos);
			unsigned const short *lut = lookup_tbl;
			int f = pRejector->feature_num[FEATURE_IDX_IRMB2]>>1;
			{
				int code1, code2;
				
				code1 = *code_buf;
				code_buf += valid_w;
				code2 = *code_buf;
				code_buf += valid_w;
				confidence1 += lut[code1];
				lut += 720;
				confidence2 += lut[code2];
				lut += 720;
				
				code1 = *code_buf;
				code_buf += valid_w;
				code2 = *code_buf;
				code_buf += valid_w;
				confidence1 += lut[code1];
				lut += 720;
				confidence2 += lut[code2];
				lut += 720;

				code1 = *code_buf;
				code_buf += valid_w;
				code2 = *code_buf;
				code_buf += valid_w;
				confidence1 += lut[code1];
				lut += 720;
				confidence2 += lut[code2];
				lut += 720;

				code1 = *code_buf;
				code_buf += valid_w;
				code2 = *code_buf;	
				code_buf += valid_w;
				confidence1 += lut[code1];
				lut += 720;
				confidence2 += lut[code2];
				lut += 720;


				code1 = *code_buf;
				code_buf += valid_w;
				code2 = *code_buf;	
				code_buf += valid_w;
				confidence1 += lut[code1];
				lut += 720;
				confidence2 += lut[code2];
				lut += 720;

				code1 = *code_buf;
				code_buf += valid_w;
				code2 = *code_buf;	
				code_buf += valid_w;
				confidence1 += lut[code1];
				lut += 720;
				confidence2 += lut[code2];
				lut += 720;

				code1 = *code_buf;
				code_buf += valid_w;
				code2 = *code_buf;	
				code_buf += valid_w;
				confidence1 += lut[code1];
				lut += 720;
				confidence2 += lut[code2];
				lut += 720;

				code1 = *code_buf;
				code_buf += valid_w;
				code2 = *code_buf;		
				code_buf += valid_w;
				confidence1 += lut[code1];
				lut += 720;
				confidence2 += lut[code2];
				lut += 720;

				code1 = *code_buf;
				code_buf += valid_w;
				code2 = *code_buf;	
				code_buf += valid_w;
				confidence1 += lut[code1];
				lut += 720;
				confidence2 += lut[code2];
				lut += 720;

				code1 = *code_buf;
				code_buf += valid_w;
				code2 = *code_buf;	
				code_buf += valid_w;
				confidence1 += lut[code1];
				lut += 720;
				confidence2 += lut[code2];
				lut += 720;

				code1 = *code_buf;
				code_buf += valid_w;
				code2 = *code_buf;		
				code_buf += valid_w;
				confidence1 += lut[code1];
				lut += 720;
				confidence2 += lut[code2];
				lut += 720;

				code1 = *code_buf;
				code_buf += valid_w;
				code2 = *code_buf;	
				code_buf += valid_w;
				confidence1 += lut[code1];
				lut += 720;
				confidence2 += lut[code2];
				lut += 720;

				code1 = *code_buf;
				code_buf += valid_w;
				code2 = *code_buf;	
				code_buf += valid_w;
				confidence1 += lut[code1];
				lut += 720;
				confidence2 += lut[code2];
				lut += 720;

				code1 = *code_buf;
				code_buf += valid_w;
				code2 = *code_buf;	
				code_buf += valid_w;
				confidence1 += lut[code1];
				lut += 720;
				confidence2 += lut[code2];
				lut += 720;

				code1 = *code_buf;
				code_buf += valid_w;
				code2 =*code_buf;		
				code_buf += valid_w;
				confidence1 += lut[code1];
				lut += 720;
				confidence2 += lut[code2];
				lut += 720;

				code1 = *code_buf;
				code_buf += valid_w;
				code2 = *code_buf;	
				confidence1 += lut[code1];
				lut += 720;
				confidence2 += lut[code2];

			}
#endif
			int oidx = y*pFeature->width+x;
			if((confidence1 + confidence2) > rej_threshold)
			{
#if 1
				int fnum = pClassifier->feature_num[FEATURE_IDX_IRMB2];
				int fpos = x >> 1;

				unsigned short *lut = (unsigned short *)pClassifier->lookup_tbl[FEATURE_IDX_IRMB2];
#if 0
				unsigned short confidence[8] = {0,};
				for(int f = 0; f < fnum; ++f)
				{
					unsigned short *one_lut = lut;
					int code = code_image_buf[fpos];
					unsigned short *weak_result = &one_lut[code<<3];
					
					for(int i = 0; i < 8; ++i)
					{
						confidence[i] += weak_result[i];
					}

					fpos += valid_w;
					lut += 720*8;
				}
				
#else
				unsigned short confidence[8];
				uint16x8_t conf_16x8, temp_16x8;
				conf_16x8 = veorq_u16(conf_16x8, conf_16x8);
				for(int f = 0; f < fnum; ++f)
				{
					unsigned short *one_lut = lut;
					int code = code_image_buf[fpos];
					unsigned short *weak_result = &one_lut[code<<3];
					
					temp_16x8 = vld1q_u16(weak_result);
					conf_16x8 = vaddq_u16(conf_16x8, temp_16x8);

					fpos += valid_w;
					lut += 720*8;
				}
				vst1q_u16(confidence,conf_16x8);
			
#endif
					
#else
				int fnum = pClassifier->feature_num[FEATURE_IDX_IRMB2];
				int fpos = x;
				unsigned short confidence[8] = {0,};
				unsigned short (*lut)[720][8] = (unsigned short (*)[720][8])pClassifier->lookup_tbl[FEATURE_IDX_IRMB2];
#if 1 
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
#else
				uint16x8_t conf_16x8,temp_16x8;
				conf_16x8 = vdupq_n_u16(0);
				unsigned short (*one_lut)[8] = lut[0];
				for(int f = 0; f < fnum; ++f)
				{
					int code = code_image_buf[fpos];
					unsigned short *weak_result = one_lut[code];
					
					temp_16x8 = vld1q_u16(weak_result);
					conf_16x8 = vaddq_u16(conf_16x8, temp_16x8);
					fpos += valid_w;
					one_lut += 720;
				}			
				vst1q_u16(confidence,conf_16x8);
#endif
#endif
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
			else output[oidx] = 0xFFFF;
		}
	}
}

void MVFD_GeneratePoseMapForTracking(FD_CONTEXT *pContext, FD_CLASSIFIER *pRejector, FD_CLASSIFIER *pClassifier, FEATURE *pFeature, float rejection_rate, unsigned short *output, int face_x, int face_y, int *pmap_ffpos[])
{
	int valid_w = pFeature->width - FD_FACE_WIDTH;
	int valid_h = pFeature->height - FD_FACE_HEIGHT;
	int aligned_width = (((pFeature->width+8)/8)*8);
	unsigned short confidences_all[8];
	
	//unsigned char *rejection_map = MVFD_MemSection_Alloc<unsigned char>(pContext, MEM_SECTION_REJECTION_MAP, valid_w);
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
#if 	(USE_LCODE_GEN_ASM == 1)
			MVFD_IRMB_lcode_Gen_arm(fpos, p, valid_w, &wp);
#else
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
#endif //(USE_LCODE_GEN_ASM == 1)
		}

		int fnum = pClassifier->feature_num[FEATURE_IDX_IRMB2];
		unsigned short *p = code_image_buf;
		unsigned short *sp = code_image_buf;
		while(fnum--)
		{
			int xcnt = valid_w>>2;
			int remain = valid_w-(xcnt<<2);
			p = sp;
#if (USE_LCODE_2_SHORT_ASM == 0)		
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
#else
			MVFD_IRMB_longtoshort_arm_new(irmb2_table, p, xcnt,remain );
#endif
			sp += valid_w;
		}

		unsigned short rej_threshold = (unsigned short)(pRejector->thresholds[0]*rejection_rate);
		unsigned short *pose_thresholds = (unsigned short *)pClassifier->thresholds;
		for(int x = 0; x < valid_w; x++)
		{
			int fpos = x;
			unsigned short *lut = pRejector->lookup_tbl[FEATURE_IDX_IRMB2];
			int f = pRejector->feature_num[FEATURE_IDX_IRMB2]>>1;
			int oidx = y*pFeature->width+x;
			if((x>>1)<<1 == x || (face_x-8 < x && x < face_x+8 && face_y-8 < y && y < face_y+8))
			{

#if 1 //(USE_FEATURE_DEP_OPT == 0)
				{
					unsigned int confidence = 0;
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
					if(confidence <= rej_threshold) 
					{
						output[oidx] = 0;
						continue;
					}
				}

#else
#warning " Rejector Feature Dependent Changes"
				unsigned short * code_buf = (unsigned short *)(code_image_buf + fpos);
				unsigned int confidence1 = 0,confidence2 = 0;

				{
					int code1, code2;
					
					code1 = *code_buf;

					code_buf += valid_w;
					code2 = *code_buf;
					code_buf += valid_w;
					confidence1 += lut[code1];
					lut += 720;
					confidence2 += lut[code2];
					lut += 720;
					
					code1 = *code_buf;
					code_buf += valid_w;
					code2 = *code_buf;
					code_buf += valid_w;
					confidence1 += lut[code1];
					lut += 720;
					confidence2 += lut[code2];
					lut += 720;

					code1 = *code_buf;
					code_buf += valid_w;
					code2 = *code_buf;
					code_buf += valid_w;
					confidence1 += lut[code1];
					lut += 720;
					confidence2 += lut[code2];
					lut += 720;

					code1 = *code_buf;
					code_buf += valid_w;
					code2 = *code_buf;	
					code_buf += valid_w;
					confidence1 += lut[code1];
					lut += 720;
					confidence2 += lut[code2];
					lut += 720;


					code1 = *code_buf;
					code_buf += valid_w;
					code2 = *code_buf;	
					code_buf += valid_w;
					confidence1 += lut[code1];
					lut += 720;
					confidence2 += lut[code2];
					lut += 720;

					code1 = *code_buf;
					code_buf += valid_w;
					code2 = *code_buf;	
					code_buf += valid_w;
					confidence1 += lut[code1];
					lut += 720;
					confidence2 += lut[code2];
					lut += 720;

					code1 = *code_buf;
					code_buf += valid_w;
					code2 = *code_buf;	
					code_buf += valid_w;
					confidence1 += lut[code1];
					lut += 720;
					confidence2 += lut[code2];
					lut += 720;

					code1 = *code_buf;
					code_buf += valid_w;
					code2 = *code_buf;		
					code_buf += valid_w;
					confidence1 += lut[code1];
					lut += 720;
					confidence2 += lut[code2];
					lut += 720;

					code1 = *code_buf;
					code_buf += valid_w;
					code2 = *code_buf;	
					code_buf += valid_w;
					confidence1 += lut[code1];
					lut += 720;
					confidence2 += lut[code2];
					lut += 720;

					code1 = *code_buf;
					code_buf += valid_w;
					code2 = *code_buf;	
					code_buf += valid_w;
					confidence1 += lut[code1];
					lut += 720;
					confidence2 += lut[code2];
					lut += 720;

					code1 = *code_buf;
					code_buf += valid_w;
					code2 = *code_buf;		
					code_buf += valid_w;
					confidence1 += lut[code1];
					lut += 720;
					confidence2 += lut[code2];
					lut += 720;

					code1 = *code_buf;
					code_buf += valid_w;
					code2 = *code_buf;	
					code_buf += valid_w;
					confidence1 += lut[code1];
					lut += 720;
					confidence2 += lut[code2];
					lut += 720;

					code1 = *code_buf;
					code_buf += valid_w;
					code2 = *code_buf;	
					code_buf += valid_w;
					confidence1 += lut[code1];
					lut += 720;
					confidence2 += lut[code2];
					lut += 720;

					code1 = *code_buf;
					code_buf += valid_w;
					code2 = *code_buf;	
					code_buf += valid_w;
					confidence1 += lut[code1];
					lut += 720;
					confidence2 += lut[code2];
					lut += 720;

					code1 = *code_buf;
					code_buf += valid_w;
					code2 =*code_buf;		
					code_buf += valid_w;
					confidence1 += lut[code1];
					lut += 720;
					confidence2 += lut[code2];
					lut += 720;

					code1 = *code_buf;
					code_buf += valid_w;
					code2 = *code_buf;	
					confidence1 += lut[code1];
					lut += 720;
					confidence2 += lut[code2];

				}
				if((confidence1 + confidence2) <= rej_threshold) 
				{
					output[oidx] = 0;
					continue;
				}
#endif

				int fnum = pClassifier->feature_num[FEATURE_IDX_IRMB2];
				int fpos = x;
#if 0
				unsigned short confidence[8] = {0,};
				//bug fixed by postech 2014.05.13
				unsigned short (*lut)[720][8] = (unsigned short (*)[720][8])pClassifier->lookup_tbl[FEATURE_IDX_IRMB2];
#if 0
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

#else
				uint16x8_t conf_16x8, temp_16x8;
				conf_16x8 = vdupq_n_u16(0);
				for(int f = 0; f < fnum; ++f)
				{
					unsigned short (*one_lut)[8] = lut[f];
					int code = code_image_buf[fpos];
					unsigned short *weak_result = one_lut[code];

					temp_16x8 = vld1q_u16(weak_result);
					conf_16x8 = vaddq_u16(conf_16x8, temp_16x8);

					fpos += valid_w;
				}

				vst1q_u16(confidence,conf_16x8);

#endif			
#else

				unsigned short *lut = (unsigned short *)pClassifier->lookup_tbl[FEATURE_IDX_IRMB2];
#if 0
				unsigned short confidence[8] = {0,};
				for(int f = 0; f < fnum; ++f)
				{
					unsigned short *one_lut = lut;
					int code = code_image_buf[fpos];
					unsigned short *weak_result = &one_lut[code<<3];
					
					for(int i = 0; i < 8; ++i)
					{
						confidence[i] += weak_result[i];
					}

					fpos += valid_w;
					lut += 720*8;
				}
				
#else
				unsigned short confidence[8];
				uint16x8_t conf_16x8, temp_16x8;
				conf_16x8 = veorq_u16(conf_16x8, conf_16x8);
				for(int f = 0; f < fnum; ++f)
				{
					unsigned short *one_lut = lut;
					int code = code_image_buf[fpos];
					unsigned short *weak_result = &one_lut[code<<3];
					
					temp_16x8 = vld1q_u16(weak_result);
					conf_16x8 = vaddq_u16(conf_16x8, temp_16x8);

					fpos += valid_w;
					lut += 720*8;
				}
				vst1q_u16(confidence,conf_16x8);
			
#endif
			
#endif
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


int MVFD_IsFace(FEATURE *pFeature, int x, int y, FD_CLASSIFIER *pClassifier,unsigned int * code_image_buf, int *all_ffpos[][FD_FEATURE_TYPE_NUM])
{
	FD_CLASSIFIER *pCls = pClassifier;
	int width = pFeature->width;
	int height = pFeature->height;
	unsigned int confidence;
	int y_width_x = y*width+x;
	
	int aligned_width = (((width+8)& 0xFFFFFFF8));
	int x_height_p_y = x*height+y;
	int y_align_width_p_x = y*aligned_width+x;	
	unsigned short *mct_image_buf = &pFeature->mct[y_width_x];
	unsigned char *lgp_image_buf = &pFeature->img[y_width_x];
	unsigned char *irmb_image_buf = &pFeature->irmb[y_align_width_p_x];
	unsigned char *simd8p_image_buf[2] = {&pFeature->img[y_width_x], &pFeature->rot[x_height_p_y]};
	
	int *(*one_stage_ffpos)[FD_FEATURE_TYPE_NUM] = all_ffpos;
	do
	{
		confidence = 0;

#if (USE_FEATURE_DEP_OPT == 1)
#warning "Check is removed as pCls->feature_num[FEATURE_IDX_MCT] is always greater then 0"
#else
		if(pCls->feature_num[FEATURE_IDX_MCT])
#endif
		{
#if	(DOUBLE_MCT_AND_FASTPOSE == 0)
			confidence +=  MVFD_IsFaceMCT_Conf_arm(pCls, pFeature,x,y);
#else
			int *ffpos = (*one_stage_ffpos)[FEATURE_IDX_MCT];
			unsigned short *mct_buf = mct_image_buf;
			unsigned short *lut = pCls->lookup_tbl[FEATURE_IDX_MCT];

			confidence = MVFD_IsFaceMCT_Conf_arm_new(mct_buf,ffpos,pCls->feature_num[FEATURE_IDX_MCT],lut);
		}
#endif

#if (USE_MCT5x5_ASM == 0)
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
#else
		if(pCls->feature_num[FEATURE_IDX_MCT5x5])
		{
			int *ffpos = (*one_stage_ffpos)[FEATURE_IDX_MCT5x5];
			unsigned char *mct5x5_buf = &pFeature->img[y*width+x];
			unsigned short *lut = pCls->lookup_tbl[FEATURE_IDX_MCT5x5];
			int fnum = pCls->feature_num[FEATURE_IDX_MCT5x5];
			int	fnum_By_16 = fnum >> 4;
			int fnum_Mod_16 = fnum & 0xF;
			unsigned char * code_buf_char =(unsigned char *)code_image_buf;
			unsigned short * code_buf_short =(unsigned short *)code_image_buf;
			
#if 1
			confidence += MVFD_IsFaceMCT5x5(mct5x5_buf,ffpos,(unsigned char *)code_image_buf,fnum_By_16, lut, width);
			
			ffpos += (fnum_By_16<<4);
			lut += 512 * (fnum_By_16<<4);
			
#else
			while(fnum_By_16--)
			{
				unsigned char * pxl = code_buf_char;
				for(int i = 0; i < 16; i++)
				{
					unsigned char *p1 = mct5x5_buf + *(ffpos++);
					unsigned char *p2 = p1 + (width<<1);
					unsigned char *p3 = p2 + (width<<1);
					*pxl++ = * p1;
					*pxl++ = *(p1+2);
					*pxl++ = *(p1+4);
					*pxl++ = * p2;
					*pxl++ = *(p2+2);
					*pxl++ = *(p2+4);
					*pxl++ = * p3;
					*pxl++ = *(p3+2);
					*pxl++ = *(p3+4);
				}
				
				pxl = code_buf_char;
				unsigned short * code = code_buf_short;
				for(int i = 0; i < 16; i++)
				{
					unsigned char m1 = *pxl++;
					unsigned char m2 = *pxl++;
					unsigned char m3 = *pxl++;
					unsigned char m4 = *pxl++;
					unsigned char m5 = *pxl++;
					unsigned char m6 = *pxl++;
					unsigned char m7 = *pxl++;
					unsigned char m8 = *pxl++;
					unsigned char m9 = *pxl++;
					unsigned int m = (m1 + m2 + m3 + m4 + m5 + m6 + m7 + m8 + m9);
					unsigned short mct_code;
					mct_code = ((unsigned int)(m - (m1 << 3) - m1) & 0x80000000) >> 31;
					mct_code |= ((unsigned int)(m - (m2 << 3) - m2) & 0x80000000) >> 30;
					mct_code |= ((unsigned int)(m - (m3 << 3) - m3) & 0x80000000) >> 29;
					mct_code |= ((unsigned int)(m - (m4 << 3) - m4) & 0x80000000) >> 28;
					mct_code |= ((unsigned int)(m - (m5 << 3) - m5) & 0x80000000) >> 27;
					mct_code |= ((unsigned int)(m - (m6 << 3) - m6) & 0x80000000) >> 26;
					mct_code |= ((unsigned int)(m - (m7 << 3) - m7) & 0x80000000) >> 25;
					mct_code |= ((unsigned int)(m - (m8 << 3) - m8) & 0x80000000) >> 24;
					mct_code |= ((unsigned int)(m - (m9 << 3) - m9) & 0x80000000) >> 23;
					
					*code++ = mct_code;
				}
				
				code = code_buf_short;
				for(int i = 0; i < 16; i++)
				{
					confidence += lut[*code++];
					lut += 512;
				}
			}
#endif

			while(fnum_Mod_16--)
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
#endif

#if (USE_FEATURE_DEP_OPT == 1)
#warning "IRMB2 and LGP are present iff SIMD8p is present"
		//SIMD8P
		if(pCls->feature_num[FEATURE_IDX_SIMD8P])
#endif
		{
			//LGP
			if(pCls->feature_num[FEATURE_IDX_LGP])
			{
				int *ffpos = (*one_stage_ffpos)[FEATURE_IDX_LGP];
				unsigned char *lgp_buf = lgp_image_buf;
				unsigned short *lut = pCls->lookup_tbl[FEATURE_IDX_LGP];
				int fnum = pCls->feature_num[FEATURE_IDX_LGP];
				while(fnum--)
				{
					unsigned char *p1 = lgp_buf + *(ffpos++);
					unsigned char *p2 = p1 + width;
					unsigned char *p3 = p2 + width;
					//__m128i xmm1, xmm2, xmm3, xmm4, xmm5, xmm6;
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
#if 0

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
				
#elif 1
				
			if(pCls->feature_num[FEATURE_IDX_IRMB2])
			{
				int *ffpos = (*one_stage_ffpos)[FEATURE_IDX_IRMB2];
				unsigned short *lut = pCls->lookup_tbl[FEATURE_IDX_IRMB2];
				unsigned char *irmb2_buf = irmb_image_buf;
				int fnum = pCls->feature_num[FEATURE_IDX_IRMB2];
				int	fnum_By_16 = fnum >> 4;
				int fnum_Mod_16 = fnum & 0xF;
				
				confidence += MVFD_IsFaceIRMB(irmb2_buf,ffpos,(unsigned char *)code_image_buf,fnum_By_16,
									lut,irmb2_table);
				
				ffpos +=  6*(fnum_By_16<<4);
				lut += 720 * (fnum_By_16<<4);
				while(fnum_Mod_16--)
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

#else
			if(pCls->feature_num[FEATURE_IDX_IRMB2])
			{
				
				unsigned char *irmb2_buf = 	irmb_image_buf;	
				int *ffpos = (*one_stage_ffpos)[FEATURE_IDX_IRMB2];
				unsigned short *lut = pCls->lookup_tbl[FEATURE_IDX_IRMB2];
				int fnum = pCls->feature_num[FEATURE_IDX_IRMB2];
				int fnum_by_4 = fnum >> 2;
				int fnum_rem_by_4 = fnum & 0x3;
	
				unsigned char *i = (unsigned char *)code_image_buf;

#if 0				
				while(fnum--)
				{
					*i++ = *(irmb2_buf + *ffpos++);
					*i++ = *(irmb2_buf + *ffpos++);
					*i++ = *(irmb2_buf + *ffpos++);
					*i++ = *(irmb2_buf + *ffpos++);
					*i++ = *(irmb2_buf + *ffpos++);
					*i++ = *(irmb2_buf + *ffpos++);
				}
				fnum = pCls->feature_num[FEATURE_IDX_IRMB2];
				i = (unsigned char *)code_image_buf;
#else
				MVFD_IsFaceIRMB_pxl_buf_gen(irmb2_buf,ffpos,(unsigned char *)code_image_buf,fnum);

#endif
				unsigned const char select1[] = {0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 3, 3, 4, 0};
				unsigned const char select2[] = {1, 2, 3, 4, 5, 2, 3, 4, 5, 3, 4, 5, 4, 5, 5, 0};
				unsigned const char mask[] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};
				
				uint8x16_t mask_8x16;
				uint8x16_t sel1_8x16,sel2_8x16;
				
				sel1_8x16 = vld1q_u8(select1);
				sel2_8x16 = vld1q_u8(select2);
				mask_8x16 = vld1q_u8(mask);

				unsigned int *lcode_buf_int = (unsigned int *)code_image_buf;
				while(fnum_by_4--)
				{
					uint8x8_t i_8x8_1, temp11_8x8_1, temp12_8x8_1, temp21_8x8_1, temp22_8x8_1;
					uint8x8_t i_8x8_2, temp11_8x8_2, temp12_8x8_2, temp21_8x8_2, temp22_8x8_2;
					uint8x8_t res1_8x8,res2_8x8;
					uint8x16_t temp11_8x16,temp12_8x16;
					
					i_8x8_1 = vld1_u8(i);
					i_8x8_2 = vld1_u8(i + 6);
					i+=12;

					temp11_8x8_1 = vtbl1_u8(i_8x8_1, vget_low_u8(sel1_8x16));
					temp11_8x8_2 = vtbl1_u8(i_8x8_2, vget_low_u8(sel1_8x16));
					temp12_8x8_1 = vtbl1_u8(i_8x8_1, vget_high_u8(sel1_8x16));
					temp12_8x8_2 = vtbl1_u8(i_8x8_2, vget_high_u8(sel1_8x16));
					temp21_8x8_1 = vtbl1_u8(i_8x8_1, vget_low_u8(sel2_8x16));
					temp21_8x8_2 = vtbl1_u8(i_8x8_2, vget_low_u8(sel2_8x16));
					temp22_8x8_1 = vtbl1_u8(i_8x8_1, vget_high_u8(sel2_8x16));
					temp22_8x8_2 = vtbl1_u8(i_8x8_2, vget_high_u8(sel2_8x16));

					temp11_8x16 = vcgeq_u8(vcombine_u8(temp11_8x8_1, temp11_8x8_2),vcombine_u8(temp21_8x8_1, temp21_8x8_2));
					temp12_8x16 = vcgeq_u8(vcombine_u8(temp12_8x8_1, temp12_8x8_2),vcombine_u8(temp22_8x8_1, temp22_8x8_2));
				
					temp11_8x16 = vandq_u8(temp11_8x16, mask_8x16);
					temp12_8x16 = vandq_u8(temp12_8x16, mask_8x16);
					
					res1_8x8 = vpadd_u8(vget_low_u8(temp11_8x16), vget_low_u8(temp12_8x16));
					res2_8x8 = vpadd_u8(vget_high_u8(temp11_8x16), vget_high_u8(temp12_8x16));

					i_8x8_1 = vld1_u8(i);
					i_8x8_2 = vld1_u8(i + 6);
					i+=12;
					
					res1_8x8 = vpadd_u8(res1_8x8, res2_8x8);
					temp11_8x8_1 = vtbl1_u8(i_8x8_1, vget_low_u8(sel1_8x16));
					temp11_8x8_2 = vtbl1_u8(i_8x8_2, vget_low_u8(sel1_8x16));
					temp12_8x8_1 = vtbl1_u8(i_8x8_1, vget_high_u8(sel1_8x16));
					temp12_8x8_2 = vtbl1_u8(i_8x8_2, vget_high_u8(sel1_8x16));
					
					res1_8x8 = vpadd_u8(res1_8x8, res1_8x8);
					temp21_8x8_1 = vtbl1_u8(i_8x8_1, vget_low_u8(sel2_8x16));
					temp21_8x8_2 = vtbl1_u8(i_8x8_2, vget_low_u8(sel2_8x16));
					temp22_8x8_1 = vtbl1_u8(i_8x8_1, vget_high_u8(sel2_8x16));
					temp22_8x8_2 = vtbl1_u8(i_8x8_2, vget_high_u8(sel2_8x16));
					
					vst1_lane_u32(lcode_buf_int++,vreinterpret_u32_u8(res1_8x8),0 );

					temp11_8x16 = vcgeq_u8(vcombine_u8(temp11_8x8_1, temp11_8x8_2),vcombine_u8(temp21_8x8_1, temp21_8x8_2));
					temp12_8x16 = vcgeq_u8(vcombine_u8(temp12_8x8_1, temp12_8x8_2),vcombine_u8(temp22_8x8_1, temp22_8x8_2));
				
					temp11_8x16 = vandq_u8(temp11_8x16, mask_8x16);
					temp12_8x16 = vandq_u8(temp12_8x16, mask_8x16);
					
					res1_8x8 = vpadd_u8(vget_low_u8(temp11_8x16), vget_low_u8(temp12_8x16));
					res2_8x8 = vpadd_u8(vget_high_u8(temp11_8x16), vget_high_u8(temp12_8x16));

					res1_8x8 = vpadd_u8(res1_8x8, res2_8x8);
					res1_8x8 = vpadd_u8(res1_8x8, res1_8x8);
					vst1_lane_u32(lcode_buf_int++,vreinterpret_u32_u8(res1_8x8),0 );
					
				}
				unsigned short *lcode_buf_short = (unsigned short *)lcode_buf_int;
				while(fnum_rem_by_4--)
				{
					uint8x8_t i_8x8, temp11_8x8, temp12_8x8, temp21_8x8, temp22_8x8;
				
					i_8x8 = vld1_u8(i);
					i+=6;
					
					temp11_8x8 = vtbl1_u8(i_8x8, vget_low_u8(sel1_8x16));
					temp12_8x8 = vtbl1_u8(i_8x8, vget_high_u8(sel1_8x16));
					temp21_8x8 = vtbl1_u8(i_8x8, vget_low_u8(sel2_8x16));
					temp22_8x8 = vtbl1_u8(i_8x8, vget_high_u8(sel2_8x16));
					
					temp11_8x8 = vcge_u8(temp11_8x8, temp21_8x8);
					temp12_8x8 = vcge_u8(temp12_8x8, temp22_8x8);
					
					temp11_8x8 = vand_u8(temp11_8x8, vget_low_u8(mask_8x16));
					temp12_8x8 = vand_u8(temp12_8x8, vget_low_u8(mask_8x16));
					
					temp11_8x8 = vpadd_u8(temp11_8x8, temp12_8x8);
					temp11_8x8 = vpadd_u8(temp11_8x8, temp11_8x8);
					temp11_8x8 = vpadd_u8(temp11_8x8, temp11_8x8);
					
					vst1_lane_u16(lcode_buf_short++,vreinterpret_u16_u8(temp11_8x8),0 );
					
				}
				

#if 0
				lcode_buf = code_image_buf;
				while(fnum_bkp--)
				{
					confidence += lut[irmb2_table[*lcode_buf++ & 0x7FFF]];
					lut += 720;
				}
#else
				confidence +=MVFD_IsFaceIRMBConf(lut,irmb2_table,(unsigned short *)code_image_buf,fnum);

#endif
			}
#endif
			//clock_end("irmb2");
#if (USE_FEATURE_DEP_OPT == 0)
			//SIMD8P
			if(pCls->feature_num[FEATURE_IDX_SIMD8P])
#endif
			{
				
				int *ffpos = (*one_stage_ffpos)[FEATURE_IDX_SIMD8P];	
				unsigned char *img_buf[2] = {simd8p_image_buf[0],simd8p_image_buf[1]};
				unsigned short *lut = pCls->lookup_tbl[FEATURE_IDX_SIMD8P];
				int fnum = pCls->feature_num[FEATURE_IDX_SIMD8P];

#if 0
				unsigned char v2_[8];
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
#else
				unsigned const char mask[] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};
				uint8x8_t mask_8x8 = vld1_u8(mask);
				
				unsigned short *code_buf_short = (unsigned short *)code_image_buf;
				
				int fnum_rem_4 = fnum & 0x3;
				int fnum_by_4 = fnum >> 2;
				while(fnum_by_4--)
				{
					unsigned char *v1 = img_buf[ffpos[0]] + ffpos[1];
					unsigned char *v2 = img_buf[ffpos[2]] + ffpos[3];
					unsigned char *m  = (unsigned char *)&ffpos[4];
					ffpos += 6;
					
					uint8x8_t v1_8x8,v2_8x8,m_8x8,res1_8x8,res2_8x8;
					
					v1_8x8 = vld1_u8(v1);
					v2_8x8 = vld1_u8(v2);
					m_8x8 = vld1_u8(m);
					v2_8x8 = vtbl1_u8(v2_8x8, m_8x8);
					
					res1_8x8 = vcge_u8(v1_8x8, v2_8x8);
					res1_8x8 = vand_u8(res1_8x8, mask_8x8);
					
					v1 = img_buf[ffpos[0]] + ffpos[1];
					v2 = img_buf[ffpos[2]] + ffpos[3];
					m  = (unsigned char *)&ffpos[4];
					ffpos += 6;

					v1_8x8 = vld1_u8(v1);
					v2_8x8 = vld1_u8(v2);
					m_8x8 = vld1_u8(m);
					v2_8x8 = vtbl1_u8(v2_8x8, m_8x8);
					
					res2_8x8 = vcge_u8(v1_8x8, v2_8x8);
					res2_8x8 = vand_u8(res2_8x8, mask_8x8);
					
					res1_8x8 = vpadd_u8(res1_8x8, res2_8x8);
					res1_8x8 = vpadd_u8(res1_8x8, res1_8x8);
					res1_8x8 = vpadd_u8(res1_8x8, res1_8x8);

					v1 = img_buf[ffpos[0]] + ffpos[1];
					v2 = img_buf[ffpos[2]] + ffpos[3];
					m  = (unsigned char *)&ffpos[4];
					ffpos += 6;
					
					v1_8x8 = vld1_u8(v1);
					v2_8x8 = vld1_u8(v2);
					m_8x8 = vld1_u8(m);
					v2_8x8 = vtbl1_u8(v2_8x8, m_8x8);
					
					vst1_lane_u16(code_buf_short++,vreinterpret_u16_u8(res1_8x8),0 );

					res1_8x8 = vcge_u8(v1_8x8, v2_8x8);
					res1_8x8 = vand_u8(res1_8x8, mask_8x8);
					
					v1 = img_buf[ffpos[0]] + ffpos[1];
					v2 = img_buf[ffpos[2]] + ffpos[3];
					m  = (unsigned char *)&ffpos[4];
					ffpos += 6;

					v1_8x8 = vld1_u8(v1);
					v2_8x8 = vld1_u8(v2);
					m_8x8 = vld1_u8(m);
					v2_8x8 = vtbl1_u8(v2_8x8, m_8x8);
					
					res2_8x8 = vcge_u8(v1_8x8, v2_8x8);
					res2_8x8 = vand_u8(res2_8x8, mask_8x8);
					
					res1_8x8 = vpadd_u8(res1_8x8, res2_8x8);
					res1_8x8 = vpadd_u8(res1_8x8, res1_8x8);
					res1_8x8 = vpadd_u8(res1_8x8, res1_8x8);
					
					vst1_lane_u16(code_buf_short++,vreinterpret_u16_u8(res1_8x8),0 );
					
				}
				unsigned char *code_buf_char = (unsigned char *)code_buf_short;
				while(fnum_rem_4--)
				{
					unsigned char *v1 = img_buf[ffpos[0]] + ffpos[1];
					unsigned char *v2 = img_buf[ffpos[2]] + ffpos[3];
					unsigned char *m  = (unsigned char *)&ffpos[4];
					ffpos += 6;
					
					uint8x8_t v1_8x8,v2_8x8,m_8x8;
					
					v1_8x8 = vld1_u8(v1);
					v2_8x8 = vld1_u8(v2);
					m_8x8 = vld1_u8(m);
					v2_8x8 = vtbl1_u8(v2_8x8, m_8x8);
					
					v2_8x8 = vcge_u8(v1_8x8, v2_8x8);
					v2_8x8 = vand_u8(v2_8x8, mask_8x8);
					v2_8x8 = vpadd_u8(v2_8x8, v2_8x8);
					v2_8x8 = vpadd_u8(v2_8x8, v2_8x8);
					v2_8x8 = vpadd_u8(v2_8x8, v2_8x8);
					
					vst1_lane_u8(code_buf_char++,v2_8x8,0 );
				}
			
				code_buf_char = (unsigned char *)code_image_buf;
#if 0
				while(fnum--)
				{
					
					confidence += lut[*code_buf_char++];
					lut += 256;				

				}
#else
				confidence +=MVFD_IsFaceSIMD8PConf(lut,(unsigned char *)code_image_buf,fnum);

#endif
#endif
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
#if	(DOUBLE_MCT_AND_FASTPOSE == 0)
			*mct_ffpos++ = fpos->y*width+fpos->x;
#else
			*mct_ffpos++ = (fpos->y*width+fpos->x)<<1;
#endif
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
