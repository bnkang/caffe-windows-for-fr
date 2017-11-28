/*
 * Copyright IM Lab,(http://imlab.postech.ac.kr)
 * 
 * All rights reserved.
 * 
 */

#ifndef __PIMMATH_H__
#define __PIMMATH_H__

#include "PIMTypes.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 *  @brief 
 *
 *  @param nom [IN] : 
 *  @param denom [IN] : 
 *
 *  @return 
 */
PIM_Uint32 PIM_Math_Devide(PIM_Uint32 nom, PIM_Uint32 denom);

/**
 *  @brief 
 *
 *  @param x [IN] : 
 *
 *  @return 
 */
PIM_Uint32 PIM_Math_Sqrt(PIM_Uint32 x);

/**
 *  @brief 
 *
 *  @param x [IN] : 
 *
 *  @return 
 */
PIM_Int32 PIM_Math_SquareInt( PIM_Int32 x);

/**
 *  @brief 
 *
 *  @param x [IN] : 
 *
 *  @return 
 */
PIM_Int32 PIM_Math_SquareRoot(PIM_Int32 x);

/*
 *
 */
#define PIM_Math_Max(_A_,_B_)      (((_A_) > (_B_)) ? (_A_) : (_B_))
#define PIM_Math_Min(_A_,_B_)      (((_A_) < (_B_)) ? (_A_) : (_B_))
#define PIM_Math_Abs(_A_)          (((_A_) > 0)   ? (_A_) : -(_A_))
#define PIM_Math_ClipToByte(_A_)   ( (_A_) < 0 ? 0 : ( (_A_) > 255 ) ? 255 : ( _A_ ) )

/**
 *  @brief 
 *
 *  @param x [IN] : 
 *
 *  @return 
 */
extern PIM_Int32 PIM_Math_Log10000(PIM_Int32 x);


#define PIM_Math_KCosSBase 65536
#define PIM_Math_KCosSBaseShifter	16		// = log2(PIM_Math_KCosSBase)
#define PIM_Math_KTanSBaseShifter   10		//  = log2(1024)

/**
 *  @brief 
 *
 *  @param degree [IN] : 
 *
 *  @return 
 */
extern PIM_Int32 PIM_Math_SinS(PIM_Int32 degree);

/**
 *  @brief 
 *
 *  @param degree [IN] : 
 *
 *  @return 
 */
extern PIM_Int32 PIM_Math_CosS(PIM_Int32 degree);	

/**
 *  @brief 
 *
 *  @param y0 [IN] : 
 *  @param x0 [IN] : 
 *
 *  @return 
 */
extern PIM_Int32 PIM_Math_ArcTan(PIM_Int32 y0, PIM_Int32 x0);



/***********************************************************************************
 * ??
 **********************************************************************************/
int mySqrtInt(int x);
int myDevideInt(int nom, int denom);

extern void make_integralimage_0padding(int **img, int **img_out, int n_row, int n_col);
extern void make_integralimage_0padding_Blink(int **img, int **img_out, int n_row, int n_col);
extern void cal_sqmat(int **img, int **img_out, int n_row, int n_col);

#define cal_min_twoint	PIM_Math_Min

extern void copy_mat_int(int **mat_in, int srow_in, int scol_in, int erow_in, int ecol_in, int **mat_out, int srow_out, int scol_out);
extern void vinv_copy_mat_int(int **mat_in, int **mat_out, int n_row, int n_col_in, int n_col_out);

#ifdef __cplusplus
}
#endif

#endif /*__PIMMATH_H__*/
