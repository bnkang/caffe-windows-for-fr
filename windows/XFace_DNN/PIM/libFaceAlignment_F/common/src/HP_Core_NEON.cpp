#include "HP_Defines.h"
#include "irmb3_table.h"
#include <stdlib.h>
#include <math.h>
#include <memory.h>

#ifdef HP_USE_NEON
void EstimatePoseRect(unsigned char *pimage, int fd_rop_idx, float p_i[], float p[], REGRESSOR *R)
{
	//will to be implemented by Samsung
}

float ComputeROP(unsigned char *image, int width, int height, HP_CLASSIFIER *pCls, int fd_rop_idx)
{
	//will to be implemented by Samsung
	return 0;
}

void WarpImage(unsigned char *src, int src_w, int src_h, unsigned char *dst, int dst_w, int dst_h, float param[2][3])
{
	//will to be implemented by Samsung
}
#endif