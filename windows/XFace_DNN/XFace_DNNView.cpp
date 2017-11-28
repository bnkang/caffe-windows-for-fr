
// XFace_DNNView.cpp : CXFace_DNNView 클래스의 구현
//

#include "stdafx.h"
// SHARED_HANDLERS는 미리 보기, 축소판 그림 및 검색 필터 처리기를 구현하는 ATL 프로젝트에서 정의할 수 있으며
// 해당 프로젝트와 문서 코드를 공유하도록 해 줍니다.
#ifndef SHARED_HANDLERS
#include "XFace_DNN.h"
#endif

#include "XFace_DNNDoc.h"
#include "XFace_DNNView.h"

#include "exp/PIM_Define_op.h"
#include "exp/PIMTypes.h"
#include "libPlatform/inc/PIMBitmap.h"

#include "libFaceRecognizer_V/inc/FR_Params.h"
#include "libFaceAlignment_F/alignment/src/LBFTester.h"

#include "opencv2/opencv.hpp"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//#define DEF_COMMANDLINE_MODE	/* 특정 폴더안의 이미지를 테스트 할 경우 */
//#define DEF_AUTO_ENROLLMENT	/* 특정 폴더안의 등록 이미지를 자동 등록을 할 경우 */
//#define _WRITE_LOG_		

#define PATH_BACKGROUND_IMAGE	"./data/back_3.JPG"
#define PATH_FD_BUTTON			"./data/btn_facedetection_selected.bmp"
#define PATH_FE_BUTTON			"./data/btn_faceEnrollment_selected.bmp"
#define PATH_FR_BUTTON			"./data/btn_faceRecognition_selected.bmp"

#define PATH_NEXT_PAGE_BUTTON	"./data/btn_next_page_enabled.png"
#define PATH_PREV_PAGE_BUTTON	"./data/btn_prev_page_enabled.png"

#define PATH_NEXT_BUTTON		"./data/btn_nextFileRead.png"

//메뉴부
#define BACK_BUTTON_TOP				16
#define BACK_BUTTON_BOTTOM			64	
#define BACK_BUTTON_FD_LEFT			31
#define BACK_BUTTON_FD_RIGHT		172
#define BACK_BUTTON_EN_LEFT			207
#define BACK_BUTTON_EN_RIGHT		347	
#define BACK_BUTTON_FR_LEFT			382
#define BACK_BUTTON_FR_RIGHT		524

//카메라부
#define DEF_CAMERA_WIDTH			640//320
#define DEF_CAMERA_HEIGHT			480//240

#define BACK_CAMERA_X				(15)
#define BACK_CAMERA_Y				(90)

//등록부
#define BACK_ENROLL_FACE_X			(680)
#define BACK_ENROLL_FACE_Y			(7)

#define BACK_RECO_FACE_X			143
#define BACK_RECO_FACE_Y			610
#define BACK_RECO_FACE_WIDTH		80
#define BACK_RECO_FACE_HEIGHT		80

// Next Button
#define BACK_BUTTON_FR_NEXT_LEFT	16
#define BACK_BUTTON_FR_NEXT_RIGHT   216
#define BACK_BUTTON_FR_NEXT_TOP		450
#define BACK_BUTTON_FR_NEXT_BOTTOM	575

// enrolled gallery prev, next button
#define BACK_BUTTON_ENROLL_PREV_LEFT	843//910//1180//910
#define BACK_BUTTON_ENROLL_PREV_TOP		585//30//560//30//581
#define BACK_BUTTON_ENROLL_PREV_RIGHT	(BACK_BUTTON_ENROLL_PREV_LEFT+30)
#define BACK_BUTTON_ENROLL_PREV_BOTTOM	(BACK_BUTTON_ENROLL_PREV_TOP+38)
#define BACK_BUTTON_ENROLL_NEXT_LEFT	1106//959//1200//959
#define BACK_BUTTON_ENROLL_NEXT_TOP		585//30//560//30//581
#define BACK_BUTTON_ENROLL_NEXT_RIGHT	(BACK_BUTTON_ENROLL_NEXT_LEFT+30)
#define BACK_BUTTON_ENROLL_NEXT_BOTTOM	(BACK_BUTTON_ENROLL_NEXT_TOP+38)



// CXFace_DNNView

IMPLEMENT_DYNCREATE(CXFace_DNNView, CView)

BEGIN_MESSAGE_MAP(CXFace_DNNView, CView)
	// 표준 인쇄 명령입니다.
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_LBUTTONDOWN()
	ON_WM_DESTROY()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_MENU_POPUP_DELETE, OnMenuPopupDelete)
	ON_COMMAND(ID_MENU_CAMERA_SOURCE, OnMenuCameraSource)
	ON_COMMAND(ID_MENU_CAMERA_FORMAT, OnMenuCameraFormat)
	ON_WM_TIMER()
	ON_WM_DROPFILES()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_TEST_DOTEST, &CXFace_DNNView::OnTestDotest)
	ON_COMMAND(ID_DOTEST_TEST, &CXFace_DNNView::OnDotestTest)
	ON_COMMAND(ID_TATESTMODE_IMAGEMODE, &CXFace_DNNView::OnTatestmodeImagemode)
	ON_COMMAND(ID_TATESTMODE_SEQ, &CXFace_DNNView::OnTatestmodeSeq)
	ON_COMMAND(ID_PROCESSMODE_WEBCAM, &CXFace_DNNView::OnProcessmodeWebcam)
	ON_UPDATE_COMMAND_UI(ID_PROCESSMODE_WEBCAM, &CXFace_DNNView::OnUpdateProcessmodeWebcam)
	ON_COMMAND(ID_PROCESSMODE_VIDEO, &CXFace_DNNView::OnProcessmodeVideo)
	ON_UPDATE_COMMAND_UI(ID_PROCESSMODE_VIDEO, &CXFace_DNNView::OnUpdateProcessmodeVideo)
	ON_COMMAND(ID_PROCESSMODE_IMAGESET, &CXFace_DNNView::OnProcessmodeImageset)
	ON_UPDATE_COMMAND_UI(ID_PROCESSMODE_IMAGESET, &CXFace_DNNView::OnUpdateProcessmodeImageset)
END_MESSAGE_MAP()


int PIM_DBG_SetBitmapFromIplimage(PIM_Bitmap *bitmap, IplImage *srcipl);
int PIM_DBG_SetBitmapFromIplimageGray(PIM_Bitmap* bitmap, IplImage* srcipl);
int PIM_DBG_SetBitmapFromIplimageGrayExt(PIM_Bitmap* bitmap, IplImage* srcipl);

int PIM_DBG_SetBitmapFromIplimage(PIM_Bitmap *bitmap, IplImage *srcipl)
{
	int i;

	bitmap->width = srcipl->width / 4 * 4;
	bitmap->height = srcipl->height;
	bitmap->imageFormat = PIM_IMAGE_FORMAT_BGR888;
	bitmap->imageData = (PIM_Uint8 *)malloc(bitmap->width * bitmap->height * srcipl->nChannels);
	memset(bitmap->imageData, 0, bitmap->width * bitmap->height * srcipl->nChannels);
	bitmap->orientation = (PIM_ImageOrientation)0;
	bitmap->auxImageData = NULL;
	bitmap->reference = 0;

	for( i = 0; i < bitmap->height; i++ )
	{
		memcpy(&bitmap->imageData[bitmap->width*i * 3], &srcipl->imageData[srcipl->widthStep*i], bitmap->width * 3);
	}

	return 0;
}

int PIM_DBG_SetBitmapFromIplimageGray(PIM_Bitmap* bitmap, IplImage* srcipl)
{
	int i;

	bitmap->width = srcipl->width / 4 * 4;
	bitmap->height = srcipl->height;
	bitmap->imageFormat = PIM_IMAGE_FORMAT_YUVPLANAR_NV21;
	bitmap->imageData = /*(PIM_Uint8 *)(srcipl->imageData);*/(PIM_Uint8 *)malloc(bitmap->width * bitmap->height * 1);
	bitmap->orientation = (PIM_ImageOrientation)0;
	bitmap->auxImageData = NULL;
	bitmap->reference = 0;

	for( i = 0; i < bitmap->height; i++ )
	{
		memcpy(&bitmap->imageData[bitmap->width*i * 1], &srcipl->imageData[srcipl->widthStep*i], bitmap->width * 1);
	}

	return 0;
}

