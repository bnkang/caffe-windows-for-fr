/*
 * Copyright IM Lab,(http://imlab.postech.ac.kr)
 * 
 * All rights reserved.
 * 
 */

#ifndef __IMAGE_PROCESS_H__
#define __IMAGE_PROCESS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "PIMTypes.h"


/**
 *  @brief 
 */
#define image2LBP8_1 preprocessByteImage


extern void Resize_BI(PIM_Uint8 *src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Uint8 *des_data, PIM_Int32 des_w, PIM_Int32 des_h);
extern void Resize_BI_BGR(PIM_Uint8 *src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Uint8 *des_data, PIM_Int32 des_w, PIM_Int32 des_h);
extern void resize_bi(PIM_Uint8 *srcData, PIM_Int32 nSrcW, PIM_Int32 nSrcH, PIM_Uint8 *desData, PIM_Int32 nDesW, PIM_Int32 nDesH);
extern void Resize_NN(PIM_Uint8 *src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Uint8 *des_data, PIM_Int32 des_w, PIM_Int32 des_h);

/**
 *  @brief 
 *
 *  @param src_data [IN] : 
 *  @param src_w [IN] : 
 *  @param src_h [IN] : 
 *  @param lbp_data [OUT] : 
 */
extern PIM_Void preprocessByteImage(PIM_Uint8* src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Uint8* lbp_data);

/**
 *  @brief 
 *
 *  @param src_data [IN] : 
 *  @param src_w [IN] : 
 *  @param src_h [IN] : 
 *  @param lbp_data [OUT] : 
 */
extern PIM_Void image2LBP8_1_Reco(PIM_Uint8* src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Uint8* lbp_data);

/**
 *  @brief 
 *
 *  @param src_data [IN] : 
 *  @param src_w [IN] : 
 *  @param src_h [IN] : 
 *  @param lbp_data [OUT] : 
 */
extern PIM_Void image2LBP8_1_FR(PIM_Uint8* src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Uint8* lbp_data);

/**
 *  @brief 
 *
 *  @param src_data [IN] : 
 *  @param src_w [IN] : 
 *  @param src_h [IN] : 
 *  @param lbp_data [OUT] : 
 */
extern PIM_Void image2LBP8_2_FR(PIM_Uint8* src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Uint8* lbp_data);

/**
 *  @brief 
 *
 *  @param src_data [IN] : 
 *  @param src_w [IN] : 
 *  @param src_h [IN] : 
 *  @param des_data [OUT] : 
 *  @param rcRegion [IN] : 
 *
 *  @return ret : 
 */
extern PIM_Bool Region(PIM_Uint8* src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Uint8* des_data, PIM_Rect rcRegion);
extern PIM_Bool Region_RGB(PIM_Uint8 *src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Uint8 *des_data, PIM_Rect rcRegion);
extern PIM_Bool Region_INT(PIM_Int32* src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Int32* des_data, PIM_Rect rcRegion);



/**
 *  @brief 
 *
 *  @param src_data [IN] : 
 *  @param src_w [IN] : 
 *  @param src_h [IN] : 
 *  @param amhe_data [OUT] : 
 */
extern PIM_Void amhe(PIM_Uint8 *src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Uint8 *amhe_data);

/**
 *  @brief 
 *
 *  @param src_data [IN] : 
 *  @param src_w [IN] : 
 *  @param src_h [IN] : 
 *  @param filt_data [OUT] : 
 */
extern PIM_Void mean_filt(PIM_Uint8* src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Uint8* filt_data);

/**
 *  @brief 
 *
 *  @param src_data [IN] : 
 *  @param src_w [IN] : 
 *  @param src_h [IN] : 
 *  @param filt_data [OUT] : 
 */
extern PIM_Void gaussian_filt(PIM_Uint8* src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Uint8* filt_data);

/**
 *  @brief 
 *
 *  @param src_data [IN] : 
 *  @param src_w [IN] : 
 *  @param src_h [IN] : 
 *  @param mct_data [OUT] : 
 */
extern PIM_Void image2MCT9_1(PIM_Uint8* src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Uint16* mct_data);

/**
 *  @brief 
 *
 *  @param src [IN] : 
 *  @param mct [IN] : 
 */
extern PIM_Void image2MCT(PIM_Bitmap *src, PIM_Uint16 *mct);

/**
 *  @brief 
 *
 *  @param src_data [IN] : 
 *  @param src_w [IN] : 
 *  @param src_h [IN] : 
 *  @param lbp_data [OUT] : 
 */
extern PIM_Void image2MCT_Final(PIM_Uint8* src_data, PIM_Int32 src_w, PIM_Int32 src_h, PIM_Uint16* lbp_data);


#ifdef __cplusplus
}
#endif

#endif /*__IMAGE_PROCESS_H__*/
