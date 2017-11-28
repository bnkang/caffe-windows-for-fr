#include "MVFD_Defines.h"

extern int MVFDCascadeNum[FD_MAX_CLASS_NUM];
extern FD_CLASSIFIER *MVFD_Detector[FD_MAX_CLASS_NUM+1][FD_MAX_STAGE_NUM];
extern FD_CLASSIFIER *MVFD_PoseClassifier;
extern FD_CLASSIFIER *MVFD_BkgndRejector;

extern float MVFD_merged_threshold;
extern float MVFD_pose_weights[];;
extern unsigned int MVFD_enroll_thresholds[];

void MVFD_RotateRectToTopUp(FD_RECT *rect1, FD_RECT *rect2, int width, int height, int from_rip);
void MVFD_RotateTopUpRectToRIP(FD_RECT *rect1, FD_RECT *rect2, int width, int height, int to_rip);
void MVFD_RotatePoint(FD_POINT *pt, int width, int height, int rip);

void MVFD_SetPoseWeight(float weights[]);
void MVFD_SetInSensitivity(float insensitivity);