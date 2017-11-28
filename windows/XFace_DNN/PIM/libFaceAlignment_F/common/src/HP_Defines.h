#pragma once

#include "HP_Interface.h"

//#define HP_USE_SEE
//#define HP_USE_NEON
#define  FEATURE_POINTS_NUM 	5
#define  IMAGE_WIDTH 			90
#define  IMAGE_HEIGHT 			90
#define  FCODE_RANGE 			120
#define  ALL_PARAM_NUM 			4
#define  ROP_NUM 				3
#define  PIDX_CX 				0
#define  PIDX_CY 				1
#define  PIDX_SIZE 				2
#define  PIDX_RIP 				3

#define  HP_USED_FEATURE_NUM 	3
#define  FEATURE_IDX_MCT3x3 	0
#define  FEATURE_IDX_MCT5x5 	1
#define  FEATURE_IDX_MCT7x7 	2

typedef struct 
{
	int fidx;
	float (*lut)[ROP_NUM][ALL_PARAM_NUM];
} WR;

typedef struct
{
	int fnum;
	float mean[ALL_PARAM_NUM];
	float var[ALL_PARAM_NUM];
	float (*fpos_x)[FEATURE_POINTS_NUM];
	float (*fpos_y)[FEATURE_POINTS_NUM];
	WR *wr;
	bool from_header;
} REGRESSOR;

typedef struct
{
	float lut[FCODE_RANGE][ROP_NUM][ALL_PARAM_NUM];
} LOOKUPTBL;

typedef struct _HP_POINT_FEATURE_POS
{
	int x, y;
} HP_POINT_FEATURE_POS;

typedef struct _HP_CLASSIFIER
{
	int class_num;
	unsigned int likelihood[8][100];
	unsigned int fv_lower[8];
	unsigned int fv_upper[8];
	unsigned int thresholds[24];
	unsigned int threshold;
	double minv;
	double double2short;
	int total_feature_num;
	int feature_type_num;
	int feature_type[10];
	int feature_num[10];
	char *feature_pos[10];
	int  *fast_fpos[10];
	int pose;
	int from_header;
	unsigned short *lookup_tbl[10];
	struct _HP_CLASSIFIER *next;
} HP_CLASSIFIER;

#define HP_BETWEEN(a, lower, upper) ((lower) <= (a) && (a) <= (upper))
#define HP_CLIP(a, min, max) ((a) < (min) ? (min) : (((a) > (max)) ? (max) : (a)))
#define HP_MAX(a, b) ((a) > (b) ? (a) : (b))
#define HP_MIN(a, b) ((a) < (b) ? (a) : (b))
