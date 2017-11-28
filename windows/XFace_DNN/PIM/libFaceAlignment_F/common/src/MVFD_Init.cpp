#include "stdafx.h"

#include "MVFD_Defines.h"
#include "MVFD_Global.h"
#include "MVFD_Enroll.h"
#include "MVFD_Memory.h"
#include "MVFD_Init.h"
#include <stdlib.h>
//#include <memory.h>
#include <stdio.h>
#include "MVFD2_data(9)_Fast.h"
//#include "MVFD2_data(9)_Accurate.h"
//#include "MVFD2_data(9)_Normal.h"
#include "MVFD_Likelihood_New.h"
typedef struct _BUFFER
{
	char *ptr;
	int size;
	int rd_pos;
} BUFFER;

void MVFD_buffer_write(BUFFER *buf, void *src, int size)
{
	fd_memcpy(&buf->ptr[buf->size], src, size);
	buf->size += size;
}

void MVFD_buffer_write_align16(BUFFER *buf, void *src, int size)
{
	int align_pos = ((buf->size+15)>>4)<<4;
	fd_memcpy(&buf->ptr[align_pos], src, size);
	buf->size = align_pos+size;
}
void *MVFD_buffer_read(BUFFER *buf, int size)
{
	int prev_rd_pos = buf->rd_pos;
	buf->rd_pos += size;
	return &buf->ptr[prev_rd_pos];
}

void *MVFD_buffer_read_align16(BUFFER *buf, int size)
{
	int prev_rd_pos = ((buf->rd_pos+15)>>4)<<4;
	buf->rd_pos = prev_rd_pos+size;
	return &buf->ptr[prev_rd_pos];
}

void MVFD_buffer_read_copy(BUFFER *buf, void *dst, int size)
{
	fd_memcpy(dst, &buf->ptr[buf->rd_pos], size);
	buf->rd_pos += size;
}

#ifndef WIN32
FD_CLASSIFIER *MVFD_LoadClassifier16(PResFileIO pResFileIO, const char *fpath)
{
	FD_CLASSIFIER *pNode = (FD_CLASSIFIER *)MVFD_mem_alloc(sizeof(FD_CLASSIFIER));
	eRESULT ret;
	
	fd_memset(pNode, 0, sizeof(FD_CLASSIFIER));
	ret = ALPI_ResourceFileOpen(pResFileIO, fpath);
	if(ret == eRESULT_FAIL)
	{
		LOGE("Unable to load [%s].\n", fpath);
		return NULL;
	}
	int width, height;
	unsigned long readsize;
	//double minv, double2short;
	readsize = sizeof(int);
	ALPI_ResourceFileRead(pResFileIO, &pNode->class_num, &readsize);
	readsize = sizeof(int);
	ALPI_ResourceFileRead(pResFileIO, &width, &readsize);
	readsize = sizeof(int);
	ALPI_ResourceFileRead(pResFileIO, &height, &readsize);
	readsize = sizeof(double);
	ALPI_ResourceFileRead(pResFileIO, &pNode->minv, &readsize);
	readsize = sizeof(double);
	ALPI_ResourceFileRead(pResFileIO, &pNode->double2short, &readsize);
	
	int thresholds[FD_MAX_CLASS_NUM];

	readsize = sizeof(unsigned int) * pNode->class_num;
	ALPI_ResourceFileRead(pResFileIO, thresholds, &readsize);
	fd_memset(pNode->thresholds, 0, sizeof(short)*FD_MAX_CLASS_NUM);

	//intel compiler버그 때문에 수정
	unsigned short t[100];
	for(int i = 0; i < pNode->class_num; ++i) 
	{
		t[i] = thresholds[i];
	}
	fd_memcpy(pNode->thresholds, t, sizeof(unsigned short)*pNode->class_num);

	readsize = sizeof(unsigned int) * pNode->class_num;
	ALPI_ResourceFileRead(pResFileIO, pNode->fv_lower, &readsize);
	readsize = sizeof(unsigned int) * pNode->class_num;
	ALPI_ResourceFileRead(pResFileIO, pNode->fv_upper, &readsize);
	
	for(int i = 0; i < pNode->class_num; ++i)
	{
		readsize = sizeof(unsigned int)*100;
		ALPI_ResourceFileRead(pResFileIO, pNode->likelihood[i], &readsize);
	}
	readsize = sizeof(int);
	ALPI_ResourceFileRead(pResFileIO, &pNode->total_feature_num, &readsize);
	
	int ptset_size;
	
	readsize = sizeof(int);
	ALPI_ResourceFileRead(pResFileIO, &ptset_size, &readsize);
	
	int file_pos;
	int feature_bin_num[FD_FEATURE_TYPE_NUM] = {512, 720, 256, 256, 512};
	int one_ffpos_size[FD_FEATURE_TYPE_NUM] = {4, 24, 24, 4, 4};
	int fpos_size[FD_FEATURE_TYPE_NUM];
	fpos_size[0] = sizeof(POINT_FEATURE_POS);
	fpos_size[1] = sizeof(IRMB2_FEATURE_POS);
	fpos_size[2] = sizeof(SIMD8P_FEATURE_POS);
	fpos_size[3] = sizeof(POINT_FEATURE_POS);
	fpos_size[4] = sizeof(POINT_FEATURE_POS);
	
	for(int i = 0; i < 2; ++i)
	{
		if(i == 0) {
			ALPI_ResourceFileGetPos(pResFileIO, &file_pos);
		}
		else
		{
			ALPI_ResourceFileSeek(pResFileIO, file_pos, SEEK_SET);
			for(int j = 0; j < FD_FEATURE_TYPE_NUM; ++j)
			{
				pNode->feature_pos[j] = (char *)MVFD_mem_alloc(pNode->feature_num[j]*fpos_size[j]);	
				pNode->lookup_tbl[j] = (unsigned short *)MVFD_mem_alloc(sizeof(short)*pNode->class_num*feature_bin_num[j]*pNode->feature_num[j]);
				pNode->feature_num[j] = 0;
			}
		}
		for(int f = 0; f < pNode->total_feature_num; ++f)
		{
			int feature_type, class_num, value_range;

			readsize = sizeof(int);
			ALPI_ResourceFileRead(pResFileIO, &feature_type, &readsize);
			readsize = sizeof(int);
			ALPI_ResourceFileRead(pResFileIO, &class_num, &readsize);
	
			int feature_idx;
			switch(feature_type)
			{
			case 0:
				feature_idx = 0;
				break;
			case 9:
				feature_idx = 1;
				break;
			case 10:
				feature_idx = 2;
				break;
			case 1:
				feature_idx = 3;
				break;
			case 11:
				feature_idx = 4;
				break;
			default:
				FDASSERT(0);
				//_asm int 3;
				break;
			}
			if(pNode->feature_pos[feature_idx])
			{
				readsize = fpos_size[feature_idx];
				ALPI_ResourceFileRead(pResFileIO, &pNode->feature_pos[feature_idx][pNode->feature_num[feature_idx]*fpos_size[feature_idx]], &readsize);
				readsize = sizeof(int);
				ALPI_ResourceFileRead(pResFileIO, &value_range, &readsize);
			
				unsigned short *lookup_tbl = &pNode->lookup_tbl[feature_idx][pNode->feature_num[feature_idx]*feature_bin_num[feature_idx]*class_num];

				readsize = pNode->class_num*feature_bin_num[feature_idx]*sizeof(unsigned short);
				ALPI_ResourceFileRead(pResFileIO, lookup_tbl, &readsize);
			}
			else
			{
				ALPI_ResourceFileSeek(pResFileIO, fpos_size[feature_idx], SEEK_CUR);
				readsize = sizeof(int);
				ALPI_ResourceFileRead(pResFileIO, &value_range, &readsize);
				{
					int Offset = 0;
					for(int j = 0; j < value_range; ++j)
					{
						Offset += pNode->class_num*sizeof(unsigned short);
						//fseek(fp, pNode->class_num*sizeof(unsigned short), SEEK_CUR);
					}
					ALPI_ResourceFileSeek(pResFileIO, Offset, SEEK_CUR);
				}
			}
			pNode->feature_num[feature_idx]++;
		}
	}
	ALPI_ResourceFileClose(pResFileIO);
	LOGE("Success Reading [%s].\n", fpath);	
	return pNode;
}

