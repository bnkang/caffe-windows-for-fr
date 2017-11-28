#pragma once
#include "MVFD_Init.h"
FD_CLASSIFIER *MVFD_LoadClassifier(char *fpath);
void MVFD_FreeClassifier(FD_CLASSIFIER *pClassifier);
void MVFD_Image2Feature(unsigned char *image, int width, int height, unsigned char *feature_buf);
short MVFD_ClassifyPose(FD_CLASSIFIER *pClassifier, unsigned char *feature, int src_w, int src_h, int x, int y);
void MVFD_FullScan(FD_CLASSIFIER *pClassifier, unsigned char *feature, int src_w, int src_h, unsigned short *output);
void MVFD_FullScan2(FD_CLASSIFIER *pClassifier, unsigned char *feature, int src_w, int src_h, unsigned short *output);
void MVFD_RescaleFeaturePositions(FD_CLASSIFIER *pClassifier, int src_w, int src_h);

void MVFD_Scan3Prepare();
void MVFD_RescaleScan3(int src_w, int src_h);
void MVFD_FullScan3(FD_CLASSIFIER *pClassifier, unsigned short *mct, int src_w, int src_h, unsigned short *output);
void MVFD_FullScan4(FD_CLASSIFIER *pClassifier, unsigned short *mct, int src_w, int src_h, unsigned short *output);