#pragma once

typedef struct
{
	int fnum;
	float mean[ALL_PARAM_NUM];
	float var[ALL_PARAM_NUM];
	float (*fpos_x)[FEATURE_POINTS_NUM];
	float (*fpos_y)[FEATURE_POINTS_NUM];
	typedef struct 
	{
		int fidx;
		float (*lut)[ROP_NUM][ALL_PARAM_NUM];
	} WR;
	WR *wr;
	bool from_header;
} REGRESSOR;

typedef struct
{
	float x, y;
} JPOINTF2;

typedef struct
{
	int left, top, right, bottom;
} JRECT;

HP_CLASSIFIER *HP_LoadClassifier(char *fpath);
HP_CLASSIFIER *HP_LoadClassifier16(char *fpath);
float ComputeROP(unsigned char *image, int width, int height, HP_CLASSIFIER *pCls, int fd_rop_idx);