//Load a classifier specified as fpath
FD_CLASSIFIER *MVFD_LoadClassifier(PResFileIO pResFileIO, const char *fpath)
{
	FD_CLASSIFIER *pNode = (FD_CLASSIFIER *)MVFD_mem_alloc(sizeof(FD_CLASSIFIER));
	eRESULT ret;
	fd_memset(pNode, 0, sizeof(FD_CLASSIFIER));

	ret = ALPI_ResourceFileOpen(pResFileIO, fpath);
	if(ret == eRESULT_FAIL)
	{
		LOGE("Unable to load [%s].\n", fpath);
		return NULL;
	}
	LOGE("Success Reading [%s].\n", fpath);
	
	int width, height;
	int one_elem_bits_num, sum_elem_bits_num;
	double double_range;
	unsigned int double2int;
	unsigned long readsize;

	readsize = sizeof(int);
	ALPI_ResourceFileRead(pResFileIO, &pNode->class_num, &readsize);
	readsize = sizeof(int);
	ALPI_ResourceFileRead(pResFileIO, &width, &readsize);
	readsize = sizeof(int);
	ALPI_ResourceFileRead(pResFileIO, &height, &readsize);
	
	readsize = sizeof(int);
	ALPI_ResourceFileRead(pResFileIO, &one_elem_bits_num, &readsize);
	readsize = sizeof(int);
	ALPI_ResourceFileRead(pResFileIO, &sum_elem_bits_num, &readsize);
	readsize = sizeof(double);
	ALPI_ResourceFileRead(pResFileIO, &double_range, &readsize);
	readsize = sizeof(unsigned int);
	ALPI_ResourceFileRead(pResFileIO, &double2int, &readsize);
	readsize = sizeof(unsigned int);
	ALPI_ResourceFileRead(pResFileIO, &pNode->threshold, &readsize);
	
	readsize = sizeof(unsigned int)*pNode->class_num;
	ALPI_ResourceFileRead(pResFileIO, pNode->fv_lower, &readsize);
	readsize = sizeof(unsigned int)*pNode->class_num;
	ALPI_ResourceFileRead(pResFileIO, pNode->fv_upper, &readsize);
	
	for(int i = 0; i < pNode->class_num; ++i)
	{
		readsize = sizeof(unsigned int)*100;
		ALPI_ResourceFileRead(pResFileIO, pNode->likelihood[i], &readsize);
	}
	readsize = sizeof(int);
	ALPI_ResourceFileRead(pResFileIO, &pNode->total_feature_num, &readsize);

	int ptset_size;
	readsize = sizeof(int);
	ALPI_ResourceFileRead(pResFileIO, &ptset_size, &readsize);
	
	int file_pos;
	int feature_bin_num[FD_FEATURE_TYPE_NUM] = {512, 720, 256, 256, 512};
	int one_ffpos_size[FD_FEATURE_TYPE_NUM] = {4, 24, 24, 4, 4};
	int fpos_size[FD_FEATURE_TYPE_NUM];
	fpos_size[0] = sizeof(POINT_FEATURE_POS);
	fpos_size[1] = sizeof(IRMB2_FEATURE_POS);
	fpos_size[2] = sizeof(SIMD8P_FEATURE_POS);
	fpos_size[3] = sizeof(POINT_FEATURE_POS);
	fpos_size[4] = sizeof(POINT_FEATURE_POS);

	for(int i = 0; i < 2; ++i)
	{
		if(i == 0) {
			ALPI_ResourceFileGetPos(pResFileIO, &file_pos);
		}
		else
		{
			ALPI_ResourceFileSeek(pResFileIO, file_pos, SEEK_SET);
			for(int j = 0; j < FD_FEATURE_TYPE_NUM; ++j)
			{
				if(pNode->feature_num[j] > 0)
				{
					pNode->feature_pos[j] = (char *)MVFD_mem_alloc(pNode->feature_num[j]*fpos_size[j]);	
					pNode->lookup_tbl[j] = (unsigned short *)MVFD_mem_alloc(sizeof(short)*pNode->class_num*feature_bin_num[j]*pNode->feature_num[j]);
					pNode->feature_num[j] = 0;
				}
			}
		}
		for(int f = 0; f < pNode->total_feature_num; ++f)
		{
			int feature_type, class_num, value_range;

			readsize = sizeof(int);
			ALPI_ResourceFileRead(pResFileIO, &feature_type, &readsize);
			readsize = sizeof(int);
			ALPI_ResourceFileRead(pResFileIO, &class_num, &readsize);
	
			int feature_idx;
			switch(feature_type)
			{
			case 0:
				feature_idx = 0;
				break;
			case 9:
				feature_idx = 1;
				break;
			case 10:
				feature_idx = 2;
				break;
			case 1:
				feature_idx = 3;
				break;
			case 11:
				feature_idx = 4;
				break;
			default:
				FDASSERT(0);
				//_asm int 3;
				break;
			}
			if(pNode->feature_pos[feature_idx])
			{
				readsize = fpos_size[feature_idx];
				ALPI_ResourceFileRead(pResFileIO, &pNode->feature_pos[feature_idx][pNode->feature_num[feature_idx]*fpos_size[feature_idx]], &readsize);
				readsize = sizeof(int);
				ALPI_ResourceFileRead(pResFileIO, &value_range, &readsize);
			
				unsigned short *lookup_tbl = &pNode->lookup_tbl[feature_idx][pNode->feature_num[feature_idx]*feature_bin_num[feature_idx]*class_num];

				readsize = pNode->class_num*feature_bin_num[feature_idx]*sizeof(unsigned short);
				ALPI_ResourceFileRead(pResFileIO, lookup_tbl, &readsize);
			}
			else
			{
				ALPI_ResourceFileSeek(pResFileIO, fpos_size[feature_idx], SEEK_CUR);
				readsize = sizeof(int);
				ALPI_ResourceFileRead(pResFileIO, &value_range, &readsize);
				{
					int Offset =0;
					for(int j = 0; j < value_range; ++j)
					{
						Offset += pNode->class_num*sizeof(unsigned short);
						//fseek(fp, pNode->class_num*sizeof(unsigned short), SEEK_CUR);
					}
					ALPI_ResourceFileSeek(pResFileIO, Offset, SEEK_CUR);
				}
			}
			pNode->feature_num[feature_idx]++;
		}
	}
	ALPI_ResourceFileClose(pResFileIO);

	int prev_num = pNode->feature_num[FEATURE_IDX_MCT];
	int align_num = (prev_num+3)/4*4;
	if(prev_num != align_num)
	{
		char *new_fpos = (char *)MVFD_mem_alloc(align_num*fpos_size[FEATURE_IDX_MCT]);	
		fd_memcpy(new_fpos, pNode->feature_pos[FEATURE_IDX_MCT], prev_num*fpos_size[FEATURE_IDX_MCT]);
		POINT_FEATURE_POS *fpos = (POINT_FEATURE_POS *)new_fpos;
		for(int i = prev_num; i < align_num; ++i) fpos[i] = fpos[prev_num-1];
		pNode->feature_pos[FEATURE_IDX_MCT] = new_fpos;

		unsigned short *new_lut = (unsigned short *)MVFD_mem_alloc(sizeof(short)*pNode->class_num*feature_bin_num[FEATURE_IDX_MCT]*align_num);
		fd_memset(new_lut, 0, sizeof(short)*pNode->class_num*feature_bin_num[FEATURE_IDX_MCT]*align_num);
		fd_memcpy(new_lut, pNode->lookup_tbl[FEATURE_IDX_MCT], sizeof(short)*pNode->class_num*feature_bin_num[FEATURE_IDX_MCT]*prev_num);
		pNode->lookup_tbl[FEATURE_IDX_MCT] = new_lut;

		pNode->feature_num[FEATURE_IDX_MCT] = align_num;
	}
	return pNode;
}
#endif

void MVFD_FreeClassifier(FD_CLASSIFIER *pClassifier)
{
	for(int i = 0; i < FD_FEATURE_TYPE_NUM; ++i)
	{
		if(pClassifier->feature_pos[i]) MVFD_mem_free(pClassifier->feature_pos[i]);
		if(pClassifier->lookup_tbl[i] && !pClassifier->from_header)
		{
			MVFD_mem_free(pClassifier->lookup_tbl[i]);
		}
	}
	MVFD_mem_free(pClassifier);
}

