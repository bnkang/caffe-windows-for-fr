#pragma once
#include "MVFD_Defines.h"
void MVFD_RescaleFeaturePositions(FD_CONTEXT *pContext, FD_CLASSIFIER *pClassifier, int width, int height, int *ffpos[]);
#ifndef USE_NEON
void MVFD_GeneratePoseMap(FD_CONTEXT *pContext, FD_CLASSIFIER *pRejector, FD_CLASSIFIER *pClassifier, FEATURE *pFeature, float rejection_rate, unsigned short *output, int *ffpos[]);
#else
void MVFD_GeneratePoseMap(FD_CONTEXT *pContext, FD_CLASSIFIER *pRejector, FD_CLASSIFIER *pClassifier, FEATURE *pFeature, unsigned short *output, int *ffpos[]);
#endif

void MVFD_GeneratePoseMapForTracking(FD_CONTEXT *pContext, FD_CLASSIFIER *pRejector, FD_CLASSIFIER *pClassifier, FEATURE *pFeature, float rejection_rate, unsigned short *output, int face_x, int face_y, int *ffpos[]);
#ifdef USE_NEON	
int MVFD_IsFace(FEATURE *pFeature, int x, int y, FD_CLASSIFIER *pClassifier,unsigned int *scratchBuffer, int *ffpos[][FD_FEATURE_TYPE_NUM]);
#else
int MVFD_IsFace(FEATURE *pFeature, int x, int y, FD_CLASSIFIER *pClassifier, int *ffpos[][FD_FEATURE_TYPE_NUM]);
#endif
