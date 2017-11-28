#include "stdafx.h"
#include "FaceDetectionParameters_Frontal_cas4(20만장).h"
#include "FaceDetectionParameters_ProfileL.h"
#include "FaceDetectionParameters_ProfileR.h"

#include "FaceDetection.h"
#include "ImageProcess.h"
#include <stdio.h>
#include <memory.h>
#include <malloc.h>
	
#define FD_MAX_FIND											200
#define FD_INTERSECTION_VALID_COUNT_FRONTAL		2
#define FD_INTERSECTION_VALID_COUNT_PROFILE		2
#define FD_BIT_SHIFT											15

int		m_nFaceCount = 0;								//최종 판별된 얼굴검출 개수
struct SMyRect	m_rcFace[FD_MAX_FIND];

bool	m_bFaceValid[FD_MAX_FIND];				//겹쳐진 것중에서 유효하지 않다고 판단되는 것
int		m_nFaceIntersection[FD_MAX_FIND];
int		m_nFaceConfidence[FD_MAX_FIND];		//겹쳐서 판별된 얼굴중에서 가장 낮은 값의 CONFIDENCE(낮을 수록 학습에 이용된 얼굴의 데이터와 가장 유사)
int		m_nFaceROP[FD_MAX_FIND];

int		m_nFaceFinalCount = 0;
struct SMyRect	m_rcFinalFace[FD_MAX_FIND];					//최종 판별된 얼굴검출 위치
int		m_nFaceFinalROP[FD_MAX_FIND];
int		m_nFaceFinalIntersectionCount[FD_MAX_FIND];
int		m_nFaceFinalConfidence[FD_MAX_FIND];

int		m_nFaceCandidateCountFrontal = 0;						//얼굴후보로 판별 된 개수
int		m_nFaceCandidateCountProfileL = 0;						//얼굴후보로 판별 된 개수
int		m_nFaceCandidateCountProfileR = 0;						//얼굴후보로 판별 된 개수
struct SMyRect	m_rcFaceCandidateFrontal[FD_MAX_FIND];			//얼굴후보로 판별 된 위치
struct SMyRect	m_rcFaceCandidateProfileL[FD_MAX_FIND];			//얼굴후보로 판별 된 위치
struct SMyRect	m_rcFaceCandidateProfileR[FD_MAX_FIND];			//얼굴후보로 판별 된 위치
struct SMyRect	m_rcFaceCandidateFrontalSum[FD_MAX_FIND];			//얼굴후보로 판별 된 위치
struct SMyRect	m_rcFaceCandidateProfileLSum[FD_MAX_FIND];			//얼굴후보로 판별 된 위치
struct SMyRect	m_rcFaceCandidateProfileRSum[FD_MAX_FIND];			//얼굴후보로 판별 된 위치

int		m_nFaceCandidateIntersectionCountFrontal[FD_MAX_FIND];	//겹쳐서 판별된 얼굴의 개수
int		m_nFaceCandidateIntersectionCountProfileL[FD_MAX_FIND];	//겹쳐서 판별된 얼굴의 개수
int		m_nFaceCandidateIntersectionCountProfileR[FD_MAX_FIND];	//겹쳐서 판별된 얼굴의 개수
int		m_nFaceCandidateConfidenceFrontal[FD_MAX_FIND];		//겹쳐서 판별된 얼굴중에서 가장 낮은 값의 CONFIDENCE(낮을 수록 학습에 이용된 얼굴의 데이터와 가장 유사)
int		m_nFaceCandidateConfidenceProfileL[FD_MAX_FIND];		//겹쳐서 판별된 얼굴중에서 가장 낮은 값의 CONFIDENCE(낮을 수록 학습에 이용된 얼굴의 데이터와 가장 유사)
int		m_nFaceCandidateConfidenceProfileR[FD_MAX_FIND];		//겹쳐서 판별된 얼굴중에서 가장 낮은 값의 CONFIDENCE(낮을 수록 학습에 이용된 얼굴의 데이터와 가장 유사)
bool	m_bFaceCandidateValidFrontal[FD_MAX_FIND];				//겹쳐진 것중에서 유효하지 않다고 판단되는 것
bool	m_bFaceCandidateValidProfileL[FD_MAX_FIND];				//겹쳐진 것중에서 유효하지 않다고 판단되는 것
bool	m_bFaceCandidateValidProfileR[FD_MAX_FIND];				//겹쳐진 것중에서 유효하지 않다고 판단되는 것

