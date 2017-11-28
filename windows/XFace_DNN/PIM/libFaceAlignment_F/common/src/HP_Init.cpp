#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
//#include <vector>
#include "HP_Defines.h"
#include "HP_Data.h"
//using namespace std;

REGRESSOR *PoseR = NULL;
HP_CLASSIFIER *ROP_Classifier[3] = {0,};

typedef struct _TRACKING_FACE_INFO TRACKING_FACE_INFO;
//extern vector<TRACKING_FACE_INFO *> tfi;
typedef struct _BUFFER
{
	char *ptr;
	int size;
	int rd_pos;
} BUFFER;

HP_CLASSIFIER *HP_LoadClassifier(char *fpath);
void HP_WriteClassifier_Buffer(BUFFER *buffer, HP_CLASSIFIER *pClassifier);
HP_CLASSIFIER *HP_LoadClassifier_Buffer(BUFFER *buffer);

static void buffer_write(BUFFER *buf, void *src, int size)
{
	memcpy(&buf->ptr[buf->size], src, size);
	buf->size += size;
}
static void *buffer_read(BUFFER *buf, int size)
{
	int prev_rd_pos = buf->rd_pos;
	buf->rd_pos += size;
	return &buf->ptr[prev_rd_pos];
}
static void buffer_read_copy(BUFFER *buf, void *dst, int size)
{
	memcpy(dst, &buf->ptr[buf->rd_pos], size);
	buf->rd_pos += size;
}

#if 0
//Currently It is not used
REGRESSOR *LoadR(char *fpath)
{
	REGRESSOR *R = new REGRESSOR;
	FILE *fp = fopen(fpath, "rb");
	fread(&R->fnum, sizeof(int), 1, fp);
	
	fread(R->mean, sizeof(float)*ALL_PARAM_NUM, 1, fp);
	fread(R->var,  sizeof(float)*ALL_PARAM_NUM, 1, fp);

	R->fpos_x = new float[R->fnum][FEATURE_POINTS_NUM];
	R->fpos_y = new float[R->fnum][FEATURE_POINTS_NUM];
	fread(R->fpos_x, R->fnum*FEATURE_POINTS_NUM*sizeof(float), 1, fp);
	fread(R->fpos_y, R->fnum*FEATURE_POINTS_NUM*sizeof(float), 1, fp);

	R->wr = new REGRESSOR::WR[R->fnum];
	R->from_header = false;
	for(int i = 0; i < R->fnum; ++i)
	{
		R->wr[i].lut = new float[FCODE_RANGE][ROP_NUM][ALL_PARAM_NUM];
		fread(R->wr[i].lut, FCODE_RANGE*ROP_NUM*ALL_PARAM_NUM*sizeof(float), 1, fp);
	}
	fclose(fp);
	return R;
}

#endif

void HP_FreeR(REGRESSOR *R)
{
	if(!R->from_header)
	{
		delete [] R->fpos_x;
		delete [] R->fpos_y;

		free(R->wr);
/*
		for(int i = 0; i < R->fnum; ++i)
		{
			//delete [] R->wr[i].lut;
		}
*/

	}
	delete [] R->wr;

	//delete R;
	free(R);
}

void HP_FreeClassifier(HP_CLASSIFIER *pCls)
{
	if(!pCls->from_header)
	{
		for(int j = 0; j < HP_USED_FEATURE_NUM; ++j)
		{
			if(pCls->feature_num[j] > 0)
			{
				free(pCls->feature_pos[j]);
				free(pCls->lookup_tbl[j]);
				free(pCls->fast_fpos[j]);
			}
		}
	}
	delete pCls;
}

