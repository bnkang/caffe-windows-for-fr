#ifndef _IMAGE_PROCESS_H_
#define _IMAGE_PROCESS_H_

#include "define_op.h"

void Image2LBP8_1(unsigned char* src_data, int src_w, int src_h, unsigned char* lbp_data);

void Resize_BI(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data, int des_w, int des_h);
void Resize_NN(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data, int des_w, int des_h);

void image2MCT9_1(unsigned char* src_data, int src_w, int src_h, unsigned short* lbp_data);
void image2MCT_FD(unsigned char *src_data, int src_w, int src_h, unsigned short *mct);
 
int  Region(unsigned char* src_data, int src_w, int src_h, unsigned char* des_data, SMyRect rcRegion);
int  Region_int(int* src_data, int src_w, int src_h, int* des_data, SMyRect rcRegion);
int Region_double(double* src_data, int src_w, int src_h, double* des_data, SMyRect rcRegion);
int Region_float(float* src_data, int src_w, int src_h, float* des_data, SMyRect rcRegion);
void FlipX(unsigned char* src_data, int src_w, int src_h, unsigned char* des_data);

void rgb2gray(unsigned char *rgb, int w, int h, unsigned char *gray);

void Rotate(unsigned char *src_data, int src_w, int src_h, int angle, unsigned char *des_data);
void InvRotate(SMyRect src, int cpx, int cpy, int angle, SMyRect* dest);

void illuminationCompensation(unsigned char* src_data, int src_w, int src_h);
void flitMean(unsigned char* src_data, int src_w, int src_h);

void face_normalization_50_40(unsigned char* src_data, unsigned char* des_data, int src_w, int src_h, struct SMyPoint ptLEye, struct SMyPoint ptREye);
void face_normalization_20_20(unsigned char* src_data, unsigned char* des_data, int src_w, int src_h, struct SMyPoint ptLEye, struct SMyPoint ptREye);
bool face_normalization_48_40(const unsigned char* src_data, const int src_w, const int src_h, const SMyPoint ptSrcLEye, const SMyPoint ptSrcREye, unsigned char* des_data);
bool face_normalization_52_44(const unsigned char* src_data, const int src_w, const int src_h, const struct SMyPoint ptSrcLEye, const struct SMyPoint ptSrcREye, unsigned char* des_data);
bool face_normalization_52_44_bilinear(const unsigned char* src_data, const int src_w, const int src_h, const struct SMyPoint ptSrcLEye, const struct SMyPoint ptSrcREye, unsigned char* des_data);
bool face_normalization_52_44_2(const unsigned char* src_data, const int src_w, const int src_h, const struct SMyPoint ptSrcLEye, const struct SMyPoint ptSrcREye, unsigned char* des_data);
bool face_normalization_52_44_2_bilinear(const unsigned char* src_data, const int src_w, const int src_h, const struct SMyPoint ptSrcLEye, const struct SMyPoint ptSrcREye, unsigned char* des_data);

void integral_image(unsigned char *src, int width, int height, int *dst);
void square_integral_image(unsigned char *src, int width, int height, int *dst);
#endif 