//Transform features in a specified classifier.
FD_CLASSIFIER *MVFD_FeatureTransform(FD_CLASSIFIER *pCLS, int transform_type)
{
	//int sz = sizeof(SIMD8P_FEATURE_POS);
	FD_CLASSIFIER *pNewCLS = new FD_CLASSIFIER;
	fd_memcpy(pNewCLS, pCLS, sizeof(FD_CLASSIFIER));
	
	if(pCLS->feature_num[FEATURE_IDX_MCT] > 0)
	{
		int fnum = pCLS->feature_num[FEATURE_IDX_MCT];		
		pNewCLS->feature_pos[FEATURE_IDX_MCT] = (char *)MVFD_mem_alloc(fnum*sizeof(POINT_FEATURE_POS));
		pNewCLS->lookup_tbl[FEATURE_IDX_MCT] = (unsigned short *)MVFD_mem_alloc(fnum*512*sizeof(unsigned short));
		POINT_FEATURE_POS *new_fpos = (POINT_FEATURE_POS *)pNewCLS->feature_pos[FEATURE_IDX_MCT];
		POINT_FEATURE_POS *fpos = (POINT_FEATURE_POS *)pCLS->feature_pos[FEATURE_IDX_MCT];
		
		unsigned short *lut = pCLS->lookup_tbl[FEATURE_IDX_MCT];
		unsigned short *new_lut = pNewCLS->lookup_tbl[FEATURE_IDX_MCT];
		switch(transform_type)
		{
			case FEATURE_TRANSFORM_XFLIP:
				for(int i = 0; i < fnum; ++i)
				{
					new_fpos->x = FD_FACE_WIDTH-fpos->x-1;
					new_fpos->y = fpos->y;
					for(int code = 0; code < 512; ++code)
					{
						int m[10], m_[10];
						m[1] = code & 0x01;
						m[2] = (code & 0x02)>>1;
						m[3] = (code & 0x04)>>2;
						m[4] = (code & 0x08)>>3;
						m[5] = (code & 0x10)>>4;
						m[6] = (code & 0x20)>>5;
						m[7] = (code & 0x40)>>6;
						m[8] = (code & 0x80)>>7;
						m[9] = (code & 0x100)>>8;
						m_[1] = m[3]; m_[2] = m[2]; m_[3] = m[1]; m_[4] = m[6];
						m_[5] = m[5]; m_[6] = m[4]; m_[7] = m[9]; m_[8] = m[8];
						m_[9] = m[7];
						int newcode = (m_[9]<<8) | (m_[8]<<7) | (m_[7]<<6) | (m_[6]<<5) | (m_[5]<<4) | (m_[4]<<3) | (m_[3]<<2) | (m_[2] <<1) | m_[1];
						new_lut[newcode] = lut[code];
					}
					new_fpos++;
					fpos++;
					lut += 512;
					new_lut += 512;
				}
			break;
			case FEATURE_TRANSFORM_YFLIP:
				for(int i = 0; i < fnum; ++i)
				{
					new_fpos->x = fpos->x;
					new_fpos->y = FD_FACE_HEIGHT-fpos->y-1;
					for(int code = 0; code < 512; ++code)
					{
						int m[10], m_[10];
						m[1] = code & 0x01;
						m[2] = (code & 0x02)>>1;
						m[3] = (code & 0x04)>>2;
						m[4] = (code & 0x08)>>3;
						m[5] = (code & 0x10)>>4;
						m[6] = (code & 0x20)>>5;
						m[7] = (code & 0x40)>>6;
						m[8] = (code & 0x80)>>7;
						m[9] = (code & 0x100)>>8;
						m_[1] = m[7]; m_[2] = m[8]; m_[3] = m[9]; m_[4] = m[4];
						m_[5] = m[5]; m_[6] = m[6]; m_[7] = m[1]; m_[8] = m[2];
						m_[9] = m[3];
						int newcode = (m_[9]<<8) | (m_[8]<<7) | (m_[7]<<6) | (m_[6]<<5) | (m_[5]<<4) | (m_[4]<<3) | (m_[3]<<2) | (m_[2] <<1) | m_[1];
						new_lut[newcode] = lut[code];
					}
					new_fpos++;
					fpos++;
					lut += 512;
					new_lut += 512;
				}
			break;
			case FEATURE_TRANSFORM_ROT_90CW:
				for(int i = 0; i < fnum; ++i)
				{
					new_fpos->x = FD_FACE_HEIGHT-fpos->y-1;
					new_fpos->y = fpos->x;
					for(int code = 0; code < 512; ++code)
					{
						int m[10], m_[10];
						m[1] = code & 0x01;
						m[2] = (code & 0x02)>>1;
						m[3] = (code & 0x04)>>2;
						m[4] = (code & 0x08)>>3;
						m[5] = (code & 0x10)>>4;
						m[6] = (code & 0x20)>>5;
						m[7] = (code & 0x40)>>6;
						m[8] = (code & 0x80)>>7;
						m[9] = (code & 0x100)>>8;
						m_[1] = m[7]; m_[2] = m[4]; m_[3] = m[1]; m_[4] = m[8];
						m_[5] = m[5]; m_[6] = m[2]; m_[7] = m[9]; m_[8] = m[6];
						m_[9] = m[3];
						int newcode = (m_[9]<<8) | (m_[8]<<7) | (m_[7]<<6) | (m_[6]<<5) | (m_[5]<<4) | (m_[4]<<3) | (m_[3]<<2) | (m_[2] <<1) | m_[1];
						new_lut[newcode] = lut[code];
					}
					new_fpos++;
					fpos++;
					lut += 512;
					new_lut += 512;
				}
			break;
			case FEATURE_TRANSFORM_ROT_90CCW:
				for(int i = 0; i < fnum; ++i)
				{
					new_fpos->x = fpos->y;
					new_fpos->y = FD_FACE_WIDTH-fpos->x-1;
					for(int code = 0; code < 512; ++code)
					{
						int m[10], m_[10];
						m[1] = code & 0x01;
						m[2] = (code & 0x02)>>1;
						m[3] = (code & 0x04)>>2;
						m[4] = (code & 0x08)>>3;
						m[5] = (code & 0x10)>>4;
						m[6] = (code & 0x20)>>5;
						m[7] = (code & 0x40)>>6;
						m[8] = (code & 0x80)>>7;
						m[9] = (code & 0x100)>>8;
						m_[1] = m[3]; m_[2] = m[6]; m_[3] = m[9]; m_[4] = m[2];
						m_[5] = m[5]; m_[6] = m[8]; m_[7] = m[1]; m_[8] = m[4];
						m_[9] = m[7];
						int newcode = (m_[9]<<8) | (m_[8]<<7) | (m_[7]<<6) | (m_[6]<<5) | (m_[5]<<4) | (m_[4]<<3) | (m_[3]<<2) | (m_[2] <<1) | m_[1];
						new_lut[newcode] = lut[code];
					}
					new_fpos++;
					fpos++;
					lut += 512;
					new_lut += 512;
				}
			break;
		}
	}
	if(pCLS->feature_num[FEATURE_IDX_LGP] > 0)
	{
		int fnum = pCLS->feature_num[FEATURE_IDX_LGP];		
		pNewCLS->feature_pos[FEATURE_IDX_LGP] = (char *)MVFD_mem_alloc(fnum*sizeof(POINT_FEATURE_POS));
		pNewCLS->lookup_tbl[FEATURE_IDX_LGP] = (unsigned short *)MVFD_mem_alloc(fnum*256*sizeof(unsigned short));
		POINT_FEATURE_POS *new_fpos = (POINT_FEATURE_POS *)pNewCLS->feature_pos[FEATURE_IDX_LGP];
		POINT_FEATURE_POS *fpos = (POINT_FEATURE_POS *)pCLS->feature_pos[FEATURE_IDX_LGP];
		
		unsigned short *lut = pCLS->lookup_tbl[FEATURE_IDX_LGP];
		unsigned short *new_lut = pNewCLS->lookup_tbl[FEATURE_IDX_LGP];
		switch(transform_type)
		{
		case FEATURE_TRANSFORM_XFLIP:
			for(int i = 0; i < fnum; ++i)
			{
				new_fpos->x = FD_FACE_WIDTH-fpos->x-1;
				new_fpos->y = fpos->y;
				for(int code = 0; code < 256; ++code)
				{
					int m[9], m_[9];
					m[1] = code & 0x01;
					m[2] = (code & 0x02)>>1;
					m[3] = (code & 0x04)>>2;
					m[4] = (code & 0x08)>>3;
					m[5] = (code & 0x10)>>4;
					m[6] = (code & 0x20)>>5;
					m[7] = (code & 0x40)>>6;
					m[8] = (code & 0x80)>>7;
					m_[1] = m[3]; m_[2] = m[2]; m_[3] = m[1]; m_[4] = m[8];
					m_[5] = m[7]; m_[6] = m[6]; m_[7] = m[5]; m_[8] = m[4];
					int newcode = (m_[8]<<7) | (m_[7]<<6) | (m_[6]<<5) | (m_[5]<<4) | (m_[4]<<3) | (m_[3]<<2) | (m_[2] <<1) | m_[1];
					new_lut[newcode] = lut[code];
				}
				new_fpos++;
				fpos++;
				lut += 256;
				new_lut += 256;
			}
			break;
		case FEATURE_TRANSFORM_YFLIP:
			for(int i = 0; i < fnum; ++i)
			{
				new_fpos->x = fpos->x;
				new_fpos->y = FD_FACE_HEIGHT-fpos->y-1;
				for(int code = 0; code < 256; ++code)
				{
					int m[9], m_[9];
					m[1] = code & 0x01;
					m[2] = (code & 0x02)>>1;
					m[3] = (code & 0x04)>>2;
					m[4] = (code & 0x08)>>3;
					m[5] = (code & 0x10)>>4;
					m[6] = (code & 0x20)>>5;
					m[7] = (code & 0x40)>>6;
					m[8] = (code & 0x80)>>7;
					m_[1] = m[7]; m_[2] = m[6]; m_[3] = m[5]; m_[4] = m[4];
					m_[5] = m[3]; m_[6] = m[2]; m_[7] = m[1]; m_[8] = m[8];
					int newcode = (m_[8]<<7) | (m_[7]<<6) | (m_[6]<<5) | (m_[5]<<4) | (m_[4]<<3) | (m_[3]<<2) | (m_[2] <<1) | m_[1];
					new_lut[newcode] = lut[code];
				}
				new_fpos++;
				fpos++;
				lut += 256;
				new_lut += 256;
			}
			break;
		case FEATURE_TRANSFORM_ROT_90CW:
			for(int i = 0; i < fnum; ++i)
			{
				new_fpos->x = FD_FACE_HEIGHT-fpos->y-1;
				new_fpos->y = fpos->x;
				for(int code = 0; code < 256; ++code)
				{
					int m[9], m_[9];
					m[1] = code & 0x01;
					m[2] = (code & 0x02)>>1;
					m[3] = (code & 0x04)>>2;
					m[4] = (code & 0x08)>>3;
					m[5] = (code & 0x10)>>4;
					m[6] = (code & 0x20)>>5;
					m[7] = (code & 0x40)>>6;
					m[8] = (code & 0x80)>>7;
					m_[1] = m[7]; m_[2] = m[8]; m_[3] = m[1]; m_[4] = m[2];
					m_[5] = m[3]; m_[6] = m[4]; m_[7] = m[5]; m_[8] = m[6];
					int newcode = (m_[8]<<7) | (m_[7]<<6) | (m_[6]<<5) | (m_[5]<<4) | (m_[4]<<3) | (m_[3]<<2) | (m_[2] <<1) | m_[1];
					new_lut[newcode] = lut[code];
				}
				new_fpos++;
				fpos++;
				lut += 256;
				new_lut += 256;
			}
			break;
		case FEATURE_TRANSFORM_ROT_90CCW:
			for(int i = 0; i < fnum; ++i)
			{
				new_fpos->x = fpos->y;
				new_fpos->y = FD_FACE_WIDTH-fpos->x-1;
				for(int code = 0; code < 256; ++code)
				{
					int m[9], m_[9];
					m[1] = code & 0x01;
					m[2] = (code & 0x02)>>1;
					m[3] = (code & 0x04)>>2;
					m[4] = (code & 0x08)>>3;
					m[5] = (code & 0x10)>>4;
					m[6] = (code & 0x20)>>5;
					m[7] = (code & 0x40)>>6;
					m[8] = (code & 0x80)>>7;
					m_[1] = m[3]; m_[2] = m[4]; m_[3] = m[5]; m_[4] = m[6];
					m_[5] = m[7]; m_[6] = m[8]; m_[7] = m[1]; m_[8] = m[2];
					int newcode = (m_[8]<<7) | (m_[7]<<6) | (m_[6]<<5) | (m_[5]<<4) | (m_[4]<<3) | (m_[3]<<2) | (m_[2] <<1) | m_[1];
					new_lut[newcode] = lut[code];
				}
				new_fpos++;
				fpos++;
				lut += 256;
				new_lut += 256;
			}
			break;
		}
	}

	if(pCLS->feature_num[FEATURE_IDX_IRMB2] > 0)
	{
		int fnum = pCLS->feature_num[FEATURE_IDX_IRMB2];		
		pNewCLS->feature_pos[FEATURE_IDX_IRMB2] = (char *)MVFD_mem_alloc(fnum*sizeof(IRMB2_FEATURE_POS));
		pNewCLS->lookup_tbl[FEATURE_IDX_IRMB2] = (unsigned short *)MVFD_mem_alloc(fnum*720*sizeof(unsigned short));
		fd_memcpy(pNewCLS->lookup_tbl[FEATURE_IDX_IRMB2], pCLS->lookup_tbl[FEATURE_IDX_IRMB2], sizeof(unsigned short)*720*fnum);
		IRMB2_FEATURE_POS *new_fpos = (IRMB2_FEATURE_POS *)pNewCLS->feature_pos[FEATURE_IDX_IRMB2];
		IRMB2_FEATURE_POS *fpos = (IRMB2_FEATURE_POS *)pCLS->feature_pos[FEATURE_IDX_IRMB2];

		switch(transform_type)
		{
			case FEATURE_TRANSFORM_XFLIP:
				for(int i = 0; i < fnum; ++i)
				{
					for(int j = 0; j < 6; ++j)
					{
						new_fpos->p[j].x = FD_FACE_WIDTH-fpos->p[j].x-(1<<fpos->scale[j]);
						new_fpos->p[j].y = fpos->p[j].y;
						new_fpos->scale[j] = fpos->scale[j];
					}
					new_fpos++;
					fpos++;
				}
				break;
			case FEATURE_TRANSFORM_YFLIP:
				for(int i = 0; i < fnum; ++i)
				{
					for(int j = 0; j < 6; ++j)
					{
						new_fpos->p[j].x = fpos->p[j].x;
						new_fpos->p[j].y = FD_FACE_HEIGHT-fpos->p[j].y-(1<<fpos->scale[j]);
						new_fpos->scale[j] = fpos->scale[j];
					}
					new_fpos++;
					fpos++;
				}
				break;
			case FEATURE_TRANSFORM_ROT_90CW:
				for(int i = 0; i < fnum; ++i)
				{
					for(int j = 0; j < 6; ++j)
					{
						new_fpos->p[j].x = FD_FACE_HEIGHT-fpos->p[j].y-(1<<fpos->scale[j]);
						new_fpos->p[j].y = fpos->p[j].x;
						new_fpos->scale[j] = fpos->scale[j];
					}
					new_fpos++;
					fpos++;
				}
				break;
			case FEATURE_TRANSFORM_ROT_90CCW:
				for(int i = 0; i < fnum; ++i)
				{
					for(int j = 0; j < 6; ++j)
					{
						new_fpos->p[j].x = fpos->p[j].y;
						new_fpos->p[j].y = FD_FACE_WIDTH-fpos->p[j].x-(1<<fpos->scale[j]);
						new_fpos->scale[j] = fpos->scale[j];
					}
					new_fpos++;
					fpos++;
				}
				break;
		}
	}	
	
	if(pCLS->feature_num[FEATURE_IDX_SIMD8P] > 0)
	{
		int fnum = pCLS->feature_num[FEATURE_IDX_SIMD8P];		
		pNewCLS->feature_pos[FEATURE_IDX_SIMD8P] = (char *)MVFD_mem_alloc(fnum*sizeof(SIMD8P_FEATURE_POS));
		pNewCLS->lookup_tbl[FEATURE_IDX_SIMD8P] = (unsigned short *)MVFD_mem_alloc(fnum*256*sizeof(unsigned short));
		SIMD8P_FEATURE_POS *new_fpos = (SIMD8P_FEATURE_POS *)pNewCLS->feature_pos[FEATURE_IDX_SIMD8P];
		SIMD8P_FEATURE_POS *fpos = (SIMD8P_FEATURE_POS *)pCLS->feature_pos[FEATURE_IDX_SIMD8P];
		unsigned short *new_lut = pNewCLS->lookup_tbl[FEATURE_IDX_SIMD8P];
		unsigned short *src_lut = pCLS->lookup_tbl[FEATURE_IDX_SIMD8P];

		switch(transform_type)
		{
		case FEATURE_TRANSFORM_XFLIP:
			for(int i = 0; i < fnum; ++i)
			{
				for(int j = 0; j < 2; ++j)
				{
					if(fpos->p[j].type == 0)
					{
						new_fpos->p[j].x = FD_FACE_WIDTH-fpos->p[j].x-8;
						new_fpos->p[j].y = fpos->p[j].y;
					}
					else
					{
						new_fpos->p[j].x = FD_FACE_WIDTH-fpos->p[j].x-1;
						new_fpos->p[j].y = fpos->p[j].y;
					}
					new_fpos->p[j].type = fpos->p[j].type;
				}
				char order_type[2][2][4] = { {{3, 4, 4, 3},		//H+H
									{1, 2, 2, 1}},    //H+V
									{{2, 1, 1, 2},	    //V+H
									{4, 3, 3, 4}} 	//V+V
								};
				int ot = order_type[fpos->p[0].type][fpos->p[1].type][transform_type];
				bool bLUTrev = false;
				switch(ot)
				{
				case 1:
					for(int j = 0; j < 8; ++j) new_fpos->mask[7-j] = fpos->mask[j];
					bLUTrev = true;
					break;
				case 2:
					for(int j = 0; j < 8; ++j) new_fpos->mask[j] = 7-fpos->mask[j];
					break;
				case 3:
					for(int j = 0; j < 8; ++j) new_fpos->mask[7-j] = 7-fpos->mask[j];
					bLUTrev = true;
					break;
				case 4:
					for(int j = 0; j < 8; ++j) new_fpos->mask[j] = fpos->mask[j];
					break;
				}
				if(bLUTrev)
				{
					for(int code = 0; code < 256; ++code)
					{
						int newcode;
						newcode = ((code & 0x01) << 7) |
									((code & 0x02) << 5) |
									((code & 0x04) << 3) |
									((code & 0x08) << 1) |
									((code & 0x10) >> 1) |
									((code & 0x20) >> 3) |
									((code & 0x40) >> 5) |
									((code & 0x80) >> 7);
						new_lut[newcode] = src_lut[code];
					}
				}
				else fd_memcpy(new_lut, src_lut, 256*sizeof(unsigned short));
				src_lut += 256;
				new_lut += 256;
				new_fpos++;
				fpos++;
			}
			break;
		case FEATURE_TRANSFORM_YFLIP:
			for(int i = 0; i < fnum; ++i)
			{
				for(int j = 0; j < 2; ++j)
				{
					if(fpos->p[j].type == 0)
					{
						new_fpos->p[j].x = fpos->p[j].x;
						new_fpos->p[j].y = FD_FACE_HEIGHT-fpos->p[j].y-1;
					}
					else
					{	
						new_fpos->p[j].x = fpos->p[j].x;
						new_fpos->p[j].y = FD_FACE_HEIGHT-fpos->p[j].y-8;
					}
					new_fpos->p[j].type = fpos->p[j].type;
				}
				char order_type[2][2][4] = { {{3, 4, 4, 3},		//H+H
									{1, 2, 2, 1}},    //H+V
									{{2, 1, 1, 2},	    //V+H
									{4, 3, 3, 4}} 	//V+V
								};
				int ot = order_type[fpos->p[0].type][fpos->p[1].type][transform_type];
				bool bLUTrev = false;
				switch(ot)
				{
				case 1:
					for(int j = 0; j < 8; ++j) new_fpos->mask[7-j] = fpos->mask[j];
					bLUTrev = true;
					break;
				case 2:
					for(int j = 0; j < 8; ++j) new_fpos->mask[j] = 7-fpos->mask[j];
					break;
				case 3:
					for(int j = 0; j < 8; ++j) new_fpos->mask[7-j] = 7-fpos->mask[j];
					bLUTrev = true;
					break;
				case 4:
					for(int j = 0; j < 8; ++j) new_fpos->mask[j] = fpos->mask[j];
					break;
				}
				if(bLUTrev)
				{
					for(int code = 0; code < 256; ++code)
					{
						int newcode;
						newcode = ((code & 0x01) << 7) |
									((code & 0x02) << 5) |
									((code & 0x04) << 3) |
									((code & 0x08) << 1) |
									((code & 0x10) >> 1) |
									((code & 0x20) >> 3) |
									((code & 0x40) >> 5) |
									((code & 0x80) >> 7);
						new_lut[newcode] = src_lut[code];
					}
				}
				else fd_memcpy(new_lut, src_lut, 256*sizeof(unsigned short));
				src_lut += 256;
				new_lut += 256;
				new_fpos++;
				fpos++;
			}
			break;
		case FEATURE_TRANSFORM_ROT_90CW:
			for(int i = 0; i < fnum; ++i)
			{
				for(int j = 0; j < 2; ++j)
				{
					if(fpos->p[j].type == 0)
					{
						new_fpos->p[j].x = FD_FACE_HEIGHT-fpos->p[j].y-1;
						new_fpos->p[j].y = fpos->p[j].x;
					}
					else
					{
						new_fpos->p[j].x = FD_FACE_HEIGHT-fpos->p[j].y-8;
						new_fpos->p[j].y = fpos->p[j].x;
					}
					new_fpos->p[j].type = !fpos->p[j].type;
				}
				char order_type[2][2][4] = { {{3, 4, 4, 3},		//H+H
									{1, 2, 2, 1}},    //H+V
									{{2, 1, 1, 2},	    //V+H
									{4, 3, 3, 4}} 	//V+V
								};
				int ot = order_type[fpos->p[0].type][fpos->p[1].type][transform_type];
				bool bLUTrev = false;
				switch(ot)
				{
				case 1:
					for(int j = 0; j < 8; ++j) new_fpos->mask[7-j] = fpos->mask[j];
					bLUTrev = true;
					break;
				case 2:
					for(int j = 0; j < 8; ++j) new_fpos->mask[j] = 7-fpos->mask[j];
					break;
				case 3:
					for(int j = 0; j < 8; ++j) new_fpos->mask[7-j] = 7-fpos->mask[j];
					bLUTrev = true;
					break;
				case 4:
					for(int j = 0; j < 8; ++j) new_fpos->mask[j] = fpos->mask[j];
					break;
				}
				if(bLUTrev)
				{
					for(int code = 0; code < 256; ++code)
					{
						int newcode;
						newcode = ((code & 0x01) << 7) |
									((code & 0x02) << 5) |
									((code & 0x04) << 3) |
									((code & 0x08) << 1) |
									((code & 0x10) >> 1) |
									((code & 0x20) >> 3) |
									((code & 0x40) >> 5) |
									((code & 0x80) >> 7);
						new_lut[newcode] = src_lut[code];
					}
				}
				else fd_memcpy(new_lut, src_lut, 256*sizeof(unsigned short));
				src_lut += 256;
				new_lut += 256;
				new_fpos++;
				fpos++;
			}
			break;
		case FEATURE_TRANSFORM_ROT_90CCW:
			for(int i = 0; i < fnum; ++i)
			{
				for(int j = 0; j < 2; ++j)
				{
					if(fpos->p[j].type == 0)
					{
						new_fpos->p[j].x = fpos->p[j].y;
						new_fpos->p[j].y = FD_FACE_WIDTH-fpos->p[j].x-8;
					}
					else
					{
						new_fpos->p[j].x = fpos->p[j].y;
						new_fpos->p[j].y = FD_FACE_WIDTH-fpos->p[j].x-1;
					}
					new_fpos->p[j].type = !fpos->p[j].type;
				}
				char order_type[2][2][4] = { {{3, 4, 4, 3},		//H+H
									{1, 2, 2, 1}},    //H+V
									{{2, 1, 1, 2},	    //V+H
									{4, 3, 3, 4}} 	//V+V
								};
				int ot = order_type[fpos->p[0].type][fpos->p[1].type][transform_type];
				bool bLUTrev = false;
				switch(ot)
				{
				case 1:
					for(int j = 0; j < 8; ++j) new_fpos->mask[7-j] = fpos->mask[j];
					bLUTrev = true;
					break;
				case 2:
					for(int j = 0; j < 8; ++j) new_fpos->mask[j] = 7-fpos->mask[j];
					break;
				case 3:
					for(int j = 0; j < 8; ++j) new_fpos->mask[7-j] = 7-fpos->mask[j];
					bLUTrev = true;
					break;
				case 4:
					for(int j = 0; j < 8; ++j) new_fpos->mask[j] = fpos->mask[j];
					break;
				}
				if(bLUTrev)
				{
					for(int code = 0; code < 256; ++code)
					{
						int newcode;
						newcode = ((code & 0x01) << 7) |
									((code & 0x02) << 5) |
									((code & 0x04) << 3) |
									((code & 0x08) << 1) |
									((code & 0x10) >> 1) |
									((code & 0x20) >> 3) |
									((code & 0x40) >> 5) |
									((code & 0x80) >> 7);
						new_lut[newcode] = src_lut[code];
					}
				}
				else fd_memcpy(new_lut, src_lut, 256*sizeof(unsigned short));
				src_lut += 256;
				new_lut += 256;
				new_fpos++;
				fpos++;
			}
			break;
		}
	}
	

	return pNewCLS;
}