#if 0
void HP_Init()
{
	//PoseR = LoadR("result_irmb2_5bin_iter1300_GA_newtrain3.R");	
	PoseR = LoadR("result_irmb2_5bin_iter2000_GA_newtrain3.R");	
	ROP_Classifier[0] = HP_LoadClassifier("result_left_new.cls.fix");
	ROP_Classifier[1] = HP_LoadClassifier("result_front_new.cls.fix");
	ROP_Classifier[2] = HP_LoadClassifier("result_right_new.cls.fix");

	//for making header file
#if 1
	BUFFER wr_buf;
	wr_buf.ptr = new char[50*1024*1024];
	memset(wr_buf.ptr, 0, 50*1024*1024);
	wr_buf.rd_pos = 0;
	wr_buf.size = 0;

	buffer_write(&wr_buf, &PoseR->fnum, sizeof(int));	
	buffer_write(&wr_buf, PoseR->mean, sizeof(float)*ALL_PARAM_NUM);	
	buffer_write(&wr_buf, PoseR->var,  sizeof(float)*ALL_PARAM_NUM);

	buffer_write(&wr_buf, PoseR->fpos_x,  PoseR->fnum*FEATURE_POINTS_NUM*sizeof(float));		
	buffer_write(&wr_buf, PoseR->fpos_y,  PoseR->fnum*FEATURE_POINTS_NUM*sizeof(float));

	for(int i = 0; i < PoseR->fnum; ++i)
	{
		buffer_write(&wr_buf, PoseR->wr[i].lut,  FCODE_RANGE*ROP_NUM*ALL_PARAM_NUM*sizeof(float));
	}
	HP_WriteClassifier_Buffer(&wr_buf, ROP_Classifier[0]);
	HP_WriteClassifier_Buffer(&wr_buf, ROP_Classifier[1]);
	HP_WriteClassifier_Buffer(&wr_buf, ROP_Classifier[2]);

	FILE *wfp = fopen("HP_Data.h", "wt");
	fprintf(wfp, "#pragma once\n");
	fprintf(wfp, "const unsigned long long HP_Data[]={\n");
	unsigned long long *ptr = (unsigned long long *)wr_buf.ptr;
	int size_8b = (wr_buf.size+7)/8;
	for(int i = 0; i < size_8b; ++i)
	{
		fprintf(wfp, "0x%I64x,", ptr[i]);
		if(i != 0 && i % 100 == 0) fprintf(wfp, "\n");
	}
	fprintf(wfp, "};\n");
	fclose(wfp);
	exit(0);
#endif
}
#endif

#if 1
void HP_Init()
{
	BUFFER rd_buf;
	rd_buf.ptr = (char *)HP_Data;
	rd_buf.rd_pos = 0;
	rd_buf.size = sizeof(HP_Data);
	//PoseR = new REGRESSOR;
	PoseR = (REGRESSOR *) malloc(sizeof(REGRESSOR));
	
	buffer_read_copy(&rd_buf, &PoseR->fnum, sizeof(int));	
	buffer_read_copy(&rd_buf, PoseR->mean, sizeof(float)*ALL_PARAM_NUM);	
	buffer_read_copy(&rd_buf, PoseR->var,  sizeof(float)*ALL_PARAM_NUM);

	PoseR->fpos_x = (float (*)[FEATURE_POINTS_NUM])buffer_read(&rd_buf, PoseR->fnum*FEATURE_POINTS_NUM*sizeof(float));		
	PoseR->fpos_y = (float (*)[FEATURE_POINTS_NUM])buffer_read(&rd_buf, PoseR->fnum*FEATURE_POINTS_NUM*sizeof(float));

	//PoseR->wr = new REGRESSOR::WR[PoseR->fnum];
	PoseR->wr = (WR *) malloc(sizeof(WR) * PoseR->fnum);
	for(int i = 0; i < PoseR->fnum; ++i)
	{
		PoseR->wr[i].lut = (float (*)[ROP_NUM][ALL_PARAM_NUM])buffer_read(&rd_buf, FCODE_RANGE*ROP_NUM*ALL_PARAM_NUM*sizeof(float));
	}	
	
	ROP_Classifier[0] = HP_LoadClassifier_Buffer(&rd_buf);
	ROP_Classifier[1] = HP_LoadClassifier_Buffer(&rd_buf);
	ROP_Classifier[2] = HP_LoadClassifier_Buffer(&rd_buf);

	PoseR->from_header = true;
}
#endif

void HP_Release()
{
	if(PoseR)
	{
		HP_FreeR(PoseR);
		HP_FreeClassifier(ROP_Classifier[0]);
		HP_FreeClassifier(ROP_Classifier[1]);
		HP_FreeClassifier(ROP_Classifier[2]);

		PoseR = NULL;
		ROP_Classifier[0] = NULL;
		ROP_Classifier[1] = NULL;
		ROP_Classifier[2] = NULL;

		
		//for(int i = 0; i < tfi.size(); ++i) delete tfi[i];
		//tfi.clear();
	}
}

