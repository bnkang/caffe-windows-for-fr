#ifndef _EYE_DETECTION_H_
#define _EYE_DETECTION_H_

#include "Define_op.h"

long SDIC_EyeDetection(unsigned char* pData, long nWidth, long nHeight, FD_INFO *pFD_Info);
long SDIC_EyeDetection2(unsigned char* pData, long nDataWidth, long nDataHeight, 
					   long nPreviewWidth, long nPreviewHeight, FD_INFO *pFD_Info);

long SDIC_EyeDetection_IMAGE(unsigned char* pData, unsigned char* nRotateData_25_0, unsigned char* nRotateData_25_1, unsigned char* nRotateData_90_0, unsigned char* nRotateData_90_1, 
						long nWidth, long nHeight, FD_INFO *pFD_Info);

void SDIC_FaceAngle(unsigned char*pData, long nWidth, long nHeight, FD_INFO *pFD_Info);

#endif 
