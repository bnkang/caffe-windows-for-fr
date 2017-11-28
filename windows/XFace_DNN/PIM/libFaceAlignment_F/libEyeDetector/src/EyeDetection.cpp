//#include "stdafx.h"

#include "../inc/EyeDetectionParameters_MCT_20090427_c_15x15.h"
#include "../inc/EyeDetection.h"
#include "../../libFaceDetector/inc/FA_ImageProcess.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <malloc.h>

#define ED_MAX_FIND						10
/*
// 첫 image pyramid 크기 변경
*/
#define ED_BEST_SIZE					33
#define ED_BIT_SHIFT					15
/*
// Search step 변경
*/
#define ED_PYRAMID_STEP					1
#define ED_INTERSECTION					3
#define ED_MAXIMUM_CANDIDATE			500

int m_nEyeLowerW = (int)(ED_FRONTAL_MCT_WINDOW_WIDTH * 1.7);
int m_nEyeLowerH = (int)(ED_FRONTAL_MCT_WINDOW_HEIGHT * 1.7);

unsigned char m_nEDResizeData[ED_BEST_SIZE * ED_BEST_SIZE * 2];
unsigned short m_nEDMCTData[ED_BEST_SIZE * ED_BEST_SIZE * 2];
unsigned char *m_nEyeCandidateLeftData;
unsigned char *m_nEyeCandidateRightData;
unsigned char *m_nEyeCandidateRightData_Flip;

unsigned short m_nEyePositionLeft_Left[ED_MAXIMUM_CANDIDATE], m_nEyePositionLeft_Top[ED_MAXIMUM_CANDIDATE], m_nEyePositionLeft_Right[ED_MAXIMUM_CANDIDATE], m_nEyePositionLeft_Bottom[ED_MAXIMUM_CANDIDATE];
unsigned short m_nEyePositionRight_Left[ED_MAXIMUM_CANDIDATE], m_nEyePositionRight_Top[ED_MAXIMUM_CANDIDATE], m_nEyePositionRight_Right[ED_MAXIMUM_CANDIDATE], m_nEyePositionRight_Bottom[ED_MAXIMUM_CANDIDATE];
unsigned char m_nEyeClassLeft[ED_MAXIMUM_CANDIDATE], m_nEyeClassRight[ED_MAXIMUM_CANDIDATE];
unsigned short m_nEyeConfidenceLeft[ED_MAXIMUM_CANDIDATE], m_nEyeConfidenceRight[ED_MAXIMUM_CANDIDATE];

int		m_nEyeLeftTotal, m_nEyeRightTotal;

int		m_nEyeLeftCandidateCount, m_nEyeRightCandidateCount;
struct SMyRect m_ed_rcEyeCandidateLeft, m_ed_rcEyeCandidateRight;
struct SMyRect m_ed_rcLeft, m_ed_rcRight;
struct SMyPoint m_ed_ptLeft, m_ed_ptRight;

int		m_nEyeLeftCandidateConfidence[ED_MAX_FIND], m_nEyeRightCandidateConfidence[ED_MAX_FIND];
int		m_bEyeLeftCandidateValid[ED_MAX_FIND], m_bEyeRightCandidateValid[ED_MAX_FIND];
int		m_nEyeLeftCandidateIntersectionCount[ED_MAX_FIND], m_nEyeRightCandidateIntersectionCount[ED_MAX_FIND];
struct SMyRect m_ed_rcLeftCandidate[ED_MAX_FIND], m_ed_rcRightCandidate[ED_MAX_FIND];

void ed_scan(int nResizeW, int nResizeH, int nSrcW, int nSrcH, unsigned short* nMCTData, int bIsRight);

void ed_enrollLeft(int nEyeX, int nEyeY, int nEyeW, int nEyeH, int nConfidence);
void ed_enrollRight(int nEyeX, int nEyeY, int nEyeW, int nEyeH, int nConfidence);
int ed_isintersection(struct SMyRect rc1, struct SMyRect rc2);
void ed_localizeLeft(unsigned char nclass);
void ed_localizeRight(unsigned char nclass);