//Load a classifier specified as fpath
HP_CLASSIFIER *HP_LoadClassifier(char *fpath)
{
	HP_CLASSIFIER *pCls = new HP_CLASSIFIER;
	memset(pCls, 0, sizeof(HP_CLASSIFIER));

	FILE *fp = fopen(fpath, "rb");
	if(!fp)
	{
		printf("Unable to load [%s].\n", fpath);
		exit(-1);
	}
	int width, height;
	int one_elem_bits_num, sum_elem_bits_num;
	double double_range;
	unsigned int double2int;
	fread(&pCls->class_num, sizeof(int), 1, fp);
	fread(&width, sizeof(int), 1, fp);
	fread(&height, sizeof(int), 1, fp);
	
	fread(&one_elem_bits_num, sizeof(int), 1, fp);
	fread(&sum_elem_bits_num, sizeof(int), 1, fp);
	fread(&double_range, sizeof(double), 1, fp);
	fread(&double2int, sizeof(unsigned int), 1, fp);

	fread(pCls->thresholds, sizeof(unsigned int)*pCls->class_num, 1, fp);

	fread(pCls->fv_lower, sizeof(unsigned int)*pCls->class_num, 1, fp);
	fread(pCls->fv_upper, sizeof(unsigned int)*pCls->class_num, 1, fp);
	for(int i = 0; i < pCls->class_num; ++i)
	{
		fread(pCls->likelihood[i], sizeof(unsigned int)*100, 1, fp);
	}
	fread(&pCls->total_feature_num, sizeof(int), 1, fp);
	int ptset_size;
	fread(&ptset_size, sizeof(int), 1, fp);
	
	int file_pos;
	int feature_bin_num[3] = {512, 512, 512};
	int one_ffpos_size[3] = {4, 4, 4};
	int fpos_size[3];
	fpos_size[0] = sizeof(HP_POINT_FEATURE_POS);
	fpos_size[1] = sizeof(HP_POINT_FEATURE_POS);
	fpos_size[2] = sizeof(HP_POINT_FEATURE_POS);

	for(int i = 0; i < 2; ++i)
	{
		if(i == 0) file_pos = ftell(fp);
		else
		{
			fseek(fp, file_pos, SEEK_SET);
			for(int j = 0; j < HP_USED_FEATURE_NUM; ++j)
			{
				if(pCls->feature_num[j] > 0)
				{
					pCls->feature_pos[j] = (char *)malloc(pCls->feature_num[j]*fpos_size[j]);	
					pCls->lookup_tbl[j] = (unsigned short *)malloc(sizeof(short)*pCls->class_num*feature_bin_num[j]*pCls->feature_num[j]);
					pCls->fast_fpos[j] = (int *)malloc(pCls->feature_num[j]*one_ffpos_size[j]);
					pCls->feature_num[j] = 0;
				}
			}
		}
		for(int f = 0; f < pCls->total_feature_num; ++f)
		{
			int feature_type, class_num, value_range;
			fread(&feature_type, sizeof(int), 1, fp);
			fread(&class_num, sizeof(int), 1, fp);
			int feature_idx;
			switch(feature_type)
			{
			case 0:
				feature_idx = FEATURE_IDX_MCT3x3;
				break;
			case 11:
				feature_idx = FEATURE_IDX_MCT5x5;
				break;
			case 12:
				feature_idx = FEATURE_IDX_MCT7x7;
				break;
			default:
				break;
			}
			if(pCls->feature_pos[feature_idx])
			{
				fread(&pCls->feature_pos[feature_idx][pCls->feature_num[feature_idx]*fpos_size[feature_idx]], fpos_size[feature_idx], 1, fp);			
				fread(&value_range, sizeof(int), 1, fp);
				unsigned short *lookup_tbl = &pCls->lookup_tbl[feature_idx][pCls->feature_num[feature_idx]*feature_bin_num[feature_idx]*class_num];
				
				fread(lookup_tbl, pCls->class_num*feature_bin_num[feature_idx]*sizeof(unsigned short), 1, fp);
			}
			else
			{
				fseek(fp, fpos_size[feature_idx], SEEK_CUR);
				fread(&value_range, sizeof(int), 1, fp);
				for(int j = 0; j < value_range; ++j)
				{
					fseek(fp, pCls->class_num*sizeof(unsigned short), SEEK_CUR);
				}
			}
			pCls->feature_num[feature_idx]++;
		}
	}
	fclose(fp);
	return pCls;
}

