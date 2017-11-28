#include "stdafx.h"

#include "HP_Defines.h"
#include "irmb3_table.h"
#include <stdlib.h>
#include <math.h>
#include <memory.h>

#if !defined(HP_USE_SEE) && !defined(HP_USE_NEON)
void EstimatePoseRect(unsigned char *pimage, int fd_rop_idx, float p_i[], float p[], REGRESSOR *R)
{
	float *var = R->var;
	float *mean = R->mean;
	memcpy(p, p_i, sizeof(float)*ALL_PARAM_NUM);
	for(int f = 0; f < R->fnum; ++f)
	{
		int fidx = R->wr[f].fidx;
		float (*lut)[ROP_NUM][ALL_PARAM_NUM] = R->wr[f].lut;

		float cx = p[PIDX_CX]*var[PIDX_CX]+mean[PIDX_CX];
		float cy = p[PIDX_CY]*var[PIDX_CY]+mean[PIDX_CY];
		float csize = (p[PIDX_SIZE]*var[PIDX_SIZE]+mean[PIDX_SIZE])/2.0;
		float rip = p[PIDX_RIP]*var[PIDX_RIP]+mean[PIDX_RIP];
		float cosv = cos(rip*3.141592/180.0);
		float sinv = sin(rip*3.141592/180.0);

		float *fx = R->fpos_x[f];
		float *fy = R->fpos_y[f];
		unsigned char interpol_i[FEATURE_POINTS_NUM];
		for(int k = 0; k < FEATURE_POINTS_NUM; ++k)
		{
			float src_x =  csize*(fx[k]*cosv+fy[k]*sinv)+cx;
			float src_y = -csize*(fx[k]*sinv-fy[k]*cosv)+cy;
			int lx = src_x;
			int rx = HP_MIN(int(lx)+1, IMAGE_WIDTH-1);
			int uy = src_y;
			int by = HP_MIN(int(uy)+1, IMAGE_HEIGHT-1);
			float lt = pimage[IMAGE_WIDTH*uy+lx];
			float rt = pimage[IMAGE_WIDTH*uy+rx];
			float lb = pimage[IMAGE_WIDTH*by+lx];
			float rb = pimage[IMAGE_WIDTH*by+rx];
			float dx = src_x-lx;
			float dy = src_y-uy;
			float dxdy = dx*dy;
			interpol_i[k] = HP_CLIP(lt*(1.0f-dx-dy+dxdy) + rt*(dx-dxdy) + lb*(dy-dxdy) + rb*dxdy, 0, 255);
		}
		unsigned char *i = interpol_i;
		int lcode = (i[0]>=i[1]) | ((i[0] >=i[2])<<1) | ((i[0]>=i[3])<<2) | ((i[0]>=i[4])<<3) |
					((i[1]>=i[2])<<4) | ((i[1]>=i[3])<<5) | ((i[1]>=i[4])<<6) |
					((i[2]>=i[3])<<7) | ((i[2]>=i[4])<<8) |
					((i[3] >=i[4])<<9);

		int code = irmb3_table[lcode];
		float *lut_param = lut[code][fd_rop_idx];

		p[PIDX_CX] += lut_param[PIDX_CX];
		p[PIDX_CY] += lut_param[PIDX_CY];
		p[PIDX_SIZE] += lut_param[PIDX_SIZE];
		p[PIDX_RIP] += lut_param[PIDX_RIP];
	}
}

