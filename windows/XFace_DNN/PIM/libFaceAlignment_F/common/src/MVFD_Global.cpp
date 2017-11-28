#include "stdafx.h"

#include "MVFD_Defines.h"

int MVFDCascadeNum[FD_MAX_CLASS_NUM];

FD_CLASSIFIER *MVFD_Detector[FD_MAX_CLASS_NUM+1][FD_MAX_STAGE_NUM] = {0};	//face detector corresponding each pose
FD_CLASSIFIER *MVFD_PoseClassifier = NULL;	//pose classifier
FD_CLASSIFIER *MVFD_BkgndRejector = NULL;	//rejector
float MVFD_merged_threshold = MVFD_DEFAULT_THRESHOLD;
float MVFD_pose_weights[FD_POSE_NUM] = {0.45f, 0.9f, 0.45f, 0.8f, 0.8f, 0.45f, 0.9f, 0.45f, 1.0f};
unsigned int MVFD_enroll_thresholds[FD_POSE_NUM] = { 1500,	//Left -30
													 1500,	//Front -30
													 1500,	//Right -30
													 1500,	//Left 0
													 1500,	//Right 0
													 1500,	//Left 30
													 1500,	//Front 30
													 1500,	//Right 30
													 1500,	//Front 0
													};

/*
unsigned int MVFD_enroll_thresholds[FD_POSE_NUM] = { 2500,	//Left -30
													 1400,	//Front -30
													 2500,	//Right -30
													 2500,	//Left 0
													 2500,	//Right 0
													 2500,	//Left 30
													 1400,	//Front 30
													 2500,	//Right 30
													 1400,	//Front 0
													};
*/

void MVFD_SetPoseWeight(float weights[])
{
	memcpy(MVFD_pose_weights, weights, FD_POSE_NUM*sizeof(float));
}

void MVFD_SetInSensitivity(float insensitivity)
{
	if(insensitivity >= 1.0f)
	{
		insensitivity = FD_MIN(insensitivity, 2.0f);
		MVFD_merged_threshold = MVFD_DEFAULT_THRESHOLD + (insensitivity-1.0)*(MVFD_UPPERBOUND_THRESHOLD-MVFD_DEFAULT_THRESHOLD);
	}
	else
	{
		insensitivity = FD_MAX(insensitivity, 0.0f);
		MVFD_merged_threshold = MVFD_LOWERBOUND_THRESHOLD + insensitivity*(MVFD_DEFAULT_THRESHOLD-MVFD_LOWERBOUND_THRESHOLD);
	}
}

void MVFD_RotateRectToTopUp(FD_RECT *rect1, FD_RECT *rect2, int width, int height, int from_rip)
{
	switch(from_rip)
	{
		case ROTATE_TOP_UP:
			*rect2 = *rect1;
			break;
		case ROTATE_LEFT_UP:
			rect2->right = rect1->bottom;
			rect2->bottom = height-rect1->left;		
			rect2->left = rect1->top;
			rect2->top = height-rect1->right;
			break;
		case ROTATE_RIGHT_UP:
			rect2->left = width-rect1->bottom;
			rect2->top = rect1->left;
			rect2->right = width-rect1->top;
			rect2->bottom = rect1->right;
			break;
		case ROTATE_BOTTOM_UP:
			rect2->left = rect1->left;
			rect2->top = height-rect1->bottom;
			rect2->right = rect1->right;
			rect2->bottom = height-rect1->top;
			break;
		default:
			FDASSERT(false);
	}

}

void MVFD_RotateTopUpRectToRIP(FD_RECT *rect1, FD_RECT *rect2, int width, int height, int to_rip)
{
	switch(to_rip)
	{
		case ROTATE_TOP_UP:
			*rect2 = *rect1;
			break;
		case ROTATE_LEFT_UP:
			rect2->bottom = rect1->right;
			rect2->left = height-rect1->bottom;
			rect2->top = rect1->left;
			rect2->right = height-rect1->top;
			break;
		case ROTATE_RIGHT_UP:
			rect2->left = rect1->top;
			rect2->top = width-rect1->right;
			rect2->right = rect1->bottom;
			rect2->bottom = width-rect1->left;
			break;
		case ROTATE_BOTTOM_UP:
			rect2->left = rect1->left;
			rect2->top = height-rect1->bottom;
			rect2->right = rect1->right;
			rect2->bottom = height-rect1->top;
			break;
		default:
			FDASSERT(false);
	}

}

void MVFD_RotatePoint(FD_POINT *pt, int width, int height, int rip)
{
	FD_POINT new_pt;
	switch(rip)
	{
	case ROTATE_TOP_UP:
		new_pt = *pt;
		break;
	case ROTATE_BOTTOM_UP:
		new_pt.x = pt->x;
		new_pt.y = height-pt->y;
		break;
	case ROTATE_LEFT_UP:
		new_pt.x = pt->y;
		new_pt.y = width-pt->x;
		break;
	case ROTATE_RIGHT_UP:
		new_pt.x = height-pt->y;
		new_pt.y = pt->x;
		break;
	}
	*pt = new_pt;
}