int fd_find_run(unsigned char* srcData, int nSrcW, int nSrcH, bool bROP);
void fd_scan_frontal(int resize_w, int resize_h, int src_w, int src_h, unsigned char* nData);		//Adaboost를 적용한 얼굴검출 수행
void fd_scan_profile(int resize_w, int resize_h, int src_w, int src_h, unsigned char* nData, int nLRMode);		//Adaboost를 적용한 얼굴검출 수행
void fd_enroll_Frontal(int nFaceX, int nFaceY, int nFaceW, int nFaceH, int nConfidence);	//검출된 얼굴좌표를 등록		
void fd_enroll_ProfileL(int nFaceX, int nFaceY, int nFaceW, int nFaceH, int nConfidence);
void fd_enroll_ProfileR(int nFaceX, int nFaceY, int nFaceW, int nFaceH, int nConfidence);
bool fd_isintersection(struct SMyRect rc1, struct SMyRect rc2);						//두개의 사각영역(검출된 얼굴영역의 좌표)을 받아서 겹치는지를 체크 
void fd_final();

int fd_find(unsigned char* srcData, int nSrcW, int nSrcH, bool bROP)						//얼굴검출 수행
{
	int i;

	m_nFaceCount = 0;
	m_nFaceFinalCount = 0;
	memset(m_nFaceConfidence, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_nFaceROP, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_bFaceValid, 0, sizeof(bool)*FD_MAX_FIND);

	fd_find_run(srcData, nSrcW, nSrcH, bROP);
	fd_final();

	m_nFaceFinalCount = 0;
	for(i=0; i<m_nFaceCount; i++) {
		if(m_bFaceValid[i]) {
			m_nFaceFinalROP[m_nFaceFinalCount] = m_nFaceROP[i];
			m_nFaceFinalIntersectionCount[m_nFaceFinalCount] = m_nFaceIntersection[i];
			m_nFaceFinalConfidence[m_nFaceFinalCount] = m_nFaceConfidence[i];
			m_rcFinalFace[m_nFaceFinalCount++] = m_rcFace[i];					
		}
	}	
	return m_nFaceFinalCount;
}

