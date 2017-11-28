#pragma once
#include "MVFD_Defines.h"
#include "MVFD_Global.h"

#ifndef WIN32
#include "FacePlatformInterface.h"
#endif

FEATURE *MakeFeature(unsigned char *src, int width, int height);
void ReleaseFeature(FEATURE *pFeature);

#ifndef WIN32
FD_CLASSIFIER *MVFD_LoadClassifier(PResFileIO pResFileIO, const char *fpath);
FD_CLASSIFIER *MVFD_LoadClassifier16(PResFileIO pResFileIO, const char *fpath);
#endif

FD_CLASSIFIER *MVFD_FeatureTransform(FD_CLASSIFIER *pCLS, int transform_type);
void MVFD_FreeClassifier(FD_CLASSIFIER *pClassifier);