int PIM_DBG_SetBitmapFromIplimageGrayExt(PIM_Bitmap* bitmap, IplImage* srcipl)
{
	int i;

	bitmap->width = (srcipl->width * 2) / 4 * 4;
	bitmap->height = srcipl->height * 2;
	bitmap->imageFormat = PIM_IMAGE_FORMAT_YUVPLANAR_NV21;
	bitmap->imageData = /*(PIM_Uint8 *)(srcipl->imageData);*/(PIM_Uint8 *)malloc(bitmap->width * 2 * bitmap->height * 2 * 1);
	memset(bitmap->imageData, 0, sizeof(PIM_Uint8)*bitmap->width * bitmap->height);
	bitmap->orientation = (PIM_ImageOrientation)0;
	bitmap->auxImageData = NULL;
	bitmap->reference = 0;

	for( i = 0; i < srcipl->height; i++ )
	{
		memcpy(&bitmap->imageData[bitmap->width*(i + 124) * 1 + 124], &srcipl->imageData[srcipl->widthStep*i], srcipl->width);
	}

	return 0;
}


// CXFace_DNNView 생성/소멸

CXFace_DNNView::CXFace_DNNView()
{
	// TODO: 여기에 생성 코드를 추가합니다.
	m_bInitCam = true;
	m_bMode4Video = false;

	m_GT_faceInfo = NULL;
	m_pFaceRects = NULL;
	m_pEyePoints = NULL;

	m_pFile = NULL;

	m_strGTFolderName = "";
	m_strGTFolderPath = "";
	m_strGTFilePath = "";
	m_strTestImgFolderPath = "";
	m_strTestImgName = "";
	m_strTestImgGTPath = "";
	m_strImgExt = "";

	m_nFileIdx = 0;
	m_pStrImgNameSet = NULL;

	m_nRunMode = 0;
	m_nSelectedEnrollFaceIndex = -1;
	m_bShowEye = true;

	m_enrollIndex = 0;

	m_scaleCalculated = false;

	for( int i = 0; i < BACK_ENROLL_SIZE; i++ )
	{
		sprintf_s(m_strEnrollName[i], "");
	}
}

CXFace_DNNView::~CXFace_DNNView()
{
	int debug_p = 0;

	if( m_GT_faceInfo != NULL )
	{
		delete[] m_GT_faceInfo;
		m_GT_faceInfo = NULL;
	}

	if( m_pFaceRects != NULL )
	{
		delete[] m_pFaceRects;
		m_pFaceRects = NULL;
	}

	if( m_pEyePoints != NULL )
	{
		delete[] m_pEyePoints;
		m_pEyePoints = NULL;
	}

	if( m_pStrImgNameSet != NULL )
	{
		delete m_pStrImgNameSet;
		m_pStrImgNameSet = NULL;
	}
}

void CXFace_DNNView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	char strPath[MAX_PATH];
	sprintf_s(strPath, PATH_BACKGROUND_IMAGE);

	if( m_ip.load(strPath, m_backImage) == false )
	{
		AfxMessageBox("fail to read background image(./data/back_3.JPG).");
		exit(-1);
	}

	sprintf_s(strPath, PATH_FD_BUTTON);
	if( m_ip.load(strPath, m_imgBtnFD) == false )
	{
		AfxMessageBox("fail to read background image(./data/btn_facedetection_selected.bmp).");
		exit(-1);
	}

	sprintf_s(strPath, PATH_FE_BUTTON);
	if( m_ip.load(strPath, m_imgBtnFE) == false )
	{
		AfxMessageBox("fail to read background image(./data/btn_faceEnrollment_selected.bmp).");
		exit(-1);
	}

	sprintf_s(strPath, PATH_FR_BUTTON);
	if( m_ip.load(strPath, m_imgBtnFR) == false )
	{
		AfxMessageBox("fail to read background image(./data/btn_faceRecognition_selected.bmp).");
		exit(-1);
	}

	sprintf_s(strPath, PATH_NEXT_BUTTON);
	if( m_ip.load(strPath, m_imgBtnNext) == false )
	{
		AfxMessageBox("fail to read background image(./data/btn_nextFileRead.png).");
		exit(-1);
	}

	sprintf_s(strPath, PATH_PREV_PAGE_BUTTON);
	if( m_ip.load(strPath, m_imgBtnPrevPage) == false )
	{
		AfxMessageBox("fail to read prev page button image(./data/prev_page_enabled.png).");
		exit(-1);
	}

	sprintf_s(strPath, PATH_NEXT_PAGE_BUTTON);
	if( m_ip.load(strPath, m_imgBtnNextPage) == false )
	{
		AfxMessageBox("fail to read next page button image(./data/next_page_enabled.png).");
		exit(-1);
	}

	int x, y, k = 0;
	int left, top;

	for( y = 0; y < 5; y++ )
	{
		top = BACK_ENROLL_FACE_Y + (BACK_RECO_FACE_HEIGHT + 35)*y;
		for( x = 0; x < 6; x++ )
		{
			left = BACK_ENROLL_FACE_X + (BACK_RECO_FACE_WIDTH + 10)*x;
			m_rcEnroll[k].left = left;
			m_rcEnroll[k].top = top;
			m_rcEnroll[k].right = left + BACK_RECO_FACE_WIDTH + 4;
			m_rcEnroll[k].bottom = top + BACK_RECO_FACE_HEIGHT + 4;
			k++;
		}
	}
	for( k = 0; k < BACK_ENROLL_SIZE; k++ )
	{
		m_ip.draw_rect(m_backImage, m_rcEnroll[k].left, m_rcEnroll[k].top, m_rcEnroll[k].right - m_rcEnroll[k].left, m_rcEnroll[k].bottom - m_rcEnroll[k].top, 0, 0, 0, 2);
		m_ip.draw_rect(m_backImage, m_rcEnroll[k].left, m_rcEnroll[k].bottom, BACK_RECO_FACE_WIDTH + 4, 25, 0, 0, 0, 2);
	}


	//	Initialize variables for preview mode	//////////////////////////////////////////////////////////////////////////
	m_nDetectedFaces = 0;

	if (m_camera.Initialize(640, 480, CV_CAP_ANY) == 0)
	{
		MessageBox("Can't initialize camera. Try to change format", "Error", MB_OK | MB_ICONERROR);
	}
	if( m_camera.IsInitialized() )
	{
		SetTimer(1, 10, NULL);
		m_camera.Start();
	}
	m_nProcessMode = 1;
	m_nRunMode = 1;

	reduced_enroll_info.size = 0;
	reduced_enroll_info.person_count = 0;
	memset(reduced_enroll_info.id, 0, sizeof(int)*FR_ENROLL_MAX_SIZE);
	memset(reduced_enroll_info.aligned_face, 0, sizeof(unsigned char)*ALIGN_IMG_WIDTH*ALIGN_IMG_HEIGHT * 3 * FR_ENROLL_MAX_SIZE);
	memset(reduced_enroll_info.feature, 0.0f, sizeof(float)*FR_ENROLL_MAX_SIZE*PCA_FEATURE_DIM);

	m_FR.Load_GalleryDB();
	m_FR.Reset_PermanentID(m_FFT.m_data);
	//m_FR.Reset_PermanentID(m_FFT.m_prev_data);
	m_nFrameNumber = 9952;

	m_strPostfix = "result";

	ShowEnrollImage(m_backImage);

 	//m_pFileWriting = fopen("fr_log_seq_01.txt", "a");
// 	FILE *fp;
// 	fp = fopen("frame_number.txt", "r");
// 
// 	if( fp == NULL )
// 	{
// 		m_nFrameNumber = 0;
// 		m_nWebCamFrmNumber = 0;
// 	}
// 	else
// 	{
// 		fscanf(fp, "%d", &m_nFrameNumber);
// 		fclose(fp);
// 	}

	DragAcceptFiles(TRUE);
}


