#pragma once
#include <Windows.h>
#include <stdio.h>

enum FD_PARAM_ROTATION {
	ROTATE_TOP_UP = 0, ROTATE_LEFT_UP = 1, ROTATE_RIGHT_UP = 2, ROTATE_BOTTOM_UP = 3, ROTATE_FULL_SEARCH = 4, ROTATE_180_SEARCH = 5
};

enum FD_PARAM_POSE {
	POSE_LEFT_M30 = 1, POSE_HLEFT_M30 = 2, POSE_FRONT_M30 = 3, POSE_HRIGHT_M30 = 4, POSE_RIGHT_M30 = 5,
	POSE_LEFT_0 = 6, POSE_HLEFT_0 = 7, POSE_FRONT_0 = 8, POSE_HRIGHT_0 = 9, POSE_RIGHT_0 = 10,
	POSE_LEFT_P30 = 11, POSE_HLEFT_P30 = 12, POSE_FRONT_P30 = 13, POSE_HRIGHT_P30 = 14, POSE_RIGHT_P30 = 15
};

enum FD_PARAM_SPEED {
	SPD_VERY_ACCURATE=1, SPD_ACCURATE=2, SPD_MODERATE=3, SPD_FAST=4, SPD_FASTEST=5
};

class CMVFD
{
public:
	CMVFD(char *fd_dll_path = "MVFD.dll");
	~CMVFD();

	///////////////////////////////////////////////// 얼굴 검출 함수 //////////////////////////////////////////

	//이미지에서 검출 수행
	int Detect_Image(unsigned char *img, int width, int height);

	//이미지에서 thresholding 하지 않고 모든 얼굴을 검출한다. (성능 평가용)
	int Detect_Image_AllFaces(unsigned char *img, int width, int height);

	//비디오에서 검출 수행(partial search 적용)
	int Detect_Video(unsigned char *img, int width, int height);

	/////////////////////////////////////// 검출된 얼굴 정보를 얻어오는 함수 //////////////////////////////////

	//검출된 얼굴의 사각형을 얻어옴
	bool GetFaceRect(int idx, RECT &rect);

	//검출된 얼굴의 각도를 얻어옴
	bool GetFaceAngle(int idx, float &rip_angle, float &rop_angle);

	//검출된 얼굴의 겹치는 개수를 얻어옴
	bool GetFaceOverlapNum(int idx, int &overlap_num);

	//검출된 얼굴의 사각형을 얻어옴
	bool GetFaceScore(int idx, float &score);


	/////////////////////////////////////////////// 검출 환경 설정 함수 ////////////////////////////////////////

	//검출을 수행할 ROI를 지정
	void SetROI(int left, int top, int right, int bottom);

	//ROI를 해제
	void ClearROI(int left, int top, int right, int bottom);

	//스레드 개수를 지정
	void SetNumThreads(int num_of_threads);

	//검출성능 vs. 속도를 설정
	void SetSpeed(FD_PARAM_SPEED speed);

	//Precision을 설정 (0~1.0), precision을 높일수록 검출율은 떨어지나 FA가 덜 발생함
	void SetPrecision(float precision);
	
	//이미지 크기 대비 어느정도 크기 이상의 잘린얼굴을 검출할지 설정한다. 
	//ex) 0.3 : 이미지 크기의 1/3이상 되는 얼굴은 잘린얼굴 검출을 수행한다. (잘린 얼굴 검출을 수행하면 속도가 좀 느려짐)
	void SetMinCropFaceRatio(float precision);

	//검출될 얼굴의 최소 크기를 지정 (크기가 작은 얼굴을 검출할수록 속도가 느려짐)
	void SetMinFaceSz(int face_size);

	//검출할 얼굴의 최대 크기를 지정
	void SetMaxFaceSz(int face_size);

	//입력 이미지의 가로 크기를 기준으로 검출될 얼굴의 최소 크기를 비율로 지정
	void SetMinFaceRatio(float ratio);

	//입력 이미지의 가로 크기를 기준으로 검출될 얼굴의 최대 크기를 비율로 지정
	void SetMaxFaceRatio(float ratio);

	//입력 이미지를 어느방향으로 돌려서 얼굴 검출을 수행할지 설정
	void SetRotation(FD_PARAM_ROTATION rotation);

	//주어진 개수 이상 겹치는 얼굴만 검출
	void SetMinOverlapNum(int overlap_num);

	//포즈별 중요도를 설정 (기본값 1.0, 중요도가 높을수록 다른 포즈에 비해 검출율이 높아짐)
	void SetPoseWeight(FD_PARAM_POSE pose, float weight);

	//특정 포즈에 대한 검출을 활성화함. 초기에는 모두 활성화 되어 있음
	bool EnablePose(FD_PARAM_POSE pose);

	//특정 포즈에 대한 검출을 비활성화함.
	bool DisablePose(FD_PARAM_POSE pose);

private:
	static HMODULE m_hModule;
	static int m_instance_num;
	void *m_fd_instance;
	RECT m_ROI;
	int m_MinFaceSz, m_MaxFaceSz;
	FD_PARAM_ROTATION m_Rotation;
};