int fd_find_run(unsigned char* srcData, int nSrcW, int nSrcH, bool bROP)	//얼굴검출 수행
{	
	int i, nResizeW, nResizeH;
	int nRatioHeightAndWidth = nSrcW > nSrcH ? (nSrcH<<FD_BIT_SHIFT)/nSrcW : (nSrcW<<FD_BIT_SHIFT)/nSrcH;
	int nROPMode = 0;
	unsigned char* nResizeData = (unsigned char*)malloc(nSrcW*nSrcH);
	unsigned char* nData = (unsigned char*)malloc(sizeof(unsigned char)*nSrcW*nSrcH);

	int imgSize = nSrcW > nSrcH ? nSrcW : nSrcH;	
	int size = imgSize;
	
	m_nFaceCandidateCountFrontal = 0;	
	m_nFaceCandidateCountProfileL = 0;	
	m_nFaceCandidateCountProfileR = 0;	
	memset(m_nFaceCandidateIntersectionCountFrontal, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_nFaceCandidateIntersectionCountProfileL, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_nFaceCandidateIntersectionCountProfileR, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_nFaceCandidateConfidenceFrontal, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_nFaceCandidateConfidenceProfileL, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_nFaceCandidateConfidenceProfileR, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_bFaceCandidateValidFrontal, 0, sizeof(bool)*FD_MAX_FIND);
	memset(m_bFaceCandidateValidProfileL, 0, sizeof(bool)*FD_MAX_FIND);
	memset(m_bFaceCandidateValidProfileR, 0, sizeof(bool)*FD_MAX_FIND);
	
	while(true) {		
		nResizeW = nSrcW > nSrcH ? size : (size * nRatioHeightAndWidth)>>FD_BIT_SHIFT;
		nResizeH = nSrcW < nSrcH ? size : (size * nRatioHeightAndWidth)>>FD_BIT_SHIFT;

		if(nResizeW < 3 || nResizeH < 3) break;
		if(nResizeW != nSrcW || nResizeH != nSrcH) {
			Resize_NN(srcData, nSrcW, nSrcH, nResizeData, nResizeW, nResizeH);
			Image2LBP8_1(nResizeData, nResizeW, nResizeH, nData);
		}
		else {
			Image2LBP8_1(srcData, nSrcW, nSrcH, nData);
		}

		fd_scan_frontal(nResizeW, nResizeH, nSrcW, nSrcH, nData);
		if(bROP) { 
			fd_scan_profile(nResizeW, nResizeH, nSrcW, nSrcH, nData, 0);
			fd_scan_profile(nResizeW, nResizeH, nSrcW, nSrcH, nData, 1);
		}

		size = size - (size>>3);
		if(size < 24) break;
	}
	free(nResizeData);
	free(nData);
	
	for(i=0; i<m_nFaceCandidateCountFrontal; i++) {
		if(m_bFaceCandidateValidFrontal[i]) {
			if(m_nFaceCandidateIntersectionCountFrontal[i] > FD_INTERSECTION_VALID_COUNT_FRONTAL) {				
				m_nFaceROP[m_nFaceCount] = 0;
				m_nFaceIntersection[m_nFaceCount] = m_nFaceCandidateIntersectionCountFrontal[i];
				m_nFaceConfidence[m_nFaceCount] = m_nFaceCandidateConfidenceFrontal[i];
				m_bFaceValid[m_nFaceCount] = true;
				m_rcFace[m_nFaceCount].left = m_rcFaceCandidateFrontalSum[i].left / m_nFaceCandidateIntersectionCountFrontal[i];
				m_rcFace[m_nFaceCount].top = m_rcFaceCandidateFrontalSum[i].top / m_nFaceCandidateIntersectionCountFrontal[i];
				m_rcFace[m_nFaceCount].right = m_rcFaceCandidateFrontalSum[i].right / m_nFaceCandidateIntersectionCountFrontal[i];
				m_rcFace[m_nFaceCount].bottom = m_rcFaceCandidateFrontalSum[i].bottom / m_nFaceCandidateIntersectionCountFrontal[i];
				m_nFaceCount++;
			}
		}
	}
	for(i=0; i<m_nFaceCandidateCountProfileL; i++) {
		if(m_bFaceCandidateValidProfileL[i]) {
			if(m_nFaceCandidateIntersectionCountProfileL[i] > FD_INTERSECTION_VALID_COUNT_PROFILE) {
				m_nFaceROP[m_nFaceCount] = -90;
				m_nFaceIntersection[m_nFaceCount] = m_nFaceCandidateIntersectionCountProfileL[i];
				m_nFaceConfidence[m_nFaceCount] = m_nFaceCandidateConfidenceProfileL[i];
				m_bFaceValid[m_nFaceCount] = true;
				m_rcFace[m_nFaceCount].left = m_rcFaceCandidateProfileLSum[i].left / m_nFaceCandidateIntersectionCountProfileL[i];
				m_rcFace[m_nFaceCount].top = m_rcFaceCandidateProfileLSum[i].top / m_nFaceCandidateIntersectionCountProfileL[i];
				m_rcFace[m_nFaceCount].right = m_rcFaceCandidateProfileLSum[i].right / m_nFaceCandidateIntersectionCountProfileL[i];
				m_rcFace[m_nFaceCount].bottom = m_rcFaceCandidateProfileLSum[i].bottom / m_nFaceCandidateIntersectionCountProfileL[i];
				m_nFaceCount++;
			}
		}
	}
	for(i=0; i<m_nFaceCandidateCountProfileR; i++) {
		if(m_bFaceCandidateValidProfileR[i]) {
			if(m_nFaceCandidateIntersectionCountProfileR[i] > FD_INTERSECTION_VALID_COUNT_PROFILE) {
				m_nFaceROP[m_nFaceCount] = 90;
				m_nFaceIntersection[m_nFaceCount] = m_nFaceCandidateIntersectionCountProfileR[i];
				m_nFaceConfidence[m_nFaceCount] = m_nFaceCandidateConfidenceProfileR[i];
				m_bFaceValid[m_nFaceCount] = true;
				m_rcFace[m_nFaceCount].left = m_rcFaceCandidateProfileRSum[i].left / m_nFaceCandidateIntersectionCountProfileR[i];
				m_rcFace[m_nFaceCount].top = m_rcFaceCandidateProfileRSum[i].top / m_nFaceCandidateIntersectionCountProfileR[i];
				m_rcFace[m_nFaceCount].right = m_rcFaceCandidateProfileRSum[i].right / m_nFaceCandidateIntersectionCountProfileR[i];
				m_rcFace[m_nFaceCount].bottom = m_rcFaceCandidateProfileRSum[i].bottom / m_nFaceCandidateIntersectionCountProfileR[i];
				m_nFaceCount++;
			}
		}
	}
	return m_nFaceCount;
}

bool fd_where(struct SMyRect* rcFace, int* nFaceROP, int* nFaceIntersectionCount, int* nFaceConfidence)
{
	if(m_nFaceFinalCount == 0) return false;
	memcpy(nFaceROP, m_nFaceFinalROP, sizeof(int)*m_nFaceFinalCount);
	memcpy(nFaceIntersectionCount, m_nFaceFinalIntersectionCount, sizeof(int)*m_nFaceFinalCount);
	memcpy(nFaceConfidence, m_nFaceFinalConfidence, sizeof(int)*m_nFaceFinalCount);
	memcpy(rcFace, m_rcFinalFace, sizeof(struct SMyRect)*m_nFaceFinalCount);
	return true;
}