BOOL CXFace_DNNView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

void CXFace_DNNView::OnDraw(CDC* pDC)
{
	CXFace_DNNDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if( !pDoc )
		return;

	if( m_bInitCam == false )
		return;

	if( m_backImage.GetImage() == NULL )
		return;

	if( m_scaleCalculated == false ) {
		CRect rect;
		GetClientRect(rect);
		m_scaleX = 1250.0 / rect.Width();
		m_scaleY = 686.0 / rect.Height();
		m_scaleCalculated = true;
	}

	CvImage srcImage, grayImage, backImage;
	m_ip.clone(m_backImage, backImage);
	CRect rc;

	CDC MemBufDC;
	CBitmap Bmp, *OldBmp;
	CRect ar;
	GetClientRect(&ar);

	if( m_camera.IsRunning() == true )
	{
		m_camera.GetFrame(srcImage);

		if( !m_camera.IsRunning() || srcImage.GetImage() == NULL )
		{
			Bmp.CreateCompatibleBitmap(pDC, ar.Width(), ar.Height());
			MemBufDC.CreateCompatibleDC(pDC);
			OldBmp = (CBitmap*)MemBufDC.SelectObject(&Bmp);
			MemBufDC.Rectangle(ar.left, ar.top, ar.right, ar.bottom);

			backImage.DrawToHDC(MemBufDC.GetSafeHdc(), &ar);

			pDC->BitBlt(0, 0, ar.Width(), ar.Height(), &MemBufDC, 0, 0, SRCCOPY);
			MemBufDC.SelectObject(OldBmp);
			MemBufDC.DeleteDC();
			return;
		}

		int w, h, c;
		m_ip.size(srcImage, w, h, c);
		if( c == 1 )
		{
			m_ip.clone(srcImage, grayImage);
		}
		else
		{
			m_ip.bgr2gray(srcImage, grayImage);
		}

		if( m_nRunMode == 2 )
		{
			Run_Enroll_Image(grayImage, srcImage, backImage, m_nRunMode);
		}
		else if( m_nRunMode == 0 )
		{
			Run_Detect(grayImage, srcImage, backImage, m_nRunMode);
		}
		else
		{
			// Recognition Process 4 Webcam preview mode.
			m_nFrameNumber++;
			Run_Who_Image(grayImage, srcImage, backImage, m_nRunMode);
		}

		CvImage imgLargeDraw;
		m_ip.resize(srcImage, imgLargeDraw, 640, 480);
		m_ip.combine_image(backImage, imgLargeDraw, BACK_CAMERA_X, BACK_CAMERA_Y, backImage);
		m_ip.destroy(imgLargeDraw);
	}
	else
	{
		m_nFrameNumber++;
		if( m_drawImage.Width() > 320 )
		{
			CvImage imgTemp;
			m_ip.resize(m_drawImage, imgTemp, 640, 480);
			m_ip.clone(imgTemp, m_drawImage);
			m_ip.destroy(imgTemp);
		}

		if( m_drawImage.GetImage() != NULL )
		{
			m_ip.combine_image(backImage, m_drawImage, BACK_CAMERA_X, BACK_CAMERA_Y, backImage);
		}
	}

	if( m_nRunMode == 0 )
	{
		m_ip.combine_image(backImage, m_imgBtnFD, 16, 13, backImage);
	}
	if( m_nRunMode == 1 )
	{
		m_ip.combine_image(backImage, m_imgBtnFR, 366, 13, backImage);
	}
	if( m_nRunMode == 2 )
	{
		m_ip.combine_image(backImage, m_imgBtnFE, 191, 13, backImage);
	}

	if( m_pStrImgNameSet != NULL )
	{
		m_ip.combine_image(backImage, m_imgBtnNext, BACK_BUTTON_FR_NEXT_LEFT, BACK_BUTTON_FR_NEXT_TOP, backImage);
	}

	if( m_enrollIndex > 0 )
	{
		m_ip.combine_image(backImage, m_imgBtnPrevPage, BACK_BUTTON_ENROLL_PREV_LEFT, BACK_BUTTON_ENROLL_PREV_TOP, backImage);
	}

	if( m_enrollIndex + BACK_ENROLL_SIZE < m_FR.Get_Enroll_Size() )
	{
		m_ip.combine_image(backImage, m_imgBtnNextPage, BACK_BUTTON_ENROLL_NEXT_LEFT, BACK_BUTTON_ENROLL_NEXT_TOP, backImage);
	}

	Bmp.CreateCompatibleBitmap(pDC, ar.Width(), ar.Height());
	MemBufDC.CreateCompatibleDC(pDC);
	OldBmp = (CBitmap*)MemBufDC.SelectObject(&Bmp);
	MemBufDC.Rectangle(ar.left, ar.top, ar.right, ar.bottom);

	backImage.DrawToHDC(MemBufDC.GetSafeHdc(), &ar);

	pDC->BitBlt(0, 0, ar.Width(), ar.Height(), &MemBufDC, 0, 0, SRCCOPY);
	MemBufDC.SelectObject(OldBmp);
	MemBufDC.DeleteDC();
// 
// 	if (m_nFrameNumber > 39538)
// 	{
// 		char szCaptureName[1024];
// 		sprintf_s(szCaptureName, "E:/Works/2017/[02] Project/[03] KIST/[00] Robot_High_School_DB/JPG/Recog_Out/Out/img_out_%06d.jpg", m_nFrameNumber);
// 		m_ip.save(szCaptureName, backImage);
// 	}

	m_ip.destroy(srcImage);
	m_ip.destroy(grayImage);
	m_ip.destroy(backImage);
}


// CXFace_DNNView 인쇄

BOOL CXFace_DNNView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 기본적인 준비
	return DoPreparePrinting(pInfo);
}

void CXFace_DNNView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄하기 전에 추가 초기화 작업을 추가합니다.
}

void CXFace_DNNView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄 후 정리 작업을 추가합니다.
}


// CXFace_DNNView 진단

#ifdef _DEBUG
void CXFace_DNNView::AssertValid() const
{
	CView::AssertValid();
}

void CXFace_DNNView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CXFace_DNNDoc* CXFace_DNNView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CXFace_DNNDoc)));
	return (CXFace_DNNDoc*)m_pDocument;
}
#endif //_DEBUG


// CXFace_DNNView 메시지 처리기

int CXFace_DNNView::GetTransPosition(int nInput, int nSrc, int nDst)
{
	return (int)((double)(nDst*nInput) / (double)nSrc);
}

void CXFace_DNNView::OnLButtonDown(UINT nFlags, CPoint point)
{
#ifdef CAM_MODE
	int x = point.x;// *m_scaleX;
	int y = point.y;// *m_scaleY;

	printf("x: %d, y: %d\n", x, y);

	if( y > BACK_BUTTON_TOP && y < BACK_BUTTON_BOTTOM )
	{
		if( x > BACK_BUTTON_FD_LEFT && x < BACK_BUTTON_FD_RIGHT )
		{
			OnButtonFaceDetection();
		}
		else if( x > BACK_BUTTON_EN_LEFT && x < BACK_BUTTON_EN_RIGHT )
		{
			OnButtonFaceEnrollment();
		}
		else if( x > BACK_BUTTON_FR_LEFT && x < BACK_BUTTON_FR_RIGHT )
		{
			OnButtonFaceRecognition();
		}
	}
	else if( y > BACK_CAMERA_Y && y < (BACK_CAMERA_Y + 240) && m_nRunMode == 2 && m_nProcessMode == 3 )
	{
		if( x > BACK_CAMERA_X && x < (BACK_CAMERA_X + 320) )
		{
			OnFaceEnrollment(x - BACK_CAMERA_X, y - BACK_CAMERA_Y);
		}
	}
	else if( y > BACK_BUTTON_FR_NEXT_TOP && y < BACK_BUTTON_FR_NEXT_BOTTOM && m_nRunMode == 2 && m_nProcessMode == 3 )
	{
		if( x > BACK_BUTTON_FR_NEXT_LEFT && x < BACK_BUTTON_FR_NEXT_RIGHT )
		{
			m_nFileIdx++;
		}
	}
	else if( y > BACK_BUTTON_ENROLL_PREV_TOP - 38 && y < BACK_BUTTON_ENROLL_PREV_BOTTOM - 38 )
	{
		if( x > BACK_BUTTON_ENROLL_PREV_LEFT && x < BACK_BUTTON_ENROLL_PREV_RIGHT )
		{

			int beforeIndex = m_enrollIndex;
			if( m_enrollIndex > 0 ) {
				m_enrollIndex -= BACK_ENROLL_SIZE;
			}
			else {
				m_enrollIndex = 0;
			}
			ShowEnrollImage(m_backImage);
		}
		else if( x > BACK_BUTTON_ENROLL_NEXT_LEFT && x < BACK_BUTTON_ENROLL_NEXT_RIGHT )
		{
			int beforeIndex = m_enrollIndex;
			if( m_enrollIndex + BACK_ENROLL_SIZE < m_FR.Get_Enroll_Size() ) {
				m_enrollIndex += BACK_ENROLL_SIZE;
			}
			ShowEnrollImage(m_backImage);
		}
	}
#endif

	CView::OnLButtonDown(nFlags, point);
}

