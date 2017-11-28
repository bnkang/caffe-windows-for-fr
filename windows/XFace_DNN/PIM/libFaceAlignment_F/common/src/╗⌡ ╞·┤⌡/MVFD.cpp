#include "MVFD.h"

#include <Windows.h>
#include <stdio.h>
typedef void(*FUNC_TYPE_FD_Init) ();
typedef void *(*FUNC_TYPE_FD_Create_Instance)();
typedef void(*FUNC_TYPE_FD_Delete_Instance)(void *);
typedef void(*FUNC_TYPE_FD_Set_Num_Threads)(void *, int);
typedef void(*FUNC_TYPE_FD_Set_Speed)(void *, FD_PARAM_SPEED);
typedef void(*FUNC_TYPE_FD_Set_MinOverlapNum)(void *, int);
typedef void(*FUNC_TYPE_FD_Set_PoseWeight)(void *, int, float);
typedef void(*FUNC_TYPE_FD_Set_Precision)(void *, float);
typedef void(*FUNC_TYPE_FD_Set_MinCropFaceRatio)(void *, float);

typedef void(*FUNC_TYPE_FD_Set_MinFaceRatio)(void *, float);
typedef void(*FUNC_TYPE_FD_Set_MaxFaceRatio)(void *, float);
typedef void(*FUNC_TYPE_FD_Set_MinFaceSize)(void *, int);
typedef void(*FUNC_TYPE_FD_Set_MaxFaceSize)(void *, int);


typedef void(*FUNC_TYPE_FD_Release)();
typedef int(*FUNC_TYPE_FD_Image)(void *, unsigned char *, int, int, RECT &, FD_PARAM_ROTATION);
typedef int(*FUNC_TYPE_FD_Video)(void *, unsigned char *, int, int, RECT &, FD_PARAM_ROTATION);
typedef int(*FUNC_TYPE_FD_Image_AllFaces)(void *, unsigned char *, int, int);

typedef bool(*FUNC_TYPE_FD_Get_FaceRect)(void *, int, RECT &);
typedef bool(*FUNC_TYPE_FD_Get_FaceAngle)(void *, int, float &, float &);
typedef bool(*FUNC_TYPE_FD_Get_FaceScore)(void *, int, float &);
typedef bool(*FUNC_TYPE_FD_Get_FaceOverlapNum)(void *, int, int &);
typedef bool(*FUNC_TYPE_FD_Enable_Pose)(void *, FD_PARAM_POSE);
typedef bool(*FUNC_TYPE_FD_Disable_Pose)(void *, FD_PARAM_POSE);

HMODULE CMVFD::m_hModule = NULL;
int CMVFD::m_instance_num = 0;

CMVFD::CMVFD(char *fd_dll_path)
{
	if (m_instance_num == 0)
	{
		m_hModule = ::LoadLibrary(fd_dll_path);
		if (!m_hModule)
		{
			printf("Failed to load [%s].\n", fd_dll_path);
			exit(0);
		}
		FUNC_TYPE_FD_Init fd_init = (FUNC_TYPE_FD_Init)GetProcAddress(m_hModule, "FD_Init");
		fd_init();
	}
	FUNC_TYPE_FD_Create_Instance fd_create_instance = (FUNC_TYPE_FD_Create_Instance)GetProcAddress(m_hModule, "FD_Create_Instance");
	m_fd_instance = fd_create_instance();
	m_instance_num++;
	m_MinFaceSz = -1;
	m_MaxFaceSz = -1;
	m_Rotation = ROTATE_TOP_UP;
	m_ROI.left = 0; m_ROI.top = 0; m_ROI.right = 0; m_ROI.bottom = 0;
}

void CMVFD::SetROI(int left, int top, int right, int bottom) {
	m_ROI.left = left; m_ROI.top = top; m_ROI.right = right; m_ROI.bottom = bottom;
}

void CMVFD::ClearROI(int left, int top, int right, int bottom)
{
	m_ROI.left = 0; m_ROI.top = 0; m_ROI.right = 0; m_ROI.bottom = 0;
}

