/*
 * Copyright IM Lab,(http://imlab.postech.ac.kr)
 * 
 * All rights reserved.
 * 
 */

#ifndef __MATRIX_H__
#define __MATRIX_H__
/********************************************************************/
/* Header File of Matrix-related Functions							*/
/********************************************************************/
/* Editors:
	1. Chunghoon Kim, April 17, 2007								*/
//  2. modified by CB	May 16, 2008	
/********************************************************************/

#include "PIMTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @brief 
 *
 *  @param n [IN] :  
 *
 *  @return 
 */
extern PIM_Int32 *int_vector(PIM_Int32 n);

/**
 *  @brief 
 *
 *  @param n [IN] :  
 *  @param m [IN] :  
 *
 *  @return 
 */
extern PIM_Int32 **int_matrix(PIM_Int32 n, PIM_Int32 m);

/**
 *  @brief 
 *
 *  @param v [IN/OUT] :  
 */
extern void free_int_vector(PIM_Int32 *v);//,PIM_Int32 n);

/**
 *  @brief 
 *
 *  @param mat [IN/OUT] :  
 */
extern void free_int_matrix(PIM_Int32 **mat);

/**
 *  @brief 
 *
 *  @param n [IN] :  
 *
 *  @return 
 */
extern PIM_Int32 *int_vector_start_index(PIM_Int32 n);

/**
 *  @brief 
 *
 *  @param n [IN] :  
 *  @param m [IN] :  
 *
 *  @return 
 */
extern PIM_Int32 **int_matrix_start_index(PIM_Int32 n, PIM_Int32 m);

/**
 *  @brief 
 *
 *  @param v [IN/OUT] :  
 */
extern void free_int_vector_start_index(PIM_Int32 *v);//,PIM_Int32 n);

/**
 *  @brief 
 *
 *  @param mat [IN/OUT] :  
 */
extern void free_int_matrix_start_index(PIM_Int32 **mat);

/**
 *  @brief 
 *
 *  @param mat [IN/OUT] :  
 */
extern void free_uc_matrix(PIM_Uint8 **mat );// by cheon

/**
 *  @brief 
 *
 *  @param mat [IN/OUT] :  
 */
extern void free_uc_matrix_D3(PIM_Uint8 ***ten );// by cheon

/**
 *  @brief 
 *
 *  @param l [IN] :  
 *  @param n [IN] :  
 *  @param m [IN] :  
 *
 *  @return 
 */
extern PIM_Uint8 ***uc_matrix_D3(PIM_Int32 l, PIM_Int32 n, PIM_Int32 m);		// by cheon

/**
 *  @brief 
 *
 *  @param n [IN] :  
 *  @param m [IN] :  
 *
 *  @return 
 */
extern PIM_Uint8  **uc_matrix(PIM_Int32 n, PIM_Int32 m);// by cheon


#ifdef __cplusplus
}
#endif


#endif // __MATRIX_H__