void CXFace_DNNView::OnButtonCamearFormat()
{
}

void CXFace_DNNView::OnButtonCamearSource()
{
}

void CXFace_DNNView::OnButtonFaceDetection()
{
	m_nRunMode = 0;
}

void CXFace_DNNView::OnButtonFaceRecognition()
{
	m_FR.Reset_PermanentID(m_FFT.m_data);
	m_nRunMode = 1;
}

void CXFace_DNNView::OnButtonFaceEnrollment()
{
	m_nRunMode = 2;
	m_nEnrollFrame = 0;
}

void CXFace_DNNView::OnDestroy()
{
	CView::OnDestroy();

	if( m_camera.IsInitialized() )
	{
		KillTimer(1);
		m_camera.Uninitialize();
	}
}

BOOL CXFace_DNNView::PreTranslateMessage(MSG* pMsg)
{
	if( pMsg->message == WM_RBUTTONDOWN )
	{
		CPoint p(GetCurrentMessage()->pt);
		CPoint pt(p);
		ScreenToClient(&pt);
		p = pt;
		OnContextMenu(NULL, p);
		return TRUE;
	}

	return CView::PreTranslateMessage(pMsg);
}

void CXFace_DNNView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if( pWnd != NULL )
		return;

	if( (m_nSelectedEnrollFaceIndex = GetRMouseSelectedEnrollFace(point)) == -1 )
		return;

	CPoint pt = point;
	ClientToScreen(&pt);

	CMenu menu;
	VERIFY(menu.LoadMenu(IDR_POPUP_MENU));
	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
}

int CXFace_DNNView::GetRMouseSelectedEnrollFace(CPoint point)
{
	int n = m_FR.Get_Enroll_Size();
	if( n > BACK_ENROLL_SIZE )
	{
		n = BACK_ENROLL_SIZE;
	}

	int x = point.x;/* *m_scaleX;*/
	int y = point.y;/* *m_scaleY;*/

	for( int k = 0; k < n; k++ )
	{
		if( m_rcEnroll[k].left < x && m_rcEnroll[k].right > x &&
			m_rcEnroll[k].top < y && m_rcEnroll[k].bottom > y )
		{
			return m_enrollIndex + k;
		}
	}

	return -1;
}

void CXFace_DNNView::OnMenuPopupDelete()
{
	if( m_nSelectedEnrollFaceIndex == -1 )
		return;

	if( m_nProcessMode == 1 )
	{
		m_camera.Stop();
		KillTimer(1);
	}

	if( MessageBox("Do you really want to delete this face?", "PIM_FaceRecognition", MB_OKCANCEL) != IDOK )
	{
		if( m_nProcessMode == 1 )
		{
			m_camera.Start();
			SetTimer(1, 20, NULL);
		}
		return;
	}

	m_FR.Delete(m_nSelectedEnrollFaceIndex);
	m_FR.Store_GalleryDB();
	m_FR.Load_GalleryDB();

	m_nDetectedFaces = 0;

	m_FR.Reset_PermanentID(m_FFT.m_data);
	//m_FR.Reset_PermanentID(m_FFT.m_prev_data);

	while( m_enrollIndex >= m_FR.Get_Enroll_Size() )
	{
		m_enrollIndex -= BACK_ENROLL_SIZE;
	};

	if( m_enrollIndex < 0 )
	{
		m_enrollIndex = 0;
	}

	ShowEnrollImage(m_backImage);

	if( m_nProcessMode == 1 )
	{
		m_camera.Start();
		SetTimer(1, 1, NULL);
	}

	Invalidate(FALSE);
}

void CXFace_DNNView::ShowEnrollImage(CvImage& canvasImage)
{
	CvImage faceImage, faceResizeImage;
	char strName[100];
	unsigned char* face_data;

	int nWidth = m_FR.Get_Face_Width();
	int nHeight = m_FR.Get_Face_Height();

	face_data = (unsigned char*)malloc(nWidth * nHeight * 3);

	for( int j = m_enrollIndex; j < BACK_ENROLL_SIZE + m_enrollIndex; j++ )
	{
		char strMsg[1024];
		int nFace = 0;
		int k = j - m_enrollIndex;

		if( m_FR.Get_Enroll_Info(j, strName, face_data, &nFace) )
		{
			m_ip.setraw(faceImage, nWidth, nHeight, 3, face_data);
			m_ip.resize(faceImage, faceResizeImage, BACK_RECO_FACE_WIDTH, BACK_RECO_FACE_HEIGHT);
			sprintf_s(strMsg, "%d", nFace);
			m_ip.draw_text(faceResizeImage, strMsg, 5, 70, 255, 255, 255, 1);
			m_ip.combine_image(canvasImage, faceResizeImage, m_rcEnroll[k].left + 2, m_rcEnroll[k].top + 2, canvasImage);
			m_ip.draw_rect(m_backImage, m_rcEnroll[k].left + 1, m_rcEnroll[k].bottom + 1, BACK_RECO_FACE_WIDTH + 2, 23, 255, 255, 255, -1);
			CClientDC dc(this);
			m_ip.draw_rich_text(dc, canvasImage, strName, m_rcEnroll[k].left + 5, m_rcEnroll[k].bottom + 3, 0, 0, 0, 12, 2);			
		}
		else
		{
			m_ip.draw_rect(m_backImage, m_rcEnroll[k].left, m_rcEnroll[k].top, m_rcEnroll[k].right - m_rcEnroll[k].left, m_rcEnroll[k].bottom - m_rcEnroll[k].top, 0, 0, 0, -1);
			m_ip.draw_rect(m_backImage, m_rcEnroll[k].left + 1, m_rcEnroll[k].bottom + 1, BACK_RECO_FACE_WIDTH + 2, 23, 255, 255, 255, -1);
		}
	}

	m_ip.destroy(faceImage);
	m_ip.destroy(faceResizeImage);


	free(face_data);
}

void CXFace_DNNView::OnMenuCameraSource()
{
	OnButtonCamearSource();
}

void CXFace_DNNView::OnMenuCameraFormat()
{
	OnButtonCamearFormat();
}

void CXFace_DNNView::OnTimer(UINT_PTR nIDEvent)
{
	if( nIDEvent == 1 )		// cam mode
	{
		InvalidateRect(NULL, FALSE);
	}
	else if( nIDEvent == 2 )	// video mode
	{
		bool bSuccess = m_video_capture.read(m_video_frame);
		if( bSuccess )
		{
			int w, h, c;
			CvImage srcImage, grayImage, backImage;
			CvImage srcImage2;

			m_ip.size(srcImage, w, h, c);

			srcImage.Create(m_video_frame.cols, m_video_frame.rows, 24);
			memcpy(srcImage.GetImage()->imageData, m_video_frame.data, m_video_frame.rows*m_video_frame.cols * 3);

			if( c == 1 )
			{
				m_ip.clone(srcImage, grayImage);
			}
			else
			{
				m_ip.bgr2gray(srcImage, grayImage);
			}

			m_nFrameNumber++;

			srcImage2.Create(w, h, 24);
			m_ip.clone(srcImage, srcImage2);

			Run_Who_Image(grayImage, srcImage2, m_backImage, 1);

			m_ip.clone(srcImage2, m_drawImage);



			Invalidate(FALSE);
		}
		else
		{
			KillTimer(2);

			if( m_pFile != NULL )
			{
				fclose(m_pFile);
			}
		}
	}

	CView::OnTimer(nIDEvent);
}