void fd_scan_frontal(int nResizeW, int nResizeH, int nSrcW, int nSrcH, unsigned char* nData) 
{	
	int x, y, c, t;
	int nSearchW, nSearchH;
	nSearchW = nResizeW-FD_FRONTAL_WINDOW_WIDTH;
	nSearchH = nResizeH-FD_FRONTAL_WINDOW_HEIGHT-2; 
	int nDataStep;
	int nRatioSrcPerResize = (nSrcW<<FD_BIT_SHIFT)/nResizeW;
	int nFaceX, nFaceY, nFaceW, nFaceH;
	int nAlphaCount=0;	
	int nConfidence, nOneCascadeFeatureSize;
	bool bFind = false;
	unsigned char *pData = nData;
	unsigned char *pFeatureY, *pFeatureX;
	int	nFeatureGlobalPosition[FD_FRONTAL_FEATURE_SIZE_TOTAL];
	int *pFeatureGlobalPosition = nFeatureGlobalPosition;

	nDataStep=FD_FRONTAL_WINDOW_WIDTH+nResizeW;
	//nDataStep=FD_FRONTAL_WINDOW_WIDTH;
	//for(t=0; t<FD_Y_STEP-1; t++) nDataStep += nResizeW;

	nFaceW = (FD_FRONTAL_WINDOW_WIDTH*nRatioSrcPerResize)>>FD_BIT_SHIFT;
	nFaceH = (FD_FRONTAL_WINDOW_HEIGHT*nRatioSrcPerResize)>>FD_BIT_SHIFT;
	pFeatureY = m_nFrontalFeatureY, pFeatureX = m_nFrontalFeatureX;
			
	for(t=0; t<FD_FRONTAL_FEATURE_SIZE_TOTAL;) {
		*pFeatureGlobalPosition++ = *pFeatureY++ * nResizeW + *pFeatureX++;
		*pFeatureGlobalPosition++ = *pFeatureY++ * nResizeW + *pFeatureX++;		
		t+=2;
	}
	
//	FILE* tmp_fp1 = fopen("para1.txt", "a+t");
//	FILE* tmp_fp2 = fopen("para2.txt", "a+t");
//	FILE* tmp_fp3 = fopen("para3.txt", "a+t");
//	FILE* tmp_fp4 = fopen("para4.txt", "a+t");

	for(y=0; y<nSearchH; y+=2) {		
		for(x=0; x<nSearchW; x++) {
			nAlphaCount = 0;			
			pFeatureGlobalPosition = nFeatureGlobalPosition;				
			for(c=0; c<FD_FRONTAL_CASCADE_SIZE; c++) { 
				nConfidence = 0;
				bFind = false;
				nOneCascadeFeatureSize = m_nFrontalFeatureSize[c];								
				for(t=0; t<nOneCascadeFeatureSize; ) {
					nConfidence += m_nFrontalAlpha[(nAlphaCount + *(pData + *pFeatureGlobalPosition++))];
					nAlphaCount += FD_FRONTAL_VALUE_RANGE;
					
					nConfidence += m_nFrontalAlpha[(nAlphaCount + *(pData + *pFeatureGlobalPosition++))];
					nAlphaCount += FD_FRONTAL_VALUE_RANGE;
					
					t+=2;
				}				
				if(nConfidence > m_nFrontalThreshold[c]) break;				
				bFind = true;
//				bFind = false;
//				if(c==0) fprintf(tmp_fp1, "%d ", nConfidence);
//				if(c==1) fprintf(tmp_fp2, "%d ", nConfidence);
//				if(c==2) fprintf(tmp_fp3, "%d ", nConfidence);
//				if(c==3) fprintf(tmp_fp4, "%d ", nConfidence);
			}

			if(bFind) {
				nFaceX = (x*nRatioSrcPerResize)>>FD_BIT_SHIFT;
				nFaceY = (y*nRatioSrcPerResize)>>FD_BIT_SHIFT;
				nConfidence = nConfidence >> 2;				
				fd_enroll_Frontal(nFaceX, nFaceY, nFaceW, nFaceH, nConfidence);
			}
			pData++;
		}
		pData += nDataStep;
//		fprintf(tmp_fp1, "\n");
//		fprintf(tmp_fp2, "\n");
//		fprintf(tmp_fp3, "\n");
//		fprintf(tmp_fp4, "\n");
	}
//	fclose(tmp_fp1);
//	fclose(tmp_fp2);
//	fclose(tmp_fp3);
//	fclose(tmp_fp4);
}

