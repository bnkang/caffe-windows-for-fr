
// XFace_DNN.h : XFace_DNN ���� ���α׷��� ���� �� ��� ����
//
#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"       // �� ��ȣ�Դϴ�.


// CXFace_DNNApp:
// �� Ŭ������ ������ ���ؼ��� XFace_DNN.cpp�� �����Ͻʽÿ�.
//

class CXFace_DNNApp : public CWinApp
{
public:
	CXFace_DNNApp();


// �������Դϴ�.
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// �����Դϴ�.
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CXFace_DNNApp theApp;
