
#include "stdafx.h"
#include "EyeDetectionParameters.h"
#include "EyeDetection.h"
#include "sdic_ImageProcess.h"
//#include "FaceDetection.h"

#ifdef _DEBUG
// 	#include "ImageProcessing.h"
#endif


#define max( a, b )            ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )

#define min( a, b )            ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )


#include "define_op.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

//#define ZORAN_PLATFORM

#ifdef ZORAN_PLATFORM
#include "Allocator.h"
#define myMalloc(x) PoolAlloc(NULL, x, POOL_MEMORY_CACHED)
#define myFree(x) PoolFree(NULL, x)
#else
#define myMalloc(x) malloc(x)
#define myFree(x) free(x)
#endif

#define ED_MAX_FIND						10
/*
// 첫 image pyramid 크기 변경
*/
#define ED_BEST_SIZE					33
#define ED_BIT_SHIFT					15
/*
// Image pyramid step 변경
*/
#define ED_PYRAMID_STEP					2

#define ED_MAX(a,b)        ((a)>(b)?(a):(b)) 
#define ED_MIN(a,b)        ((a)<(b)?(a):(b)) 

int		m_nEyeLeftCandidateCount = 0, m_nEyeRightCandidateCount = 0;
 SMyRect m_ed_rcEyeCandidateLeft, m_ed_rcEyeCandidateRight;
 SMyRect m_ed_rcLeft, m_ed_rcRight;

int		m_bEyeLeftCandidateValid[ED_MAX_FIND], m_bEyeRightCandidateValid[ED_MAX_FIND];
int		m_nEyeLeftCandidateIntersectionCount[ED_MAX_FIND], m_nEyeRightCandidateIntersectionCount[ED_MAX_FIND];
int		m_nEyeLeftCandidateConfidence[ED_MAX_FIND], m_nEyeRightCandidateConfidence[ED_MAX_FIND];
 SMyRect m_ed_rcLeftCandidate[ED_MAX_FIND], m_ed_rcRightCandidate[ED_MAX_FIND];

unsigned short *m_nEyePositionLeft_Left, *m_nEyePositionLeft_Top, *m_nEyePositionLeft_Right, *m_nEyePositionLeft_Bottom;
unsigned short *m_nEyePositionRight_Left, *m_nEyePositionRight_Top, *m_nEyePositionRight_Right, *m_nEyePositionRight_Bottom;
unsigned char *m_nEyeClassLeft, *m_nEyeClassRight;
unsigned short *m_nEyeConfidenceLeft, *m_nEyeConfidenceRight;

int		m_nEyeLeftTotalLimit, m_nEyeRightTotalLimit;
int		m_nEyeLeftTotal, m_nEyeRightTotal;

int sdic_ed_find(unsigned char* srcData, int src_w, int src_h,  SMyRect rcFace, int rop_angle);
int sdic_ed_whereBox( SMyRect* rectLEye,  SMyRect* rectREye);
int sdic_ed_where( SMyPoint* ptLEye,  SMyPoint* ptREye);

void sdic_ed_scan(int nResizeW, int nResizeH, int nSrcW, int nSrcH, unsigned short* nMCTData, int bIsRight);

void sdic_ed_enrollLeft(int nEyeX, int nEyeY, int nEyeW, int nEyeH, int nConfidence);
void sdic_ed_enrollRight(int nEyeX, int nEyeY, int nEyeW, int nEyeH, int nConfidence);
int sdic_ed_isintersection( SMyRect rc1,  SMyRect rc2);
void sdic_ed_localizeLeft(unsigned char nclass);
void sdic_ed_localizeRight(unsigned char nclass);

/* -----------------------------------------------------------------------------------------
long SDIC_EyeDetection2(unsigned char* pData, long nDataWidth, long nDataHeight,
						long nPreviewWidth, long nPreviewHeight, FD_INFO *pFD_Info);
- 눈 검출 수행 및 눈 위치 정보 저장, 얼굴검출에 사용한 영상의 크기와 눈검출에 사용할 영상의
크기가 다를 경우에 사용
- [IN]
* unsigned char *pData:
GRAY 타입(8Bits)의 RAW 데이터(입력영상), angle에 따라 회전된 영상
데이터 크기: nWidth*nHeight
* long nDataWidth:				입력영상의 width
* long nDataHeight:				입력영상의 height
* long nPreviewWidth:			얼굴검출에 사용했던 영상 width
* long nPreviewHeight:			얼굴검출에 사용했던 영상 height
* FD_INFO *pFD_Info: 		FD_INFO
- [return]:					얼굴 개수
------------------------------------------------------------------------------------------*/
long SDIC_EyeDetection2(unsigned char* pData, long nDataWidth, long nDataHeight,
					   long nPreviewWidth, long nPreviewHeight, FD_INFO *pFD_Info)
{
	int i;
    double fsw;
    double fsh;
	// 입력 영상의 크기와 얼굴 검출에 사용된 영상의 크기가 같다면
	if(nDataWidth == nPreviewWidth && nDataHeight == nPreviewHeight) {
		return SDIC_EyeDetection(pData, nDataWidth, nDataHeight, pFD_Info);
	}

	// 두 영상의 가로, 세로에 대응하는 비를 구하여 FD_INFO에 곱함
	fsw = nDataWidth / nPreviewWidth;
	fsh = nDataHeight / nPreviewHeight;
	for (i = 0; i < pFD_Info->cnt; i++) {
		pFD_Info->boundRect[i].left = (long)(pFD_Info->boundRect[i].left * fsw);
		pFD_Info->boundRect[i].top = (long)(pFD_Info->boundRect[i].top * fsh);
		pFD_Info->boundRect[i].width = (long)(pFD_Info->boundRect[i].width * fsw);
		pFD_Info->boundRect[i].height = (long)(pFD_Info->boundRect[i].height * fsh);
	}
	return SDIC_EyeDetection(pData, nDataWidth, nDataHeight, pFD_Info);
}