void CMVFD::SetMinFaceSz(int face_size)
{
	FUNC_TYPE_FD_Set_MinFaceSize fd_set_minface_size = (FUNC_TYPE_FD_Set_MinFaceSize)GetProcAddress(m_hModule, "FD_Set_MinFaceSize");
	fd_set_minface_size(m_fd_instance, face_size);
}
void CMVFD::SetMaxFaceSz(int face_size)
{
	FUNC_TYPE_FD_Set_MaxFaceSize fd_set_maxface_size = (FUNC_TYPE_FD_Set_MaxFaceSize)GetProcAddress(m_hModule, "FD_Set_MaxFaceSize");
	fd_set_maxface_size(m_fd_instance, face_size);
}
void CMVFD::SetMinFaceRatio(float ratio)
{
	FUNC_TYPE_FD_Set_MinFaceRatio fd_set_minface_ratio = (FUNC_TYPE_FD_Set_MinFaceRatio)GetProcAddress(m_hModule, "FD_Set_MinFaceRatio");
	fd_set_minface_ratio(m_fd_instance, ratio);
}
void CMVFD::SetMaxFaceRatio(float ratio)
{
	FUNC_TYPE_FD_Set_MaxFaceRatio fd_set_maxface_ratio = (FUNC_TYPE_FD_Set_MinFaceRatio)GetProcAddress(m_hModule, "FD_Set_MaxFaceRatio");
	fd_set_maxface_ratio(m_fd_instance, ratio);
}

void CMVFD::SetRotation(FD_PARAM_ROTATION rotation) { m_Rotation = rotation; }
int CMVFD::Detect_Image(unsigned char *img, int width, int height)
{
	RECT roi;
	if (m_ROI.left == 0 && m_ROI.top == 0 && m_ROI.right == 0 && m_ROI.bottom == 0)
	{
		roi.left = 0; roi.top = 0; roi.right = width; roi.bottom = height;
	}
	FUNC_TYPE_FD_Image fd_det_image = (FUNC_TYPE_FD_Image)GetProcAddress(m_hModule, "FD_Image");
	return fd_det_image(m_fd_instance, img, width, height, roi, m_Rotation);
}

int CMVFD::Detect_Video(unsigned char *img, int width, int height)
{
	RECT roi;
	if (m_ROI.left == 0 && m_ROI.top == 0 && m_ROI.right == 0 && m_ROI.bottom == 0)
	{
		roi.left = 0; roi.top = 0; roi.right = width; roi.bottom = height;
	}
	FUNC_TYPE_FD_Video fd_det_video = (FUNC_TYPE_FD_Video)GetProcAddress(m_hModule, "FD_Video");
	return fd_det_video(m_fd_instance, img, width, height, roi, m_Rotation);
}

int CMVFD::Detect_Image_AllFaces(unsigned char *img, int width, int height)
{
	FUNC_TYPE_FD_Image_AllFaces fd_get_face_all = (FUNC_TYPE_FD_Image_AllFaces)GetProcAddress(m_hModule, "FD_Image_AllFaces");
	return fd_get_face_all(m_fd_instance, img, width, height);
}

bool CMVFD::GetFaceRect(int idx, RECT &rect)
{
	FUNC_TYPE_FD_Get_FaceRect fd_getface_rect = (FUNC_TYPE_FD_Get_FaceRect)GetProcAddress(m_hModule, "FD_Get_FaceRect");
	return fd_getface_rect(m_fd_instance, idx, rect);
}

bool CMVFD::GetFaceAngle(int idx, float &rip_angle, float &rop_angle)
{
	FUNC_TYPE_FD_Get_FaceAngle fd_getface_angle = (FUNC_TYPE_FD_Get_FaceAngle)GetProcAddress(m_hModule, "FD_Get_FaceAngle");
	return fd_getface_angle(m_fd_instance, idx, rip_angle, rop_angle);
}

