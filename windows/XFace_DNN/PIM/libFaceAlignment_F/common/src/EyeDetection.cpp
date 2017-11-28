
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
// ù image pyramid ũ�� ����
*/
#define ED_BEST_SIZE					33
#define ED_BIT_SHIFT					15
/*
// Image pyramid step ����
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
- �� ���� ���� �� �� ��ġ ���� ����, �󱼰��⿡ ����� ������ ũ��� �����⿡ ����� ������
ũ�Ⱑ �ٸ� ��쿡 ���
- [IN]
* unsigned char *pData:
GRAY Ÿ��(8Bits)�� RAW ������(�Է¿���), angle�� ���� ȸ���� ����
������ ũ��: nWidth*nHeight
* long nDataWidth:				�Է¿����� width
* long nDataHeight:				�Է¿����� height
* long nPreviewWidth:			�󱼰��⿡ ����ߴ� ���� width
* long nPreviewHeight:			�󱼰��⿡ ����ߴ� ���� height
* FD_INFO *pFD_Info: 		FD_INFO
- [return]:					�� ����
------------------------------------------------------------------------------------------*/
long SDIC_EyeDetection2(unsigned char* pData, long nDataWidth, long nDataHeight,
					   long nPreviewWidth, long nPreviewHeight, FD_INFO *pFD_Info)
{
	int i;
    double fsw;
    double fsh;
	// �Է� ������ ũ��� �� ���⿡ ���� ������ ũ�Ⱑ ���ٸ�
	if(nDataWidth == nPreviewWidth && nDataHeight == nPreviewHeight) {
		return SDIC_EyeDetection(pData, nDataWidth, nDataHeight, pFD_Info);
	}

	// �� ������ ����, ���ο� �����ϴ� �� ���Ͽ� FD_INFO�� ����
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
- �� ���� ���� �� �� ��ġ ���� ����
- [IN]
* unsigned char *pData:
GRAY Ÿ��(8Bits)�� RAW ������(�Է¿���), angle�� ���� ȸ���� ����
������ ũ��: nWidth*nHeight
* long nWidth:				�Է¿����� width
* long nHeight:				�Է¿����� height
* FD_INFO *pFD_Info: 		FD_INFO
- [return]:					�� ����
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
		/* ������ face�� ���ؼ� �� ���� ������ ���� pFD_Info�� ������ �󱼿� ���� rcFace�� assign*/
		rcFace.left = pFD_Info->boundRect[i].left;
		rcFace.right = pFD_Info->boundRect[i].left + pFD_Info->boundRect[i].width;
		rcFace.top = pFD_Info->boundRect[i].top;
		rcFace.bottom = pFD_Info->boundRect[i].top + pFD_Info->boundRect[i].height;

		// Set face region for eye detection
		// �Է��� angle�� ȸ���� ����������, ���� ��ġ�� ȸ������ ���� ���� ���� ��ġ�̹Ƿ�
		// ������ ���� ��ġ�� ȸ���� ���� �°� ��ġ ȸ���� �ؾ���

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

		// 0 - �� ���� ����
		// 1 - �� �� ���� ����
		// 2 - ������ �� ���� ����
		// 3 - ���� �� ���� ����
		if (bEDFind == 0) {		
			myFree(crop_faceData);
			myFree(rotated_faceData); 
			continue;
		}
		if (bEDFind == 1 || bEDFind == 2) pFD_Info->eyeL[i].enable = 1;
		if (bEDFind == 1 || bEDFind == 3) pFD_Info->eyeR[i].enable = 1;

		// �� ��ġ�� ���ͼ� ��� �� ����
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
- �� ���� ������ �� ���� ����
- [IN]
* unsigned char* srcData:
GRAY Ÿ��(8Bits)�� RAW ������(�Է¿���)
������ ũ��: src_w*src_h
* int src_w:				�Է¿����� width
* int src_h:				�Է¿����� height
*  SMyRect rcFace: 	����� ���� �簢�� ����
- [return]:					���� ���� ���� (���� ���� ��� ���� �Ǿ�� 1 ��ȯ)
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

	// �� ��ġ �ʱ�ȭ
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

	// �� �������� �� ���� ���� ����
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

	// residual �߰�
	// ���� ��, ������ �� candidate ������ ��ġ�� ����
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





	// �� ���� ������ �ȵǸ� return 0
	// ������ �� ������ ���� flip�� �Ͽ� ���� �� ����� ���
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
		// �ּ� ���� ũ�� ����
		*/
		if(nResizeW <= ED_FRONTAL_MCT_WINDOW_WIDTH * 1.5 || nResizeH <= ED_FRONTAL_MCT_WINDOW_HEIGHT * 1.5) break;

		// Image pyramid ���� �� �� image�� ���� MCT ��ȯ
		sdic_Resize_BI(m_nEyeCandidateLeftData, nEyeCandidateLeftW, nEyeCandidateLeftH, m_nEDResizeData, nResizeW, nResizeH);
		sdic_image2MCT9_1(m_nEDResizeData, nResizeW, nResizeH, m_nEDMCTData);
		// �� ����� �� ���� �� ����� ����
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
		// �ּ� ���� ũ�� ����
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

	// ����, ������ �� ������ �˻��Ͽ� �� �ĺ��� ������
	if(m_nEyeLeftCandidateCount == 0) bFoundLeft = 0;
	if(m_nEyeRightCandidateCount == 0) bFoundRight = 0;

	// ���� �� ������ ���ؼ� �� �ĺ��� �߿� intersection�� ���� ���� �ĺ��� ���� ������ ����
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

	// ������ �� ������ ���ؼ� �� �ĺ��� �߿� intersection�� ���� ���� �ĺ��� ������ ������ ����
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
- ����� ���� �簢�� ���� ������ ����
- [OUT]
*  SMyRect* rectLEye:	sdic_ed_find( )���� ���� ���� �� �簢�� ��ġ ������ ������
*  SMyRect* rectREye:	sdic_ed_find( )���� ���� ������ �� �簢�� ��ġ ������ ������
- [return]:
0->���������� ����Ǹ� 0 ��ȯ
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
- ����� ���� ��� ��ġ ������ ����
- [OUT]
*  SMyPoint* ptLEye:	sdic_ed_find( )���� ���� ���� �� ��� ��ġ ������ ������
*  SMyPoint* ptREye:	sdic_ed_find( )���� ���� ������ �� ��� ��ġ ������ ������
- [return]:
0->���������� ����Ǹ� 0 ��ȯ
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
- MCT �����Ϳ��� window�� 1 pixel�� �̵��� �ϸ� �� �ĺ� ���� ����
- ������ ����� ���ʰ� �������� �����Ͽ� �ĺ�(global ����)�� ���
- [IN]
* int nResizeW:					Scan �� ������ width
* int nResizeH:					Scan �� ������ height
* int nSrcW:					�Է� ������ width
* int nSrcH:					�Է� ������ height
* unsigned short* nMCTData:		Scan �� ������ MCT data
* int bIsRight:					���� �� ���� ������ �� ���� ����
- [return]:						����
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

	// pixel (x,y)�� window�� left top���� �����Ͽ� window ���� MCT������ �հ� �̸� �н��� threshold�� ��
	for(y=0; y<nSearchH; y++) {
		for(x=0; x<nSearchW; x++) {
			nMCTAlphaCount = 0;
			pMCTFeatureGlobalPosition = nMCTFeatureGlobalPosition;

			// �ӵ� ����� ���ؼ� cascade ����
			for(c=0; c<ED_FRONTAL_MCT_CASCADE_SIZE; c++) {
				nMCTConfidence = 0;
				bFind = 0;
				nMCTOneCascadeFeatureSize = ED_FRONTAL_MCT_FEATURE_SIZE[c];

				// �� cascade�� �ش��ϴ� feature���� MCT���� ���� ���� ����
				for(t=0; t<nMCTOneCascadeFeatureSize; ) {
					nMCTConfidence += m_nEDMCTAlpha[(nMCTAlphaCount + *(pMCTData + *pMCTFeatureGlobalPosition++))];
					nMCTAlphaCount += ED_FRONTAL_MCT_VALUE_RANGE;

					nMCTConfidence += m_nEDMCTAlpha[(nMCTAlphaCount + *(pMCTData + *pMCTFeatureGlobalPosition++))];
					nMCTAlphaCount += ED_FRONTAL_MCT_VALUE_RANGE;
					t+=2;
				}

				// MCT threshold ���� window�� MCT ���� ������ �� �ĺ��� ã�� ������ ����
				if(nMCTConfidence > ED_FRONTAL_MCT_THRESHOLD[c]) break;

				bFind = 1;
			}
			// �� �ĺ��� ã�Ҵٸ� �ĺ��� ���
			if(bFind == 1) {
				nEyeX = (x*nRatioSrcPerResize)>>ED_BIT_SHIFT;
				nEyeY = (y*nRatioSrcPerResize)>>ED_BIT_SHIFT;

				// ����, �������� �����Ͽ� ���
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
- ������ ���� �� ������ �ĺ��� ���
- [IN]
* int nEyeX:					���� �� ������ x ��ǥ
* int nEyeY:					���� �� ������ y ��ǥ
* int nEyeW:					���� �� ������ ��
* int nEyeH:					���� �� ������ ����
* int nConfidence:				������ ���� �� ������ mct��
* int isClosed:					�� �����, ���� �� ����� �� ��� �������
- [return]:						����
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



	// ������ ��ϵ� �ĺ��� ��ġ�鼭 MCT ���� ���ٸ�
	// ���� �ĺ��� ���� ����Ϸ��� �ĺ��� ��ü�ϰ� intersection count�� ����
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

	// ��ġ�� �ĺ����� ���ٸ� ���ο� �ĺ��� ���
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

	// ������ ��ϵǾ� �ִ� �ĺ� �� ���� ����� �ĺ��鿡 ���ؼ� ��ġ�� �ĺ����� ��� ��ħ
	// MCT ���� ���� �ĺ��� ��ü�ϰ� intersection count�� ��ħ
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
- ������ ������ �� ������ �ĺ��� ���
- [IN]
* int nEyeX:					������ �� ������ x ��ǥ
* int nEyeY:					������ �� ������ y ��ǥ
* int nEyeW:					������ �� ������ ��
* int nEyeH:					������ �� ������ ����
* int nConfidence:				������ ������ �� ������ mct��
* int isClosed:					�� �����, ���� �� ����� �� ��� �������
- [return]:						����
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
- �� �簢�� ������ ��ġ������ �Ǵ�
- [IN]
*  SMyRect rc1:		�簢�� 1
*  SMyRect rc2:		�簢�� 2
- [return]:
0->��ġ�� ������ 0 ��ȯ
1->��ġ�� 1 ��ȯ
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