/* ----------------------------------------------------------------------------------------
long SDIC_EyeDetection(unsigned char *pData, long nWidth, long nHeight, FD_INFO *pFD_Info);
- 눈 검출 수행 및 눈 위치 정보 저장
- [IN]
* unsigned char *pData:
GRAY 타입(8Bits)의 RAW 데이터(입력영상), angle에 따라 회전된 영상
데이터 크기: nWidth*nHeight
* long nWidth:				입력영상의 width
* long nHeight:				입력영상의 height
* FD_INFO *pFD_Info: 		FD_INFO
- [return]:					얼굴 개수
-----------------------------------------------------------------------------------------*/
/*
#ifdef WIN32
void viewimage(unsigned char *img, int w, int h)
{
	CImage image, color_image;
	CImageProcessing ip;
	ip.setraw(image, w, h, 1, img);
	ip.gray2bgr(image, color_image);
	cvNamedWindow("test");
	cvShowImage("test", color_image.GetImage());
	//cvWaitKey(0);
}
#endif
*/
long SDIC_EyeDetection(unsigned char *pData, long nWidth, long nHeight, FD_INFO *pFD_Info)
{
    //pData = FD_Ydata;
	int crop_sz, leye_sz, reye_sz, delta;
	int i, bEDFind;
	SMyRect rcFace, faceRegion;
	SMyRect rcLEye_org, rcREye_org;
	SMyPoint ptLEye_org, ptREye_org;
	SMyPoint ptLEye_rot, ptREye_rot;
	unsigned char *crop_faceData, *rotated_faceData;
	SMyRect crop_box;


    if(	pData == NULL)  return -5;

	for (i = 0; i < pFD_Info->cnt; i++)
	{
		/* 각각의 face에 대해서 눈 검출 수행을 위해 pFD_Info의 각각의 얼굴에 대해 rcFace로 assign*/
		rcFace.left = pFD_Info->boundRect[i].left;
		rcFace.right = pFD_Info->boundRect[i].left + pFD_Info->boundRect[i].width;
		rcFace.top = pFD_Info->boundRect[i].top;
		rcFace.bottom = pFD_Info->boundRect[i].top + pFD_Info->boundRect[i].height;

		// Set face region for eye detection
		// 입력은 angle로 회전된 영상이지만, 얼굴의 위치는 회전되지 않은 영상에 대한 위치이므로
		// 각각의 얼굴의 위치를 회전된 영상에 맞게 위치 회전을 해야함

		delta = (rcFace.right-rcFace.left)>>3;
		do
		{
			crop_box.left = ED_MAX(rcFace.left - delta, 0);
			crop_box.top  = ED_MAX(rcFace.top - delta, 0);
			crop_box.right = ED_MIN(rcFace.right + delta, nWidth-1);
			crop_box.bottom = ED_MIN(rcFace.bottom + delta, nHeight-1);
			--delta;
			if(delta < 0) goto NEXT_LOOP;			
		} while(crop_box.right-crop_box.left != crop_box.bottom-crop_box.top);

		crop_sz = (crop_box.right-crop_box.left);
		crop_faceData = (unsigned char *)myMalloc(crop_sz*crop_sz);
		rotated_faceData = (unsigned char *)myMalloc(crop_sz*crop_sz);
		sdic_Region(pData, nWidth, nHeight, crop_faceData, crop_box);
		sdic_Rotate(crop_faceData, crop_sz, crop_sz, pFD_Info->rip_angle[i], rotated_faceData);

		/////////////////////////////////////////////////////////
#ifdef _DEBUG
		{
// 			CImageProcessing m_ip;
// 			cv::Mat tempImage;
// 			m_ip.setraw(tempImage, crop_sz, crop_sz, 1, rotated_faceData);
// 			m_ip.save("crop_face.bmp", tempImage);
		}
#endif
		/////////////////////////////////////////////////////////

		faceRegion.left = 0; 
		faceRegion.top = 0;
		faceRegion.right = crop_sz;
		faceRegion.bottom = crop_sz;
		// Eye detection
		bEDFind = sdic_ed_find(rotated_faceData, crop_sz, crop_sz, faceRegion, pFD_Info->rop_angle[i]);

		pFD_Info->eyeL[i].enable = 0;
		pFD_Info->eyeR[i].enable = 0;

		// 0 - 눈 검출 실패
		// 1 - 두 눈 검출 성공
		// 2 - 오른쪽 눈 검출 실패
		// 3 - 왼쪽 눈 검출 실패
		if (bEDFind == 0) {		
			myFree(crop_faceData);
			myFree(rotated_faceData); 
			continue;
		}
		if (bEDFind == 1 || bEDFind == 2) pFD_Info->eyeL[i].enable = 1;
		if (bEDFind == 1 || bEDFind == 3) pFD_Info->eyeR[i].enable = 1;

		// 눈 위치를 얻어와서 가운데 점 설정
		sdic_ed_whereBox(&rcLEye_org, &rcREye_org);

		ptLEye_org.x = (rcLEye_org.left + rcLEye_org.right)>>1;
		ptLEye_org.y = (rcLEye_org.top + rcLEye_org.bottom)>>1;
		ptREye_org.x = (rcREye_org.left + rcREye_org.right)>>1;
		ptREye_org.y = (rcREye_org.top + rcREye_org.bottom)>>1;

		sdic_Position_Org2Angle(ptLEye_org, crop_sz >> 1, crop_sz >> 1, &ptLEye_rot, -pFD_Info->rip_angle[i]);
		sdic_Position_Org2Angle(ptREye_org, crop_sz >> 1, crop_sz >> 1, &ptREye_rot, -pFD_Info->rip_angle[i]);

		leye_sz = rcLEye_org.right - rcLEye_org.left;
		reye_sz = rcREye_org.right - rcREye_org.left;
		
		pFD_Info->eyeL[i].left = crop_box.left + ED_MAX(ptLEye_rot.x - (leye_sz>>1), 0);
		pFD_Info->eyeL[i].top  = crop_box.top + ED_MAX(ptLEye_rot.y - (leye_sz>>1), 0);
		pFD_Info->eyeL[i].width = leye_sz;
		pFD_Info->eyeL[i].height  = leye_sz;

		pFD_Info->eyeR[i].left = crop_box.left + ED_MAX(ptREye_rot.x - (reye_sz>>1), 0);
		pFD_Info->eyeR[i].top  = crop_box.top + ED_MAX(ptREye_rot.y - (reye_sz>>1), 0);
		pFD_Info->eyeR[i].width = reye_sz;
		pFD_Info->eyeR[i].height  = reye_sz;
		
		myFree(crop_faceData);
		myFree(rotated_faceData);

NEXT_LOOP:;
	}

	return pFD_Info->cnt;
}

void SDIC_FaceAngle(unsigned char*pData, long nWidth, long nHeight, FD_INFO *pFD_Info, int* nEyeAngle)
{
    int i;
	int ptLx, ptRx, ptLy, ptRy;
	int theta;
    double thetaPI;

	for (i = 0; i < pFD_Info->cnt; i++)
	{
        nEyeAngle[i] = 0;
		if (pFD_Info->eyeL[i].enable == 0 || pFD_Info->eyeR[i].enable == 0)
		{
			continue;
		}

		ptLx = pFD_Info->eyeL[i].left + ( pFD_Info->eyeL[i].width >> 1 );
		ptRx = pFD_Info->eyeR[i].left + ( pFD_Info->eyeR[i].width >> 1 );
		ptLy = pFD_Info->eyeL[i].top + ( pFD_Info->eyeL[i].height >> 1 );
		ptRy = pFD_Info->eyeR[i].top + ( pFD_Info->eyeR[i].height >> 1 );

		thetaPI = atan( ( (double)ptRy - ptLy ) / ( ptRx - ptLx ) );
		theta = (int)(thetaPI * 57 + 1);  //		theta = (int)(thetaPI * 180 / 3.141592 + 0.5);

		//pFD_Info->angle[i] = theta;
		nEyeAngle[i] = theta;
	}
}