void fd_scan_profile(int nResizeW, int nResizeH, int nSrcW, int nSrcH, unsigned char* nData, int nLRMode) 
{	
	int x, y, c, t;
	int nSearchW, nSearchH;
	nSearchW = nResizeW-FD_PROFILE_WINDOW_WIDTH;
	nSearchH = nResizeH-FD_PROFILE_WINDOW_HEIGHT-2;
	int nDataStep;
	int nRatioSrcPerResize = (nSrcW<<FD_BIT_SHIFT)/nResizeW;
	int nFaceX, nFaceY, nFaceW, nFaceH;
	int nAlphaCount=0;	
	int nConfidence, nOneCascadeFeatureSize;
	bool bFind = false;
	unsigned char *pData = nData;
	unsigned char *pFeatureY, *pFeatureX, *pProfileAlpha;
	int	nFeatureGlobalPosition[FD_PROFILE_FEATURE_SIZE_TOTAL];
	int *pFeatureGlobalPosition = nFeatureGlobalPosition;	

	nDataStep=FD_PROFILE_WINDOW_WIDTH+nResizeW;
	nFaceW = (FD_PROFILE_WINDOW_WIDTH*nRatioSrcPerResize)>>FD_BIT_SHIFT;
	nFaceH = (FD_PROFILE_WINDOW_HEIGHT*nRatioSrcPerResize)>>FD_BIT_SHIFT;

	pFeatureY = m_nProfileFeatureY;
	if(nLRMode) { pFeatureX = m_nRProfileFeatureX; pProfileAlpha = m_nRProfileAlpha; }
	else { pFeatureX = m_nLProfileFeatureX; pProfileAlpha = m_nLProfileAlpha; }
			
	for(t=0; t<FD_PROFILE_FEATURE_SIZE_TOTAL;) {
		*pFeatureGlobalPosition++ = *pFeatureY++ * nResizeW + *pFeatureX++;
		*pFeatureGlobalPosition++ = *pFeatureY++ * nResizeW + *pFeatureX++;		
		t+=2;
	}
	
	for(y=0; y<nSearchH; y+=2) {		
		for(x=0; x<nSearchW; x++) {
			nAlphaCount = 0;			
			pFeatureGlobalPosition = nFeatureGlobalPosition;
			for(c=0; c<FD_PROFILE_CASCADE_SIZE; c++) { 
				nConfidence = 0;
				bFind = false;
				nOneCascadeFeatureSize = m_nProfileFeatureSize[c];								
				for(t=0; t<nOneCascadeFeatureSize; ) {
					nConfidence += pProfileAlpha[(nAlphaCount + *(pData + *pFeatureGlobalPosition++))];
					nAlphaCount += FD_PROFILE_VALUE_RANGE;
					
					nConfidence += pProfileAlpha[(nAlphaCount + *(pData + *pFeatureGlobalPosition++))];
					nAlphaCount += FD_PROFILE_VALUE_RANGE;
					
					t+=2;
				}
				if(nConfidence > m_nProfileThreshold[c]) break;
				bFind = true;
			}
			
			if(bFind) {
				nFaceX = (x*nRatioSrcPerResize)>>FD_BIT_SHIFT;
				nFaceY = (y*nRatioSrcPerResize)>>FD_BIT_SHIFT;
				if(nLRMode == 0) fd_enroll_ProfileL(nFaceX, nFaceY, nFaceW, nFaceH, nConfidence);
				else fd_enroll_ProfileR(nFaceX, nFaceY, nFaceW, nFaceH, nConfidence);
			}
			pData++;
		}
		pData += nDataStep;
	}
}

