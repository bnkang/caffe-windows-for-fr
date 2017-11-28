#ifndef _IMAGE_PROCESS_H_
#define _IMAGE_PROCESS_H_
#include "define_op.h"

void sdic_Resize_BI(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data, int des_w, int des_h);
void sdic_Resize_NN(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data, int des_w, int des_h);
void sdic_image2LBP8_1(unsigned char* src_data, int src_w, int src_h, unsigned char* lbp_data);
void sdic_image2LBP8_1_FR(unsigned char* src_data, int src_w, int src_h, unsigned char* lbp_data);
void sdic_image2LBP8_2_FR(unsigned char* src_data, int src_w, int src_h, unsigned char* lbp_data);
void sdic_image2MCT9_1(unsigned char* src_data, int src_w, int src_h, unsigned short* lbp_data);

int  sdic_Region(unsigned char* src_data, int src_w, int src_h, unsigned char* des_data,  SMyRect rcRegion);
void sdic_FlipX(unsigned char* src_data, int src_w, int src_h, unsigned char* des_data);
void sdic_FlipY(unsigned char* src_data, int src_w, int src_h, unsigned char* des_data);

void sdic_he_small(unsigned char *src_data, int src_w, int src_h);
int  sdic_he(unsigned char *image, int w, int h, unsigned char *amhe_data);

void sdic_Rotate(unsigned char *src_data, int src_w, int src_h, int angle, unsigned char *des_data);
void sdic_InvRotate( SMyRect src, int cpx, int cpy, int angle,  SMyRect* dest);
void sdic_Rotate_Rect( SMyRect src, int cpx, int cpy, int angle,  SMyRect* dest);

/*
void sdic_Rotate_25(unsigned char *src_data, int src_w, int src_h, int angle, unsigned char *des_data);
void sdic_InvRotate_25( SMyRect src, int cpx, int cpy, int angle,  SMyRect* dest);
*/
void sdic_Rotation_L(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data);
void sdic_Rotation_R(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data);
void sdic_Rotation_180(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data);

void sdic_Position_L2Org(int nWidth, int nHeight,  SMyRect src_rect,  SMyRect *des_rect);
void sdic_Position_R2Org(int nWidth, int nHeight,  SMyRect src_rect,  SMyRect *des_rect);

void sdic_Position_L2Org2(int nWidth, int nHeight,  SMyRect src_rect,  SMyRect *des_rect);
void sdic_Position_R2Org2(int nWidth, int nHeight,  SMyRect src_rect,  SMyRect *des_rect);
void sdic_Position_Yflip(int nWidth, int nHeight, SMyRect src_rect, SMyRect *des_rect);


void sdic_Position_Org2L25( SMyPoint src_pt, int cx, int cy,  SMyPoint *des_pt);
void sdic_Position_Org2R25( SMyPoint src_pt, int cx, int cy,  SMyPoint *des_pt);

void sdic_Position_Org2L65( SMyPoint src_pt, int cx, int cy,  SMyPoint *des_pt);
void sdic_Position_Org2R65( SMyPoint src_pt, int cx, int cy,  SMyPoint *des_pt);

void sdic_Position_Org2L( SMyPoint src_pt, int cx, int cy, int nAngle, SMyPoint *des_pt);
void sdic_Position_Org2R( SMyPoint src_pt, int cx, int cy, int nAngle, SMyPoint *des_pt);

void sdic_Position_Org2Angle( SMyPoint src_pt, int cx, int cy,  SMyPoint *des_pt, int angle);

void SDIC_SetEyeToRot(long nWidth, long nHeight, FD_INFO *pFD_Info);

void sdic_Resize_Image2LBP(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data, int des_w, int des_h);

void Image2LGP_Rect(unsigned char *src_data, int src_w, int src_h, int x1, int y1, int x2, int y2, unsigned char *lgp_data);
void image2LGP8_1(unsigned char* src_data, int src_w, int src_h, unsigned char* lbp_data);

#endif 