/* -----------------------------------------------------------------------------------
int sdic_ed_find(unsigned char* srcData, int src_w, int src_h,  SMyRect rcFace);
- 얼굴 영역 내에서 눈 검출 수행
- [IN]
* unsigned char* srcData:
GRAY 타입(8Bits)의 RAW 데이터(입력영상)
데이터 크기: src_w*src_h
* int src_w:				입력영상의 width
* int src_h:				입력영상의 height
*  SMyRect rcFace: 	검출된 얼굴의 사각형 영역
- [return]:					눈의 검출 여부 (양쪽 눈이 모두 검출 되어야 1 반환)
------------------------------------------------------------------------------------*/
int sdic_ed_find(unsigned char* srcData, int src_w, int src_h,  SMyRect rcFace, int rop_angle)
{
	int i, ED_MAXCountIdx;
	int nResizeW, nResizeH;
	double fRatioHeightPerWidth;
	int bFoundLeft = 1, bFoundRight = 1;

	int nFaceW, nFaceH;
	int nEyeCandidateLeftW, nEyeCandidateLeftH;
	int nEyeCandidateRightW, nEyeCandidateRightH;
	 SMyRect rcEyeCandidateLeft, rcEyeCandidateRight;

	unsigned char *m_nEyeCandidateLeftData;
	unsigned char *m_nEyeCandidateRightData;
	unsigned char *m_nEyeCandidateRightData_Flip;

	unsigned char *m_nEDResizeData;
	unsigned short *m_nEDMCTData;

	// 눈 위치 초기화
	m_ed_rcLeft.left = m_ed_rcLeft.top = m_ed_rcLeft.right = m_ed_rcLeft.bottom = 0;
	m_ed_rcRight.left = m_ed_rcRight.top = m_ed_rcRight.right = m_ed_rcRight.bottom = 0;

	m_nEyeLeftCandidateCount = 0; m_nEyeRightCandidateCount = 0;
	memset(m_nEyeLeftCandidateIntersectionCount, 0, sizeof(int)*ED_MAX_FIND);
	memset(m_nEyeRightCandidateIntersectionCount, 0, sizeof(int)*ED_MAX_FIND);
	memset(m_nEyeLeftCandidateConfidence, 0, sizeof(int)*ED_MAX_FIND);
	memset(m_nEyeRightCandidateConfidence, 0, sizeof(int)*ED_MAX_FIND);
	memset(m_bEyeLeftCandidateValid, 0, sizeof(int)*ED_MAX_FIND);
	memset(m_bEyeRightCandidateValid, 0, sizeof(int)*ED_MAX_FIND);

	nFaceW = rcFace.right - rcFace.left;
	nFaceH = rcFace.bottom - rcFace.top;

	// 얼굴 영역에서 눈 검출 영역 추출
	rcEyeCandidateLeft.left = rcFace.left;
	rcEyeCandidateLeft.top = rcFace.top;
	//rcEyeCandidateLeft.right = rcFace.left + (int)(nFaceW / 2.0  + 0.5);
	if (rop_angle < 0) {
		int half = nFaceW / 2.0;
		int residual = sin(min(max(-rop_angle*0.9, 0), 90) * 3.141592 / 180.0) * half;
		rcEyeCandidateLeft.right = rcFace.left +half - residual ;
	}
	else {
		int half = nFaceW / 2.0;
		int residual = sin(min(max(rop_angle*0.9, 0), 90) * 3.141592 / 180.0) * half;
		rcEyeCandidateLeft.right = rcFace.left + half + residual;
	}

	rcEyeCandidateLeft.bottom = rcFace.top + (int)(nFaceH * 0.6667 + 0.5);


	rcEyeCandidateRight.left = rcEyeCandidateLeft.right;
	rcEyeCandidateRight.top = rcEyeCandidateLeft.top;
	rcEyeCandidateRight.right = rcFace.right;
	rcEyeCandidateRight.bottom = rcEyeCandidateLeft.bottom;

	// residual 추가
	// 왼쪽 눈, 오른쪽 눈 candidate 영역을 겹치게 해줌
	rcEyeCandidateLeft.right += (int)(nFaceW * 0.1);
	rcEyeCandidateRight.left -=  (int)(nFaceW * 0.1);


	m_ed_rcEyeCandidateLeft = rcEyeCandidateLeft;
	m_ed_rcEyeCandidateRight = rcEyeCandidateRight;

	nEyeCandidateLeftW = rcEyeCandidateLeft.right - rcEyeCandidateLeft.left;
	nEyeCandidateLeftH = rcEyeCandidateLeft.bottom - rcEyeCandidateLeft.top;
	nEyeCandidateRightW = rcEyeCandidateRight.right - rcEyeCandidateRight.left;
	nEyeCandidateRightH = rcEyeCandidateRight.bottom - rcEyeCandidateRight.top;

	m_nEyeCandidateLeftData = (unsigned char*)myMalloc(nEyeCandidateLeftW*nEyeCandidateLeftH);
	m_nEyeCandidateRightData = (unsigned char*)myMalloc(nEyeCandidateRightW*nEyeCandidateRightH);
	m_nEyeCandidateRightData_Flip = (unsigned char*)myMalloc(nEyeCandidateRightW*nEyeCandidateRightH);





	// 눈 영역 추출이 안되면 return 0
	// 오른쪽 눈 영역은 영상 flip을 하여 왼쪽 눈 검출기 사용
	int left_result = sdic_Region(srcData, src_w, src_h, m_nEyeCandidateLeftData, rcEyeCandidateLeft);
	int right_result = sdic_Region(srcData, src_w, src_h, m_nEyeCandidateRightData, rcEyeCandidateRight);

	//////////////////////////////////////////
#ifdef _DEBUG
// 	CImageProcessing m_ip;
// 	cv::Mat tempImage;
// 	m_ip.setraw(tempImage, nEyeCandidateLeftW, nEyeCandidateLeftH, 1, m_nEyeCandidateLeftData);
// 	m_ip.save("left_eye.bmp", tempImage);
// 	m_ip.setraw(tempImage, nEyeCandidateRightW, nEyeCandidateRightH, 1, m_nEyeCandidateRightData);
// 	m_ip.save("right_eye.bmp", tempImage);
#endif

	if(left_result == 0 || right_result == 0)
	{


		myFree(m_nEyeCandidateLeftData); myFree(m_nEyeCandidateRightData); myFree(m_nEyeCandidateRightData_Flip);
		return 0;;
	}
	sdic_FlipX(m_nEyeCandidateRightData, nEyeCandidateRightW, nEyeCandidateRightH, m_nEyeCandidateRightData_Flip);



	// To localize
	m_nEyeLeftTotalLimit = 200, m_nEyeRightTotalLimit = 200;
	m_nEyeLeftTotal = 0, m_nEyeRightTotal = 0;
	m_nEyePositionLeft_Left = (unsigned short*)myMalloc(sizeof(unsigned short)*m_nEyeLeftTotalLimit);
	m_nEyePositionLeft_Top = (unsigned short*)myMalloc(sizeof(unsigned short)*m_nEyeLeftTotalLimit);
	m_nEyePositionLeft_Right = (unsigned short*)myMalloc(sizeof(unsigned short)*m_nEyeLeftTotalLimit);
	m_nEyePositionLeft_Bottom = (unsigned short*)myMalloc(sizeof(unsigned short)*m_nEyeLeftTotalLimit);

	m_nEyePositionRight_Left = (unsigned short*)myMalloc(sizeof(unsigned short)*m_nEyeRightTotalLimit);
	m_nEyePositionRight_Top = (unsigned short*)myMalloc(sizeof(unsigned short)*m_nEyeRightTotalLimit);
	m_nEyePositionRight_Right = (unsigned short*)myMalloc(sizeof(unsigned short)*m_nEyeRightTotalLimit);
	m_nEyePositionRight_Bottom = (unsigned short*)myMalloc(sizeof(unsigned short)*m_nEyeRightTotalLimit);

	m_nEyeClassLeft = (unsigned char*)myMalloc(sizeof(unsigned char)*m_nEyeLeftTotalLimit);
	m_nEyeClassRight = (unsigned char*)myMalloc(sizeof(unsigned char)*m_nEyeRightTotalLimit);
	m_nEyeConfidenceLeft = (unsigned short*)myMalloc(sizeof(unsigned short)*m_nEyeLeftTotalLimit);
	m_nEyeConfidenceRight = (unsigned short*)myMalloc(sizeof(unsigned short)*m_nEyeRightTotalLimit);

	/**************		Left	********************/
	fRatioHeightPerWidth = (double)nEyeCandidateLeftH/(double)nEyeCandidateLeftW;
	nResizeW = ED_BEST_SIZE;
	nResizeH = (int)(nResizeW * fRatioHeightPerWidth + 0.5);

	m_nEDResizeData = (unsigned char*)myMalloc(nResizeW*nResizeH);;
	m_nEDMCTData = (unsigned short*)myMalloc(sizeof(unsigned short)*nResizeW*nResizeH);

	while(1) {
		/*
		// 최소 검출 크기 조절
		*/
		if(nResizeW <= ED_FRONTAL_MCT_WINDOW_WIDTH * 1.5 || nResizeH <= ED_FRONTAL_MCT_WINDOW_HEIGHT * 1.5) break;

		// Image pyramid 생성 및 각 image에 대해 MCT 변환
		sdic_Resize_BI(m_nEyeCandidateLeftData, nEyeCandidateLeftW, nEyeCandidateLeftH, m_nEDResizeData, nResizeW, nResizeH);
		sdic_image2MCT9_1(m_nEDResizeData, nResizeW, nResizeH, m_nEDMCTData);
		// 눈 검출기 및 감은 눈 검출기 수행
		sdic_ed_scan(nResizeW, nResizeH, nEyeCandidateLeftW, nEyeCandidateLeftH, m_nEDMCTData, 0);
		nResizeW = nResizeW - ED_PYRAMID_STEP;
		nResizeH = (int)(nResizeW * fRatioHeightPerWidth + 0.5);
	}
	myFree( m_nEDResizeData ); myFree( m_nEDMCTData );

	/**************		Right	********************/
	fRatioHeightPerWidth = (double)nEyeCandidateRightH/(double)nEyeCandidateRightW;
	nResizeW = ED_BEST_SIZE;
	nResizeH = (int)(nResizeW * fRatioHeightPerWidth + 0.5);

	m_nEDResizeData = (unsigned char*)myMalloc(nResizeW*nResizeH);
	m_nEDMCTData = (unsigned short*)myMalloc(sizeof(unsigned short)*nResizeW*nResizeH);

	while(1) {
		/*
		// 최소 검출 크기 조절
		*/
		if(nResizeW <= ED_FRONTAL_MCT_WINDOW_WIDTH * 1.5 || nResizeH <= ED_FRONTAL_MCT_WINDOW_HEIGHT * 1.5) break;

		sdic_Resize_BI(m_nEyeCandidateRightData_Flip,nEyeCandidateRightW,nEyeCandidateRightH,m_nEDResizeData,nResizeW,nResizeH);
		sdic_image2MCT9_1(m_nEDResizeData, nResizeW, nResizeH, m_nEDMCTData);
		sdic_ed_scan(nResizeW, nResizeH, nEyeCandidateRightW, nEyeCandidateRightH, m_nEDMCTData, 1);
		nResizeW = nResizeW - ED_PYRAMID_STEP;
		nResizeH = (int)(nResizeW * fRatioHeightPerWidth + 0.5);
	}

	myFree(m_nEDResizeData);
	myFree(m_nEDMCTData);
	myFree(m_nEyeCandidateLeftData);
	myFree(m_nEyeCandidateRightData);
	myFree(m_nEyeCandidateRightData_Flip);

	// 왼쪽, 오른쪽 눈 영역을 검색하여 눈 후보가 없으면
	if(m_nEyeLeftCandidateCount == 0) bFoundLeft = 0;
	if(m_nEyeRightCandidateCount == 0) bFoundRight = 0;

	// 왼쪽 눈 영역에 대해서 눈 후보들 중에 intersection이 가장 많은 후보를 왼쪽 눈으로 선택
	ED_MAXCountIdx = 0;
	for(i=0; i<m_nEyeLeftCandidateCount; i++) {
		if(m_bEyeLeftCandidateValid[i]) {
			if(m_nEyeLeftCandidateIntersectionCount[i] > m_nEyeLeftCandidateIntersectionCount[ED_MAXCountIdx]) {
				ED_MAXCountIdx = i;
			}
		}
	}
	m_ed_rcLeft = m_ed_rcLeftCandidate[ED_MAXCountIdx];
	if (m_nEyeLeftCandidateIntersectionCount[ED_MAXCountIdx] < 3 ) {
		bFoundLeft = 0;
	}
	else sdic_ed_localizeLeft(ED_MAXCountIdx);

	// 오른쪽 눈 영역에 대해서 눈 후보들 중에 intersection이 가장 많은 후보를 오른쪽 눈으로 선택
	ED_MAXCountIdx = 0;
	for(i=0; i<m_nEyeRightCandidateCount; i++) {
		if(m_bEyeRightCandidateValid[i]) {
			if(m_nEyeRightCandidateIntersectionCount[i] > m_nEyeRightCandidateIntersectionCount[ED_MAXCountIdx]) {
				ED_MAXCountIdx = i;
			}
		}
	}
	m_ed_rcRight = m_ed_rcRightCandidate[ED_MAXCountIdx];
	if (m_nEyeRightCandidateIntersectionCount[ED_MAXCountIdx] < 3 ) {
		bFoundRight = 0;
	}
	else sdic_ed_localizeRight(ED_MAXCountIdx);

	myFree(m_nEyePositionLeft_Left); myFree(m_nEyePositionLeft_Top); myFree(m_nEyePositionLeft_Right); myFree(m_nEyePositionLeft_Bottom);
	myFree(m_nEyePositionRight_Left); myFree(m_nEyePositionRight_Top); myFree(m_nEyePositionRight_Right); myFree(m_nEyePositionRight_Bottom);
	myFree(m_nEyeClassLeft); myFree(m_nEyeClassRight);
	myFree(m_nEyeConfidenceLeft); myFree(m_nEyeConfidenceRight);

	if ( bFoundLeft == 1 && bFoundRight == 1 )
 		return 1;
 	else if ( bFoundLeft == 1 && bFoundRight == 0 )
 		return 2;
 	else if ( bFoundLeft == 0 && bFoundRight == 1 )
 		return 3;

 	return 0;
}