void fd_enroll_Frontal(int nFaceX, int nFaceY, int nFaceW, int nFaceH, int nConfidence)
{
	int i, j;
	bool bIntersection = false;
	struct SMyRect rcFace;
	if(FD_MAX_FIND <= m_nFaceCandidateCountFrontal) {		
		return;
	}
	rcFace.left = nFaceX;
	rcFace.top = nFaceY;
	rcFace.right = nFaceX + nFaceW;
	rcFace.bottom = nFaceY + nFaceH;	

	//기존에 등록된 얼굴 좌표와 교집합(IsInterction())이 존재하는지를 확인하고,
	//만일 존재하면 m_nFaceCandidateIntersectionCount를 하나 증가하고
	//기존 좌표와 현재 좌표 사이에서 nConfidence가 작은 것으로 교체를 한다.
	for(i=0; i<m_nFaceCandidateCountFrontal; i++) {
		if(m_bFaceCandidateValidFrontal[i] == false) continue;
		if(fd_isintersection(m_rcFaceCandidateFrontal[i], rcFace)) {			
			bIntersection = true;
			m_nFaceCandidateIntersectionCountFrontal[i]++;
			m_rcFaceCandidateFrontalSum[i].left += rcFace.left;
			m_rcFaceCandidateFrontalSum[i].top += rcFace.top;
			m_rcFaceCandidateFrontalSum[i].right += rcFace.right;
			m_rcFaceCandidateFrontalSum[i].bottom += rcFace.bottom;

			if(m_nFaceCandidateConfidenceFrontal[i] > nConfidence) {
				m_rcFaceCandidateFrontal[i] = rcFace;
				m_nFaceCandidateConfidenceFrontal[i] = nConfidence;
			}
			break;
		}
	}

	//기존에 등록된 얼굴 좌표와 교집합이 존재하는 것이 없으면
	//새로운 얼굴로 보고, 추가로 등록한다.
	if(bIntersection == false) {
		m_rcFaceCandidateFrontal[m_nFaceCandidateCountFrontal] = rcFace; 
		m_rcFaceCandidateFrontalSum[m_nFaceCandidateCountFrontal] = rcFace;
		m_nFaceCandidateIntersectionCountFrontal[m_nFaceCandidateCountFrontal]++;
		m_nFaceCandidateConfidenceFrontal[m_nFaceCandidateCountFrontal] = nConfidence;
		m_bFaceCandidateValidFrontal[m_nFaceCandidateCountFrontal] = true;
		m_nFaceCandidateCountFrontal++;
	}

	//기존에 등록된 것들 중에 프로그램이 진행이 되면서 상호간에 교집합이 존재하는 것이 발생할 수 있는데,
	//이때 둘 사이에서 교집합이 발생을 하면, nConfidence가 작은 것을 기준으로 재처리를 한다.
	for(i=0; i<m_nFaceCandidateCountFrontal; i++) {
		if(m_bFaceCandidateValidFrontal[i] == false) continue;
		for(j=i+1; j<m_nFaceCandidateCountFrontal; j++) {
			if(m_bFaceCandidateValidFrontal[j] == false) continue;
			if(fd_isintersection(m_rcFaceCandidateFrontal[i], m_rcFaceCandidateFrontal[j]) == false) continue;
			if(m_nFaceCandidateConfidenceFrontal[i] > m_nFaceCandidateConfidenceFrontal[j]) {	//valid is j
				m_nFaceCandidateIntersectionCountFrontal[j] += m_nFaceCandidateIntersectionCountFrontal[i];
				m_rcFaceCandidateFrontalSum[j].left += m_rcFaceCandidateFrontalSum[i].left;
				m_rcFaceCandidateFrontalSum[j].top += m_rcFaceCandidateFrontalSum[i].top;
				m_rcFaceCandidateFrontalSum[j].right += m_rcFaceCandidateFrontalSum[i].right;
				m_rcFaceCandidateFrontalSum[j].bottom += m_rcFaceCandidateFrontalSum[i].bottom;
				m_bFaceCandidateValidFrontal[i] = false;
			}
			else { //valid is i
				m_nFaceCandidateIntersectionCountFrontal[i] += m_nFaceCandidateIntersectionCountFrontal[j];
				m_rcFaceCandidateFrontalSum[i].left += m_rcFaceCandidateFrontalSum[j].left;
				m_rcFaceCandidateFrontalSum[i].top += m_rcFaceCandidateFrontalSum[j].top;
				m_rcFaceCandidateFrontalSum[i].right += m_rcFaceCandidateFrontalSum[j].right;
				m_rcFaceCandidateFrontalSum[i].bottom += m_rcFaceCandidateFrontalSum[j].bottom;
				m_bFaceCandidateValidFrontal[j] = false;
			}					
		}
	}
}

