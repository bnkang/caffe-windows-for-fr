#ifndef _EYE_DETECTION_H_
#define _EYE_DETECTION_H_

#include "../../../exp/PIM_Define_op.h"

int ed_find(unsigned char* srcData, int src_w, int src_h, struct SMyRect rcFace);
int ed_whereBox(struct SMyRect* rectLEye, struct SMyRect* rectREye);
int ed_where(struct SMyPoint* ptLEye, struct SMyPoint* ptREye);	

#endif 
