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

	///////////////////////////////////////////////// �� ���� �Լ� //////////////////////////////////////////

	//�̹������� ���� ����
	int Detect_Image(unsigned char *img, int width, int height);

	//�̹������� thresholding ���� �ʰ� ��� ���� �����Ѵ�. (���� �򰡿�)
	int Detect_Image_AllFaces(unsigned char *img, int width, int height);

	//�������� ���� ����(partial search ����)
	int Detect_Video(unsigned char *img, int width, int height);

	/////////////////////////////////////// ����� �� ������ ������ �Լ� //////////////////////////////////

	//����� ���� �簢���� ����
	bool GetFaceRect(int idx, RECT &rect);

	//����� ���� ������ ����
	bool GetFaceAngle(int idx, float &rip_angle, float &rop_angle);

	//����� ���� ��ġ�� ������ ����
	bool GetFaceOverlapNum(int idx, int &overlap_num);

	//����� ���� �簢���� ����
	bool GetFaceScore(int idx, float &score);


	/////////////////////////////////////////////// ���� ȯ�� ���� �Լ� ////////////////////////////////////////

	//������ ������ ROI�� ����
	void SetROI(int left, int top, int right, int bottom);

	//ROI�� ����
	void ClearROI(int left, int top, int right, int bottom);

	//������ ������ ����
	void SetNumThreads(int num_of_threads);

	//���⼺�� vs. �ӵ��� ����
	void SetSpeed(FD_PARAM_SPEED speed);

	//Precision�� ���� (0~1.0), precision�� ���ϼ��� �������� �������� FA�� �� �߻���
	void SetPrecision(float precision);
	
	//�̹��� ũ�� ��� ������� ũ�� �̻��� �߸����� �������� �����Ѵ�. 
	//ex) 0.3 : �̹��� ũ���� 1/3�̻� �Ǵ� ���� �߸��� ������ �����Ѵ�. (�߸� �� ������ �����ϸ� �ӵ��� �� ������)
	void SetMinCropFaceRatio(float precision);

	//����� ���� �ּ� ũ�⸦ ���� (ũ�Ⱑ ���� ���� �����Ҽ��� �ӵ��� ������)
	void SetMinFaceSz(int face_size);

	//������ ���� �ִ� ũ�⸦ ����
	void SetMaxFaceSz(int face_size);

	//�Է� �̹����� ���� ũ�⸦ �������� ����� ���� �ּ� ũ�⸦ ������ ����
	void SetMinFaceRatio(float ratio);

	//�Է� �̹����� ���� ũ�⸦ �������� ����� ���� �ִ� ũ�⸦ ������ ����
	void SetMaxFaceRatio(float ratio);

	//�Է� �̹����� ����������� ������ �� ������ �������� ����
	void SetRotation(FD_PARAM_ROTATION rotation);

	//�־��� ���� �̻� ��ġ�� �󱼸� ����
	void SetMinOverlapNum(int overlap_num);

	//��� �߿䵵�� ���� (�⺻�� 1.0, �߿䵵�� �������� �ٸ� ��� ���� �������� ������)
	void SetPoseWeight(FD_PARAM_POSE pose, float weight);

	//Ư�� ��� ���� ������ Ȱ��ȭ��. �ʱ⿡�� ��� Ȱ��ȭ �Ǿ� ����
	bool EnablePose(FD_PARAM_POSE pose);

	//Ư�� ��� ���� ������ ��Ȱ��ȭ��.
	bool DisablePose(FD_PARAM_POSE pose);

private:
	static HMODULE m_hModule;
	static int m_instance_num;
	void *m_fd_instance;
	RECT m_ROI;
	int m_MinFaceSz, m_MaxFaceSz;
	FD_PARAM_ROTATION m_Rotation;
};