void fd_enroll_ProfileL(int nFaceX, int nFaceY, int nFaceW, int nFaceH, int nConfidence)
{
	int i, j;
	bool bIntersection = false;
	struct SMyRect rcFace;
	if(FD_MAX_FIND <= m_nFaceCandidateCountProfileL) {		
		return;
	}
	rcFace.left = nFaceX;
	rcFace.top = nFaceY;
	rcFace.right = nFaceX + nFaceW;
	rcFace.bottom = nFaceY + nFaceH;	

	//기존에 등록된 얼굴 좌표와 교집합(IsInterction())이 존재하는지를 확인하고,
	//만일 존재하면 m_nFaceCandidateIntersectionCount를 하나 증가하고
	//기존 좌표와 현재 좌표 사이에서 nConfidence가 작은 것으로 교체를 한다.
	for(i=0; i<m_nFaceCandidateCountProfileL; i++) {
		if(m_bFaceCandidateValidProfileL[i] == false) continue;
		if(fd_isintersection(m_rcFaceCandidateProfileL[i], rcFace)) {			
			bIntersection = true;
			m_nFaceCandidateIntersectionCountProfileL[i]++;
			m_rcFaceCandidateProfileLSum[i].left += rcFace.left;
			m_rcFaceCandidateProfileLSum[i].top += rcFace.top;
			m_rcFaceCandidateProfileLSum[i].right += rcFace.right;
			m_rcFaceCandidateProfileLSum[i].bottom += rcFace.bottom;	

			if(m_nFaceCandidateConfidenceProfileL[i] > nConfidence) {
				m_rcFaceCandidateProfileL[i] = rcFace;
				m_nFaceCandidateConfidenceProfileL[i] = nConfidence;
			}
			break;
		}
	}

	//기존에 등록된 얼굴 좌표와 교집합이 존재하는 것이 없으면
	//새로운 얼굴로 보고, 추가로 등록한다.
	if(bIntersection == false) {
		m_rcFaceCandidateProfileL[m_nFaceCandidateCountProfileL] = rcFace; 
		m_rcFaceCandidateProfileLSum[m_nFaceCandidateCountProfileL] = rcFace; 
		m_nFaceCandidateIntersectionCountProfileL[m_nFaceCandidateCountProfileL]++;
		m_nFaceCandidateConfidenceProfileL[m_nFaceCandidateCountProfileL] = nConfidence;
		m_bFaceCandidateValidProfileL[m_nFaceCandidateCountProfileL] = true;
		m_nFaceCandidateCountProfileL++;
	}

	//기존에 등록된 것들 중에 프로그램이 진행이 되면서 상호간에 교집합이 존재하는 것이 발생할 수 있는데,
	//이때 둘 사이에서 교집합이 발생을 하면, nConfidence가 작은 것을 기준으로 재처리를 한다.
	for(i=0; i<m_nFaceCandidateCountProfileL; i++) {
		if(m_bFaceCandidateValidProfileL[i] == false) continue;
		for(j=i+1; j<m_nFaceCandidateCountProfileL; j++) {
			if(m_bFaceCandidateValidProfileL[j] == false) continue;
			if(fd_isintersection(m_rcFaceCandidateProfileL[i], m_rcFaceCandidateProfileL[j]) == false) continue;
			if(m_nFaceCandidateConfidenceProfileL[i] > m_nFaceCandidateConfidenceProfileL[j]) {	//valid is j
				m_nFaceCandidateIntersectionCountProfileL[j] += m_nFaceCandidateIntersectionCountProfileL[i];
				m_rcFaceCandidateProfileLSum[j].left += m_rcFaceCandidateProfileLSum[i].left;
				m_rcFaceCandidateProfileLSum[j].top += m_rcFaceCandidateProfileLSum[i].top;
				m_rcFaceCandidateProfileLSum[j].right += m_rcFaceCandidateProfileLSum[i].right;
				m_rcFaceCandidateProfileLSum[j].bottom += m_rcFaceCandidateProfileLSum[i].bottom;
				m_bFaceCandidateValidProfileL[i] = false;
			}
			else { //valid is i
				m_nFaceCandidateIntersectionCountProfileL[i] += m_nFaceCandidateIntersectionCountProfileL[j];
				m_rcFaceCandidateProfileLSum[i].left += m_rcFaceCandidateProfileLSum[j].left;
				m_rcFaceCandidateProfileLSum[i].top += m_rcFaceCandidateProfileLSum[j].top;
				m_rcFaceCandidateProfileLSum[i].right += m_rcFaceCandidateProfileLSum[j].right;
				m_rcFaceCandidateProfileLSum[i].bottom += m_rcFaceCandidateProfileLSum[j].bottom;
				m_bFaceCandidateValidProfileL[j] = false;
			}					
		}
	}
}