void CXFace_DNNView::OnDropFiles(HDROP hDropInfo)
{
	m_bMode4Video = true;

	KillTimer(1);
	m_camera.Stop();
	m_camera.Uninitialize();

	m_FR.Reset_PermanentID(m_FFT.m_data);

	POINT lpp;
	DragQueryPoint(hDropInfo, &lpp);
	int nMode = 0;
	int x = lpp.x, y = lpp.y;

	if( y > BACK_BUTTON_TOP && y < BACK_BUTTON_BOTTOM )
	{
		if( x > BACK_BUTTON_EN_LEFT && x < BACK_BUTTON_EN_RIGHT )
		{
			nMode = 2;
			m_nRunMode = 2;
		}
		//else if (x > BACK_BUTTON_FR_LEFT && x < BACK_BUTTON_FR_RIGHT && m_nProcessMode != 1)
		else if( x > BACK_BUTTON_FR_LEFT && x < BACK_BUTTON_FR_RIGHT )
		{
			nMode = 1;
			m_nRunMode = 1;
			m_nProcessMode = 3;
		}
		else if( x > BACK_BUTTON_FD_LEFT && x < BACK_BUTTON_FD_RIGHT )
		{
			nMode = 0;
		}
	}
	else
	{
		return;
	}


	if( nMode == 1 )
	{
		char folderPath_t[2048];
		DragQueryFile(hDropInfo, 0, folderPath_t, MAX_PATH);     // Get of the first file path 

		CString strExt;

		strExt.Format("%s", folderPath_t + strlen(folderPath_t) - 3);
		m_strImgExt = strExt;
		if( strExt.CompareNoCase("JPG") == 0 || strExt.CompareNoCase("BMP") == 0 || strExt.CompareNoCase("PNG") == 0 || strExt.CompareNoCase("TIF") == 0
			|| strExt.CompareNoCase("PPM") == 0 )
		{
			KillTimer(1);
			KillTimer(2);

			int w, h, c;
			int idx, idx2;
			char szGTFileName[2048];

			CvImage srcImage, grayImage, backImage;
			CString strImgsPath, strTestImgFolderPath, strGTFolderName;

			strImgsPath.Format("%s", folderPath_t);
			idx = strImgsPath.ReverseFind('\\');
			strTestImgFolderPath = strImgsPath.Left(idx);
			strTestImgFolderPath.Replace('\\', '/');
			m_strTestImgFolderPath = strTestImgFolderPath;

			m_strImgName = strImgsPath.Mid(idx + 1);

			idx2 = strTestImgFolderPath.ReverseFind('/');
			m_strGTFolderName = strGTFolderName = strTestImgFolderPath.Mid(idx2 + 1);

			sprintf_s(szGTFileName, "%s/%s.txt", strTestImgFolderPath, strGTFolderName);
			m_strTestImgGTPath.Format("%s", szGTFileName);

			m_ip.load(folderPath_t, srcImage);
			m_ip.size(srcImage, w, h, c);

			if( c == 1 )
			{
				m_ip.clone(srcImage, grayImage);
			}
			else
			{
				m_ip.bgr2gray(srcImage, grayImage);
			}

			m_FR.Reset_PermanentID(m_FFT.m_data);

			Run_Who_Image(grayImage, srcImage, m_backImage, 1);

			m_ip.clone(srcImage, m_drawImage);

			Invalidate(FALSE);
		}
		else if( strExt.CompareNoCase("AVI") == 0 || strExt.CompareNoCase("MP4") == 0 || strExt.CompareNoCase("3GP") == 0 ||
			strExt.CompareNoCase("WMV") == 0 || strExt.CompareNoCase("MKV") == 0 || strExt.CompareNoCase("MOV") == 0 )
		{
			/*USES_CONVERSION*/;
			CvImage srcImage, drawImage, grayImage, grabImage, grabFaceImage;

			m_video_capture.open(folderPath_t);

			m_strVideoFileName.Format("%s", folderPath_t);
			char strGTfileName[2048];
			sprintf_s(strGTfileName, "%s.txt", folderPath_t);
			fopen_s(&m_pFile, strGTfileName, "r");
			m_nTrueAcception = 0;
			m_nFalseRejection = 0;
			m_nFalseAcception = 0;
			m_nFalseAcception2 = 0;
			m_nMissed = 0;
			m_nTotalTA = 0;
			m_nTotalFA = 0;
			m_nFrameNumber = 0;

// 			sprintf_s(strGTfileName, "%s_result_video.txt", folderPath_t);
// 			fopen_s(&m_pFileWriting2, strGTfileName, "w");

			if( m_video_capture.isOpened() )
			{
				SetTimer(2, 10, NULL);

			}
			else
			{
				return;
			}
		}
		else
		{
			KillTimer(1);
			KillTimer(2);

			int idx, idx2;
			char szGTFileName[2048];

			CvImage srcImage, grayImage, backImage;
			CString strImgsPath, strTestImgFolderPath, strGTFolderName;

			strImgsPath.Format("%s", folderPath_t);
			idx = strImgsPath.ReverseFind('\\');
			strTestImgFolderPath = strImgsPath.Left(idx);
			strTestImgFolderPath.Replace('\\', '/');
			m_strTestImgFolderPath = strTestImgFolderPath;

			m_strTestImgFolderPath = strImgsPath;

			idx2 = strTestImgFolderPath.ReverseFind('/');
			m_strGTFolderName = strGTFolderName = strTestImgFolderPath.Mid(idx2 + 1);

			sprintf_s(szGTFileName, "%s/%s.txt", strTestImgFolderPath, strGTFolderName);
			m_strTestImgGTPath.Format("%s", szGTFileName);

			DelegateRunImageSet();

			//fclose(m_pFileWriting);
		}
	}
	else if( nMode == 2 )
	{
		CvImage srcImage, grayImage, backImage;
		char strPath[2048];
		DragQueryFile(hDropInfo, 0, strPath, MAX_PATH);

		CString strBuffer;
		strBuffer.Format("%s", strPath + strlen(strPath) - 3);

		if( m_GT_faceInfo != NULL )
		{
			delete[] m_GT_faceInfo;
			m_GT_faceInfo = NULL;
		}

		if( m_pStrImgNameSet != NULL )
		{
			delete m_pStrImgNameSet;
			m_pStrImgNameSet = NULL;
		}

		if( (strBuffer.CompareNoCase("JPG") == 0) || (strBuffer.CompareNoCase("BMP") == 0) || (strBuffer.CompareNoCase("PNG") == 0) || (strBuffer.CompareNoCase("TIF") == 0) )
		{
			int idx, idx2;
			int nFaceGT = 0;

			char szGTFileName[2048];
			char szImgFileName[2048];

			CString strTempGTPath, strGTFolderPath, strGTFolderName;
			CString strInputImgName;
			CString strTempGTContent, strGTContentImgName;

			m_nGTFaceCnt = 0;

			strTempGTPath.Format("%s", strPath);

			idx = strTempGTPath.ReverseFind('\\');
			strGTFolderPath = strTempGTPath.Left(idx);
			strInputImgName = strTempGTPath.Mid(idx + 1);

			idx2 = strGTFolderPath.ReverseFind('\\');
			strGTFolderName = strGTFolderPath.Mid(idx2 + 1);

			strGTFolderPath.Replace('\\', '/');
			m_strGTFolderPath.Format("%s", strGTFolderPath);

			strGTFolderName.Replace('\\', '/');

			CFileFind finder;
			BOOL bWorking = finder.FindFile(strGTFolderPath + "/*." + strBuffer);
			if( bWorking )
			{
				m_pStrImgNameSet = new CStringArray();
				m_nFileIdx = 0;

				while( bWorking )
				{
					bWorking = finder.FindNextFile();
					if( !finder.IsDirectory() )
					{
						m_pStrImgNameSet->Add(finder.GetFileName());
					}
				}
			}

			sprintf_s(szGTFileName, "%s/%s.txt", strGTFolderPath, strGTFolderName);
			m_strGTFilePath.Format("%s", szGTFileName);
			fopen_s(&m_pFile, szGTFileName, "r");

			if( m_pFile != NULL )
			{
				CString strImgName = m_pStrImgNameSet->GetAt(m_nFileIdx);

				m_ip.load(strGTFolderPath + "/" + strImgName, srcImage);
				m_ip.load(strGTFolderPath + "/" + strImgName, m_SrcImage);

				while( !feof(m_pFile) )
				{
					fscanf_s(m_pFile, "%s", szImgFileName);

					strTempGTContent.Format("%s", szImgFileName);

					idx = strTempGTContent.ReverseFind('/');
					strGTContentImgName = strTempGTContent.Mid(idx + 1);

					m_nGTFaceCnt = 0;

					if( strImgName.Compare(strGTContentImgName) == 0 )
					{
						fscanf_s(m_pFile, "%d", &nFaceGT);

						if( nFaceGT == 0 )
						{
							m_nGTFaceCnt = 0;
							break;
						}

						m_nGTFaceCnt = nFaceGT;
						m_GT_faceInfo = new FaceInfoGT[nFaceGT + 1];

						for( int index = 0; index < nFaceGT; index++ )
						{
							char buf[1024];
							fscanf_s(m_pFile, "%d %d %s", &(m_GT_faceInfo[index].faceCenter.x), &(m_GT_faceInfo[index].faceCenter.y), buf);

							m_GT_faceInfo[index].faceID.Format("%s", buf);
							m_GT_faceInfo[index].faceRecogID.Format("%s", ".");
							m_GT_faceInfo[index].faceLEye.x = 0;
							m_GT_faceInfo[index].faceLEye.y = 0;
							m_GT_faceInfo[index].faceREye.x = 0;
							m_GT_faceInfo[index].faceREye.y = 0;
							m_GT_faceInfo[index].bAssigned = false;
							m_GT_faceInfo[index].nSimilarity = -10000;
							m_GT_faceInfo[index].nRank = -10000;
							m_GT_faceInfo[index].faceRect.left = 0;
							m_GT_faceInfo[index].faceRect.top = 0;
							m_GT_faceInfo[index].faceRect.right = 0;
							m_GT_faceInfo[index].faceRect.bottom = 0;
						}
						break;
					}
					else
					{
						m_nGTFaceCnt = 0;
						fscanf_s(m_pFile, "%d", &nFaceGT);
						if( nFaceGT == 0 )
						{
							continue;
						}

						int dummyX, dummyY;
						CString dummyName;

						for( int index = 0; index < nFaceGT; index++ )
						{
							fscanf_s(m_pFile, "%d %d %s", &dummyX, &dummyY, dummyName);
						}
					}
				}

				if( m_pFile != NULL )
				{
					fclose(m_pFile);
					m_pFile = NULL;
				}
			}
			else
			{
				if( m_GT_faceInfo != NULL )
				{
					delete[] m_GT_faceInfo;
					m_GT_faceInfo = NULL;
				}

				if( m_pStrImgNameSet != NULL )
				{
					delete m_pStrImgNameSet;
					m_pStrImgNameSet = NULL;
				}

				m_ip.load(strPath, srcImage);
				m_ip.load(strPath, m_SrcImage);
			}

			int w, h, c;
			m_ip.size(srcImage, w, h, c);
			if( c == 1 )
			{
				m_ip.clone(srcImage, grayImage);
			}
			else
			{
				m_ip.bgr2gray(srcImage, grayImage);
			}
		}
		else
		{
			CView::OnDropFiles(hDropInfo);
			return;
		}

		OnButtonFaceEnrollment();

		Run_Enroll_Image(grayImage, srcImage, m_backImage, nMode);

		m_ip.clone(srcImage, m_drawImage);

		Invalidate(FALSE);
	}
	else if( nMode == 3 )
	{

	}

	DragFinish(hDropInfo);
	CView::OnDropFiles(hDropInfo);
}