/* ------------------------------------------------------------------------------
int ed_find(unsigned char* srcData, int src_w, int src_h, struct SMyRect rcFace);
- 얼굴 영역 내에서 눈 검출 수행
- [IN]
* unsigned char* srcData: 
GRAY 타입(8Bits)의 RAW 데이터(입력영상) 
데이터 크기: src_w*src_h
* int src_w:				입력영상의 width
* int src_h:				입력영상의 height		 
* struct SMyRect rcFace: 	검출된 얼굴의 사각형 영역
- [return]:					눈의 검출 여부 (양쪽 눈이 모두 검출 되어야 1 반환)
------------------------------------------------------------------------------- */
int ed_find(unsigned char* srcData, int src_w, int src_h, struct SMyRect rcFace)
{
	int i, maxCountIdx;
	int nResizeW, nResizeH;
	double fRatioHeightPerWidth;

	int nFaceW, nFaceH;
	int nEyeCandidateLeftW, nEyeCandidateLeftH;
	int nEyeCandidateRightW, nEyeCandidateRightH;
	struct SMyRect rcEyeCandidateLeft, rcEyeCandidateRight;
	struct SMyRect m_ed_rcRightTemp;

	// 눈 위치 초기화
	m_ed_rcLeft.left = m_ed_rcLeft.top = m_ed_rcLeft.right = m_ed_rcLeft.bottom = 0;
	m_ed_rcRight.left = m_ed_rcRight.top = m_ed_rcRight.right = m_ed_rcRight.bottom = 0;

	m_nEyeLeftCandidateCount = 0; m_nEyeRightCandidateCount = 0;
	memset(m_nEyeLeftCandidateIntersectionCount, 0, sizeof(int)*ED_MAX_FIND);
	memset(m_nEyeRightCandidateIntersectionCount, 0, sizeof(int)*ED_MAX_FIND);
	memset(m_bEyeLeftCandidateValid, 0, sizeof(bool)*ED_MAX_FIND);
	memset(m_bEyeRightCandidateValid, 0, sizeof(bool)*ED_MAX_FIND);

	nFaceW = rcFace.right - rcFace.left;
	nFaceH = rcFace.bottom - rcFace.top;

	// 얼굴 영역에서 눈 검출 영역 추출
	rcEyeCandidateLeft.left = rcFace.left;
	rcEyeCandidateLeft.top = rcFace.top;
	rcEyeCandidateLeft.right = rcFace.left + (int)(nFaceW / 2.0 + 0.5);
	rcEyeCandidateLeft.bottom = rcFace.top + (int)(nFaceH / 2.0 + 0.5);

	rcEyeCandidateRight.left = rcFace.left + (int)(nFaceW / 2.0 + 0.5);
	rcEyeCandidateRight.top = rcEyeCandidateLeft.top;
	rcEyeCandidateRight.right = rcFace.right;
	rcEyeCandidateRight.bottom = rcEyeCandidateLeft.bottom;

 	rcEyeCandidateLeft.left = rcEyeCandidateLeft.left < 1 ? 1 : rcEyeCandidateLeft.left;
 	rcEyeCandidateLeft.right = rcEyeCandidateLeft.right > src_w ? src_w : rcEyeCandidateLeft.right;
 	rcEyeCandidateLeft.top = rcEyeCandidateLeft.top < 1 ? 1 : rcEyeCandidateLeft.top;
 	rcEyeCandidateLeft.bottom = rcEyeCandidateLeft.bottom > src_h ? src_h : rcEyeCandidateLeft.bottom;

 	rcEyeCandidateRight.left = rcEyeCandidateRight.left < 1 ? 1 : rcEyeCandidateRight.left;
 	rcEyeCandidateRight.right = rcEyeCandidateRight.right > src_w ? src_w : rcEyeCandidateRight.right;
 	rcEyeCandidateRight.top = rcEyeCandidateRight.top < 1 ? 1 : rcEyeCandidateRight.top;
 	rcEyeCandidateRight.bottom = rcEyeCandidateRight.bottom > src_h ? src_h : rcEyeCandidateRight.bottom;

	m_ed_rcEyeCandidateLeft = rcEyeCandidateLeft;
	m_ed_rcEyeCandidateRight = rcEyeCandidateRight;

	nEyeCandidateLeftW = rcEyeCandidateLeft.right - rcEyeCandidateLeft.left;
	nEyeCandidateLeftH = rcEyeCandidateLeft.bottom - rcEyeCandidateLeft.top;
	nEyeCandidateRightW = rcEyeCandidateRight.right - rcEyeCandidateRight.left;
	nEyeCandidateRightH = rcEyeCandidateRight.bottom - rcEyeCandidateRight.top;

	m_nEyeCandidateLeftData = (unsigned char *)malloc(nEyeCandidateLeftH*nEyeCandidateLeftW);
	m_nEyeCandidateRightData = (unsigned char *)malloc(nEyeCandidateRightH*nEyeCandidateRightW);
	m_nEyeCandidateRightData_Flip = (unsigned char *)malloc(nEyeCandidateRightH*nEyeCandidateRightW);

	// 눈 영역 추출이 안되면 return 0
	// 오른쪽 눈 영역은 영상 flip을 하여 왼쪽 눈 검출기 사용
	if(Region(srcData, src_w, src_h, m_nEyeCandidateLeftData, rcEyeCandidateLeft) == 0 || Region(srcData, src_w, src_h, m_nEyeCandidateRightData, rcEyeCandidateRight) == 0) {
		free(m_nEyeCandidateLeftData); free(m_nEyeCandidateRightData); free(m_nEyeCandidateRightData_Flip);
		return 0;
	}
	FlipX(m_nEyeCandidateRightData, nEyeCandidateRightW, nEyeCandidateRightH, m_nEyeCandidateRightData_Flip);

	m_nEyeLeftTotal = 0, m_nEyeRightTotal = 0;

	/**************		Left	********************/
	fRatioHeightPerWidth = (double)nEyeCandidateLeftH/(double)nEyeCandidateLeftW;
	nResizeW = ED_BEST_SIZE;
	nResizeH = (int)(nResizeW * fRatioHeightPerWidth + 0.5);

	while(true) {
		/*
		// 최소 검출 크기 조절
		*/
		if(nResizeW <= m_nEyeLowerW || nResizeH <= m_nEyeLowerH) break;

		// Image pyramid 생성 및 각 image에 대해 MCT 변환
		Resize_BI(m_nEyeCandidateLeftData, nEyeCandidateLeftW, nEyeCandidateLeftH, m_nEDResizeData, nResizeW, nResizeH);
		image2MCT9_1(m_nEDResizeData, nResizeW, nResizeH, m_nEDMCTData);
		// 눈 검출기 및 감은 눈 검출기 수행
		ed_scan(nResizeW, nResizeH, nEyeCandidateLeftW, nEyeCandidateLeftH, m_nEDMCTData, 0);

		nResizeW = nResizeW - ED_PYRAMID_STEP;
		nResizeH = (int)(nResizeW * fRatioHeightPerWidth + 0.5);
	}

	/**************		Right	********************/
	fRatioHeightPerWidth = (double)nEyeCandidateRightH/(double)nEyeCandidateRightW;

	nResizeW = ED_BEST_SIZE;
	nResizeH = (int)(nResizeW * fRatioHeightPerWidth + 0.5);

	while(true) {
		/*
		// 최소 검출 크기 조절
		*/
		if(nResizeW <= m_nEyeLowerW || nResizeH <= m_nEyeLowerH) break;

		Resize_BI(m_nEyeCandidateRightData_Flip,nEyeCandidateRightW,nEyeCandidateRightH,m_nEDResizeData,nResizeW,nResizeH);
		image2MCT9_1(m_nEDResizeData, nResizeW, nResizeH, m_nEDMCTData);
		ed_scan(nResizeW, nResizeH, nEyeCandidateRightW, nEyeCandidateRightH, m_nEDMCTData, 1);

		nResizeW = nResizeW - ED_PYRAMID_STEP;
		nResizeH = (int)(nResizeW * fRatioHeightPerWidth + 0.5);
	}

	// 왼쪽, 오른쪽 눈 영역을 검색하여 눈 후보가 없으면 return 0
	if(m_nEyeLeftCandidateCount == 0) {
		free(m_nEyeCandidateLeftData); free(m_nEyeCandidateRightData); free(m_nEyeCandidateRightData_Flip);
		return 0;
	}

	// 왼쪽 눈 영역에 대해서 눈 후보들 중에 intersection이 가장 많은 후보를 왼쪽 눈으로 선택
	maxCountIdx = 0;
	for(i=0; i<m_nEyeLeftCandidateCount; i++) {
		if(m_bEyeLeftCandidateValid[i]) {
			if(m_nEyeLeftCandidateConfidence[i] < m_nEyeLeftCandidateConfidence[maxCountIdx]) {
				maxCountIdx = i;
			}
		}
	}
	m_ed_rcLeft = m_ed_rcLeftCandidate[maxCountIdx];

	if (m_nEyeLeftCandidateIntersectionCount[maxCountIdx] < ED_INTERSECTION) {
		free(m_nEyeCandidateLeftData); free(m_nEyeCandidateRightData); free(m_nEyeCandidateRightData_Flip);
		return 0;
	}
	ed_localizeLeft(maxCountIdx);

	if(m_nEyeRightCandidateCount == 0) {
		free(m_nEyeCandidateLeftData); free(m_nEyeCandidateRightData); free(m_nEyeCandidateRightData_Flip);
		return 0;
	}

	// 오른쪽 눈 영역에 대해서 눈 후보들 중에 intersection이 가장 많은 후보를 오른쪽 눈으로 선택
	maxCountIdx = 0;
	for(i=0; i<m_nEyeRightCandidateCount; i++) {
		if(m_bEyeRightCandidateValid[i]) {
			if(m_nEyeRightCandidateConfidence[i] < m_nEyeRightCandidateConfidence[maxCountIdx]) {
				maxCountIdx = i;
			}
		}
	}
	m_ed_rcRight = m_ed_rcRightCandidate[maxCountIdx];

	if (m_nEyeRightCandidateIntersectionCount[maxCountIdx] < ED_INTERSECTION) {
		free(m_nEyeCandidateLeftData); free(m_nEyeCandidateRightData); free(m_nEyeCandidateRightData_Flip);
		return 0;
	}
	ed_localizeRight(maxCountIdx);

	m_ed_rcLeft.left += m_ed_rcEyeCandidateLeft.left;
	m_ed_rcLeft.top += m_ed_rcEyeCandidateLeft.top;
	m_ed_rcLeft.right += m_ed_rcEyeCandidateLeft.left;
	m_ed_rcLeft.bottom += m_ed_rcEyeCandidateLeft.top;

	m_ed_rcRightTemp = m_ed_rcRight;

	m_ed_rcRight.left = m_ed_rcEyeCandidateRight.right - m_ed_rcRightTemp.right + 1;
	m_ed_rcRight.top += m_ed_rcEyeCandidateRight.top;
	m_ed_rcRight.right = m_ed_rcEyeCandidateRight.right - m_ed_rcRightTemp.left + 1;
	m_ed_rcRight.bottom += m_ed_rcEyeCandidateRight.top;

	m_ed_ptLeft.x = m_ed_rcLeft.left + ((m_ed_rcLeft.right - m_ed_rcLeft.left)>>1);
	m_ed_ptLeft.y = m_ed_rcLeft.top + ((m_ed_rcLeft.bottom - m_ed_rcLeft.top)>>1);	
	m_ed_ptRight.x = m_ed_rcRight.left + ((m_ed_rcRight.right - m_ed_rcRight.left)>>1);
	m_ed_ptRight.y = m_ed_rcRight.top + ((m_ed_rcRight.bottom - m_ed_rcRight.top)>>1);

	free(m_nEyeCandidateLeftData); free(m_nEyeCandidateRightData); free(m_nEyeCandidateRightData_Flip);
	return 1;
}

