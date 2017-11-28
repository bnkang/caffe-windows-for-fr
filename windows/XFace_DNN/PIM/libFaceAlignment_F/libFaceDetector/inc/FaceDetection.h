#ifndef _FACE_DETECTION_H_
#define _FACE_DETECTION_H_

#include "../../../exp/PIM_Define_op.h"

int fd_find(unsigned char* srcData, int nSrcW, int nSrcH, bool bROP, bool fROP, bool rRIP);
bool fd_where(struct SMyRect* rcFace, int* nFaceROP, int* nFaceIntersectionCount, int* nFaceConfidence);	

#endif 