void CXFace_DNNView::OnTestDotest()
{
}

void CXFace_DNNView::OnDotestTest()
{
}

void CXFace_DNNView::OnTatestmodeImagemode()
{
}

void CXFace_DNNView::OnTatestmodeSeq()
{
}

void CXFace_DNNView::ShowRecogResult(char * strCaller)
{
}

void CXFace_DNNView::OnFaceEnrollment(int clickX, int clickY)
{
	CvImage srcImage, grayImage;

	int transX, transY;

	int w, h, c;
	m_ip.size(m_SrcImage, w, h, c);
	if( c == 1 )
	{
		m_ip.clone(m_SrcImage, grayImage);
	}
	else
	{
		m_ip.bgr2gray(m_SrcImage, grayImage);
	}

	OnButtonFaceEnrollment();

	transX = GetTransPosition(clickX, 640, w);
	transY = GetTransPosition(clickY, 480, h);

	m_ip.clone(srcImage, m_drawImage);
	Invalidate(FALSE);
}

void CXFace_DNNView::OnProcessmodeWebcam()
{
	AfxMessageBox("WebCam");

	KillTimer(2); KillTimer(3);

	m_FFT.ReSet_FD();

	if( m_camera.Initialize(DEF_CAMERA_WIDTH, DEF_CAMERA_HEIGHT, 0) == 0 )
	{
		MessageBox("Can't initialize camera. Try to change format", "Error", MB_OK | MB_ICONERROR);
	}
	if( m_camera.IsInitialized() )
	{
		SetTimer(1, 10, NULL);
		m_camera.Start();
	}

	m_nProcessMode = 1;
	m_nRunMode = 1;
}

void CXFace_DNNView::OnUpdateProcessmodeWebcam(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(!m_camera.IsRunning() && m_nProcessMode != 1);
}

void CXFace_DNNView::OnProcessmodeVideo()
{
	AfxMessageBox("Video");

	KillTimer(1); KillTimer(3);

	m_camera.Stop();
	m_nProcessMode = 2;
}

void CXFace_DNNView::OnUpdateProcessmodeVideo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_nProcessMode != 2);
}

void CXFace_DNNView::OnProcessmodeImageset()
{
	AfxMessageBox("ImageSet: Identification");

	KillTimer(1); KillTimer(2);

	m_camera.Stop();
	m_camera.Uninitialize();
	m_nProcessMode = 3;
}

void CXFace_DNNView::OnUpdateProcessmodeImageset(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_nProcessMode != 3);
}