/* ------------------------------------------------------------
int ed_whereBox(struct SMyRect* rectLEye, struct SMyRect* rectREye);
- 검출된 눈의 사각형 영역 정보를 얻음
- [OUT]
* struct SMyRect* rectLEye:	ed_find( )에서 얻은 왼쪽 눈 사각형 위치 정보를 가져옴
* struct SMyRect* rectREye:	ed_find( )에서 얻은 오른쪽 눈 사각형 위치 정보를 가져옴
- [return]: 
0->정상적으로 종료되면 0 반환
------------------------------------------------------------- */
int ed_whereBox(struct SMyRect* rectLEye, struct SMyRect* rectREye)
{
	*rectLEye = m_ed_rcLeft;
	*rectREye = m_ed_rcRight;
	return 0;
}

/* ------------------------------------------------------------
int ed_where(struct SMyPoint* ptLEye, struct SMyPoint* ptREye);
- 검출된 눈의 가운데 위치 정보를 얻음
- [OUT]
* struct SMyPoint* ptLEye:	ed_find( )에서 얻은 왼쪽 눈 가운데 위치 정보를 가져옴
* struct SMyPoint* ptREye:	ed_find( )에서 얻은 오른쪽 눈 가운데 위치 정보를 가져옴
- [return]: 
0->정상적으로 종료되면 0 반환
------------------------------------------------------------- */
int ed_where(struct SMyPoint* ptLEye, struct SMyPoint* ptREye)
{
	*ptLEye = m_ed_ptLeft;
	*ptREye = m_ed_ptRight;
	return 0;
}