void MVFD_SetThreshold(int pose_idx, double multiplier)
{
	MVFD_Detector[pose_idx][5]->threshold = MVFD_Detector[pose_idx][5]->threshold*multiplier;
}

void MVFD_WriteSingleClassifier(BUFFER *buffer, FD_CLASSIFIER *pClassifier)
{
	int frange[] = {512, 720, 256, 256, 512};
	int fpos_sz[] = {sizeof(POINT_FEATURE_POS), sizeof(IRMB2_FEATURE_POS), sizeof(SIMD8P_FEATURE_POS), sizeof(POINT_FEATURE_POS), sizeof(POINT_FEATURE_POS)};
	MVFD_buffer_write(buffer, &pClassifier->class_num, sizeof(int));
	MVFD_buffer_write(buffer, &pClassifier->feature_num, sizeof(int)*10);
	MVFD_buffer_write(buffer, pClassifier->thresholds, sizeof(pClassifier->thresholds));
	MVFD_buffer_write(buffer, &pClassifier->threshold, sizeof(unsigned int));
		
	MVFD_buffer_write(buffer, pClassifier->likelihood, sizeof(int)*FD_MAX_CLASS_NUM*100);
	MVFD_buffer_write(buffer, pClassifier->fv_lower, sizeof(int)*FD_MAX_CLASS_NUM);
	MVFD_buffer_write(buffer, pClassifier->fv_upper, sizeof(int)*FD_MAX_CLASS_NUM);

	MVFD_buffer_write(buffer, &pClassifier->minv, sizeof(double));
	MVFD_buffer_write(buffer, &pClassifier->double2short, sizeof(double));
		
	MVFD_buffer_write(buffer, &pClassifier->total_feature_num, sizeof(int));

	for(int k = 0; k < FD_FEATURE_TYPE_NUM; ++k)
	{
		if(pClassifier->feature_num[k] > 0)
		{
			MVFD_buffer_write(buffer, pClassifier->feature_pos[k], pClassifier->feature_num[k]*fpos_sz[k]);
			int lookup_tbl_sz;
			if(pClassifier->class_num == 1) lookup_tbl_sz = pClassifier->feature_num[k]*frange[k]*sizeof(short);
			else lookup_tbl_sz = pClassifier->feature_num[k]*frange[k]*(((pClassifier->class_num+7)>>3)<<3)*sizeof(short);
			MVFD_buffer_write_align16(buffer, pClassifier->lookup_tbl[k], lookup_tbl_sz);
		}
	}
}