void CXFace_DNNView::DelegateRunImageSet()
{
	CvImage srcImage, grayImage, backImage;

	m_nTrueAcception = 0;
	m_nFalseRejection = 0;
	m_nFalseAcception = 0;
	m_nFalseAcception2 = 0;
	m_nMissed = 0;
	m_nTotalTA = 0;
	m_nTotalFA = 0;
	m_nFrameNumber = 39538;

	int fidx = 0;

	CFileFind finder;
	BOOL bWorking = finder.FindFile(m_strTestImgFolderPath + "/*.*");
	while( bWorking )
	{
		bWorking = finder.FindNextFile();
		if( !finder.IsDirectory() )
		{
			int w, h, c;

			fidx++;

			BOOL bDoingBackgroundProcessing = TRUE;

			m_strImgName = m_strTestImgName = finder.GetFileName();

			int idx = m_strTestImgName.ReverseFind('.');
			CString strExt = m_strTestImgName.Right(3);

			if( strExt.CompareNoCase("BMP") != 0 && strExt.CompareNoCase("JPG") != 0 && strExt.CompareNoCase("PNG") != 0 && strExt.CompareNoCase("TIF") != 0
				&& strExt.CompareNoCase("PPM") != 0 )
			{
				continue;
			}

			m_ip.load(m_strTestImgFolderPath + "/" + m_strTestImgName, srcImage);

			m_ip.size(srcImage, w, h, c);
			if( c == 1 )
			{
				m_ip.clone(srcImage, grayImage);
			}
			else
			{
				m_ip.bgr2gray(srcImage, grayImage);
			}

			//fprintf(m_pFileWriting, "#%03d, %s image: Start!!!\n", fidx, m_strTestImgName);

			//m_nFrameNumber++;
			//Run_Who_Video(grayImage, srcImage, m_backImage, 1);

			Run_Who_Image(grayImage, srcImage, m_backImage, 1);

			//fprintf(m_pFileWriting, "#%03d, %s image: End!!!\n", fidx, m_strTestImgName);

			m_ip.clone(srcImage, m_drawImage);
			CvImage imgLargeDraw;
			m_ip.resize(srcImage, imgLargeDraw, 640, 480);

			m_ip.combine_image(m_backImage, imgLargeDraw, BACK_CAMERA_X, BACK_CAMERA_Y, m_backImage);

			Invalidate(FALSE);

			OnDraw(GetDC());
		}
	}

	KillTimer(3);

	//char strGTfileName[1024];
	//sprintf_s(strGTfileName, "%s_result.txt", m_strGTFolderName);
	//fprintf(m_pFileWriting, "TA:%d FR:%d FA:%d FA2:%d Missed:%d TotalTA:%d TotalFA:%d\n", m_nTrueAcception, m_nFalseRejection, m_nFalseAcception, m_nFalseAcception2, m_nMissed, m_nTotalTA, m_nTotalFA);
}

void CXFace_DNNView::Run_Detect(CvImage &srcCamImage, CvImage &drawCamImage, CvImage &canvasImage, int nRunMode)
{
	if( nRunMode == -1 )
	{
		return;
	}

	int nFaceCnt = 0;
	int dp_ratio = DEF_CAMERA_WIDTH / 320;

	img_info img_i;

	PIM_Int32 nRankOneID = 0;
	PIM_Float fConfidence = 0.0f;

	PIM_Bitmap inbitmap = { 0 };
	IplImage *ipl_load = srcCamImage.GetImage();

	PIM_DBG_SetBitmapFromIplimageGray(&inbitmap, ipl_load);

	img_i.img_width = inbitmap.width;
	img_i.img_height = inbitmap.height;
	img_i.img = inbitmap.imageData;
	img_i.gt_enabled = false;

	if( (nFaceCnt = m_FFT.img_detect(img_i)) == 0 )
	{
		printf("Face detector error or there is no face.\n");
		PIM_Bitmap_Destroy(&inbitmap);
		return;
	}

	for( register int fidx = 0; fidx < nFaceCnt; fidx++ )
	{
		PIM_DPoint pnt_shp[68];
		SMyRect face_rect;
		CFMat face_pts;
		m_FFT.get_fd_result(fidx, face_rect);
		m_FFT.get_alignment_result(fidx, face_pts);
		for( int j = 0; j < 68; j++ )
		{
			pnt_shp[j].x = face_pts.m_data[j * 2];
			pnt_shp[j].y = face_pts.m_data[j * 2 + 1];
		}

		show_data(face_pts, drawCamImage, face_rect);
		m_ip.draw_corner_rect(drawCamImage, face_rect.left, face_rect.top, 
			face_rect.right - face_rect.left, face_rect.bottom - face_rect.top, 0, 255, 0, 2);
	}

	PIM_Bitmap_Destroy(&inbitmap);	
}

void CXFace_DNNView::show_data(CFMat &face_pts, CvImage &frame, SMyRect& rect)
{
	int i, j;
	int thick = 2;
	float *pts = face_pts.m_data;

	int dp_ratio = frame.Width() / 640;

	int start[] = { 0, 17, 22, 27, 31, 36, 42, 48, 60 };
	int end[] = { 16, 21, 26, 30, 35, 41, 47, 59, 67 };
	int b_round[] = { 0, 0, 0, 0, 0, 1, 1, 1, 1 };

	int pts_indices[17] = {
		17, 19, 21,
		36, 39,
		22, 24, 26,
		42, 45,
		31, 33, 35,
		60, 62, 64, 66
	};

	int faceWidth = rect.right - rect.left;
	int faceHeight = rect.bottom - rect.top;

	int minSize = min(faceWidth, faceHeight);

	float innerCircleRadius, middleCircleRadius, outerCircleRadius;

	innerCircleRadius = minSize * 9 / 1000;
	middleCircleRadius = innerCircleRadius + minSize * 2 / 1000 + 1;
	outerCircleRadius = middleCircleRadius + minSize * 2 / 1000 + 1;

	bool is_big_face = true;

	if( innerCircleRadius <= 1 )
	{
		is_big_face = false;
	}
	is_big_face = false;

	for( i = 1; i < 9; i++ )
	{
		for( j = start[i]; j < end[i]; j++ )
		{
			m_ip.draw_line(frame, pts[j * 2], pts[j * 2 + 1], pts[(j + 1) * 2], pts[(j + 1) * 2 + 1], 0, 255, 0, 2 * dp_ratio);
		}
		if( b_round[i] == 1 )
		{
			m_ip.draw_line(frame, pts[start[i] * 2], pts[start[i] * 2 + 1], pts[end[i] * 2], pts[end[i] * 2 + 1], 0, 255, 0, 2 * dp_ratio);
		}
	}
}

