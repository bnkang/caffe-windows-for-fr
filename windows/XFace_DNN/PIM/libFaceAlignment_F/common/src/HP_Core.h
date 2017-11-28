#pragma once
#include "HP_Defines.h"
void EstimatePoseRect(unsigned char *pimage, int fd_rop_idx, float p_i[], float p[], REGRESSOR *R);
float ComputeROP(unsigned char *image, int width, int height, HP_CLASSIFIER *pCls, int fd_rop_idx);
void WarpImage(unsigned char *src, int src_w, int src_h, unsigned char *dst, int dst_w, int dst_h, float param[2][3]);