void WriteAllToHeader()
{
	int buf_pos = 0;
	BUFFER buffer;
	buffer.ptr = (char *)MVFD_mem_alloc(100*1024*1024);	//100MB
	buffer.size = 0;
	buffer.rd_pos = 0;

	MVFD_WriteSingleClassifier(&buffer, MVFD_BkgndRejector);
	MVFD_WriteSingleClassifier(&buffer, MVFD_PoseClassifier);
	
	//int pose_num = 5;
	//int pose_idx[] = {11, 14, 10, 13, 15};	
	//int detector_num[] = {6, 6, 7, 7, 7};
	
	int pose_num = 9;
	int pose_idx[9];
	int detector_num[9];
	for(int i = 0; i < 9; ++i)
	{
		pose_idx[i] = i+1;
		detector_num[i] = MVFDCascadeNum[i+1];
	}

	for(int i = 0; i < pose_num; ++i)
	{
		for(int j = 0; j < detector_num[i]; ++j)
		{
			MVFD_WriteSingleClassifier(&buffer, MVFD_Detector[pose_idx[i]][j]);
		}
	}

	FILE *fp = fopen("MVFD2_data(9).h", "wt");
	fprintf(fp, "#pragma once\n");
	fprintf(fp, "const unsigned long long FD_FaceData[]={\n");
	unsigned long long *ptr = (unsigned long long *)buffer.ptr;
	int size_8b = (buffer.size+7)/8;
	for(int i = 0; i < size_8b; ++i)
	{
		fprintf(fp, "0x%llx,", ptr[i]);
		if(i != 0 && i % 100 == 0) fprintf(fp, "\n");
	}
	fprintf(fp, "};\n");
	fclose(fp);

	MVFD_mem_free(buffer.ptr);
}