void CXFace_DNNView::Run_Who_Image(CvImage &srcCamImage, CvImage &drawCamImage, CvImage &canvasImage, int nRunMode)
{
	if( nRunMode == -1 )
	{
		return;
	}

#if 0
	if (m_nFrameNumber <= 39538)
	{
		return;
	}
#endif

	int nFaceCnt = 0;
	int dp_ratio = DEF_CAMERA_WIDTH / 320 + 1;
	PIM_Int32 nRecogId = -1;
	int nGender = -1;

	char szRecogName[32];
	char szRankOneName[32];
	char szDrawText[1024] = { 0, };

	img_info img_i;

	PIM_Int32 nRankOneID = 0;
	PIM_Float fConfidence = 0.0f;

	PIM_Bitmap ingraybitmap = { 0 };
	PIM_Bitmap inbitmap = { 0 };
	IplImage *ipl_load = srcCamImage.GetImage();
	IplImage *ipl_load_color = drawCamImage.GetImage();
	

	PIM_DBG_SetBitmapFromIplimageGray(&ingraybitmap, ipl_load);
	PIM_DBG_SetBitmapFromIplimage(&inbitmap, ipl_load_color);

	img_i.img_width = ingraybitmap.width;
	img_i.img_height = ingraybitmap.height;
	img_i.img = ingraybitmap.imageData;
	img_i.gt_enabled = false;
	
	if( (nFaceCnt = m_FFT.img_detect(img_i)) == 0 )
	{
		printf("Face detector error or there is no face.\n");
		m_FR.Reset_PermanentID(m_FFT.m_data);
		PIM_Bitmap_Destroy(&ingraybitmap);
		PIM_Bitmap_Destroy(&inbitmap);
		return;
	}
	
#if 0
	for (register int fidx = 0; fidx < nFaceCnt; fidx++)
	{
		bool bFitFlag = true;
		PIM_DPoint pnt_shp[68];
		SMyRect face_rect;
		CFMat face_pts;
		m_FFT.get_fd_result(fidx, face_rect);

		int nWidth = face_rect.right - face_rect.left;
		if (nBigWidth < nWidth)
		{
			nBigWidth = nWidth;
			nBigIdx = fidx;
		}
	

		m_FFT.get_alignment_result(fidx, face_pts);
		for( int j = 0; j < 68; j++ )
		{
			if( face_pts.m_data[j * 2] < 0 || face_pts.m_data[j * 2] > inbitmap.width
				|| face_pts.m_data[j * 2 + 1] < 0 || face_pts.m_data[j * 2 + 1] > inbitmap.height )
			{
				bFitFlag = false;
				break;
			}
			pnt_shp[j].x = face_pts.m_data[j * 2];
			pnt_shp[j].y = face_pts.m_data[j * 2 + 1];

		}
		if( bFitFlag == true )
		{

			show_data(face_pts, drawCamImage, face_rect);
			m_ip.draw_corner_rect(drawCamImage, face_rect.left, face_rect.top,
				face_rect.right - face_rect.left, face_rect.bottom - face_rect.top, 0, 255, 0, 2);

			/* *****************************  Perform recognition ********************************* */
#if 1
// 			LARGE_INTEGER first_time, result_time;
// 			LARGE_INTEGER tick1, tick2, tick3, tick4;
// 			double cap = 0.0;
// 
// 			QueryPerformanceFrequency(&first_time);
// 			QueryPerformanceCounter(&tick1);
			
			nRecogId = m_FR.Who_DNN_Image(&inbitmap, pnt_shp, &nRankOneID, &fConfidence);

// 			QueryPerformanceCounter(&tick2);
// 			result_time.QuadPart = (LONGLONG)tick2.QuadPart - tick1.QuadPart;
// 			cap = (double)result_time.QuadPart / (double)first_time.QuadPart * 1000.0;
// 			printf("Process Time(Recognition) = %lf\n", cap);

			if( !m_FR.Get_Enroll_Info(nRecogId, szRecogName, NULL) )
			{
				sprintf_s(szRecogName, "Unknown");
			}

			if( !m_FR.Get_Enroll_Info(nRankOneID, szRankOneName, NULL) )
			{
				sprintf_s(szRankOneName, "Unknown");
			}
			sprintf_s(szDrawText, "%s, %s, %f", szRecogName, szRankOneName, fConfidence);

			CClientDC dc(this);
			m_ip.draw_rich_text(dc, drawCamImage, szDrawText, face_rect.left + 10, face_rect.bottom - 5, 255, 128, 64, 8 * dp_ratio, 2);
			printf("%s\n", szDrawText);
			//fprintf(m_pFileWriting, "%02d, %s\n", fidx, szDrawText);
		}
#endif
	}

#else

	int nBigWidth = 0;
	int nBigIdx = 0;

	for (register int fidx = 0; fidx < nFaceCnt; fidx++)
	{
		SMyRect face_rt;
		m_FFT.get_fd_result(fidx, face_rt);

		int width = face_rt.right - face_rt.left;
		if (nBigWidth < width)
		{
			nBigWidth = width;
			nBigIdx = fidx;
		}
	}

	CFMat face_pts;
	bool bFitFlag = true;
	PIM_DPoint pnt_shp[68];
	SMyRect face_rect;

	m_FFT.get_fd_result(nBigIdx, face_rect);
	m_FFT.get_alignment_result(nBigIdx, face_pts);
	for (register int j = 0; j < 68; j++)
	{
		if (face_pts.m_data[j * 2] < 0 || face_pts.m_data[j * 2] > inbitmap.width
			|| face_pts.m_data[j * 2 + 1] < 0 || face_pts.m_data[j * 2 + 1] > inbitmap.height)
		{
			bFitFlag = false;
			break;
		}
		pnt_shp[j].x = face_pts.m_data[j * 2];
		pnt_shp[j].y = face_pts.m_data[j * 2 + 1];
	}

	if (bFitFlag)
	{
		show_data(face_pts, drawCamImage, face_rect);
		m_ip.draw_corner_rect(drawCamImage, face_rect.left, face_rect.top, face_rect.right - face_rect.left, face_rect.bottom - face_rect.top, 0, 255, 0, 2);

		nRecogId = m_FR.Who_DNN_Image(&inbitmap, pnt_shp, &nRankOneID, &fConfidence);

		if (!m_FR.Get_Enroll_Info(nRecogId, szRecogName, NULL))
		{
			sprintf_s(szRecogName, "Unknown");
		}
		if (!m_FR.Get_Enroll_Info(nRankOneID, szRankOneName, NULL))
		{
			sprintf_s(szRankOneName, "Unknown");
		}
		sprintf_s(szDrawText, "%s, %s, %f", szRecogName, szRankOneName, fConfidence);

		CClientDC dc(this);
		m_ip.draw_rich_text(dc, drawCamImage, szDrawText, face_rect.left + 10, face_rect.bottom - 5, 255, 128, 64, 8 * dp_ratio, 2);
		//fprintf_s(m_pFileWriting, "%d, %s\n", m_nFrameNumber, szDrawText);
		//printf("%s\n", szDrawText);
	}

#endif
	
	PIM_Bitmap_Destroy(&ingraybitmap);
	PIM_Bitmap_Destroy(&inbitmap);
}

void CXFace_DNNView::Run_Enroll_Image(CvImage &srcCamImage, CvImage &drawCamImage, CvImage &canvasImage, int nRunMode)
{
	if( nRunMode == -1 )
	{
		return;
	}

	int nFaceCnt = 0;
	PIM_Bitmap inbitmap = { 0 };
	PIM_Bitmap inbitmap_color = { 0 };

	IplImage *ipl_load = srcCamImage.GetImage();
	IplImage *ipl_load_color = drawCamImage.GetImage();
	PIM_DBG_SetBitmapFromIplimageGray(&inbitmap, ipl_load);
	PIM_DBG_SetBitmapFromIplimage(&inbitmap_color, ipl_load_color);

	img_info img_i;
	img_i.img_width = inbitmap.width;
	img_i.img_height = inbitmap.height;
	img_i.img = inbitmap.imageData;
	img_i.gt_enabled = false;

	if( (nFaceCnt = m_FFT.img_detect(img_i)) == 0 )
	{
		printf("Face detector error\n");
		PIM_Bitmap_Destroy(&inbitmap);
		PIM_Bitmap_Destroy(&inbitmap_color);
		return;
	}

	PIM_DPoint pnt_shp[68];
	SMyRect face_rect;
	CFMat face_pts;

	int nBigFacdIdx = 0;
	int nBigFaceWidth = 0;
	for( int i = 0; i < nFaceCnt; i++ )
	{
		m_FFT.get_fd_result(i, face_rect);
		int nWidth = face_rect.right - face_rect.left;
		if( nBigFaceWidth < nWidth )
		{
			nBigFaceWidth = nWidth;
			nBigFacdIdx = i;
		}
	}

	m_FFT.get_fd_result(nBigFacdIdx, face_rect);
	m_FFT.get_alignment_result(nBigFacdIdx, face_pts);

	for( int j = 0; j < 68; j++ )
	{
		pnt_shp[j].x = face_pts.m_data[j * 2];
		pnt_shp[j].y = face_pts.m_data[j * 2 + 1];
	}

	m_camera.Stop();
	if( m_FR.AddPerson(&inbitmap_color, pnt_shp) == PIM_TRUE )
	{
		m_camera.Stop();
		if( m_objEnrollDlg.DoModal() == IDOK )
		{
			char szName[32];
			m_nRunMode = 1;

			sprintf_s(szName, "%s", m_objEnrollDlg.m_strName);
			m_FR.Enroll(NULL, szName);
			m_FR.Store_GalleryDB();
			m_FR.Load_GalleryDB();
			ShowEnrollImage(m_backImage);
		}
		else
		{
			m_nRunMode = 1;
			m_FR.Person_Delete_Buffer();
			ShowEnrollImage(m_backImage);
		}
	}
	
	PIM_Bitmap_Destroy(&inbitmap);
	PIM_Bitmap_Destroy(&inbitmap_color);

	m_FFT.ReSet_FD();

	if( m_nProcessMode == 1 )
	{
		m_nRunMode = 1;
		m_nProcessMode = 1;


		if( m_camera.IsInitialized() )
		{
			KillTimer(2); KillTimer(3);
			SetTimer(1, 10, NULL);
			m_camera.Start();
		}
		else
		{
			if( m_camera.Initialize(DEF_CAMERA_WIDTH, DEF_CAMERA_HEIGHT, 0) == 0 )
			{
				MessageBox("Can't initialize camera. Try to change format", "Error", MB_OK | MB_ICONERROR);
			}
			if( m_camera.IsInitialized() )
			{
				KillTimer(2); KillTimer(3);
				SetTimer(1, 10, NULL);
				m_camera.Start();
			}
		}		
	}
}
