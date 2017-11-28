
// XFace_DNNView.h : CXFace_DNNView 클래스의 인터페이스
//

#pragma once


#define PIM_RealtimeRecognizer_KMaxDetectableFaces			10
#define CAM_MODE

#include "PIMTypes.h"
#include "libFaceAlignment_F/alignment/src/LBFTester.h"
#include "libFaceRecognizer_V/inc/FaceRecognizer_TL_JointBayesian.h"

#include "CvImage.h"
#include "ImageProcess.h"
#include "ImageProcessing.h"
#include "Camera.h"

#include "EnrollDlg.h"
#include "ShowEnrollmentDlg.h"

#define ACC_RECOG_NFRAME 10
#define ACC_RECOG_WEIGHT_UNKNOWN 1

#define BACK_ENROLL_SIZE	30
#define FRAME_SKIP_SIZE		1


struct FaceInfoGT
{
	MyRect faceRect;
	MyPoint faceCenter;
	CString faceID;
	CString faceRecogID;
	MyPoint faceLEye;
	MyPoint faceREye;
	bool bAssigned;
	int nSimilarity;
	int nRatio;
	int nRank;
	int nMaxCount;
};


class CXFace_DNNView : public CView
{
protected: // serialization에서만 만들어집니다.
	CXFace_DNNView();
	DECLARE_DYNCREATE(CXFace_DNNView)

	// 특성입니다.
public:
	CXFace_DNNDoc* GetDocument() const;

	// 작업입니다.
public:
	//	Member functions
	void OnButtonFaceEnrollment();
	void OnButtonFaceRecognition();
	void OnButtonFaceDetection();
	void OnButtonCamearSource();
	void OnButtonCamearFormat();

	void ShowEnrollImage(CvImage& canvasImage);
	int GetRMouseSelectedEnrollFace(CPoint point);
	void ShowRecogResult(char * strCaller);
	int GetTransPosition(int nInput, int nSrc, int nDst);


	void OnFaceEnrollment(int clickX, int clickY);
	void DelegateRunImageSet();


	void Run_Detect(CvImage &srcCamImage, CvImage &drawCamImage, CvImage &canvasImage, int nRunMode);
	void Run_Who_Image(CvImage &srcCamImage, CvImage &drawCamImage, CvImage &canvasImage, int nRunMode);
	void Run_Enroll_Image(CvImage &srcCamImage, CvImage &drawCamImage, CvImage &canvasImage, int nRunMode);

	void show_data(CFMat &face_pts, CvImage &frame, SMyRect& rect);

	CString m_strVideoPath;


	// 재정의입니다.
public:
	virtual void OnDraw(CDC* pDC);  // 이 뷰를 그리기 위해 재정의되었습니다.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

	// 구현입니다.
public:
	virtual ~CXFace_DNNView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	// 생성된 메시지 맵 함수
protected:
	//{{AFX_MSG(CXFace_DNNView)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnMenuPopupDelete();
	afx_msg void OnMenuCameraSource();
	afx_msg void OnMenuCameraFormat();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnFrame();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTestDotest();
	afx_msg void OnDotestTest();
	afx_msg void OnTatestmodeImagemode();
	afx_msg void OnTatestmodeSeq();
	afx_msg void OnProcessmodeWebcam();
	afx_msg void OnUpdateProcessmodeWebcam(CCmdUI *pCmdUI);
	afx_msg void OnProcessmodeVideo();
	afx_msg void OnUpdateProcessmodeVideo(CCmdUI *pCmdUI);
	afx_msg void OnProcessmodeImageset();
	afx_msg void OnUpdateProcessmodeImageset(CCmdUI *pCmdUI);
	afx_msg void OnProcessmodeImageverification();
	afx_msg void OnUpdateProcessmodeImageverification(CCmdUI *pCmdUI);
	//	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

public:
	fr_enroll_info reduced_enroll_info;
	PIM_Int32 arCandidateIdx[5];

private:
	bool m_bInitCam;

	CString m_strVideoFileName;

	CImageProcess m_ip;
	CCamera m_camera;

	CEnrollDlg m_objEnrollDlg;
	int m_nSelectedEnrollFaceIndex;
	int m_nRunMode;
	bool m_bShowEye;
	PIM_Rect m_rcEnroll[BACK_ENROLL_SIZE];

	char m_strRecoName[100];
	char m_strEnrollName[BACK_ENROLL_SIZE][100];

	CvImage m_backImage, m_camImage, m_drawImage, m_faceImage;
	CvImage m_imgBtnFD, m_imgBtnFE, m_imgBtnFR;
	CvImage m_imgBtnNextPage, m_imgBtnPrevPage;

	//	variables for preview mode
	PIM_Int32 m_nDetectedFaces;
	PIM_Int32 m_faceInfoIndex[PIM_RealtimeRecognizer_KMaxDetectableFaces];
	//RtFaceInfo m_faceInfo[PIM_RealtimeRecognizer_KMaxDetectableFaces];

	FILE *m_pFileWriting;
	FILE *m_pFileWriting2;
	long m_nFrameNumber;
	int m_nWebCamFrmNumber;

	//	variables for video FR
	bool m_bCacheHit;
	int m_nEnrollFrame;

	bool m_bMode4Video;

	cv::VideoCapture m_video_capture;
	cv::Mat m_video_frame;

	// 	VideoDecoder *m_video_pvd;
	// 	VideoDecoderRGBFrame m_video_frame;
	// 	VideoDecoderErrorCode m_video_err;


	//	variables for GT
	FILE *m_pFile;
	int m_nTrueAcception, m_nFalseRejection, m_nFalseAcception, m_nMissed, m_nTotalTA, m_nTotalFA, m_nFalseAcception2;
	CString m_strPostfix;

	// enroll & recognition for image
	FaceInfoGT *m_GT_faceInfo;
	int m_nGTFaceCnt;

	CvImage m_SrcImage;

	CvImage m_imgBtnNext;

	int m_nFaceCount4Enroll;
	PIM_Rect *m_pFaceRects;
	PIM_Point *m_pEyePoints;

	int m_nProcessMode;

	CString m_strGTFolderName;
	CString m_strGTFolderPath;
	CString m_strGTFilePath;
	CString m_strTestImgFolderPath;
	CString m_strTestImgName;
	CString m_strTestImgGTPath;
	CString m_strImgName;

	CString m_strLogName;

	CString m_strImgExt;

	int m_nFileIdx;
	CStringArray *m_pStrImgNameSet;

	CShowEnrollmentDlg m_objShowEnrollDlg;


	CLBFTester m_FFT;
	CFaceRecognizer_TL_JointBayesian m_FR;
	int m_enrollIndex;

	boolean m_scaleCalculated;
	float m_scaleX;
	float m_scaleY;
};

#ifndef _DEBUG  // XFace_DNNView.cpp의 디버그 버전
inline CXFace_DNNDoc* CXFace_DNNView::GetDocument() const
{
	return reinterpret_cast<CXFace_DNNDoc*>(m_pDocument);
}
#endif