void LoadSingleClassifier(BUFFER *buffer, FD_CLASSIFIER *pClassifier)
{
	int one_ffpos_size[5] = {4, 24, 24, 4, 4};
	int frange[] = {512, 720, 256, 256, 512};
	int fpos_sz[] = {sizeof(POINT_FEATURE_POS), sizeof(IRMB2_FEATURE_POS), sizeof(SIMD8P_FEATURE_POS), sizeof(POINT_FEATURE_POS), sizeof(POINT_FEATURE_POS)};
	
	fd_memset(pClassifier, 0, sizeof(FD_CLASSIFIER));

	pClassifier->from_header = 1;
	MVFD_buffer_read_copy(buffer, &pClassifier->class_num, sizeof(int));
	MVFD_buffer_read_copy(buffer, &pClassifier->feature_num, sizeof(int)*10);
	MVFD_buffer_read_copy(buffer, pClassifier->thresholds, sizeof(pClassifier->thresholds));
	MVFD_buffer_read_copy(buffer, &pClassifier->threshold, sizeof(unsigned int));
		
	MVFD_buffer_read_copy(buffer, pClassifier->likelihood, sizeof(int)*FD_MAX_CLASS_NUM*100);
	MVFD_buffer_read_copy(buffer, pClassifier->fv_lower, sizeof(int)*FD_MAX_CLASS_NUM);
	MVFD_buffer_read_copy(buffer, pClassifier->fv_upper, sizeof(int)*FD_MAX_CLASS_NUM);

	MVFD_buffer_read_copy(buffer, &pClassifier->minv, sizeof(double));
	MVFD_buffer_read_copy(buffer, &pClassifier->double2short, sizeof(double));

	MVFD_buffer_read_copy(buffer, &pClassifier->total_feature_num, sizeof(int));

	for(int k = 0; k < FD_FEATURE_TYPE_NUM; ++k)
	{
		if(pClassifier->feature_num[k] > 0)
		{
			pClassifier->feature_pos[k] = (char *)MVFD_mem_alloc(pClassifier->feature_num[k]*fpos_sz[k]);
			MVFD_buffer_read_copy(buffer, pClassifier->feature_pos[k], pClassifier->feature_num[k]*fpos_sz[k]);

			int lookup_tbl_sz;
			if(pClassifier->class_num == 1) lookup_tbl_sz = pClassifier->feature_num[k]*frange[k]*sizeof(short);
			else lookup_tbl_sz = pClassifier->feature_num[k]*frange[k]*(((pClassifier->class_num+7)>>3)<<3)*sizeof(short);

			pClassifier->lookup_tbl[k] = (unsigned short *)MVFD_buffer_read_align16(buffer, lookup_tbl_sz);
		}
	}
}

#if 1
void MVFD_LoadFromHeader()
{
	int buf_pos = 0;
	BUFFER buffer;
	buffer.ptr = (char *)FD_FaceData;
	buffer.rd_pos = 0;

	int pose_num = 9;
	int detector_num[10] = {0, 7, 6, 7, 7, 7, 7, 6, 7, 6};

	MVFD_BkgndRejector = (FD_CLASSIFIER *)MVFD_mem_alloc(sizeof(FD_CLASSIFIER));
	LoadSingleClassifier(&buffer, MVFD_BkgndRejector);
	MVFD_PoseClassifier = (FD_CLASSIFIER *)MVFD_mem_alloc(sizeof(FD_CLASSIFIER));
	LoadSingleClassifier(&buffer, MVFD_PoseClassifier);
	for(int i = 1; i <= pose_num; ++i)
	{
		for(int j = 0; j < detector_num[i]; ++j)
		{
			MVFD_Detector[i][j] = (FD_CLASSIFIER *)MVFD_mem_alloc(sizeof(FD_CLASSIFIER));
			LoadSingleClassifier(&buffer, MVFD_Detector[i][j]);
			MVFD_Detector[i][j]->pose = i;
		}
	}
}
#endif

#ifndef WIN32
void MVFD_Init(PResFileIO pResFileIO)
{
	//FD_CLASSIFIER *FD_Temp[8];
	
	//Background Rejector
	MVFD_BkgndRejector = MVFD_LoadClassifier16(pResFileIO, "cls/rejector_32.cls.f16"); ///storage/sdcard0/MVFD/cls/rejector_32.cls.f16

	//Pose classifier
	MVFD_PoseClassifier = MVFD_LoadClassifier16(pResFileIO, "cls/pose_32.cls.f16");

	int fnum = MVFD_PoseClassifier->feature_num[FEATURE_IDX_IRMB2];
	IRMB2_FEATURE_POS *pose_fpos = (IRMB2_FEATURE_POS *)MVFD_PoseClassifier->feature_pos[FEATURE_IDX_IRMB2];
	IRMB2_FEATURE_POS *rej_fpos = (IRMB2_FEATURE_POS *)MVFD_BkgndRejector->feature_pos[FEATURE_IDX_IRMB2];
	unsigned short *pose_lut = MVFD_PoseClassifier->lookup_tbl[FEATURE_IDX_IRMB2];
	unsigned short *rej_lut = MVFD_BkgndRejector->lookup_tbl[FEATURE_IDX_IRMB2];
	unsigned short *new_lut = new unsigned short[fnum*720*8];
	for(int i = 0; i < fnum; ++i)
	{
		for(int j = 0; j < fnum; ++j)
		{
			int sz = sizeof(IRMB2_FEATURE_POS);
			if(fd_memcmp(&rej_fpos[i], &pose_fpos[j], sizeof(IRMB2_FEATURE_POS)) == 0)
			{
				fd_memcpy(&new_lut[i*720*8], &pose_lut[j*720*8], 720*8*2);
				break;
			}
		}
	}
	fd_memcpy(pose_fpos, rej_fpos, fnum*sizeof(IRMB2_FEATURE_POS));
	fd_memcpy(pose_lut, new_lut, fnum*720*8*2);
	delete [] new_lut;


	//0' frontal face
	MVFD_Detector[9][0] = MVFD_LoadClassifier(pResFileIO, "cls/front0/cascade1.cls.f");
	MVFD_Detector[9][1] = MVFD_LoadClassifier(pResFileIO, "cls/front0/cascade2.cls.f");
	MVFD_Detector[9][2] = MVFD_LoadClassifier(pResFileIO, "cls/front0/cascade3.cls.f");
	MVFD_Detector[9][3] = MVFD_LoadClassifier(pResFileIO, "cls/front0/cascade4.cls.f");
	MVFD_Detector[9][4] = MVFD_LoadClassifier(pResFileIO, "cls/front0/cascade5.cls.f");
	MVFD_Detector[9][5] = MVFD_LoadClassifier(pResFileIO, "cls/front0/cascade6.cls.f");
	MVFD_Detector[9][5]->pose = 9;
	
	//30' frontal face
	MVFD_Detector[7][0] = MVFD_LoadClassifier(pResFileIO, "cls/front30/cascade1.cls.f");
	MVFD_Detector[7][1] = MVFD_LoadClassifier(pResFileIO, "cls/front30/cascade2.cls.f");
	MVFD_Detector[7][2] = MVFD_LoadClassifier(pResFileIO, "cls/front30/cascade3.cls.f");
	MVFD_Detector[7][3] = MVFD_LoadClassifier(pResFileIO, "cls/front30/cascade4.cls.f");
	MVFD_Detector[7][4] = MVFD_LoadClassifier(pResFileIO, "cls/front30/cascade5.cls.f");
	MVFD_Detector[7][5] = MVFD_LoadClassifier(pResFileIO, "cls/front30/cascade6.cls.f");
	MVFD_Detector[7][5]->threshold *= 1.002;
	MVFD_Detector[7][5]->pose = 7;

	//-30' frontal face
	for(int i = 0; i < 6; ++i)
		MVFD_Detector[2][i] = MVFD_FeatureTransform(MVFD_Detector[7][i], FEATURE_TRANSFORM_XFLIP);
	MVFD_Detector[2][5]->pose = 2;

	//0' left profile
	MVFD_Detector[4][0] = MVFD_LoadClassifier(pResFileIO, "cls/left0/cascade1.cls.f");
	MVFD_Detector[4][1] = MVFD_LoadClassifier(pResFileIO, "cls/left0/cascade2.cls.f");
	MVFD_Detector[4][2] = MVFD_LoadClassifier(pResFileIO, "cls/left0/cascade3.cls.f");
	MVFD_Detector[4][3] = MVFD_LoadClassifier(pResFileIO, "cls/left0/cascade4.cls.f");
	MVFD_Detector[4][4] = MVFD_LoadClassifier(pResFileIO, "cls/left0/cascade5.cls.f");
	MVFD_Detector[4][5] = MVFD_LoadClassifier(pResFileIO, "cls/left0/cascade6.cls.f");
	MVFD_Detector[4][6] = MVFD_LoadClassifier(pResFileIO, "cls/left0/cascade7.cls.f");
	MVFD_Detector[4][6]->pose = 4;
	MVFD_Detector[4][6]->threshold *= 1.005;

	//0' right profile
	for(int i = 0; i < 7; ++i)
		MVFD_Detector[5][i] = MVFD_FeatureTransform(MVFD_Detector[4][i], FEATURE_TRANSFORM_XFLIP);
	MVFD_Detector[5][6]->pose = 5;
	
	//30' left profile
	MVFD_Detector[6][0] = MVFD_LoadClassifier(pResFileIO, "cls/left30/cascade1.cls.f");
	MVFD_Detector[6][1] = MVFD_LoadClassifier(pResFileIO, "cls/left30/cascade2.cls.f");
	MVFD_Detector[6][2] = MVFD_LoadClassifier(pResFileIO, "cls/left30/cascade3.cls.f");
	MVFD_Detector[6][3] = MVFD_LoadClassifier(pResFileIO, "cls/left30/cascade4.cls.f");
	MVFD_Detector[6][4] = MVFD_LoadClassifier(pResFileIO, "cls/left30/cascade5.cls.f");
	MVFD_Detector[6][5] = MVFD_LoadClassifier(pResFileIO, "cls/left30/cascade6.cls.f");
	MVFD_Detector[6][6] = MVFD_LoadClassifier(pResFileIO, "cls/left30/cascade7.cls.f");
	MVFD_Detector[6][6]->pose = 6;
	MVFD_Detector[6][6]->threshold *= 1.005;
	
	//-30' right profile
	for(int i = 0; i < 7; ++i)
		MVFD_Detector[3][i] = MVFD_FeatureTransform(MVFD_Detector[6][i], FEATURE_TRANSFORM_XFLIP);
	MVFD_Detector[3][6]->pose = 3;
	
	//30' right profile
	MVFD_Detector[8][0] = MVFD_LoadClassifier(pResFileIO, "cls/right30/cascade1.cls.f");
	MVFD_Detector[8][1] = MVFD_LoadClassifier(pResFileIO, "cls/right30/cascade2.cls.f");
	MVFD_Detector[8][2] = MVFD_LoadClassifier(pResFileIO, "cls/right30/cascade3.cls.f");
	MVFD_Detector[8][3] = MVFD_LoadClassifier(pResFileIO, "cls/right30/cascade4.cls.f");
	MVFD_Detector[8][4] = MVFD_LoadClassifier(pResFileIO, "cls/right30/cascade5.cls.f");
	MVFD_Detector[8][5] = MVFD_LoadClassifier(pResFileIO, "cls/right30/cascade6.cls.f");
	MVFD_Detector[8][6] = MVFD_LoadClassifier(pResFileIO, "cls/right30/cascade7.cls.f");
	MVFD_Detector[8][6]->pose = 8;
	MVFD_Detector[8][6]->threshold *= 1.005;

	//-30' left profile
	for(int i = 0; i < 7; ++i)
		MVFD_Detector[1][i] = MVFD_FeatureTransform(MVFD_Detector[8][i], FEATURE_TRANSFORM_XFLIP);
	MVFD_Detector[1][6]->pose = 1;
	
	for(int i = 1; i <= 9; ++i)
	{
		if(MVFD_Detector[i][0])
		{
			for(int j = 0;;j++)
			{
				if(MVFD_Detector[i][j+1]) MVFD_Detector[i][j]->next = MVFD_Detector[i][j+1];
				else
				{
					MVFD_Detector[i][j]->next = NULL;
					MVFDCascadeNum[i] = j+1;
					break;
				}
			}
		}
	}

	//set new likelihoods
	//front0
	MVFD_Detector[9][5]->likelihood_new = (unsigned int *)lkh_front0_likelihood;
	MVFD_Detector[9][5]->fv_lower_new = (unsigned int)lkh_front0_lower;
	MVFD_Detector[9][5]->fv_upper_new = (unsigned int)lkh_front0_upper;

	//front30
	MVFD_Detector[7][5]->likelihood_new = (unsigned int *)lkh_front30_likelihood;
	MVFD_Detector[7][5]->fv_lower_new = (unsigned int)lkh_front30_lower;
	MVFD_Detector[7][5]->fv_upper_new = (unsigned int)lkh_front30_upper;

	//front-30
	MVFD_Detector[2][5]->likelihood_new = (unsigned int *)lkh_front30_likelihood;
	MVFD_Detector[2][5]->fv_lower_new = (unsigned int)lkh_front30_lower;
	MVFD_Detector[2][5]->fv_upper_new = (unsigned int)lkh_front30_upper;

	//left0
	FD_Detector[4][6]->likelihood_new = (unsigned int *)lkh_left0_likelihood;
	FD_Detector[4][6]->fv_lower_new = (unsigned int)lkh_left0_lower;
	FD_Detector[4][6]->fv_upper_new = (unsigned int)lkh_left0_upper;

	//right0
	MVFD_Detector[5][6]->likelihood_new = (unsigned int *)lkh_left0_likelihood;
	MVFD_Detector[5][6]->fv_lower_new = (unsigned int)lkh_left0_lower;
	MVFD_Detector[5][6]->fv_upper_new = (unsigned int)lkh_left0_upper;
	
	//left30
	MVFD_Detector[6][6]->likelihood_new = (unsigned int *)lkh_left30_likelihood;
	MVFD_Detector[6][6]->fv_lower_new = (unsigned int)lkh_left30_lower;
	MVFD_Detector[6][6]->fv_upper_new = (unsigned int)lkh_left30_upper;
	
	//right-30
	MVFD_Detector[3][6]->likelihood_new = (unsigned int *)lkh_left30_likelihood;
	MVFD_Detector[3][6]->fv_lower_new = (unsigned int)lkh_left30_lower;
	MVFD_Detector[3][6]->fv_upper_new = (unsigned int)lkh_left30_upper;

	//right30
	MVFD_Detector[8][6]->likelihood_new = (unsigned int *)lkh_right30_likelihood;
	MVFD_Detector[8][6]->fv_lower_new = (unsigned int)lkh_right30_lower;
	MVFD_Detector[8][6]->fv_upper_new = (unsigned int)lkh_right30_upper;
	
	//left-30
	MVFD_Detector[1][6]->likelihood_new = (unsigned int *)lkh_right30_likelihood;
	MVFD_Detector[1][6]->fv_lower_new = (unsigned int)lkh_right30_lower;
	MVFD_Detector[1][6]->fv_upper_new = (unsigned int)lkh_right30_upper;

	//for making header file
	//WriteAllToHeader();
	//exit(0);
}
#endif

