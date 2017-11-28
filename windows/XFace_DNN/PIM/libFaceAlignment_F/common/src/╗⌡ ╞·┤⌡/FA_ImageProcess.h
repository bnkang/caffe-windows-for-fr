#ifndef _IMAGE_PROCESS_H_
#define _IMAGE_PROCESS_H_
#include "../../../exp/PIM_Define_op.h"

void Resize_BI(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data, int des_w, int des_h);
void Resize_NN(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data, int des_w, int des_h);

void Image2LBP8_1(unsigned char* src_data, int src_w, int src_h, unsigned char* lbp_data);
void image2MCT9_1(unsigned char* src_data, int src_w, int src_h, unsigned short* mct_data);

int  Region(unsigned char* src_data, int src_w, int src_h, unsigned char* des_data, struct SMyRect rcRegion);
void FlipX(unsigned char* src_data, int src_w, int src_h, unsigned char* des_data);

void face_normalization_20_20(unsigned char* src_data, unsigned char* des_data, int src_w, int src_h, struct SMyPoint ptLEye, struct SMyPoint ptREye);

#endif 