/* -------------------------------------------------------------------------------------
int sdic_ed_whereBox( SMyRect* rectLEye,  SMyRect* rectREye);
- 검출된 눈의 사각형 영역 정보를 얻음
- [OUT]
*  SMyRect* rectLEye:	sdic_ed_find( )에서 얻은 왼쪽 눈 사각형 위치 정보를 가져옴
*  SMyRect* rectREye:	sdic_ed_find( )에서 얻은 오른쪽 눈 사각형 위치 정보를 가져옴
- [return]:
0->정상적으로 종료되면 0 반환
------------------------------------------------------------------------------------- */
int sdic_ed_whereBox( SMyRect* rectLEye,  SMyRect* rectREye)
{
	rectLEye->left = m_ed_rcEyeCandidateLeft.left + m_ed_rcLeft.left;
	rectLEye->top = m_ed_rcEyeCandidateLeft.top + m_ed_rcLeft.top;
	rectLEye->right = m_ed_rcEyeCandidateLeft.left + m_ed_rcLeft.right;
	rectLEye->bottom = m_ed_rcEyeCandidateLeft.top + m_ed_rcLeft.bottom;

	rectREye->left = m_ed_rcEyeCandidateRight.right - m_ed_rcRight.right;
	rectREye->top = m_ed_rcEyeCandidateRight.top + m_ed_rcRight.top;
	rectREye->right = m_ed_rcEyeCandidateRight.right - m_ed_rcRight.left;
	rectREye->bottom = m_ed_rcEyeCandidateRight.top + m_ed_rcRight.bottom;
	return 0;
}

/* -------------------------------------------------------------------------------------
int sdic_ed_where( SMyPoint* ptLEye,  SMyPoint* ptREye);
- 검출된 눈의 가운데 위치 정보를 얻음
- [OUT]
*  SMyPoint* ptLEye:	sdic_ed_find( )에서 얻은 왼쪽 눈 가운데 위치 정보를 가져옴
*  SMyPoint* ptREye:	sdic_ed_find( )에서 얻은 오른쪽 눈 가운데 위치 정보를 가져옴
- [return]:
0->정상적으로 종료되면 0 반환
------------------------------------------------------------------------------------- */
int sdic_ed_where( SMyPoint* ptLEye,  SMyPoint* ptREye)
{
	ptLEye->x = m_ed_rcEyeCandidateLeft.left + m_ed_rcLeft.left + ((m_ed_rcLeft.right - m_ed_rcLeft.left)>>1);
	ptLEye->y = m_ed_rcEyeCandidateLeft.top + m_ed_rcLeft.top + ((m_ed_rcLeft.bottom - m_ed_rcLeft.top)>>1);
	ptREye->x = m_ed_rcEyeCandidateRight.right - (m_ed_rcRight.left + ((m_ed_rcRight.right - m_ed_rcRight.left)>>1));
	ptREye->y = m_ed_rcEyeCandidateRight.top + m_ed_rcRight.top + ((m_ed_rcRight.bottom - m_ed_rcRight.top)>>1);
	return 0;
}

