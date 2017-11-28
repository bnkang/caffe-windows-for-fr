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
// ù image pyramid ũ�� ����
*/
#define ED_BEST_SIZE					33
#define ED_BIT_SHIFT					15
/*
// Search step ����
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
- �� ���� ������ �� ���� ����
- [IN]
* unsigned char* srcData: 
GRAY Ÿ��(8Bits)�� RAW ������(�Է¿���) 
������ ũ��: src_w*src_h
* int src_w:				�Է¿����� width
* int src_h:				�Է¿����� height		 
* struct SMyRect rcFace: 	����� ���� �簢�� ����
- [return]:					���� ���� ���� (���� ���� ��� ���� �Ǿ�� 1 ��ȯ)
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

	// �� ��ġ �ʱ�ȭ
	m_ed_rcLeft.left = m_ed_rcLeft.top = m_ed_rcLeft.right = m_ed_rcLeft.bottom = 0;
	m_ed_rcRight.left = m_ed_rcRight.top = m_ed_rcRight.right = m_ed_rcRight.bottom = 0;

	m_nEyeLeftCandidateCount = 0; m_nEyeRightCandidateCount = 0;
	memset(m_nEyeLeftCandidateIntersectionCount, 0, sizeof(int)*ED_MAX_FIND);
	memset(m_nEyeRightCandidateIntersectionCount, 0, sizeof(int)*ED_MAX_FIND);
	memset(m_bEyeLeftCandidateValid, 0, sizeof(bool)*ED_MAX_FIND);
	memset(m_bEyeRightCandidateValid, 0, sizeof(bool)*ED_MAX_FIND);

	nFaceW = rcFace.right - rcFace.left;
	nFaceH = rcFace.bottom - rcFace.top;

	// �� �������� �� ���� ���� ����
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

	// �� ���� ������ �ȵǸ� return 0
	// ������ �� ������ ���� flip�� �Ͽ� ���� �� ����� ���
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
		// �ּ� ���� ũ�� ����
		*/
		if(nResizeW <= m_nEyeLowerW || nResizeH <= m_nEyeLowerH) break;

		// Image pyramid ���� �� �� image�� ���� MCT ��ȯ
		Resize_BI(m_nEyeCandidateLeftData, nEyeCandidateLeftW, nEyeCandidateLeftH, m_nEDResizeData, nResizeW, nResizeH);
		image2MCT9_1(m_nEDResizeData, nResizeW, nResizeH, m_nEDMCTData);
		// �� ����� �� ���� �� ����� ����
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
		// �ּ� ���� ũ�� ����
		*/
		if(nResizeW <= m_nEyeLowerW || nResizeH <= m_nEyeLowerH) break;

		Resize_BI(m_nEyeCandidateRightData_Flip,nEyeCandidateRightW,nEyeCandidateRightH,m_nEDResizeData,nResizeW,nResizeH);
		image2MCT9_1(m_nEDResizeData, nResizeW, nResizeH, m_nEDMCTData);
		ed_scan(nResizeW, nResizeH, nEyeCandidateRightW, nEyeCandidateRightH, m_nEDMCTData, 1);

		nResizeW = nResizeW - ED_PYRAMID_STEP;
		nResizeH = (int)(nResizeW * fRatioHeightPerWidth + 0.5);
	}

	// ����, ������ �� ������ �˻��Ͽ� �� �ĺ��� ������ return 0
	if(m_nEyeLeftCandidateCount == 0) {
		free(m_nEyeCandidateLeftData); free(m_nEyeCandidateRightData); free(m_nEyeCandidateRightData_Flip);
		return 0;
	}

	// ���� �� ������ ���ؼ� �� �ĺ��� �߿� intersection�� ���� ���� �ĺ��� ���� ������ ����
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

	// ������ �� ������ ���ؼ� �� �ĺ��� �߿� intersection�� ���� ���� �ĺ��� ������ ������ ����
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
- ����� ���� �簢�� ���� ������ ����
- [OUT]
* struct SMyRect* rectLEye:	ed_find( )���� ���� ���� �� �簢�� ��ġ ������ ������
* struct SMyRect* rectREye:	ed_find( )���� ���� ������ �� �簢�� ��ġ ������ ������
- [return]: 
0->���������� ����Ǹ� 0 ��ȯ
------------------------------------------------------------- */
int ed_whereBox(struct SMyRect* rectLEye, struct SMyRect* rectREye)
{
	*rectLEye = m_ed_rcLeft;
	*rectREye = m_ed_rcRight;
	return 0;
}