void MVFD_Init_Header()
{
	MVFD_LoadFromHeader();
	
	int pose_num = 9;
	int detector_num[10] = {0, 7, 6, 7, 7, 7, 7, 6, 7, 6};
	for(int i = 1; i <= pose_num; ++i)
	{
		if(MVFD_Detector[i][0])
		{
			for(int j = 0; j < detector_num[i]; j++)
			{
				if(MVFD_Detector[i][j+1]) MVFD_Detector[i][j]->next = MVFD_Detector[i][j+1];
				else
				{
					MVFD_Detector[i][j]->next = NULL;
					MVFDCascadeNum[i] = j+1;
					break;
				}
			}
		}
	}

	//set new likelihoods
	//front0
	MVFD_Detector[9][5]->likelihood_new = (unsigned int *)lkh_front0_likelihood;
	MVFD_Detector[9][5]->fv_lower_new = (unsigned int)lkh_front0_lower;
	MVFD_Detector[9][5]->fv_upper_new = (unsigned int)lkh_front0_upper;

	//front30
	MVFD_Detector[7][5]->likelihood_new = (unsigned int *)lkh_front30_likelihood;
	MVFD_Detector[7][5]->fv_lower_new = (unsigned int)lkh_front30_lower;
	MVFD_Detector[7][5]->fv_upper_new = (unsigned int)lkh_front30_upper;

	//front-30
	MVFD_Detector[2][5]->likelihood_new = (unsigned int *)lkh_front30_likelihood;
	MVFD_Detector[2][5]->fv_lower_new = (unsigned int)lkh_front30_lower;
	MVFD_Detector[2][5]->fv_upper_new = (unsigned int)lkh_front30_upper;

	//left0
	MVFD_Detector[4][6]->likelihood_new = (unsigned int *)lkh_left0_likelihood;
	MVFD_Detector[4][6]->fv_lower_new = (unsigned int)lkh_left0_lower;
	MVFD_Detector[4][6]->fv_upper_new = (unsigned int)lkh_left0_upper;

	//right0
	MVFD_Detector[5][6]->likelihood_new = (unsigned int *)lkh_left0_likelihood;
	MVFD_Detector[5][6]->fv_lower_new = (unsigned int)lkh_left0_lower;
	MVFD_Detector[5][6]->fv_upper_new = (unsigned int)lkh_left0_upper;
	
	//left30
	MVFD_Detector[6][6]->likelihood_new = (unsigned int *)lkh_left30_likelihood;
	MVFD_Detector[6][6]->fv_lower_new = (unsigned int)lkh_left30_lower;
	MVFD_Detector[6][6]->fv_upper_new = (unsigned int)lkh_left30_upper;
	
	//right-30
	MVFD_Detector[3][6]->likelihood_new = (unsigned int *)lkh_left30_likelihood;
	MVFD_Detector[3][6]->fv_lower_new = (unsigned int)lkh_left30_lower;
	MVFD_Detector[3][6]->fv_upper_new = (unsigned int)lkh_left30_upper;

	//right30
	MVFD_Detector[8][6]->likelihood_new = (unsigned int *)lkh_right30_likelihood;
	MVFD_Detector[8][6]->fv_lower_new = (unsigned int)lkh_right30_lower;
	MVFD_Detector[8][6]->fv_upper_new = (unsigned int)lkh_right30_upper;
	
	//left-30
	MVFD_Detector[1][6]->likelihood_new = (unsigned int *)lkh_right30_likelihood;
	MVFD_Detector[1][6]->fv_lower_new = (unsigned int)lkh_right30_lower;
	MVFD_Detector[1][6]->fv_upper_new = (unsigned int)lkh_right30_upper;
}

//release memories used in MVFD
void MVFD_Release()
{
	for(int i = 1; i <= FD_MAX_CLASS_NUM; ++i)
	{
		for(int j = 0; j < FD_MAX_STAGE_NUM; ++j) 
		{
			if(MVFD_Detector[i][j])
			{
				MVFD_FreeClassifier(MVFD_Detector[i][j]);
				MVFD_Detector[i][j] = NULL;
			}
		}
	}
	if(MVFD_BkgndRejector) MVFD_FreeClassifier(MVFD_BkgndRejector);
	if(MVFD_PoseClassifier) MVFD_FreeClassifier(MVFD_PoseClassifier);
	MVFD_PoseClassifier = NULL;
	MVFD_BkgndRejector = NULL;
}