/* ---------------------------------------------------------------------------------------------------------
void sdic_ed_scan(int nResizeW, int nResizeH, int nSrcW, int nSrcH, unsigned short* nMCTData, int bIsRight);
- MCT 데이터에서 window를 1 pixel씩 이동을 하며 눈 후보 영역 검출
- 검출한 결과는 왼쪽과 오른쪽을 구분하여 후보(global 변수)로 등록
- [IN]
* int nResizeW:					Scan 할 영상의 width
* int nResizeH:					Scan 할 영상의 height
* int nSrcW:					입력 영상의 width
* int nSrcH:					입력 영상의 height
* unsigned short* nMCTData:		Scan 할 영상의 MCT data
* int bIsRight:					왼쪽 눈 인지 오른쪽 눈 인지 구분
- [return]:						없음
--------------------------------------------------------------------------------------------------------- */
void sdic_ed_scan(int nResizeW, int nResizeH, int nSrcW, int nSrcH, unsigned short* nMCTData, int bIsRight)
{
	int x, y, t, c;
	int bFind;
	int nSearchW, nSearchH;


	int nMCTDataStep;
	int nRatioSrcPerResize = (nSrcW<<ED_BIT_SHIFT)/nResizeW;
	int nEyeX, nEyeY, nEyeW, nEyeH;
	int nMCTAlphaCount=0;
	int nMCTConfidence, nMCTOneCascadeFeatureSize;
	unsigned short *pMCTData = nMCTData;
	unsigned short *pMCTFeatureY, *pMCTFeatureX;
	int *nMCTFeatureGlobalPosition = (int*)myMalloc(sizeof(int)*ED_FRONTAL_MCT_FEATURE_SIZE_TOTAL);
	int *pMCTFeatureGlobalPosition = nMCTFeatureGlobalPosition;
	nSearchW = nResizeW-ED_FRONTAL_MCT_WINDOW_WIDTH;
	nSearchH = nResizeH-ED_FRONTAL_MCT_WINDOW_HEIGHT;
	nMCTDataStep=ED_FRONTAL_MCT_WINDOW_WIDTH;
	nEyeW = (ED_FRONTAL_MCT_WINDOW_WIDTH*nRatioSrcPerResize)>>ED_BIT_SHIFT;
	nEyeH = (ED_FRONTAL_MCT_WINDOW_HEIGHT*nRatioSrcPerResize)>>ED_BIT_SHIFT;
	pMCTFeatureY = m_nEyeMCTFeatureY, pMCTFeatureX = m_nEyeMCTFeatureX;

	for(t=0; t<ED_FRONTAL_MCT_FEATURE_SIZE_TOTAL;) {
		*pMCTFeatureGlobalPosition++ = *pMCTFeatureY++ * nResizeW + *pMCTFeatureX++;
		*pMCTFeatureGlobalPosition++ = *pMCTFeatureY++ * nResizeW + *pMCTFeatureX++;
		t+=2;
	}

	// pixel (x,y)를 window의 left top으로 지정하여 window 내의 MCT값들의 합과 미리 학습한 threshold를 비교
	for(y=0; y<nSearchH; y++) {
		for(x=0; x<nSearchW; x++) {
			nMCTAlphaCount = 0;
			pMCTFeatureGlobalPosition = nMCTFeatureGlobalPosition;

			// 속도 향상을 위해서 cascade 적용
			for(c=0; c<ED_FRONTAL_MCT_CASCADE_SIZE; c++) {
				nMCTConfidence = 0;
				bFind = 0;
				nMCTOneCascadeFeatureSize = ED_FRONTAL_MCT_FEATURE_SIZE[c];

				// 각 cascade에 해당하는 feature들의 MCT값에 대한 합을 구함
				for(t=0; t<nMCTOneCascadeFeatureSize; ) {
					nMCTConfidence += m_nEDMCTAlpha[(nMCTAlphaCount + *(pMCTData + *pMCTFeatureGlobalPosition++))];
					nMCTAlphaCount += ED_FRONTAL_MCT_VALUE_RANGE;

					nMCTConfidence += m_nEDMCTAlpha[(nMCTAlphaCount + *(pMCTData + *pMCTFeatureGlobalPosition++))];
					nMCTAlphaCount += ED_FRONTAL_MCT_VALUE_RANGE;
					t+=2;
				}

				// MCT threshold 보다 window의 MCT 값이 작으면 눈 후보를 찾은 것으로 간주
				if(nMCTConfidence > ED_FRONTAL_MCT_THRESHOLD[c]) break;

				bFind = 1;
			}
			// 눈 후보를 찾았다면 후보를 등록
			if(bFind == 1) {
				nEyeX = (x*nRatioSrcPerResize)>>ED_BIT_SHIFT;
				nEyeY = (y*nRatioSrcPerResize)>>ED_BIT_SHIFT;

				// 왼쪽, 오른쪽을 구분하여 등록
				if(bIsRight) sdic_ed_enrollRight(nEyeX, nEyeY, nEyeW, nEyeH, nMCTConfidence);
				else sdic_ed_enrollLeft(nEyeX, nEyeY, nEyeW, nEyeH, nMCTConfidence);
			}
			++pMCTData;
		}
		pMCTData += nMCTDataStep;
	}

	myFree(nMCTFeatureGlobalPosition);
}