void fd_enroll_ProfileR(int nFaceX, int nFaceY, int nFaceW, int nFaceH, int nConfidence)
{
	int i, j;
	bool bIntersection = false;
	struct SMyRect rcFace;
	if(FD_MAX_FIND <= m_nFaceCandidateCountProfileR) {		
		return;
	}
	rcFace.left = nFaceX;
	rcFace.top = nFaceY;
	rcFace.right = nFaceX + nFaceW;
	rcFace.bottom = nFaceY + nFaceH;	

	//기존에 등록된 얼굴 좌표와 교집합(IsInterction())이 존재하는지를 확인하고,
	//만일 존재하면 m_nFaceCandidateIntersectionCount를 하나 증가하고
	//기존 좌표와 현재 좌표 사이에서 nConfidence가 작은 것으로 교체를 한다.
	for(i=0; i<m_nFaceCandidateCountProfileR; i++) {
		if(m_bFaceCandidateValidProfileR[i] == false) continue;
		if(fd_isintersection(m_rcFaceCandidateProfileR[i], rcFace)) {			
			bIntersection = true;
			m_nFaceCandidateIntersectionCountProfileR[i]++;
			m_rcFaceCandidateProfileRSum[i].left += rcFace.left;
			m_rcFaceCandidateProfileRSum[i].top += rcFace.top;
			m_rcFaceCandidateProfileRSum[i].right += rcFace.right;
			m_rcFaceCandidateProfileRSum[i].bottom += rcFace.bottom;	

			if(m_nFaceCandidateConfidenceProfileR[i] > nConfidence) {
				m_rcFaceCandidateProfileR[i] = rcFace;
				m_nFaceCandidateConfidenceProfileR[i] = nConfidence;
			}
			break;
		}
	}

	//기존에 등록된 얼굴 좌표와 교집합이 존재하는 것이 없으면
	//새로운 얼굴로 보고, 추가로 등록한다.
	if(bIntersection == false) {
		m_rcFaceCandidateProfileR[m_nFaceCandidateCountProfileR] = rcFace; 
		m_rcFaceCandidateProfileRSum[m_nFaceCandidateCountProfileR] = rcFace; 
		m_nFaceCandidateIntersectionCountProfileR[m_nFaceCandidateCountProfileR]++;
		m_nFaceCandidateConfidenceProfileR[m_nFaceCandidateCountProfileR] = nConfidence;
		m_bFaceCandidateValidProfileR[m_nFaceCandidateCountProfileR] = true;
		m_nFaceCandidateCountProfileR++;
	}

	//기존에 등록된 것들 중에 프로그램이 진행이 되면서 상호간에 교집합이 존재하는 것이 발생할 수 있는데,
	//이때 둘 사이에서 교집합이 발생을 하면, nConfidence가 작은 것을 기준으로 재처리를 한다.
	for(i=0; i<m_nFaceCandidateCountProfileR; i++) {
		if(m_bFaceCandidateValidProfileR[i] == false) continue;
		for(j=i+1; j<m_nFaceCandidateCountProfileR; j++) {
			if(m_bFaceCandidateValidProfileR[j] == false) continue;
			if(fd_isintersection(m_rcFaceCandidateProfileR[i], m_rcFaceCandidateProfileR[j]) == false) continue;
			if(m_nFaceCandidateConfidenceProfileR[i] > m_nFaceCandidateConfidenceProfileR[j]) {	//valid is j
				m_nFaceCandidateIntersectionCountProfileR[j] += m_nFaceCandidateIntersectionCountProfileR[i];
				m_rcFaceCandidateProfileRSum[j].left += m_rcFaceCandidateProfileRSum[i].left;
				m_rcFaceCandidateProfileRSum[j].top += m_rcFaceCandidateProfileRSum[i].top;
				m_rcFaceCandidateProfileRSum[j].right += m_rcFaceCandidateProfileRSum[i].right;
				m_rcFaceCandidateProfileRSum[j].bottom += m_rcFaceCandidateProfileRSum[i].bottom;
				m_bFaceCandidateValidProfileR[i] = false;
			}
			else { //valid is i
				m_nFaceCandidateIntersectionCountProfileR[i] += m_nFaceCandidateIntersectionCountProfileR[j];
				m_rcFaceCandidateProfileRSum[i].left += m_rcFaceCandidateProfileRSum[j].left;
				m_rcFaceCandidateProfileRSum[i].top += m_rcFaceCandidateProfileRSum[j].top;
				m_rcFaceCandidateProfileRSum[i].right += m_rcFaceCandidateProfileRSum[j].right;
				m_rcFaceCandidateProfileRSum[i].bottom += m_rcFaceCandidateProfileRSum[j].bottom;
				m_bFaceCandidateValidProfileR[j] = false;
			}					
		}
	}
}

bool fd_isintersection(struct SMyRect rc1, struct SMyRect rc2)
{
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
	if(is_left > is_right) return false;
	if(is_top  > is_bottom) return false;
	return true;
}

void fd_final()
{
	//기존에 등록된 것들 중에 프로그램이 진행이 되면서 상호간에 교집합이 존재하는 것이 발생할 수 있는데,
	//이때 둘 사이에서 교집합이 발생을 하면, nConfidence가 작은 것을 기준으로 재처리를 한다.
	int i, j;
	for(i=0; i<m_nFaceCount; i++) {
		if(m_bFaceValid[i] == false) continue;
		for(j=i+1; j<m_nFaceCount; j++) {
			if(m_bFaceValid[j] == false) continue;
			if(fd_isintersection(m_rcFace[i], m_rcFace[j]) == false) continue;
			if(m_nFaceConfidence[i] > m_nFaceConfidence[j]) {	//valid is j
				m_nFaceIntersection[j] += m_nFaceIntersection[i];
				m_bFaceValid[i] = false;
			}
			else { //valid is i
				m_nFaceIntersection[i] += m_nFaceIntersection[j];
				m_bFaceValid[j] = false;
			}					
		}
	}
}