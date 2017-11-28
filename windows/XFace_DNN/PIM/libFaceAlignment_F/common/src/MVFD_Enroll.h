#include <stdio.h>
#include <stdlib.h>
#include "MVFD_Defines.h"

void MVFD_InitFaceEnrollStorage(FACE_ENROLL_STORAGE *storage);
void MVFD_FreeFaceEnrollStorage(FACE_ENROLL_STORAGE *storage);
void MVFD_ClearFaceEnrollStorage(FACE_ENROLL_STORAGE *storage);

void MVFD_EnrollFace(FACE_ENROLL_STORAGE *storage, FD_RECT rect,  int pose, int img_direction, int likelihood);
void MVFD_EnrollFace2(int x, int y, float inv_scale_rate, int resize_w, int resize_h, int roi_left, int roi_top, int pose_idx, int rotation, int confidence, FACE_ENROLL_STORAGE *storage);
int MVFD_BoxClustering(FACE_ENROLL_STORAGE *storage, int intersection_cnt, FD_INTERNAL_RESULT *final_result);

int MVFD_ClusteringAll();
int MVFD_ClusteringAll2(int pose);

void MVFD_RemoveOverlappedResults(FD_RESULT *result);