bool CMVFD::GetFaceScore(int idx, float &score)
{
	FUNC_TYPE_FD_Get_FaceScore fd_getface_score = (FUNC_TYPE_FD_Get_FaceScore)GetProcAddress(m_hModule, "FD_Get_FaceScore");
	return fd_getface_score(m_fd_instance, idx, score);
}

void CMVFD::SetMinCropFaceRatio(float ratio)
{
	FUNC_TYPE_FD_Set_MinCropFaceRatio fd_set_mincropfaceratio = (FUNC_TYPE_FD_Set_MinCropFaceRatio)GetProcAddress(m_hModule, "FD_Set_MinCroppedFaceRatio");
	fd_set_mincropfaceratio(m_fd_instance, ratio);
}

void CMVFD::SetPoseWeight(FD_PARAM_POSE pose, float weight)
{
	FUNC_TYPE_FD_Set_PoseWeight fd_set_poseweight = (FUNC_TYPE_FD_Set_PoseWeight)GetProcAddress(m_hModule, "FD_Set_PoseWeight");
	fd_set_poseweight(m_fd_instance, pose, weight);
}

void CMVFD::SetNumThreads(int num_of_threads)
{
	FUNC_TYPE_FD_Set_Num_Threads fd_set_thread_num = (FUNC_TYPE_FD_Set_Num_Threads)GetProcAddress(m_hModule, "FD_Set_Num_Threads");
	fd_set_thread_num(m_fd_instance, num_of_threads);
}
void CMVFD::SetPrecision(float precision)
{
	FUNC_TYPE_FD_Set_Precision fd_set_precision = (FUNC_TYPE_FD_Set_Precision)GetProcAddress(m_hModule, "FD_Set_Precision");
	fd_set_precision(m_fd_instance, precision);
}

void CMVFD::SetSpeed(FD_PARAM_SPEED speed)
{
	FUNC_TYPE_FD_Set_Speed fd_set_speed = (FUNC_TYPE_FD_Set_Speed)GetProcAddress(m_hModule, "FD_Set_Speed");
	fd_set_speed(m_fd_instance, speed);
}

void CMVFD::SetMinOverlapNum(int overlap_num)
{
	FUNC_TYPE_FD_Set_MinOverlapNum fd_set_min_overlap_num = (FUNC_TYPE_FD_Set_MinOverlapNum)GetProcAddress(m_hModule, "FD_Set_MinOverlap_Num");
	fd_set_min_overlap_num(m_fd_instance, overlap_num);
}

bool CMVFD::GetFaceOverlapNum(int idx, int &overlap_num)
{
	FUNC_TYPE_FD_Get_FaceOverlapNum fd_get_face_overlap_num = (FUNC_TYPE_FD_Get_FaceOverlapNum)GetProcAddress(m_hModule, "FD_Get_FaceOverlapNum");
	return fd_get_face_overlap_num(m_fd_instance, idx, overlap_num);
}

bool CMVFD::EnablePose(FD_PARAM_POSE pose)
{
	FUNC_TYPE_FD_Enable_Pose fd_enable_pose = (FUNC_TYPE_FD_Enable_Pose)GetProcAddress(m_hModule, "FD_Enable_Pose");
	return fd_enable_pose(m_fd_instance, pose);
}

bool CMVFD::DisablePose(FD_PARAM_POSE pose)
{
	FUNC_TYPE_FD_Enable_Pose fd_disable_pose = (FUNC_TYPE_FD_Enable_Pose)GetProcAddress(m_hModule, "FD_Disable_Pose");
	return fd_disable_pose(m_fd_instance, pose);
}

CMVFD::~CMVFD()
{
	FUNC_TYPE_FD_Delete_Instance fd_delete_instance = (FUNC_TYPE_FD_Delete_Instance)GetProcAddress(m_hModule, "FD_Delete_Instance");
	fd_delete_instance(m_fd_instance);
	if (m_instance_num == 1)
	{
		FreeLibrary(m_hModule);
	}
	m_instance_num--;
}