/* ----------------------------------------------------------------------------------
void sdic_ed_enrollLeft(int nEyeX, int nEyeY, int nEyeW, int nEyeH, int nConfidence);
- 검출한 왼쪽 눈 영역을 후보에 등록
- [IN]
* int nEyeX:					왼쪽 눈 영역의 x 좌표
* int nEyeY:					왼쪽 눈 영역의 y 좌표
* int nEyeW:					왼쪽 눈 영역의 폭
* int nEyeH:					왼쪽 눈 영역의 높이
* int nConfidence:				검출한 왼쪽 눈 영역의 mct값
* int isClosed:					눈 검출기, 감은 눈 검출기 중 어느 결과인지
- [return]:						없음
---------------------------------------------------------------------------------- */
void sdic_ed_enrollLeft(int nEyeX, int nEyeY, int nEyeW, int nEyeH, int nConfidence)
{
	int i, j, k;
	unsigned char *pClass;
	int bIntersection = 0;
	 SMyRect rcEye;
    unsigned short *m_nEyePositionLeft_Left_new;
    unsigned short *m_nEyePositionLeft_Right_new;
    unsigned short *m_nEyePositionLeft_Top_new;
    unsigned short *m_nEyePositionLeft_Bottom_new;
    unsigned char *m_nEyeClassLeft_new;
    unsigned short *m_nEyeConfidenceLeft_new;

	if(ED_MAX_FIND <= m_nEyeLeftCandidateCount) {
		return;
	}
	rcEye.left = nEyeX;
	rcEye.top = nEyeY;
	rcEye.right = nEyeX + nEyeW;
	rcEye.bottom = nEyeY + nEyeH;



	// 기존에 등록된 후보와 겹치면서 MCT 값이 낮다면
	// 기존 후보를 현재 등록하려는 후보로 교체하고 intersection count를 증가
	for(i=0; i<m_nEyeLeftCandidateCount; i++) {
		if(m_bEyeLeftCandidateValid[i] == 0) continue;
		if(sdic_ed_isintersection(m_ed_rcLeftCandidate[i], rcEye)) {
			bIntersection = 1;
			++m_nEyeLeftCandidateIntersectionCount[i];
			// Add new detected left eye
			if (m_nEyeLeftTotal == m_nEyeLeftTotalLimit) {
				m_nEyeLeftTotalLimit += 100;
				m_nEyePositionLeft_Left_new = (unsigned short*)myMalloc(sizeof(unsigned short)*m_nEyeLeftTotalLimit);
				memcpy(m_nEyePositionLeft_Left_new, m_nEyePositionLeft_Left, sizeof(unsigned short)*(m_nEyeLeftTotalLimit-100));
				myFree(m_nEyePositionLeft_Left); m_nEyePositionLeft_Left = m_nEyePositionLeft_Left_new; m_nEyePositionLeft_Left_new = NULL;

				m_nEyePositionLeft_Right_new = (unsigned short*)myMalloc(sizeof(unsigned short)*m_nEyeLeftTotalLimit);
				memcpy(m_nEyePositionLeft_Right_new, m_nEyePositionLeft_Left, sizeof(unsigned short)*(m_nEyeLeftTotalLimit-100));
				myFree(m_nEyePositionLeft_Right); m_nEyePositionLeft_Right = m_nEyePositionLeft_Right_new; m_nEyePositionLeft_Right_new = NULL;

				m_nEyePositionLeft_Top_new = (unsigned short*)myMalloc(sizeof(unsigned short)*m_nEyeLeftTotalLimit);
				memcpy(m_nEyePositionLeft_Top_new, m_nEyePositionLeft_Top, sizeof(unsigned short)*(m_nEyeLeftTotalLimit-100));
				myFree(m_nEyePositionLeft_Top); m_nEyePositionLeft_Top = m_nEyePositionLeft_Top_new; m_nEyePositionLeft_Top_new = NULL;

				m_nEyePositionLeft_Bottom_new = (unsigned short*)myMalloc(sizeof(unsigned short)*m_nEyeLeftTotalLimit);
				memcpy(m_nEyePositionLeft_Bottom_new, m_nEyePositionLeft_Bottom, sizeof(unsigned short)*(m_nEyeLeftTotalLimit-100));
				myFree(m_nEyePositionLeft_Bottom); m_nEyePositionLeft_Bottom = m_nEyePositionLeft_Bottom_new; m_nEyePositionLeft_Bottom_new = NULL;

				m_nEyeClassLeft_new = (unsigned char*)myMalloc(sizeof(unsigned char)*m_nEyeLeftTotalLimit);
				memcpy(m_nEyeClassLeft_new, m_nEyeClassLeft, sizeof(unsigned char)*(m_nEyeLeftTotalLimit-100));
				myFree(m_nEyeClassLeft); m_nEyeClassLeft = m_nEyeClassLeft_new; m_nEyeClassLeft_new = NULL;

				m_nEyeConfidenceLeft_new = (unsigned short*)myMalloc(sizeof(unsigned short)*m_nEyeLeftTotalLimit);
				memcpy(m_nEyeConfidenceLeft_new, m_nEyeConfidenceLeft, sizeof(unsigned short)*(m_nEyeLeftTotalLimit-100));
				myFree(m_nEyeConfidenceLeft); m_nEyeConfidenceLeft = m_nEyeConfidenceLeft_new; m_nEyeConfidenceLeft_new = NULL;
			}
			m_nEyePositionLeft_Left[m_nEyeLeftTotal] = nEyeX;
			m_nEyePositionLeft_Top[m_nEyeLeftTotal] = nEyeY;
			m_nEyePositionLeft_Right[m_nEyeLeftTotal] = nEyeX + nEyeW;
			m_nEyePositionLeft_Bottom[m_nEyeLeftTotal] = nEyeY + nEyeH;
			m_nEyeClassLeft[m_nEyeLeftTotal] = i;
			m_nEyeConfidenceLeft[m_nEyeLeftTotal] = nConfidence;
			++m_nEyeLeftTotal;
			//
			if(m_nEyeLeftCandidateConfidence[i] > nConfidence ) {
				m_ed_rcLeftCandidate[i] = rcEye;
				m_nEyeLeftCandidateConfidence[i] = nConfidence;
			}
			break;
		}
	}

	// 겹치는 후보들이 없다면 새로운 후보로 등록
	if(bIntersection == 0) {
		m_ed_rcLeftCandidate[m_nEyeLeftCandidateCount] = rcEye;
		++m_nEyeLeftCandidateIntersectionCount[m_nEyeLeftCandidateCount];
		m_nEyeLeftCandidateConfidence[m_nEyeLeftCandidateCount] = nConfidence;
		m_bEyeLeftCandidateValid[m_nEyeLeftCandidateCount] = 1;
		++m_nEyeLeftCandidateCount;
		// Add new detected left eye
		if (m_nEyeLeftTotal == m_nEyeLeftTotalLimit) {
			m_nEyeLeftTotalLimit += 100;
			m_nEyePositionLeft_Left_new = (unsigned short*)myMalloc(sizeof(unsigned short)*m_nEyeLeftTotalLimit);
			memcpy(m_nEyePositionLeft_Left_new, m_nEyePositionLeft_Left, sizeof(unsigned short)*(m_nEyeLeftTotalLimit-100));
			myFree(m_nEyePositionLeft_Left); m_nEyePositionLeft_Left = m_nEyePositionLeft_Left_new; m_nEyePositionLeft_Left_new = NULL;

			m_nEyePositionLeft_Right_new = (unsigned short*)myMalloc(sizeof(unsigned short)*m_nEyeLeftTotalLimit);
			memcpy(m_nEyePositionLeft_Right_new, m_nEyePositionLeft_Left, sizeof(unsigned short)*(m_nEyeLeftTotalLimit-100));
			myFree(m_nEyePositionLeft_Right); m_nEyePositionLeft_Right = m_nEyePositionLeft_Right_new; m_nEyePositionLeft_Right_new = NULL;

			m_nEyePositionLeft_Top_new = (unsigned short*)myMalloc(sizeof(unsigned short)*m_nEyeLeftTotalLimit);
			memcpy(m_nEyePositionLeft_Top_new, m_nEyePositionLeft_Top, sizeof(unsigned short)*(m_nEyeLeftTotalLimit-100));
			myFree(m_nEyePositionLeft_Top); m_nEyePositionLeft_Top = m_nEyePositionLeft_Top_new; m_nEyePositionLeft_Top_new = NULL;

			m_nEyePositionLeft_Bottom_new = (unsigned short*)myMalloc(sizeof(unsigned short)*m_nEyeLeftTotalLimit);
			memcpy(m_nEyePositionLeft_Bottom_new, m_nEyePositionLeft_Bottom, sizeof(unsigned short)*(m_nEyeLeftTotalLimit-100));
			myFree(m_nEyePositionLeft_Bottom); m_nEyePositionLeft_Bottom = m_nEyePositionLeft_Bottom_new; m_nEyePositionLeft_Bottom_new = NULL;

			m_nEyeClassLeft_new = (unsigned char*)myMalloc(sizeof(unsigned char)*m_nEyeLeftTotalLimit);
			memcpy(m_nEyeClassLeft_new, m_nEyeClassLeft, sizeof(unsigned char)*(m_nEyeLeftTotalLimit-100));
			myFree(m_nEyeClassLeft); m_nEyeClassLeft = m_nEyeClassLeft_new; m_nEyeClassLeft_new = NULL;

			m_nEyeConfidenceLeft_new = (unsigned short*)myMalloc(sizeof(unsigned short)*m_nEyeLeftTotalLimit);
			memcpy(m_nEyeConfidenceLeft_new, m_nEyeConfidenceLeft, sizeof(unsigned short)*(m_nEyeLeftTotalLimit-100));
			myFree(m_nEyeConfidenceLeft); m_nEyeConfidenceLeft = m_nEyeConfidenceLeft_new; m_nEyeConfidenceLeft_new = NULL;
		}
		m_nEyePositionLeft_Left[m_nEyeLeftTotal] = nEyeX;
		m_nEyePositionLeft_Top[m_nEyeLeftTotal] = nEyeY;
		m_nEyePositionLeft_Right[m_nEyeLeftTotal] = nEyeX + nEyeW;
		m_nEyePositionLeft_Bottom[m_nEyeLeftTotal] = nEyeY + nEyeH;
		m_nEyeClassLeft[m_nEyeLeftTotal] = i;
		m_nEyeConfidenceLeft[m_nEyeLeftTotal] = nConfidence;
		++m_nEyeLeftTotal;
		//
	}

	// 기존에 등록되어 있는 후보 및 새로 등록한 후보들에 대해서 겹치는 후보들을 모두 합침
	// MCT 값이 낮은 후보로 교체하고 intersection count를 합침
	for(i=0; i<m_nEyeLeftCandidateCount; i++) {
		if(m_bEyeLeftCandidateValid[i] == 0) continue;
		for(j=i+1; j<m_nEyeLeftCandidateCount; j++) {
			if(m_bEyeLeftCandidateValid[j] == 0) continue;
			if(sdic_ed_isintersection(m_ed_rcLeftCandidate[i], m_ed_rcLeftCandidate[j]) == 0) continue;
			if(m_nEyeLeftCandidateConfidence[i] > m_nEyeLeftCandidateConfidence[j]) {	//valid is j
				m_nEyeLeftCandidateIntersectionCount[j] += m_nEyeLeftCandidateIntersectionCount[i];
				m_bEyeLeftCandidateValid[i] = 0;
				// Re-assign class number to j
				for (pClass = m_nEyeClassLeft, k = m_nEyeLeftTotal; k--; ) {
					if (*pClass == i) *pClass = j;
					++pClass;
				}
			}
			else { //valid is i
				m_nEyeLeftCandidateIntersectionCount[i] += m_nEyeLeftCandidateIntersectionCount[j];
				m_bEyeLeftCandidateValid[j] = 0;
				// Re-assign class number to i
				for (pClass = m_nEyeClassLeft, k = m_nEyeLeftTotal; k--; ) {
					if (*pClass == j) *pClass = i;
					++pClass;
				}
			}
		}
	}
}

