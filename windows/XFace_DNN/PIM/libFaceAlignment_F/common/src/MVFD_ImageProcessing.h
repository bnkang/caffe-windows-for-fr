#pragma once
#include "MVFD_Defines.h"

void MVFD_Make_ResizeBI_Map_ROI(unsigned char *src_data, int src_w, int src_h, FD_RECT roi, unsigned char *dest);
void MVFD_Make_ResizeBI_Map(unsigned char *src_data, int src_w, int src_h, unsigned char *dest);
void MVFD_MT_Rotate_RIP(unsigned char *src, int src_w, int src_h, unsigned char *dst, int direction, int start_y, int end_y);
bool MVFD_MT_ResizeBI_R2R(unsigned char *src_data, int src_w, int src_h, FD_RECT src_roi, unsigned char *des_data, int des_w, int des_h, FD_RECT des_roi);
bool MVFD_MT_ResizeBI_R2R_UseMap(unsigned char *src_data, int src_w, int src_h, FD_RECT src_roi, unsigned char *des_data, int des_w, int des_h, FD_RECT des_roi);
void MVFD_MT_ResizeBI_Region_UseMap(unsigned char *src_data, int src_w, int src_h, FD_RECT roi, unsigned char *des_data, int des_w, int des_h, int start_y, int end_y);
void MVFD_MT_ResizeBI_Region(unsigned char *src_data, int src_w, int src_h, FD_RECT roi, unsigned char *des_data, int des_w, int des_h, int start_y, int end_y);
void MVFD_Image2IRMB(unsigned char *image, int width, int height, FD_CONTEXT *pContext, unsigned char *feature_buf);
void MVFD_MT_Image2MCT(unsigned char* src_data, int src_w, int src_h, unsigned short* mct_data, int start_y, int end_y);
void MVFD_ImageRotate(unsigned char* src_data, int src_w, int src_h, unsigned char* rot_data);
#ifdef USE_NEON
void MVFD_ImageTranspose(unsigned char* src_data, int src_w, int src_h, unsigned char* rot_data);
void MVFD_Resize_NN(unsigned char * __restrict src_data, int src_w, int src_h, unsigned char *__restrict des_data, int des_w, int des_h);
#endif
void MVFD_CropRegion(unsigned char* src_data, int src_w, int src_h, FD_RECT roi, unsigned char *dst_data);