/* ------------------------------------------------------------
int ed_where(struct SMyPoint* ptLEye, struct SMyPoint* ptREye);
- ����� ���� ��� ��ġ ������ ����
- [OUT]
* struct SMyPoint* ptLEye:	ed_find( )���� ���� ���� �� ��� ��ġ ������ ������
* struct SMyPoint* ptREye:	ed_find( )���� ���� ������ �� ��� ��ġ ������ ������
- [return]: 
0->���������� ����Ǹ� 0 ��ȯ
------------------------------------------------------------- */
int ed_where(struct SMyPoint* ptLEye, struct SMyPoint* ptREye)
{
	*ptLEye = m_ed_ptLeft;
	*ptREye = m_ed_ptRight;
	return 0;
}

/* ----------------------------------------------------------------------------------------------------
void ed_scan(int nResizeW, int nResizeH, int nSrcW, int nSrcH, unsigned short* nMCTData, int bIsRight);
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

	// pixel (x,y)�� window�� left top���� �����Ͽ� window ���� MCT������ �հ� �̸� �н��� threshold�� ��
	for(y=0; y<nSearchH; y++) {
		pMCTData = nMCTData + y*nResizeW;
		for(x=0; x<nSearchW; x+=2) {
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
				if(bIsRight) ed_enrollRight(nEyeX, nEyeY, nEyeW, nEyeH, nMCTConfidence);
				else ed_enrollLeft(nEyeX, nEyeY, nEyeW, nEyeH, nMCTConfidence);
			}
			pMCTData+=2;
		}
	}
}

/* -----------------------------------------------------------------------------
void ed_enrollLeft(int nEyeX, int nEyeY, int nEyeW, int nEyeH, int nConfidence);
- ������ ���� �� ������ �ĺ��� ���
- [IN]
* int nEyeX:					���� �� ������ x ��ǥ
* int nEyeY:					���� �� ������ y ��ǥ
* int nEyeW:					���� �� ������ ��
* int nEyeH:					���� �� ������ ����
* int nConfidence:				������ ���� �� ������ mct��
- [return]:						����
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

	// ������ ��ϵ� �ĺ��� ��ġ�鼭 MCT ���� ���ٸ�
	// ���� �ĺ��� ���� ����Ϸ��� �ĺ��� ��ü�ϰ� intersection count�� ����
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

	// ��ġ�� �ĺ����� ���ٸ� ���ο� �ĺ��� ���
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

	// ������ ��ϵǾ� �ִ� �ĺ� �� ���� ����� �ĺ��鿡 ���ؼ� ��ġ�� �ĺ����� ��� ��ħ
	// MCT ���� ���� �ĺ��� ��ü�ϰ� intersection count�� ��ħ
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
- ������ ������ �� ������ �ĺ��� ���
- [IN]
* int nEyeX:					������ �� ������ x ��ǥ
* int nEyeY:					������ �� ������ y ��ǥ
* int nEyeW:					������ �� ������ ��
* int nEyeH:					������ �� ������ ����
* int nConfidence:				������ ������ �� ������ mct��
- [return]:						����
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
- �� �簢�� ������ ��ġ������ �Ǵ�
- [IN]
* struct SMyRect rc1:		�簢�� 1
* struct SMyRect rc2:		�簢�� 2
- [return]: 
0->��ġ�� ������ 0 ��ȯ
1->��ġ�� 1 ��ȯ
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