/* -----------------------------------------------------------------------------------
void sdic_ed_enrollRight(int nEyeX, int nEyeY, int nEyeW, int nEyeH, int nConfidence);
- 검출한 오른쪽 눈 영역을 후보에 등록
- [IN]
* int nEyeX:					오른쪽 눈 영역의 x 좌표
* int nEyeY:					오른쪽 눈 영역의 y 좌표
* int nEyeW:					오른쪽 눈 영역의 폭
* int nEyeH:					오른쪽 눈 영역의 높이
* int nConfidence:				검출한 오른쪽 눈 영역의 mct값
* int isClosed:					눈 검출기, 감은 눈 검출기 중 어느 결과인지
- [return]:						없음
----------------------------------------------------------------------------------- */
void sdic_ed_enrollRight(int nEyeX, int nEyeY, int nEyeW, int nEyeH, int nConfidence)
{
	int i, j, k;
	unsigned char *pClass;
	int bIntersection = 0;
	 SMyRect rcEye;

    unsigned short *m_nEyePositionRight_Left_new;
    unsigned short *m_nEyePositionRight_Right_new;
    unsigned short *m_nEyePositionRight_Top_new;
    unsigned short *m_nEyePositionRight_Bottom_new;
    unsigned char *m_nEyeClassRight_new;
    unsigned short *m_nEyeConfidenceRight_new;

	if(ED_MAX_FIND <= m_nEyeRightCandidateCount) {
		return;
	}
	rcEye.left = nEyeX;
	rcEye.top = nEyeY;
	rcEye.right = nEyeX + nEyeW;
	rcEye.bottom = nEyeY + nEyeH;

	for(i=0; i<m_nEyeRightCandidateCount; i++) {
		if(m_bEyeRightCandidateValid[i] == 0) continue;
		if(sdic_ed_isintersection(m_ed_rcRightCandidate[i], rcEye)) {
			bIntersection = 1;
			++m_nEyeRightCandidateIntersectionCount[i];
			// Add new detected right eye
			if (m_nEyeRightTotal == m_nEyeRightTotalLimit) {
				m_nEyeRightTotalLimit += 100;
				m_nEyePositionRight_Left_new = (unsigned short*)myMalloc(sizeof(unsigned short)*m_nEyeRightTotalLimit);
				memcpy(m_nEyePositionRight_Left_new, m_nEyePositionRight_Left, sizeof(unsigned short)*(m_nEyeRightTotalLimit-100));
				myFree(m_nEyePositionRight_Left); m_nEyePositionRight_Left = m_nEyePositionRight_Left_new; m_nEyePositionRight_Left_new = NULL;

				m_nEyePositionRight_Right_new = (unsigned short*)myMalloc(sizeof(unsigned short)*m_nEyeRightTotalLimit);
				memcpy(m_nEyePositionRight_Right_new, m_nEyePositionRight_Right, sizeof(unsigned short)*(m_nEyeRightTotalLimit-100));
				myFree(m_nEyePositionRight_Right); m_nEyePositionRight_Right = m_nEyePositionRight_Right_new; m_nEyePositionRight_Right_new = NULL;

				m_nEyePositionRight_Top_new = (unsigned short*)myMalloc(sizeof(unsigned short)*m_nEyeRightTotalLimit);
				memcpy(m_nEyePositionRight_Top_new, m_nEyePositionRight_Top, sizeof(unsigned short)*(m_nEyeRightTotalLimit-100));
				myFree(m_nEyePositionRight_Top); m_nEyePositionRight_Top = m_nEyePositionRight_Top_new; m_nEyePositionRight_Top_new = NULL;

				m_nEyePositionRight_Bottom_new = (unsigned short*)myMalloc(sizeof(unsigned short)*m_nEyeRightTotalLimit);
				memcpy(m_nEyePositionRight_Bottom_new, m_nEyePositionRight_Bottom, sizeof(unsigned short)*(m_nEyeRightTotalLimit-100));
				myFree(m_nEyePositionRight_Bottom); m_nEyePositionRight_Bottom = m_nEyePositionRight_Bottom_new; m_nEyePositionRight_Bottom_new = NULL;

				m_nEyeClassRight_new = (unsigned char*)myMalloc(sizeof(unsigned char)*m_nEyeRightTotalLimit);
				memcpy(m_nEyeClassRight_new, m_nEyeClassRight, sizeof(unsigned char)*(m_nEyeRightTotalLimit-100));
				myFree(m_nEyeClassRight); m_nEyeClassRight = m_nEyeClassRight_new; m_nEyeClassRight_new = NULL;

				m_nEyeConfidenceRight_new = (unsigned short*)myMalloc(sizeof(unsigned short)*m_nEyeRightTotalLimit);
				memcpy(m_nEyeConfidenceRight_new, m_nEyeConfidenceRight, sizeof(unsigned short)*(m_nEyeRightTotalLimit-100));
				myFree(m_nEyeConfidenceRight); m_nEyeConfidenceRight = m_nEyeConfidenceRight_new; m_nEyeConfidenceRight_new = NULL;
			}
			m_nEyePositionRight_Left[m_nEyeRightTotal] = nEyeX;
			m_nEyePositionRight_Top[m_nEyeRightTotal] = nEyeY;
			m_nEyePositionRight_Right[m_nEyeRightTotal] = nEyeX + nEyeW;
			m_nEyePositionRight_Bottom[m_nEyeRightTotal] = nEyeY + nEyeH;
			m_nEyeClassRight[m_nEyeRightTotal] = i;
			m_nEyeConfidenceRight[m_nEyeRightTotal] = nConfidence;
			++m_nEyeRightTotal;
			//
			if(m_nEyeRightCandidateConfidence[i] > nConfidence) {
				m_ed_rcRightCandidate[i] = rcEye;
				m_nEyeRightCandidateConfidence[i] = nConfidence;
			}
			break;
		}
	}

	if(bIntersection == 0) {
		m_ed_rcRightCandidate[m_nEyeRightCandidateCount] = rcEye;
		++m_nEyeRightCandidateIntersectionCount[m_nEyeRightCandidateCount];
		m_nEyeRightCandidateConfidence[m_nEyeRightCandidateCount] = nConfidence;
		m_bEyeRightCandidateValid[m_nEyeRightCandidateCount] = 1;
		++m_nEyeRightCandidateCount;
		// Add new detected right eye
		if (m_nEyeRightTotal == m_nEyeRightTotalLimit) {
			m_nEyeRightTotalLimit += 100;
			m_nEyePositionRight_Left_new = (unsigned short*)myMalloc(sizeof(unsigned short)*m_nEyeRightTotalLimit);
			memcpy(m_nEyePositionRight_Left_new, m_nEyePositionRight_Left, sizeof(unsigned short)*(m_nEyeRightTotalLimit-100));
			myFree(m_nEyePositionRight_Left); m_nEyePositionRight_Left = m_nEyePositionRight_Left_new; m_nEyePositionRight_Left_new = NULL;

			m_nEyePositionRight_Right_new = (unsigned short*)myMalloc(sizeof(unsigned short)*m_nEyeRightTotalLimit);
			memcpy(m_nEyePositionRight_Right_new, m_nEyePositionRight_Right, sizeof(unsigned short)*(m_nEyeRightTotalLimit-100));
			myFree(m_nEyePositionRight_Right); m_nEyePositionRight_Right = m_nEyePositionRight_Right_new; m_nEyePositionRight_Right_new = NULL;

			m_nEyePositionRight_Top_new = (unsigned short*)myMalloc(sizeof(unsigned short)*m_nEyeRightTotalLimit);
			memcpy(m_nEyePositionRight_Top_new, m_nEyePositionRight_Top, sizeof(unsigned short)*(m_nEyeRightTotalLimit-100));
			myFree(m_nEyePositionRight_Top); m_nEyePositionRight_Top = m_nEyePositionRight_Top_new; m_nEyePositionRight_Top_new = NULL;

			m_nEyePositionRight_Bottom_new = (unsigned short*)myMalloc(sizeof(unsigned short)*m_nEyeRightTotalLimit);
			memcpy(m_nEyePositionRight_Bottom_new, m_nEyePositionRight_Bottom, sizeof(unsigned short)*(m_nEyeRightTotalLimit-100));
			myFree(m_nEyePositionRight_Bottom); m_nEyePositionRight_Bottom = m_nEyePositionRight_Bottom_new; m_nEyePositionRight_Bottom_new = NULL;

			m_nEyeClassRight_new = (unsigned char*)myMalloc(sizeof(unsigned char)*m_nEyeRightTotalLimit);
			memcpy(m_nEyeClassRight_new, m_nEyeClassRight, sizeof(unsigned char)*(m_nEyeRightTotalLimit-100));
			myFree(m_nEyeClassRight); m_nEyeClassRight = m_nEyeClassRight_new; m_nEyeClassRight_new = NULL;

			m_nEyeConfidenceRight_new = (unsigned short*)myMalloc(sizeof(unsigned short)*m_nEyeRightTotalLimit);
			memcpy(m_nEyeConfidenceRight_new, m_nEyeConfidenceRight, sizeof(unsigned short)*(m_nEyeRightTotalLimit-100));
			myFree(m_nEyeConfidenceRight); m_nEyeConfidenceRight = m_nEyeConfidenceRight_new; m_nEyeConfidenceRight_new = NULL;
		}
		m_nEyePositionRight_Left[m_nEyeRightTotal] = nEyeX;
		m_nEyePositionRight_Top[m_nEyeRightTotal] = nEyeY;
		m_nEyePositionRight_Right[m_nEyeRightTotal] = nEyeX + nEyeW;
		m_nEyePositionRight_Bottom[m_nEyeRightTotal] = nEyeY + nEyeH;
		m_nEyeClassRight[m_nEyeRightTotal] = i;
		m_nEyeConfidenceRight[m_nEyeRightTotal] = nConfidence;
		++m_nEyeRightTotal;
		//
	}

	for(i=0; i<m_nEyeRightCandidateCount; i++) {
		if(m_bEyeRightCandidateValid[i] == 0) continue;
		for(j=i+1; j<m_nEyeRightCandidateCount; j++) {
			if(m_bEyeRightCandidateValid[j] == 0) continue;
			if(sdic_ed_isintersection(m_ed_rcRightCandidate[i], m_ed_rcRightCandidate[j]) == 0) continue;
			if(m_nEyeRightCandidateConfidence[i] > m_nEyeRightCandidateConfidence[j]) {	//valid is j
				m_nEyeRightCandidateIntersectionCount[j] += m_nEyeRightCandidateIntersectionCount[i];
				m_bEyeRightCandidateValid[i] = 0;
				// Re-assign class number to j
				for (pClass = m_nEyeClassRight, k = m_nEyeRightTotal; k--; ) {
					if (*pClass == i) *pClass = j;
					pClass++;
				}
			}
			else { //valid is i
				m_nEyeRightCandidateIntersectionCount[i] += m_nEyeRightCandidateIntersectionCount[j];
				m_bEyeRightCandidateValid[j] = 0;
				// Re-assign class number to i
				for (pClass = m_nEyeClassRight, k = m_nEyeRightTotal; k--; ) {
					if (*pClass == j) *pClass = i;
					pClass++;
				}
			}
		}
	}
}

