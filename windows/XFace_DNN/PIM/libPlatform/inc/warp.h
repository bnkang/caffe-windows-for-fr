/*
 * Copyright IM Lab,(http://imlab.postech.ac.kr)
 * 
 * All rights reserved.
 * 
 */

#ifndef __WARP_H__
#define __WARP_H__

#include "PIMTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define SetBit(x,y) (x|=(1<<y))
#define ClrBit(x,y) (x&=(~(1<<y)))
#define ChkBit(x,y) ((x>>y)&1)  // caution
#define BOUND_CHCK(val, down, up) (val<down?down:val>up?up:val)
#define FLIP_POINT(x, width) (width- 1- x)



/**
 *  @brief 
 *  @note reference points was rotated upside down.
 *
 *  @param  [IN] : 
 *
 *  @return 
 */
PIM_Int32 warpPointBy2Points(PIM_Int32 in_x, PIM_Int32 in_y,
                             PIM_Int32 leyex1, PIM_Int32 leyey1, PIM_Int32 reyex1, PIM_Int32 reyey1,
                             PIM_Int32 *out_x, PIM_Int32 *out_y,
                             PIM_Int32 leyex2, PIM_Int32 leyey2, PIM_Int32 reyex2, PIM_Int32 reyey2);

/**
 *  @brief 
 *
 *  @param  [IN] : 
 *
 *  @return 
 */
PIM_Int32 warpImageBy2Points_uc_scale(PIM_Uint8 *src, PIM_Int32 w1, PIM_Int32 h1,
                                      PIM_Int32 lx1, PIM_Int32 ly1, PIM_Int32 rx1, PIM_Int32 ry1,
                                      PIM_Uint8 **dst, PIM_Int32 w2, PIM_Int32 h2,
                                      PIM_Int32 lx2, PIM_Int32 ly2, PIM_Int32 rx2, PIM_Int32 ry2 );

/**
 *  @brief 
 *
 *  @param  [IN] : 
 *
 *  @return 
 */
PIM_Void strength2Pnts_i(PIM_Int32 *pnt1, PIM_Int32 *pnt2, PIM_Int32 *out_pnt1, PIM_Int32 *out_pnt2, PIM_Int32 strengthShift5);

/**
 *  @brief 
 *
 *  @param  [IN] : 
 *
 *  @return 
 */
PIM_Void rotation_degree(PIM_Int32 * point_in, PIM_Int32* point_out, PIM_Int32 degree);

/**
 *  @brief 
 *
 *  @param  [IN] : 
 *
 *  @return 
 */
PIM_Int32 warpImageBy2Points(const PIM_Bitmap *imgGray,
                             PIM_Int32 lx1, PIM_Int32 ly1, PIM_Int32 rx1, PIM_Int32 ry1,
                             PIM_Bitmap *imgAligned,
                             PIM_Int32 lx2, PIM_Int32 ly2, PIM_Int32 rx2, PIM_Int32 ry2 );

#ifndef __CODE_SIZE_OPTIMIZE__
PIM_Void vinv_copy_mat_uc(PIM_Uint8 *mat_in, PIM_Uint8 *mat_out, PIM_Int32 n_row, PIM_Int32 n_col);             // must "mat_in != mat_out"
PIM_Void vinv_copy_mat_uc_vertical(PIM_Uint8 *mat_in, PIM_Uint8 *mat_out, PIM_Int32 n_row, PIM_Int32 n_col);    // must "mat_in != mat_out"
#endif //__CODE_SIZE_OPTIMIZE__

#ifdef __cplusplus
}
#endif


#endif