/* ----------------------------------------------------------------------------------------------------
void ed_scan(int nResizeW, int nResizeH, int nSrcW, int nSrcH, unsigned short* nMCTData, int bIsRight);
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
---------------------------------------------------------------------------------------------------- */
void ed_scan(int nResizeW, int nResizeH, int nSrcW, int nSrcH, unsigned short* nMCTData, int bIsRight) 
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
	int	nMCTFeatureGlobalPosition[ED_FRONTAL_MCT_FEATURE_SIZE_TOTAL];
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
		pMCTData = nMCTData + y*nResizeW;
		for(x=0; x<nSearchW; x+=2) {
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
				if(bIsRight) ed_enrollRight(nEyeX, nEyeY, nEyeW, nEyeH, nMCTConfidence);
				else ed_enrollLeft(nEyeX, nEyeY, nEyeW, nEyeH, nMCTConfidence);
			}
			pMCTData+=2;
		}
	}
}

/* -----------------------------------------------------------------------------
void ed_enrollLeft(int nEyeX, int nEyeY, int nEyeW, int nEyeH, int nConfidence);
- 검출한 왼쪽 눈 영역을 후보에 등록
- [IN]
* int nEyeX:					왼쪽 눈 영역의 x 좌표
* int nEyeY:					왼쪽 눈 영역의 y 좌표
* int nEyeW:					왼쪽 눈 영역의 폭
* int nEyeH:					왼쪽 눈 영역의 높이
* int nConfidence:				검출한 왼쪽 눈 영역의 mct값
- [return]:						없음
----------------------------------------------------------------------------- */
void ed_enrollLeft(int nEyeX, int nEyeY, int nEyeW, int nEyeH, int nConfidence)
{
	int i, j, k;
	unsigned char *pClass;
	int bIntersection = 0;
	struct SMyRect rcEye;
	
	rcEye.left = nEyeX;
	rcEye.top = nEyeY;
	rcEye.right = nEyeX + nEyeW;
	rcEye.bottom = nEyeY + nEyeH;

	// 기존에 등록된 후보와 겹치면서 MCT 값이 낮다면
	// 기존 후보를 현재 등록하려는 후보로 교체하고 intersection count를 증가
	for(i=0; i<m_nEyeLeftCandidateCount; i++) {
		if(m_bEyeLeftCandidateValid[i] == 0) continue;
		if(ed_isintersection(m_ed_rcLeftCandidate[i], rcEye)) {			
			bIntersection = 1;
			m_nEyeLeftCandidateIntersectionCount[i]++;
			// Add new detected left eye
			if (m_nEyeLeftTotal < ED_MAXIMUM_CANDIDATE) {
				m_nEyePositionLeft_Left[m_nEyeLeftTotal] = nEyeX;
				m_nEyePositionLeft_Top[m_nEyeLeftTotal] = nEyeY;
				m_nEyePositionLeft_Right[m_nEyeLeftTotal] = nEyeX + nEyeW;
				m_nEyePositionLeft_Bottom[m_nEyeLeftTotal] = nEyeY + nEyeH;
				m_nEyeClassLeft[m_nEyeLeftTotal] = i;
				m_nEyeConfidenceLeft[m_nEyeLeftTotal] = nConfidence;
				++m_nEyeLeftTotal;
			}
			//
			if(m_nEyeLeftCandidateConfidence[i] > nConfidence ) {
				m_ed_rcLeftCandidate[i] = rcEye;
				m_nEyeLeftCandidateConfidence[i] = nConfidence;
			}
			break;
		}
	}

	if(ED_MAX_FIND <= m_nEyeLeftCandidateCount) {
		return;
	}

	// 겹치는 후보들이 없다면 새로운 후보로 등록
	if(bIntersection == 0) {
		m_ed_rcLeftCandidate[m_nEyeLeftCandidateCount] = rcEye; 
		m_nEyeLeftCandidateIntersectionCount[m_nEyeLeftCandidateCount]++;
		m_nEyeLeftCandidateConfidence[m_nEyeLeftCandidateCount] = nConfidence;
		m_bEyeLeftCandidateValid[m_nEyeLeftCandidateCount] = 1;
		m_nEyeLeftCandidateCount++;
		// Add new detected left eye
		if (m_nEyeLeftTotal < ED_MAXIMUM_CANDIDATE) {
			m_nEyePositionLeft_Left[m_nEyeLeftTotal] = nEyeX;
			m_nEyePositionLeft_Top[m_nEyeLeftTotal] = nEyeY;
			m_nEyePositionLeft_Right[m_nEyeLeftTotal] = nEyeX + nEyeW;
			m_nEyePositionLeft_Bottom[m_nEyeLeftTotal] = nEyeY + nEyeH;
			m_nEyeClassLeft[m_nEyeLeftTotal] = i;
			m_nEyeConfidenceLeft[m_nEyeLeftTotal] = nConfidence;
			++m_nEyeLeftTotal;
		}
		//
	}

	// 기존에 등록되어 있는 후보 및 새로 등록한 후보들에 대해서 겹치는 후보들을 모두 합침
	// MCT 값이 낮은 후보로 교체하고 intersection count를 합침
	for(i=0; i<m_nEyeLeftCandidateCount; i++) {
		if(m_bEyeLeftCandidateValid[i] == 0) continue;
		for(j=i+1; j<m_nEyeLeftCandidateCount; j++) {
			if(m_bEyeLeftCandidateValid[j] == 0) continue;
			if(ed_isintersection(m_ed_rcLeftCandidate[i], m_ed_rcLeftCandidate[j]) == 0) continue;
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

/* ------------------------------------------------------------------------------
void ed_enrollRight(int nEyeX, int nEyeY, int nEyeW, int nEyeH, int nConfidence);
- 검출한 오른쪽 눈 영역을 후보에 등록
- [IN]
* int nEyeX:					오른쪽 눈 영역의 x 좌표
* int nEyeY:					오른쪽 눈 영역의 y 좌표
* int nEyeW:					오른쪽 눈 영역의 폭
* int nEyeH:					오른쪽 눈 영역의 높이
* int nConfidence:				검출한 오른쪽 눈 영역의 mct값
- [return]:						없음
------------------------------------------------------------------------------ */
void ed_enrollRight(int nEyeX, int nEyeY, int nEyeW, int nEyeH, int nConfidence)
{
	int i, j, k;
	unsigned char *pClass;
	int bIntersection = 0;
	struct SMyRect rcEye;
	
	rcEye.left = nEyeX;
	rcEye.top = nEyeY;
	rcEye.right = nEyeX + nEyeW;
	rcEye.bottom = nEyeY + nEyeH;

	for(i=0; i<m_nEyeRightCandidateCount; i++) {
		if(m_bEyeRightCandidateValid[i] == 0) continue;
		if(ed_isintersection(m_ed_rcRightCandidate[i], rcEye)) {			
			bIntersection = 1;
			m_nEyeRightCandidateIntersectionCount[i]++;
			// Add new detected right eye
			if (m_nEyeRightTotal < ED_MAXIMUM_CANDIDATE) {
				m_nEyePositionRight_Left[m_nEyeRightTotal] = nEyeX;
				m_nEyePositionRight_Top[m_nEyeRightTotal] = nEyeY;
				m_nEyePositionRight_Right[m_nEyeRightTotal] = nEyeX + nEyeW;
				m_nEyePositionRight_Bottom[m_nEyeRightTotal] = nEyeY + nEyeH;
				m_nEyeClassRight[m_nEyeRightTotal] = i;
				m_nEyeConfidenceRight[m_nEyeRightTotal] = nConfidence;
				++m_nEyeRightTotal;
			}
			//
			if(m_nEyeRightCandidateConfidence[i] > nConfidence) {
				m_ed_rcRightCandidate[i] = rcEye;
				m_nEyeRightCandidateConfidence[i] = nConfidence;
			}
			break;
		}
	}

	if(ED_MAX_FIND <= m_nEyeRightCandidateCount) {
		return;
	}

	if(bIntersection == 0) {
		m_ed_rcRightCandidate[m_nEyeRightCandidateCount] = rcEye; 
		m_nEyeRightCandidateIntersectionCount[m_nEyeRightCandidateCount]++;
		m_nEyeRightCandidateConfidence[m_nEyeRightCandidateCount] = nConfidence;
		m_bEyeRightCandidateValid[m_nEyeRightCandidateCount] = 1;
		m_nEyeRightCandidateCount++;
		// Add new detected right eye
		if (m_nEyeRightTotal < ED_MAXIMUM_CANDIDATE) {
			m_nEyePositionRight_Left[m_nEyeRightTotal] = nEyeX;
			m_nEyePositionRight_Top[m_nEyeRightTotal] = nEyeY;
			m_nEyePositionRight_Right[m_nEyeRightTotal] = nEyeX + nEyeW;
			m_nEyePositionRight_Bottom[m_nEyeRightTotal] = nEyeY + nEyeH;
			m_nEyeClassRight[m_nEyeRightTotal] = i;
			m_nEyeConfidenceRight[m_nEyeRightTotal] = nConfidence;
			++m_nEyeRightTotal;
		}
		//
	}

	for(i=0; i<m_nEyeRightCandidateCount; i++) {
		if(m_bEyeRightCandidateValid[i] == 0) continue;
		for(j=i+1; j<m_nEyeRightCandidateCount; j++) {
			if(m_bEyeRightCandidateValid[j] == 0) continue;
			if(ed_isintersection(m_ed_rcRightCandidate[i], m_ed_rcRightCandidate[j]) == 0) continue;
			if(m_nEyeRightCandidateConfidence[i] > m_nEyeRightCandidateConfidence[j]) {	//valid is j
				m_nEyeRightCandidateIntersectionCount[j] += m_nEyeRightCandidateIntersectionCount[i];
				m_bEyeRightCandidateValid[i] = 0;
				// Re-assign class number to j
				for (pClass = m_nEyeClassRight, k = m_nEyeRightTotal; k--; ) {
					if (*pClass == i) *pClass = j;
					++pClass;
				}
			}
			else { //valid is i
				m_nEyeRightCandidateIntersectionCount[i] += m_nEyeRightCandidateIntersectionCount[j];
				m_bEyeRightCandidateValid[j] = 0;
				// Re-assign class number to i
				for (pClass = m_nEyeClassRight, k = m_nEyeRightTotal; k--; ) {
					if (*pClass == j) *pClass = i;
					++pClass;
				}
			}					
		}
	}
}

/* -----------------------------------------------------------
int ed_isintersection(struct SMyRect rc1, struct SMyRect rc2);
- 두 사각형 영역이 겹치는지를 판단
- [IN]
* struct SMyRect rc1:		사각형 1
* struct SMyRect rc2:		사각형 2
- [return]: 
0->겹치지 않으면 0 반환
1->겹치면 1 반환
----------------------------------------------------------- */
int ed_isintersection(struct SMyRect rc1, struct SMyRect rc2)
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

void ed_localizeLeft(unsigned char nclass)
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
			nCandidate++;
		}
	}

	if (nCandidate < 1) return;

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
			nCandidate++;
		}
	}

	if (nCandidate < 1) return;

	avgLeft = (int)((double)(avgLeft) / nCandidate + 0.5);
	avgTop = (int)((double)(avgTop) / nCandidate + 0.5);
	avgRight = (int)((double)(avgRight) / nCandidate + 0.5);
	avgBottom = (int)((double)(avgBottom) / nCandidate + 0.5);

	m_ed_rcLeft.left = avgLeft;
	m_ed_rcLeft.right = avgRight;
	m_ed_rcLeft.top = avgTop;
	m_ed_rcLeft.bottom = avgBottom;
}

void ed_localizeRight(unsigned char nclass)
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
			nCandidate++;
		}
	}

	if (nCandidate < 1) return;

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
			nCandidate++;
		}
	}

	if (nCandidate < 1) return;

	avgLeft = (int)((double)(avgLeft) / nCandidate + 0.5);
	avgTop = (int)((double)(avgTop) / nCandidate + 0.5);
	avgRight = (int)((double)(avgRight) / nCandidate + 0.5);
	avgBottom = (int)((double)(avgBottom) / nCandidate + 0.5);

	m_ed_rcRight.left = avgLeft;
	m_ed_rcRight.right = avgRight;
	m_ed_rcRight.top = avgTop;
	m_ed_rcRight.bottom = avgBottom;
}
