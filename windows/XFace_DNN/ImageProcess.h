#ifndef _IMAGE_PROCESS_H_
#define _IMAGE_PROCESS_H_
#include "Define_op.h"

// void Resize_BI(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data, int des_w, int des_h);
// void Resize_NN(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data, int des_w, int des_h);
// void image2LBP8_1(unsigned char* src_data, int src_w, int src_h, unsigned char* lbp_data);
// void image2LBP8_1_FR(unsigned char* src_data, int src_w, int src_h, unsigned char* lbp_data);
// void image2MCT9_1(unsigned char* src_data, int src_w, int src_h, unsigned short* lbp_data);
// void image2MCT(unsigned char *src_data, int src_w, int src_h, unsigned short *mct);
// 
// int  Region(unsigned char* src_data, int src_w, int src_h, unsigned char* des_data, struct SMyRect rcRegion);
// void FlipX(unsigned char* src_data, int src_w, int src_h, unsigned char* des_data);
// 
// void he_small(unsigned char *src_data, int src_w, int src_h);
// int  he(unsigned char *image, int w, int h, unsigned char *amhe_data);
// 
// void Rotate_25(unsigned char *src_data, int src_w, int src_h, int angle, unsigned char *des_data);
// void InvRotate_25(struct SMyRect src, int cpx, int cpy, int angle, struct SMyRect* dest);
// 
// void Rotation_L(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data);
// void Rotation_R(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data);
// 
// void Position_L2Org(int nWidth, int nHeight, struct SMyRect src_rect, struct SMyRect *des_rect);
// void Position_R2Org(int nWidth, int nHeight, struct SMyRect src_rect, struct SMyRect *des_rect);
// 
// void Position_L2Org2(int nWidth, int nHeight, struct SMyRect src_rect, struct SMyRect *des_rect);
// void Position_R2Org2(int nWidth, int nHeight, struct SMyRect src_rect, struct SMyRect *des_rect);
// 
// void Position_Org2L25(struct SMyPoint src_pt, int cx, int cy, struct SMyPoint *des_pt);
// void Position_Org2R25(struct SMyPoint src_pt, int cx, int cy, struct SMyPoint *des_pt);
// 
// void Rotate(unsigned char *src_data, int src_w, int src_h, int angle, unsigned char *des_data);
// void InvRotate(SMyRect src, int cpx, int cpy, int angle, SMyRect* dest);
void Resize(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data, int des_w, int des_h);



#endif 