/* ----------------------------------------------------------------
int sdic_ed_isintersection( SMyRect rc1,  SMyRect rc2);
- 두 사각형 영역이 겹치는지를 판단
- [IN]
*  SMyRect rc1:		사각형 1
*  SMyRect rc2:		사각형 2
- [return]:
0->겹치지 않으면 0 반환
1->겹치면 1 반환
---------------------------------------------------------------- */
int sdic_ed_isintersection( SMyRect rc1,  SMyRect rc2)
{
	//	return 0;

	int w, is_left, is_top, is_right, is_bottom;
	w = (rc1.right - rc1.left)>>3;
	is_left = rc1.left + w;
	is_top = rc1.top + w;
	is_right = rc1.right - w;
	is_bottom = rc1.bottom - w;

	w = (rc2.right - rc2.left)>>3;
	rc2.left += w;
	rc2.top += w;
	rc2.right -= w;
	rc2.bottom -= w;

	if(rc2.left > is_left)		is_left = rc2.left;
	if(rc2.top > is_top)		is_top = rc2.top;
	if(rc2.right < is_right)	is_right = rc2.right;
	if(rc2.bottom < is_bottom)	is_bottom = rc2.bottom;
	if(is_left > is_right) return 0;
	if(is_top  > is_bottom) return 0;
	return 1;
}

void sdic_ed_localizeLeft(unsigned char nclass)
{
	int i, nCandidate = 0;
	int avgLeft = 0, avgTop = 0, avgRight = 0, avgBottom = 0;
	unsigned short *pLeft = m_nEyePositionLeft_Left + m_nEyeLeftTotal, *pTop = m_nEyePositionLeft_Top + m_nEyeLeftTotal;
	unsigned short *pRight = m_nEyePositionLeft_Right + m_nEyeLeftTotal, *pBottom = m_nEyePositionLeft_Bottom + m_nEyeLeftTotal;
	unsigned char *pClass = m_nEyeClassLeft + m_nEyeLeftTotal;
	unsigned short *pConfidence = m_nEyeConfidenceLeft + m_nEyeLeftTotal;
	unsigned int avgConfidence = 0;

	for (i = m_nEyeLeftTotal; i--;) {
		--pConfidence;
		if (*--pClass == nclass) {
			avgConfidence += *pConfidence;
			++nCandidate;
		}
	}

	avgConfidence = (int)((double)(avgConfidence) / nCandidate + 0.5);

	nCandidate = 0;
	pClass = m_nEyeClassLeft + m_nEyeLeftTotal; pConfidence = m_nEyeConfidenceLeft + m_nEyeLeftTotal;
	for (i = m_nEyeLeftTotal; i--;) {
		--pLeft; --pTop; --pRight; --pBottom; --pConfidence; --pClass;
		if (*pConfidence < avgConfidence && *pClass == nclass) {
			avgLeft += *pLeft;
			avgTop += *pTop;
			avgRight += *pRight;
			avgBottom += *pBottom;
			++nCandidate;
		}
	}

	avgLeft = (int)((double)(avgLeft) / nCandidate + 0.5);
	avgTop = (int)((double)(avgTop) / nCandidate + 0.5);
	avgRight = (int)((double)(avgRight) / nCandidate + 0.5);
	avgBottom = (int)((double)(avgBottom) / nCandidate + 0.5);

	m_ed_rcLeft.left = avgLeft;
	m_ed_rcLeft.right = avgRight;
	m_ed_rcLeft.top = avgTop;
	m_ed_rcLeft.bottom = avgBottom;
}

void sdic_ed_localizeRight(unsigned char nclass)
{
	int i, nCandidate = 0;
	int avgLeft = 0, avgTop = 0, avgRight = 0, avgBottom = 0;
	unsigned short *pLeft = m_nEyePositionRight_Left + m_nEyeRightTotal, *pTop = m_nEyePositionRight_Top + m_nEyeRightTotal;
	unsigned short *pRight = m_nEyePositionRight_Right + m_nEyeRightTotal, *pBottom = m_nEyePositionRight_Bottom + m_nEyeRightTotal;
	unsigned char *pClass = m_nEyeClassRight + m_nEyeRightTotal;
	unsigned short *pConfidence = m_nEyeConfidenceRight + m_nEyeRightTotal;
	unsigned int avgConfidence = 0;

	for (i = m_nEyeRightTotal; i--;) {
		--pConfidence;
		if (*--pClass == nclass) {
			avgConfidence += *pConfidence;
			++nCandidate;
		}
	}

	avgConfidence = (int)((double)(avgConfidence) / nCandidate + 0.5);

	nCandidate = 0;
	pClass = m_nEyeClassRight + m_nEyeRightTotal; pConfidence = m_nEyeConfidenceRight + m_nEyeRightTotal;
	for (i = m_nEyeRightTotal; i--;) {
		--pLeft; --pTop; --pRight; --pBottom; --pConfidence; --pClass;
		if (*pConfidence < avgConfidence && *pClass == nclass) {
			avgLeft += *pLeft;
			avgTop += *pTop;
			avgRight += *pRight;
			avgBottom += *pBottom;
			++nCandidate;
		}
	}

	avgLeft = (int)((double)(avgLeft) / nCandidate + 0.5);
	avgTop = (int)((double)(avgTop) / nCandidate + 0.5);
	avgRight = (int)((double)(avgRight) / nCandidate + 0.5);
	avgBottom = (int)((double)(avgBottom) / nCandidate + 0.5);

	m_ed_rcRight.left = avgLeft;
	m_ed_rcRight.right = avgRight;
	m_ed_rcRight.top = avgTop;
	m_ed_rcRight.bottom = avgBottom;
}