void HP_WriteClassifier_Buffer(BUFFER *buffer, HP_CLASSIFIER *pClassifier)
{
	int frange[] = {512, 512, 512};
	int fpos_sz[] = {sizeof(HP_POINT_FEATURE_POS), sizeof(HP_POINT_FEATURE_POS), sizeof(HP_POINT_FEATURE_POS)};
	buffer_write(buffer, &pClassifier->class_num, sizeof(int));
	buffer_write(buffer, &pClassifier->feature_num, sizeof(int)*HP_USED_FEATURE_NUM);
	buffer_write(buffer, pClassifier->thresholds, sizeof(unsigned int)*pClassifier->class_num);	
	buffer_write(buffer, &pClassifier->threshold, sizeof(unsigned int));		
	buffer_write(buffer, pClassifier->fv_lower, sizeof(int));
	buffer_write(buffer, pClassifier->fv_upper, sizeof(int));
	buffer_write(buffer, pClassifier->likelihood, sizeof(int)*100);
	buffer_write(buffer, &pClassifier->minv, sizeof(double));
	buffer_write(buffer, &pClassifier->double2short, sizeof(double));		
	buffer_write(buffer, &pClassifier->total_feature_num, sizeof(int));

	for(int k = 0; k < HP_USED_FEATURE_NUM; ++k)
	{
		if(pClassifier->feature_num[k] > 0)
		{
			buffer_write(buffer, pClassifier->feature_pos[k], pClassifier->feature_num[k]*fpos_sz[k]);
			int lookup_tbl_sz = pClassifier->feature_num[k]*pClassifier->class_num*frange[k]*sizeof(short);
			buffer_write(buffer, pClassifier->lookup_tbl[k], lookup_tbl_sz);
		}
	}
}

HP_CLASSIFIER *HP_LoadClassifier_Buffer(BUFFER *buffer)
{
	HP_CLASSIFIER *pClassifier = new HP_CLASSIFIER;
	memset(pClassifier, 0, sizeof(HP_CLASSIFIER));
	int frange[] = {512, 512, 512};
	int fpos_sz[] = {sizeof(HP_POINT_FEATURE_POS), sizeof(HP_POINT_FEATURE_POS), sizeof(HP_POINT_FEATURE_POS)};
	buffer_read_copy(buffer, &pClassifier->class_num, sizeof(int));
	buffer_read_copy(buffer, &pClassifier->feature_num, sizeof(int)*HP_USED_FEATURE_NUM);
	buffer_read_copy(buffer, pClassifier->thresholds, sizeof(unsigned int)*pClassifier->class_num);	
	buffer_read_copy(buffer, &pClassifier->threshold, sizeof(unsigned int));		
	buffer_read_copy(buffer, pClassifier->fv_lower, sizeof(int));
	buffer_read_copy(buffer, pClassifier->fv_upper, sizeof(int));
	buffer_read_copy(buffer, pClassifier->likelihood, sizeof(int)*100);
	buffer_read_copy(buffer, &pClassifier->minv, sizeof(double));
	buffer_read_copy(buffer, &pClassifier->double2short, sizeof(double));		
	buffer_read_copy(buffer, &pClassifier->total_feature_num, sizeof(int));

	for(int k = 0; k < HP_USED_FEATURE_NUM; ++k)
	{
		if(pClassifier->feature_num[k] > 0)
		{
			pClassifier->feature_pos[k] = (char *)buffer_read(buffer, pClassifier->feature_num[k]*fpos_sz[k]);
			int lookup_tbl_sz = pClassifier->feature_num[k]*pClassifier->class_num*frange[k]*sizeof(short);
			pClassifier->lookup_tbl[k] = (unsigned short *)buffer_read(buffer, lookup_tbl_sz);
		}
	}
	pClassifier->from_header = true;
	return pClassifier;
}