void MVFD_AllocFeaturePositions(FD_CONTEXT *pContext)
{
	int one_ffpos_size[FD_FEATURE_TYPE_NUM] = {4, 24, 24, 4, 4};
	int all_ffpos_sz = one_ffpos_size[FEATURE_IDX_IRMB2]*MVFD_PoseClassifier->feature_num[FEATURE_IDX_IRMB2];
	for(int i = 0; i < FD_MAX_CLASS_NUM; ++i)
	{
		for(int j = 0; j < FD_MAX_STAGE_NUM; ++j)
		{
			if(MVFD_Detector[i][j])
			{
				int one_stage_ffpos_sz = one_ffpos_size[FEATURE_IDX_MCT]*MVFD_Detector[i][j]->feature_num[FEATURE_IDX_MCT]+
									one_ffpos_size[FEATURE_IDX_MCT5x5]*MVFD_Detector[i][j]->feature_num[FEATURE_IDX_MCT5x5]+
									one_ffpos_size[FEATURE_IDX_LGP]*MVFD_Detector[i][j]->feature_num[FEATURE_IDX_LGP]+
									one_ffpos_size[FEATURE_IDX_SIMD8P]*MVFD_Detector[i][j]->feature_num[FEATURE_IDX_SIMD8P]+
									one_ffpos_size[FEATURE_IDX_IRMB2]*MVFD_Detector[i][j]->feature_num[FEATURE_IDX_IRMB2];
				all_ffpos_sz += one_stage_ffpos_sz;
			}
		}
	}
	all_ffpos_sz *= 2;	//one for detection, another for tracking

	char *ffpos_buf = MVFD_MemSection_Alloc<char>(pContext, MEM_SECTION_FAST_FEATURE_POSITIONS, all_ffpos_sz);
	int curr_ffpos_sz = 0;

	pContext->det_ffpos.pose_map[FEATURE_IDX_IRMB2] = (int *)&ffpos_buf[curr_ffpos_sz];
	curr_ffpos_sz += one_ffpos_size[FEATURE_IDX_IRMB2]*MVFD_PoseClassifier->feature_num[FEATURE_IDX_IRMB2];
	for(int i = 0; i < FD_MAX_CLASS_NUM; ++i)
	{
		for(int j = 0; j < FD_MAX_STAGE_NUM; ++j)
		{
			if(MVFD_Detector[i][j])
			{
				pContext->det_ffpos.face[i][j][FEATURE_IDX_MCT] = (int *)&ffpos_buf[curr_ffpos_sz];
				curr_ffpos_sz += one_ffpos_size[FEATURE_IDX_MCT]*MVFD_Detector[i][j]->feature_num[FEATURE_IDX_MCT];

				pContext->det_ffpos.face[i][j][FEATURE_IDX_MCT5x5] = (int *)&ffpos_buf[curr_ffpos_sz];
				curr_ffpos_sz += one_ffpos_size[FEATURE_IDX_MCT5x5]*MVFD_Detector[i][j]->feature_num[FEATURE_IDX_MCT5x5];

				pContext->det_ffpos.face[i][j][FEATURE_IDX_LGP] = (int *)&ffpos_buf[curr_ffpos_sz];
				curr_ffpos_sz += one_ffpos_size[FEATURE_IDX_LGP]*MVFD_Detector[i][j]->feature_num[FEATURE_IDX_LGP];
				
				pContext->det_ffpos.face[i][j][FEATURE_IDX_SIMD8P] = (int *)&ffpos_buf[curr_ffpos_sz];
				curr_ffpos_sz += one_ffpos_size[FEATURE_IDX_SIMD8P]*MVFD_Detector[i][j]->feature_num[FEATURE_IDX_SIMD8P];

				pContext->det_ffpos.face[i][j][FEATURE_IDX_IRMB2] = (int *)&ffpos_buf[curr_ffpos_sz];
				curr_ffpos_sz += one_ffpos_size[FEATURE_IDX_IRMB2]*MVFD_Detector[i][j]->feature_num[FEATURE_IDX_IRMB2];
			}
		}
	}

	pContext->trk_ffpos.pose_map[FEATURE_IDX_IRMB2] = (int *)&ffpos_buf[curr_ffpos_sz];
	curr_ffpos_sz += one_ffpos_size[FEATURE_IDX_IRMB2]*MVFD_PoseClassifier->feature_num[FEATURE_IDX_IRMB2];
	for(int i = 0; i < FD_MAX_CLASS_NUM; ++i)
	{
		for(int j = 0; j < FD_MAX_STAGE_NUM; ++j)
		{
			if(MVFD_Detector[i][j])
			{
				pContext->trk_ffpos.face[i][j][FEATURE_IDX_MCT] = (int *)&ffpos_buf[curr_ffpos_sz];
				curr_ffpos_sz += one_ffpos_size[FEATURE_IDX_MCT]*MVFD_Detector[i][j]->feature_num[FEATURE_IDX_MCT];

				pContext->trk_ffpos.face[i][j][FEATURE_IDX_MCT5x5] = (int *)&ffpos_buf[curr_ffpos_sz];
				curr_ffpos_sz += one_ffpos_size[FEATURE_IDX_MCT5x5]*MVFD_Detector[i][j]->feature_num[FEATURE_IDX_MCT5x5];

				pContext->trk_ffpos.face[i][j][FEATURE_IDX_LGP] = (int *)&ffpos_buf[curr_ffpos_sz];
				curr_ffpos_sz += one_ffpos_size[FEATURE_IDX_LGP]*MVFD_Detector[i][j]->feature_num[FEATURE_IDX_LGP];
				
				pContext->trk_ffpos.face[i][j][FEATURE_IDX_SIMD8P] = (int *)&ffpos_buf[curr_ffpos_sz];
				curr_ffpos_sz += one_ffpos_size[FEATURE_IDX_SIMD8P]*MVFD_Detector[i][j]->feature_num[FEATURE_IDX_SIMD8P];

				pContext->trk_ffpos.face[i][j][FEATURE_IDX_IRMB2] = (int *)&ffpos_buf[curr_ffpos_sz];
				curr_ffpos_sz += one_ffpos_size[FEATURE_IDX_IRMB2]*MVFD_Detector[i][j]->feature_num[FEATURE_IDX_IRMB2];
			}
		}
	}
}


void *MVFD_Detector_Thread(void *pArg);
void *MVFD_Video_Detection_Thread(void *pArg);
FD_CONTEXT *MVFD_CreateContext()
{
	FD_CONTEXT *pContext = new FD_CONTEXT;
	MVFD_Mutex_Create(&pContext->mem_section_mutex);

	pContext->wait_for_exit = false;
	pContext->subImage_width = 0;
	pContext->subImage_height = 0;
	MVFD_InitFaceEnrollStorage(&pContext->detect_enroll_faces);
	MVFD_InitFaceEnrollStorage(&pContext->track_enroll_faces);
	MVFD_Mutex_Create(&pContext->mem_section_mutex);
	MVFD_Mutex_Create(&pContext->finish_wait_mutex);
	MVFD_Mutex_Create(&pContext->finish_count_mutex);
	MVFD_Mutex_Lock(&pContext->finish_wait_mutex);

	MVFD_AllocFeaturePositions(pContext);
	for(int i = 0; i < FD_MAX_THREADS_NUM; ++i)
	{
		MVFD_Mutex_Create(&pContext->wakeup_mutex[i]);
		MVFD_Mutex_Lock(&pContext->wakeup_mutex[i]);
		pContext->face_candi_xy_buf[i] = (int *)MVFD_mem_alloc(FD_MAX_FACE_CANDIDATES_PER_THREAD*4*sizeof(int));
		pContext->found_face_cnt[i] = 0;		
		pContext->subImage[i] = NULL;
		FD_THREAD_PARAM *param = new FD_THREAD_PARAM;
		param->pArg = pContext;
		param->thread_idx = i;
		MVFD_Thread_Create(&pContext->worker_threads[i], MVFD_Detector_Thread, (void *)param);
	}	

	memset(&pContext->video_info, 0, sizeof(FD_CONTEXT::VIDEO_INFO));
	pContext->video_info.frame_complete = true;
	MVFD_Mutex_Create(&pContext->video_info.frame_recv_mutex);
	MVFD_Mutex_Lock(&pContext->video_info.frame_recv_mutex);
	MVFD_Thread_Create(&pContext->video_info.video_detection_thread, MVFD_Video_Detection_Thread, pContext);
	pContext->m_nFinalResultCnt = 0;
	pContext->finish_count = 0;	
	pContext->worker_num = 0;

	return pContext;
}

void MVFD_DestroyContext(FD_CONTEXT *pContext)
{
	MVFD_FreeFaceEnrollStorage(&pContext->detect_enroll_faces);
	MVFD_FreeFaceEnrollStorage(&pContext->track_enroll_faces);

	pContext->wait_for_exit = true;
	for(int i = 0; i < FD_MAX_THREADS_NUM; ++i) MVFD_Mutex_UnLock(&pContext->wakeup_mutex[i]);
	MVFD_Thread_Join(pContext->worker_threads, FD_MAX_THREADS_NUM);

	MVFD_Mutex_UnLock(&pContext->video_info.frame_recv_mutex);
	MVFD_Thread_Join(&pContext->video_info.video_detection_thread, 1);
	
	for(int i = 0; i < FD_MAX_THREADS_NUM; ++i)
	{
		MVFD_Mutex_Release(&pContext->wakeup_mutex[i]);	
		MVFD_mem_free(pContext->face_candi_xy_buf[i]);		
		MVFD_Thread_Release(&pContext->worker_threads[i]);
	}
	MVFD_Mutex_Release(&pContext->video_info.frame_recv_mutex);
	MVFD_Thread_Release(&pContext->video_info.video_detection_thread);

	MVFD_MemSection_FreeAll(pContext);
	MVFD_Mutex_Release(&pContext->mem_section_mutex);
	MVFD_Mutex_Release(&pContext->finish_wait_mutex);
	MVFD_Mutex_Release(&pContext->finish_count_mutex);	
	delete pContext;
}