float ComputeROP(unsigned char *image, int width, int height, HP_CLASSIFIER *pCls, int fd_rop_idx)
{
	unsigned int confidence[5] = {0,};
	//MCT
	if(pCls->feature_num[FEATURE_IDX_MCT3x3])
	{
		HP_POINT_FEATURE_POS *fpos =  (HP_POINT_FEATURE_POS *)pCls->feature_pos[FEATURE_IDX_MCT3x3];
		unsigned short (*lut)[5] = (unsigned short (*)[5])pCls->lookup_tbl[FEATURE_IDX_MCT3x3];
		int fnum = pCls->feature_num[FEATURE_IDX_MCT3x3];
		while(fnum--)
		{
			unsigned char *p1 = image + ((fpos->y-1)*width+(fpos->x-1));
			unsigned char *p2 = p1 + width;
			unsigned char *p3 = p2 + width;
			unsigned int m1 = *p1, m2 = *(p1+1), m3 = *(p1+2);
			unsigned int m4 = *p2, m5 = *(p2+1), m6 = *(p2+2);
			unsigned int m7 = *p3, m8 = *(p3+1), m9 = *(p3+2);
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
			confidence[0] += lut[mct_code][0];
			confidence[1] += lut[mct_code][1];
			confidence[2] += lut[mct_code][2];
			confidence[3] += lut[mct_code][3];
			confidence[4] += lut[mct_code][4];
			lut += 512;
			fpos++;
		}
	}
	if(pCls->feature_num[FEATURE_IDX_MCT5x5])
	{
		HP_POINT_FEATURE_POS *fpos =  (HP_POINT_FEATURE_POS *)pCls->feature_pos[FEATURE_IDX_MCT5x5];
		unsigned short (*lut)[5] = (unsigned short (*)[5])pCls->lookup_tbl[FEATURE_IDX_MCT5x5];
		int fnum = pCls->feature_num[FEATURE_IDX_MCT5x5];
		while(fnum--)
		{
			unsigned char *p1 = image + ((fpos->y-2)*width+(fpos->x-2));
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
			confidence[0] += lut[mct_code][0];
			confidence[1] += lut[mct_code][1];
			confidence[2] += lut[mct_code][2];
			confidence[3] += lut[mct_code][3];
			confidence[4] += lut[mct_code][4];
			lut += 512;
			fpos++;
		}
	}
	if(pCls->feature_num[FEATURE_IDX_MCT7x7])
	{
		HP_POINT_FEATURE_POS *fpos =  (HP_POINT_FEATURE_POS *)pCls->feature_pos[FEATURE_IDX_MCT7x7];
		unsigned short (*lut)[5] = (unsigned short (*)[5])pCls->lookup_tbl[FEATURE_IDX_MCT7x7];
		int fnum = pCls->feature_num[FEATURE_IDX_MCT7x7];
		while(fnum--)
		{
			unsigned char *p1 = image + ((fpos->y-3)*width+(fpos->x-3));
			unsigned char *p2 = p1 + (width*3);
			unsigned char *p3 = p2 + (width*3);
			unsigned int m1 = *p1, m2 = *(p1+3), m3 = *(p1+6);
			unsigned int m4 = *p2, m5 = *(p2+3), m6 = *(p2+6);
			unsigned int m7 = *p3, m8 = *(p3+3), m9 = *(p3+6);
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
			confidence[0] += lut[mct_code][0];
			confidence[1] += lut[mct_code][1];
			confidence[2] += lut[mct_code][2];
			confidence[3] += lut[mct_code][3];
			confidence[4] += lut[mct_code][4];
			lut += 512;
			fpos++;
		}
	}
	
	int max_idx = 0;
	unsigned int max_conf = confidence[0];
	for(int i = 0; i < 5; ++i)
	{
		if(confidence[i] > max_conf) 
		{
			max_idx = i;
			max_conf = confidence[i];
		}
	}
	confidence[max_idx] = 0;
	int second_idx = 0;
	unsigned int second_conf = 0;
	for(int i = 0; i < 5; ++i)
	{
		if(confidence[i] > second_conf) 
		{
			second_idx = i;
			second_conf = confidence[i];
		}
	}
	
	float rop_val;
	float rop[3][5] = {{-4, -3, -2, -1, 0},
					  {-2, -1,  0, +1, +2},
					  {0 , +1, +2, +3, +4}};

	if(second_conf > pCls->thresholds[second_idx] && abs(second_idx-max_idx) == 1)
	{
		unsigned int c1 = max_conf-pCls->thresholds[max_idx];
		unsigned int c2 = second_conf-pCls->thresholds[second_idx];
		float r = c1/float(c1+c2);
		rop_val = rop[fd_rop_idx][max_idx]*r + rop[fd_rop_idx][second_idx]*(1.0f-r);
	}
	else rop_val = rop[fd_rop_idx][max_idx];

	return rop_val;
}

void WarpImage(unsigned char *src, int src_w, int src_h, unsigned char *dst, int dst_w, int dst_h, float param[2][3])
{
	float *xp = param[0];
	float *yp = param[1];
	for(int y = 0; y < dst_h; y++)
	{
		for(int x = 0; x < dst_w; x++)
		{
			float src_x = x*xp[0]+y*xp[1]+xp[2];
			float src_y = x*yp[0]+y*yp[1]+yp[2];
			if(src_x < 0 || src_x >= src_w || src_y < 0 || src_y >= src_h)
			{
				unsigned char *dp = &dst[dst_w*y+x];				
				*dp++ = 0;
			}
			else
			{
				unsigned char *dp = &dst[dst_w*y+x];
				
				int lx = src_x;
				int rx = HP_MIN(int(src_x)+1, src_w-1);
				int uy = src_y;
				int by = HP_MIN(int(src_y)+1, src_h-1);
				float lt = src[src_w*uy+lx];
				float rt = src[src_w*uy+rx];
				float lb = src[src_w*by+lx];
				float rb = src[src_w*by+rx];
				float dx = src_x-lx;
				float dy = src_y-uy;
				float dxdy = dx*dy;
				float interpol_i = lt*(1.0f-dx-dy+dxdy) + rt*(dx-dxdy) + lb*(dy-dxdy) + rb*dxdy;
				*dp++ = interpol_i;
			}
		}
	}
}
#endif