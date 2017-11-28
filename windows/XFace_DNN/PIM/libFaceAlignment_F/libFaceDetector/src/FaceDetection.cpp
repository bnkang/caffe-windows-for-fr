//#include "stdafx.h"

#include "../inc/FaceDetectionParameters_Frontal_boot3_40M.h"
#include "../inc/FaceDetectionParameters_HalfProfileL_boot2.h"
#include "../inc/FaceDetectionParameters_HalfProfileR_boot2.h"
#include "../inc/FaceDetectionParameters_FullProfileL_boot2.h"
#include "../inc/FaceDetectionParameters_FullProfileR_boot2.h"
#include "../inc/FaceDetectionParameters_RIP30L_boot2.h"
#include "../inc/FaceDetectionParameters_RIP30R_boot2.h"
#include "../inc/FaceDetectionParameters_ProfileL.h"
#include "../inc/FaceDetectionParameters_ProfileR.h"

#include "../inc/FaceDetection.h"
#include "../inc/FA_ImageProcess.h"
//#include "../../../../../PIMDev/XFace_Win/ImageProcess.h"
#include <stdio.h>
#include <memory.h>
#include <malloc.h>

#define FD_MAX_FIND												300
#define FD_INTERSECTION_VALID_COUNT_FRONTAL			5
#define FD_INTERSECTION_VALID_COUNT_HALFPROFILE		5
#define FD_INTERSECTION_VALID_COUNT_FULLPROFILE		5
#define FD_INTERSECTION_VALID_COUNT_RIP30				5

#define FD_BIT_SHIFT												15

int		m_nFaceCount = 0;								//\C3\D6\C1\BE \C6Ǻ\B0\B5\C8 \BE󱼰\CB\C3\E2 \B0\B3\BC\F6
struct SMyRect	m_rcFace[FD_MAX_FIND];

bool	m_bFaceValid[FD_MAX_FIND];				//\B0\E3\C3\C4\C1\F8 \B0\CD\C1߿\A1\BC\AD \C0\AFȿ\C7\CF\C1\F6 \BEʴٰ\ED \C6ǴܵǴ\C2 \B0\CD
int		m_nFaceIntersection[FD_MAX_FIND];
int		m_nFaceConfidence[FD_MAX_FIND];		//\B0\E3\C3ļ\AD \C6Ǻ\B0\B5\C8 \BE\F3\B1\BC\C1߿\A1\BC\AD \B0\A1\C0\E5 \B3\B7\C0\BA \B0\AA\C0\C7 CONFIDENCE(\B3\B7\C0\BB \BC\F6\B7\CF \C7н\C0\BF\A1 \C0̿\EB\B5\C8 \BE\F3\B1\BC\C0\C7 \B5\A5\C0\CC\C5Ϳ\CD \B0\A1\C0\E5 \C0\AF\BB\E7)
int		m_nFaceROP[FD_MAX_FIND];

int		m_nFaceFinalCount = 0;
struct SMyRect	m_rcFinalFace[FD_MAX_FIND];					//\C3\D6\C1\BE \C6Ǻ\B0\B5\C8 \BE󱼰\CB\C3\E2 \C0\A7ġ
int		m_nFaceFinalROP[FD_MAX_FIND];
int		m_nFaceFinalIntersectionCount[FD_MAX_FIND];
int		m_nFaceFinalConfidence[FD_MAX_FIND];

int		m_nFaceCandidateCountFrontal = 0;						//\BE\F3\B1\BC\C8ĺ\B8\B7\CE \C6Ǻ\B0 \B5\C8 \B0\B3\BC\F6
int		m_nFaceCandidateCountHalfProfileL = 0;						//\BE\F3\B1\BC\C8ĺ\B8\B7\CE \C6Ǻ\B0 \B5\C8 \B0\B3\BC\F6
int		m_nFaceCandidateCountHalfProfileR = 0;						//\BE\F3\B1\BC\C8ĺ\B8\B7\CE \C6Ǻ\B0 \B5\C8 \B0\B3\BC\F6
int		m_nFaceCandidateCountFullProfileL = 0;
int		m_nFaceCandidateCountFullProfileR = 0;
int		m_nFaceCandidateCountRIP30L = 0;
int		m_nFaceCandidateCountRIP30R = 0;
int		m_nFaceCandidateCountProfileL = 0;
int		m_nFaceCandidateCountProfileR = 0;

struct SMyRect	m_rcFaceCandidateFrontal[FD_MAX_FIND];			//\BE\F3\B1\BC\C8ĺ\B8\B7\CE \C6Ǻ\B0 \B5\C8 \C0\A7ġ
struct SMyRect	m_rcFaceCandidateHalfProfileL[FD_MAX_FIND];			//\BE\F3\B1\BC\C8ĺ\B8\B7\CE \C6Ǻ\B0 \B5\C8 \C0\A7ġ
struct SMyRect	m_rcFaceCandidateHalfProfileR[FD_MAX_FIND];			//\BE\F3\B1\BC\C8ĺ\B8\B7\CE \C6Ǻ\B0 \B5\C8 \C0\A7ġ
struct SMyRect	m_rcFaceCandidateFullProfileL[FD_MAX_FIND];
struct SMyRect	m_rcFaceCandidateFullProfileR[FD_MAX_FIND];
struct SMyRect	m_rcFaceCandidateRIP30L[FD_MAX_FIND];
struct SMyRect	m_rcFaceCandidateRIP30R[FD_MAX_FIND];
struct SMyRect	m_rcFaceCandidateProfileL[FD_MAX_FIND];
struct SMyRect	m_rcFaceCandidateProfileR[FD_MAX_FIND];

struct SMyRect	m_rcFaceCandidateFrontalSum[FD_MAX_FIND];			//\BE\F3\B1\BC\C8ĺ\B8\B7\CE \C6Ǻ\B0 \B5\C8 \C0\A7ġ
struct SMyRect	m_rcFaceCandidateHalfProfileLSum[FD_MAX_FIND];			//\BE\F3\B1\BC\C8ĺ\B8\B7\CE \C6Ǻ\B0 \B5\C8 \C0\A7ġ
struct SMyRect	m_rcFaceCandidateHalfProfileRSum[FD_MAX_FIND];			//\BE\F3\B1\BC\C8ĺ\B8\B7\CE \C6Ǻ\B0 \B5\C8 \C0\A7ġ
struct SMyRect	m_rcFaceCandidateFullProfileLSum[FD_MAX_FIND];
struct SMyRect	m_rcFaceCandidateFullProfileRSum[FD_MAX_FIND];
struct SMyRect	m_rcFaceCandidateRIP30LSum[FD_MAX_FIND];
struct SMyRect	m_rcFaceCandidateRIP30RSum[FD_MAX_FIND];
struct SMyRect	m_rcFaceCandidateProfileLSum[FD_MAX_FIND];
struct SMyRect	m_rcFaceCandidateProfileRSum[FD_MAX_FIND];

int		m_nFaceCandidateIntersectionCountFrontal[FD_MAX_FIND];	//\B0\E3\C3ļ\AD \C6Ǻ\B0\B5\C8 \BE\F3\B1\BC\C0\C7 \B0\B3\BC\F6
int		m_nFaceCandidateIntersectionCountHalfProfileL[FD_MAX_FIND];	//\B0\E3\C3ļ\AD \C6Ǻ\B0\B5\C8 \BE\F3\B1\BC\C0\C7 \B0\B3\BC\F6
int		m_nFaceCandidateIntersectionCountHalfProfileR[FD_MAX_FIND];	//\B0\E3\C3ļ\AD \C6Ǻ\B0\B5\C8 \BE\F3\B1\BC\C0\C7 \B0\B3\BC\F6
int		m_nFaceCandidateIntersectionCountFullProfileL[FD_MAX_FIND];
int		m_nFaceCandidateIntersectionCountFullProfileR[FD_MAX_FIND];
int		m_nFaceCandidateIntersectionCountRIP30L[FD_MAX_FIND];
int		m_nFaceCandidateIntersectionCountRIP30R[FD_MAX_FIND];
int		m_nFaceCandidateIntersectionCountProfileL[FD_MAX_FIND];
int		m_nFaceCandidateIntersectionCountProfileR[FD_MAX_FIND];

int		m_nFaceCandidateConfidenceFrontal[FD_MAX_FIND];		//\B0\E3\C3ļ\AD \C6Ǻ\B0\B5\C8 \BE\F3\B1\BC\C1߿\A1\BC\AD \B0\A1\C0\E5 \B3\B7\C0\BA \B0\AA\C0\C7 CONFIDENCE(\B3\B7\C0\BB \BC\F6\B7\CF \C7н\C0\BF\A1 \C0̿\EB\B5\C8 \BE\F3\B1\BC\C0\C7 \B5\A5\C0\CC\C5Ϳ\CD \B0\A1\C0\E5 \C0\AF\BB\E7)
int		m_nFaceCandidateConfidenceHalfProfileL[FD_MAX_FIND];		//\B0\E3\C3ļ\AD \C6Ǻ\B0\B5\C8 \BE\F3\B1\BC\C1߿\A1\BC\AD \B0\A1\C0\E5 \B3\B7\C0\BA \B0\AA\C0\C7 CONFIDENCE(\B3\B7\C0\BB \BC\F6\B7\CF \C7н\C0\BF\A1 \C0̿\EB\B5\C8 \BE\F3\B1\BC\C0\C7 \B5\A5\C0\CC\C5Ϳ\CD \B0\A1\C0\E5 \C0\AF\BB\E7)
int		m_nFaceCandidateConfidenceHalfProfileR[FD_MAX_FIND];		//\B0\E3\C3ļ\AD \C6Ǻ\B0\B5\C8 \BE\F3\B1\BC\C1߿\A1\BC\AD \B0\A1\C0\E5 \B3\B7\C0\BA \B0\AA\C0\C7 CONFIDENCE(\B3\B7\C0\BB \BC\F6\B7\CF \C7н\C0\BF\A1 \C0̿\EB\B5\C8 \BE\F3\B1\BC\C0\C7 \B5\A5\C0\CC\C5Ϳ\CD \B0\A1\C0\E5 \C0\AF\BB\E7)
int		m_nFaceCandidateConfidenceFullProfileL[FD_MAX_FIND];
int		m_nFaceCandidateConfidenceFullProfileR[FD_MAX_FIND];
int		m_nFaceCandidateConfidenceRIP30L[FD_MAX_FIND];
int		m_nFaceCandidateConfidenceRIP30R[FD_MAX_FIND];
int		m_nFaceCandidateConfidenceProfileL[FD_MAX_FIND];
int		m_nFaceCandidateConfidenceProfileR[FD_MAX_FIND];

bool	m_bFaceCandidateValidFrontal[FD_MAX_FIND];				//\B0\E3\C3\C4\C1\F8 \B0\CD\C1߿\A1\BC\AD \C0\AFȿ\C7\CF\C1\F6 \BEʴٰ\ED \C6ǴܵǴ\C2 \B0\CD
bool	m_bFaceCandidateValidHalfProfileL[FD_MAX_FIND];				//\B0\E3\C3\C4\C1\F8 \B0\CD\C1߿\A1\BC\AD \C0\AFȿ\C7\CF\C1\F6 \BEʴٰ\ED \C6ǴܵǴ\C2 \B0\CD
bool	m_bFaceCandidateValidHalfProfileR[FD_MAX_FIND];				//\B0\E3\C3\C4\C1\F8 \B0\CD\C1߿\A1\BC\AD \C0\AFȿ\C7\CF\C1\F6 \BEʴٰ\ED \C6ǴܵǴ\C2 \B0\CD
bool	m_bFaceCandidateValidFullProfileL[FD_MAX_FIND];
bool	m_bFaceCandidateValidFullProfileR[FD_MAX_FIND];
bool	m_bFaceCandidateValidRIP30L[FD_MAX_FIND];
bool	m_bFaceCandidateValidRIP30R[FD_MAX_FIND];
bool	m_bFaceCandidateValidProfileL[FD_MAX_FIND];
bool	m_bFaceCandidateValidProfileR[FD_MAX_FIND];

int fd_find_run(unsigned char* srcData, int nSrcW, int nSrcH, bool hROP, bool fROP, bool rRIP);
void fd_scan_frontal(int resize_w, int resize_h, int src_w, int src_h, unsigned char* nData, bool hROP, bool fROP, bool rRIP);		//Adaboost\B8\A6 \C0\FB\BF\EB\C7\D1 \BE󱼰\CB\C3\E2 \BC\F6\C7\E0
void fd_scan_halfprofile(int resize_w, int resize_h, int src_w, int src_h, unsigned char* nData, int nLRMode);		//Adaboost\B8\A6 \C0\FB\BF\EB\C7\D1 \BE󱼰\CB\C3\E2 \BC\F6\C7\E0
void fd_scan_fullprofile(int resize_w, int resize_h, int src_w, int src_h, unsigned char* nData, int nLRMode);		//Adaboost\B8\A6 \C0\FB\BF\EB\C7\D1 \BE󱼰\CB\C3\E2 \BC\F6\C7\E0
void fd_scan_RIP30(int resize_w, int resize_h, int src_w, int src_h, unsigned char* nData, int nLRMode);		//Adaboost\B8\A6 \C0\FB\BF\EB\C7\D1 \BE󱼰\CB\C3\E2 \BC\F6\C7\E0
void fd_scan_Profile(int resize_w, int resize_h, int src_w, int src_h, unsigned char* nData, int nLRMode);		//Adaboost\B8\A6 \C0\FB\BF\EB\C7\D1 \BE󱼰\CB\C3\E2 \BC\F6\C7\E0

void fd_enroll_Frontal(int nFaceX, int nFaceY, int nFaceW, int nFaceH, int nConfidence);	//\B0\CB\C3\E2\B5\C8 \BE\F3\B1\BC\C1\C2ǥ\B8\A6 \B5\EE\B7\CF		
void fd_enroll_HalfProfileL(int nFaceX, int nFaceY, int nFaceW, int nFaceH, int nConfidence);
void fd_enroll_HalfProfileR(int nFaceX, int nFaceY, int nFaceW, int nFaceH, int nConfidence);
void fd_enroll_FullProfileL(int nFaceX, int nFaceY, int nFaceW, int nFaceH, int nConfidence);
void fd_enroll_FullProfileR(int nFaceX, int nFaceY, int nFaceW, int nFaceH, int nConfidence);
void fd_enroll_RIP30L(int nFaceX, int nFaceY, int nFaceW, int nFaceH, int nConfidence);
void fd_enroll_RIP30R(int nFaceX, int nFaceY, int nFaceW, int nFaceH, int nConfidence);
void fd_enroll_ProfileL(int nFaceX, int nFaceY, int nFaceW, int nFaceH, int nConfidence);
void fd_enroll_ProfileR(int nFaceX, int nFaceY, int nFaceW, int nFaceH, int nConfidence);

bool fd_isintersection(struct SMyRect rc1, struct SMyRect rc2);						//\B5ΰ\B3\C0\C7 \BB簢\BF\B5\BF\AA(\B0\CB\C3\E2\B5\C8 \BE󱼿\B5\BF\AA\C0\C7 \C1\C2ǥ)\C0\BB \B9޾Ƽ\AD \B0\E3ġ\B4\C2\C1\F6\B8\A6 üũ 
void fd_final();

int fd_find(unsigned char* srcData, int nSrcW, int nSrcH, bool hROP, bool fROP, bool rRIP)						//\BE󱼰\CB\C3\E2 \BC\F6\C7\E0
{
	int i;

	m_nFaceCount = 0;
	m_nFaceFinalCount = 0;
	memset(m_nFaceConfidence, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_nFaceROP, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_bFaceValid, 0, sizeof(bool)*FD_MAX_FIND);

	fd_find_run(srcData, nSrcW, nSrcH, hROP, fROP, rRIP);
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

int fd_find_run(unsigned char* srcData, int nSrcW, int nSrcH, bool hROP, bool fROP, bool rRIP)	//\BE󱼰\CB\C3\E2 \BC\F6\C7\E0
{	
	int i, nResizeW, nResizeH;
	int nRatioHeightAndWidth = nSrcW > nSrcH ? (nSrcH<<FD_BIT_SHIFT)/nSrcW : (nSrcW<<FD_BIT_SHIFT)/nSrcH;
	int nROPMode = 0;
	unsigned char* nResizeData = (unsigned char*)malloc(nSrcW*nSrcH);
	unsigned char* nData = (unsigned char*)malloc(sizeof(unsigned char)*nSrcW*nSrcH);

	int imgSize = nSrcW > nSrcH ? nSrcW : nSrcH;	
	int size = imgSize;

	m_nFaceCandidateCountFrontal = 0;	
	m_nFaceCandidateCountHalfProfileL = 0;
	m_nFaceCandidateCountHalfProfileR = 0;
	m_nFaceCandidateCountFullProfileL = 0;
	m_nFaceCandidateCountFullProfileR = 0;
	m_nFaceCandidateCountRIP30L = 0;
	m_nFaceCandidateCountRIP30R = 0;
	m_nFaceCandidateCountProfileL = 0;
	m_nFaceCandidateCountProfileR = 0;

	memset(m_nFaceCandidateIntersectionCountFrontal, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_nFaceCandidateIntersectionCountHalfProfileL, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_nFaceCandidateIntersectionCountHalfProfileR, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_nFaceCandidateIntersectionCountFullProfileL, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_nFaceCandidateIntersectionCountFullProfileR, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_nFaceCandidateIntersectionCountRIP30L, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_nFaceCandidateIntersectionCountRIP30R, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_nFaceCandidateIntersectionCountProfileL, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_nFaceCandidateIntersectionCountProfileR, 0, sizeof(int)*FD_MAX_FIND);

	memset(m_nFaceCandidateConfidenceFrontal, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_nFaceCandidateConfidenceHalfProfileL, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_nFaceCandidateConfidenceHalfProfileR, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_nFaceCandidateConfidenceFullProfileL, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_nFaceCandidateConfidenceFullProfileR, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_nFaceCandidateConfidenceRIP30L, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_nFaceCandidateConfidenceRIP30R, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_nFaceCandidateConfidenceProfileL, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_nFaceCandidateConfidenceProfileR, 0, sizeof(int)*FD_MAX_FIND);

	memset(m_bFaceCandidateValidFrontal, 0, sizeof(bool)*FD_MAX_FIND);
	memset(m_bFaceCandidateValidHalfProfileL, 0, sizeof(bool)*FD_MAX_FIND);
	memset(m_bFaceCandidateValidHalfProfileR, 0, sizeof(bool)*FD_MAX_FIND);
	memset(m_bFaceCandidateValidFullProfileL, 0, sizeof(bool)*FD_MAX_FIND);
	memset(m_bFaceCandidateValidFullProfileR, 0, sizeof(bool)*FD_MAX_FIND);
	memset(m_bFaceCandidateValidRIP30L, 0, sizeof(bool)*FD_MAX_FIND);
	memset(m_bFaceCandidateValidRIP30R, 0, sizeof(bool)*FD_MAX_FIND);
	memset(m_bFaceCandidateValidProfileL, 0, sizeof(bool)*FD_MAX_FIND);
	memset(m_bFaceCandidateValidProfileR, 0, sizeof(bool)*FD_MAX_FIND);

	while(true) {		
		nResizeW = nSrcW > nSrcH ? size : (size * nRatioHeightAndWidth)>>FD_BIT_SHIFT;
		nResizeH = nSrcW < nSrcH ? size : (size * nRatioHeightAndWidth)>>FD_BIT_SHIFT;

		if(nResizeW != nSrcW || nResizeH != nSrcH) {
			Resize_BI(srcData, nSrcW, nSrcH, nResizeData, nResizeW, nResizeH);
			Image2LBP8_1(nResizeData, nResizeW, nResizeH, nData);
		}
		else {
			Image2LBP8_1(srcData, nSrcW, nSrcH, nData);
		}

		fd_scan_frontal(nResizeW, nResizeH, nSrcW, nSrcH, nData, hROP, fROP, rRIP);

		if(hROP) { 
			fd_scan_halfprofile(nResizeW, nResizeH, nSrcW, nSrcH, nData, 0);
			fd_scan_halfprofile(nResizeW, nResizeH, nSrcW, nSrcH, nData, 1);
		}
		if(fROP) { 
			fd_scan_fullprofile(nResizeW, nResizeH, nSrcW, nSrcH, nData, 0);
			fd_scan_fullprofile(nResizeW, nResizeH, nSrcW, nSrcH, nData, 1);
		}
		if(rRIP) {
			fd_scan_RIP30(nResizeW, nResizeH, nSrcW, nSrcH, nData, 0);
			fd_scan_RIP30(nResizeW, nResizeH, nSrcW, nSrcH, nData, 1);
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
	for(i=0; i<m_nFaceCandidateCountHalfProfileL; i++) {
		if(m_bFaceCandidateValidHalfProfileL[i]) {
			if(m_nFaceCandidateIntersectionCountHalfProfileL[i] > FD_INTERSECTION_VALID_COUNT_HALFPROFILE) {
				m_nFaceROP[m_nFaceCount] = -30;
				m_nFaceIntersection[m_nFaceCount] = m_nFaceCandidateIntersectionCountHalfProfileL[i];
				m_nFaceConfidence[m_nFaceCount] = m_nFaceCandidateConfidenceHalfProfileL[i];
				m_bFaceValid[m_nFaceCount] = true;
				m_rcFace[m_nFaceCount].left = m_rcFaceCandidateHalfProfileLSum[i].left / m_nFaceCandidateIntersectionCountHalfProfileL[i];
				m_rcFace[m_nFaceCount].top = m_rcFaceCandidateHalfProfileLSum[i].top / m_nFaceCandidateIntersectionCountHalfProfileL[i];
				m_rcFace[m_nFaceCount].right = m_rcFaceCandidateHalfProfileLSum[i].right / m_nFaceCandidateIntersectionCountHalfProfileL[i];
				m_rcFace[m_nFaceCount].bottom = m_rcFaceCandidateHalfProfileLSum[i].bottom / m_nFaceCandidateIntersectionCountHalfProfileL[i];
				m_nFaceCount++;
			}
		}
	}
	for(i=0; i<m_nFaceCandidateCountHalfProfileR; i++) {
		if(m_bFaceCandidateValidHalfProfileR[i]) {
			if(m_nFaceCandidateIntersectionCountHalfProfileR[i] > FD_INTERSECTION_VALID_COUNT_HALFPROFILE) {
				m_nFaceROP[m_nFaceCount] = 30;
				m_nFaceIntersection[m_nFaceCount] = m_nFaceCandidateIntersectionCountHalfProfileR[i];
				m_nFaceConfidence[m_nFaceCount] = m_nFaceCandidateConfidenceHalfProfileR[i];
				m_bFaceValid[m_nFaceCount] = true;
				m_rcFace[m_nFaceCount].left = m_rcFaceCandidateHalfProfileRSum[i].left / m_nFaceCandidateIntersectionCountHalfProfileR[i];
				m_rcFace[m_nFaceCount].top = m_rcFaceCandidateHalfProfileRSum[i].top / m_nFaceCandidateIntersectionCountHalfProfileR[i];
				m_rcFace[m_nFaceCount].right = m_rcFaceCandidateHalfProfileRSum[i].right / m_nFaceCandidateIntersectionCountHalfProfileR[i];
				m_rcFace[m_nFaceCount].bottom = m_rcFaceCandidateHalfProfileRSum[i].bottom / m_nFaceCandidateIntersectionCountHalfProfileR[i];
				m_nFaceCount++;
			}
		}
	}
	for(i=0; i<m_nFaceCandidateCountFullProfileL; i++) {
		if(m_bFaceCandidateValidFullProfileL[i]) {
			if(m_nFaceCandidateIntersectionCountFullProfileL[i] > FD_INTERSECTION_VALID_COUNT_FULLPROFILE) {
				m_nFaceROP[m_nFaceCount] = -60;
				m_nFaceIntersection[m_nFaceCount] = m_nFaceCandidateIntersectionCountFullProfileL[i];
				m_nFaceConfidence[m_nFaceCount] = m_nFaceCandidateConfidenceFullProfileL[i];
				m_bFaceValid[m_nFaceCount] = true;
				m_rcFace[m_nFaceCount].left = m_rcFaceCandidateFullProfileLSum[i].left / m_nFaceCandidateIntersectionCountFullProfileL[i];
				m_rcFace[m_nFaceCount].top = m_rcFaceCandidateFullProfileLSum[i].top / m_nFaceCandidateIntersectionCountFullProfileL[i];
				m_rcFace[m_nFaceCount].right = m_rcFaceCandidateFullProfileLSum[i].right / m_nFaceCandidateIntersectionCountFullProfileL[i];
				m_rcFace[m_nFaceCount].bottom = m_rcFaceCandidateFullProfileLSum[i].bottom / m_nFaceCandidateIntersectionCountFullProfileL[i];
				m_nFaceCount++;
			}
		}
	}
	for(i=0; i<m_nFaceCandidateCountFullProfileR; i++) {
		if(m_bFaceCandidateValidFullProfileR[i]) {
			if(m_nFaceCandidateIntersectionCountFullProfileR[i] > FD_INTERSECTION_VALID_COUNT_FULLPROFILE) {
				m_nFaceROP[m_nFaceCount] = 60;
				m_nFaceIntersection[m_nFaceCount] = m_nFaceCandidateIntersectionCountFullProfileR[i];
				m_nFaceConfidence[m_nFaceCount] = m_nFaceCandidateConfidenceFullProfileR[i];
				m_bFaceValid[m_nFaceCount] = true;
				m_rcFace[m_nFaceCount].left = m_rcFaceCandidateFullProfileRSum[i].left / m_nFaceCandidateIntersectionCountFullProfileR[i];
				m_rcFace[m_nFaceCount].top = m_rcFaceCandidateFullProfileRSum[i].top / m_nFaceCandidateIntersectionCountFullProfileR[i];
				m_rcFace[m_nFaceCount].right = m_rcFaceCandidateFullProfileRSum[i].right / m_nFaceCandidateIntersectionCountFullProfileR[i];
				m_rcFace[m_nFaceCount].bottom = m_rcFaceCandidateFullProfileRSum[i].bottom / m_nFaceCandidateIntersectionCountFullProfileR[i];
				m_nFaceCount++;
			}
		}
	}
	for(i=0; i<m_nFaceCandidateCountRIP30L; i++) {
		if(m_bFaceCandidateValidRIP30L[i]) {
			if(m_nFaceCandidateIntersectionCountRIP30L[i] > FD_INTERSECTION_VALID_COUNT_RIP30) {
				m_nFaceROP[m_nFaceCount] = 15;
				m_nFaceIntersection[m_nFaceCount] = m_nFaceCandidateIntersectionCountRIP30L[i];
				m_nFaceConfidence[m_nFaceCount] = m_nFaceCandidateConfidenceRIP30L[i];
				m_bFaceValid[m_nFaceCount] = true;
				m_rcFace[m_nFaceCount].left = m_rcFaceCandidateRIP30LSum[i].left / m_nFaceCandidateIntersectionCountRIP30L[i];
				m_rcFace[m_nFaceCount].top = m_rcFaceCandidateRIP30LSum[i].top / m_nFaceCandidateIntersectionCountRIP30L[i];
				m_rcFace[m_nFaceCount].right = m_rcFaceCandidateRIP30LSum[i].right / m_nFaceCandidateIntersectionCountRIP30L[i];
				m_rcFace[m_nFaceCount].bottom = m_rcFaceCandidateRIP30LSum[i].bottom / m_nFaceCandidateIntersectionCountRIP30L[i];
				m_nFaceCount++;
			}
		}
	}
	for(i=0; i<m_nFaceCandidateCountRIP30R; i++) {
		if(m_bFaceCandidateValidRIP30R[i]) {
			if(m_nFaceCandidateIntersectionCountRIP30R[i] > FD_INTERSECTION_VALID_COUNT_RIP30) {
				m_nFaceROP[m_nFaceCount] = -15;
				m_nFaceIntersection[m_nFaceCount] = m_nFaceCandidateIntersectionCountRIP30R[i];
				m_nFaceConfidence[m_nFaceCount] = m_nFaceCandidateConfidenceRIP30R[i];
				m_bFaceValid[m_nFaceCount] = true;
				m_rcFace[m_nFaceCount].left = m_rcFaceCandidateRIP30RSum[i].left / m_nFaceCandidateIntersectionCountRIP30R[i];
				m_rcFace[m_nFaceCount].top = m_rcFaceCandidateRIP30RSum[i].top / m_nFaceCandidateIntersectionCountRIP30R[i];
				m_rcFace[m_nFaceCount].right = m_rcFaceCandidateRIP30RSum[i].right / m_nFaceCandidateIntersectionCountRIP30R[i];
				m_rcFace[m_nFaceCount].bottom = m_rcFaceCandidateRIP30RSum[i].bottom / m_nFaceCandidateIntersectionCountRIP30R[i];
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

void fd_scan_frontal(int nResizeW, int nResizeH, int nSrcW, int nSrcH, unsigned char* nData, bool hROP, bool fROP, bool rRIP) 
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
	nFaceW = (FD_FRONTAL_WINDOW_WIDTH*nRatioSrcPerResize)>>FD_BIT_SHIFT;
	nFaceH = (FD_FRONTAL_WINDOW_HEIGHT*nRatioSrcPerResize)>>FD_BIT_SHIFT;
	pFeatureY = m_nFrontalFeatureY, pFeatureX = m_nFrontalFeatureX;

	for(t=0; t<FD_FRONTAL_FEATURE_SIZE_TOTAL;) {
		*pFeatureGlobalPosition++ = *pFeatureY++ * nResizeW + *pFeatureX++;
		*pFeatureGlobalPosition++ = *pFeatureY++ * nResizeW + *pFeatureX++;		
		t+=2;
	}

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
	}
}

void fd_scan_halfprofile(int nResizeW, int nResizeH, int nSrcW, int nSrcH, unsigned char* nData, int nLRMode) 
{	
	int x, y, c, t;
	int nSearchW, nSearchH;
	nSearchW = nResizeW-FD_HALFPROFILE_WINDOW_WIDTH;
	nSearchH = nResizeH-FD_HALFPROFILE_WINDOW_HEIGHT-2;
	int nDataStep;
	int nRatioSrcPerResize = (nSrcW<<FD_BIT_SHIFT)/nResizeW;
	int nFaceX, nFaceY, nFaceW, nFaceH;
	int nAlphaCount=0;	
	int nConfidence, nOneCascadeFeatureSize;
	bool bFind = false;
	unsigned char *pData = nData;
	unsigned char *pFeatureY, *pFeatureX, *pProfileAlpha;
	int	nFeatureGlobalPosition[FD_HALFPROFILE_FEATURE_SIZE_TOTAL];
	int *pFeatureGlobalPosition = nFeatureGlobalPosition;	

	nDataStep=FD_HALFPROFILE_WINDOW_WIDTH+nResizeW;
	nFaceW = (FD_HALFPROFILE_WINDOW_WIDTH*nRatioSrcPerResize)>>FD_BIT_SHIFT;
	nFaceH = (FD_HALFPROFILE_WINDOW_HEIGHT*nRatioSrcPerResize)>>FD_BIT_SHIFT;

	pFeatureY = m_nHalfProfileFeatureY;
	if(nLRMode) { pFeatureX = m_nRHalfProfileFeatureX; pProfileAlpha = m_nRHalfProfileAlpha; }
	else { pFeatureX = m_nLHalfProfileFeatureX; pProfileAlpha = m_nLHalfProfileAlpha; }

	for(t=0; t<FD_HALFPROFILE_FEATURE_SIZE_TOTAL;) {
		*pFeatureGlobalPosition++ = *pFeatureY++ * nResizeW + *pFeatureX++;
		*pFeatureGlobalPosition++ = *pFeatureY++ * nResizeW + *pFeatureX++;		
		t+=2;
	}

	for(y=0; y<nSearchH; y+=2) {		
		for(x=0; x<nSearchW; x++) {
			nAlphaCount = 0;			
			pFeatureGlobalPosition = nFeatureGlobalPosition;
			for(c=0; c<FD_HALFPROFILE_CASCADE_SIZE; c++) { 
				nConfidence = 0;
				bFind = false;
				nOneCascadeFeatureSize = m_nHalfProfileFeatureSize[c];								
				for(t=0; t<nOneCascadeFeatureSize; ) {
					nConfidence += pProfileAlpha[(nAlphaCount + *(pData + *pFeatureGlobalPosition++))];
					nAlphaCount += FD_HALFPROFILE_VALUE_RANGE;

					nConfidence += pProfileAlpha[(nAlphaCount + *(pData + *pFeatureGlobalPosition++))];
					nAlphaCount += FD_HALFPROFILE_VALUE_RANGE;

					t+=2;
				}
				if(nConfidence > m_nHalfProfileThreshold[c]) break;
				bFind = true;
			}

			if(bFind) {
				nFaceX = (x*nRatioSrcPerResize)>>FD_BIT_SHIFT;
				nFaceY = (y*nRatioSrcPerResize)>>FD_BIT_SHIFT;
				if(nLRMode == 0) fd_enroll_HalfProfileL(nFaceX, nFaceY, nFaceW, nFaceH, nConfidence);
				else fd_enroll_HalfProfileR(nFaceX, nFaceY, nFaceW, nFaceH, nConfidence);
			}
			pData++;
		}
		pData += nDataStep;
	}
}

void fd_scan_fullprofile(int nResizeW, int nResizeH, int nSrcW, int nSrcH, unsigned char* nData, int nLRMode) 
{	
	int x, y, c, t;
	int nSearchW, nSearchH;
	nSearchW = nResizeW-FD_FULLPROFILE_WINDOW_WIDTH;
	nSearchH = nResizeH-FD_FULLPROFILE_WINDOW_HEIGHT-2;
	int nDataStep;
	int nRatioSrcPerResize = (nSrcW<<FD_BIT_SHIFT)/nResizeW;
	int nFaceX, nFaceY, nFaceW, nFaceH;
	int nAlphaCount=0;	
	int nConfidence, nOneCascadeFeatureSize;
	bool bFind = false;
	unsigned char *pData = nData;
	unsigned char *pFeatureY, *pFeatureX, *pProfileAlpha;
	int	nFeatureGlobalPosition[FD_FULLPROFILE_FEATURE_SIZE_TOTAL];
	int *pFeatureGlobalPosition = nFeatureGlobalPosition;	

	nDataStep=FD_FULLPROFILE_WINDOW_WIDTH+nResizeW;
	nFaceW = (FD_FULLPROFILE_WINDOW_WIDTH*nRatioSrcPerResize)>>FD_BIT_SHIFT;
	nFaceH = (FD_FULLPROFILE_WINDOW_HEIGHT*nRatioSrcPerResize)>>FD_BIT_SHIFT;

	pFeatureY = m_nFullProfileFeatureY;
	if(nLRMode) { pFeatureX = m_nRFullProfileFeatureX; pProfileAlpha = m_nRFullProfileAlpha; }
	else { pFeatureX = m_nLFullProfileFeatureX; pProfileAlpha = m_nLFullProfileAlpha; }

	for(t=0; t<FD_FULLPROFILE_FEATURE_SIZE_TOTAL;) {
		*pFeatureGlobalPosition++ = *pFeatureY++ * nResizeW + *pFeatureX++;
		*pFeatureGlobalPosition++ = *pFeatureY++ * nResizeW + *pFeatureX++;		
		t+=2;
	}

	for(y=0; y<nSearchH; y+=2) {		
		for(x=0; x<nSearchW; x++) {
			nAlphaCount = 0;			
			pFeatureGlobalPosition = nFeatureGlobalPosition;
			for(c=0; c<FD_FULLPROFILE_CASCADE_SIZE; c++) { 
				nConfidence = 0;
				bFind = false;
				nOneCascadeFeatureSize = m_nFullProfileFeatureSize[c];								
				for(t=0; t<nOneCascadeFeatureSize; ) {
					nConfidence += pProfileAlpha[(nAlphaCount + *(pData + *pFeatureGlobalPosition++))];
					nAlphaCount += FD_FULLPROFILE_VALUE_RANGE;

					nConfidence += pProfileAlpha[(nAlphaCount + *(pData + *pFeatureGlobalPosition++))];
					nAlphaCount += FD_FULLPROFILE_VALUE_RANGE;

					t+=2;
				}
				if(nConfidence > m_nFullProfileThreshold[c]) break;
				bFind = true;
			}

			if(bFind) {
				nFaceX = (x*nRatioSrcPerResize)>>FD_BIT_SHIFT;
				nFaceY = (y*nRatioSrcPerResize)>>FD_BIT_SHIFT;
				if(nLRMode == 0) fd_enroll_FullProfileL(nFaceX, nFaceY, nFaceW, nFaceH, nConfidence);
				else fd_enroll_FullProfileR(nFaceX, nFaceY, nFaceW, nFaceH, nConfidence);
			}
			pData++;
		}
		pData += nDataStep;
	}
}

void fd_scan_RIP30(int nResizeW, int nResizeH, int nSrcW, int nSrcH, unsigned char* nData, int nLRMode) 
{	
	int x, y, c, t;
	int nSearchW, nSearchH;
	nSearchW = nResizeW-FD_RIP30_WINDOW_WIDTH;
	nSearchH = nResizeH-FD_RIP30_WINDOW_HEIGHT-2;
	int nDataStep;
	int nRatioSrcPerResize = (nSrcW<<FD_BIT_SHIFT)/nResizeW;
	int nFaceX, nFaceY, nFaceW, nFaceH;
	int nAlphaCount=0;	
	int nConfidence, nOneCascadeFeatureSize;
	bool bFind = false;
	unsigned char *pData = nData;
	unsigned char *pFeatureY, *pFeatureX, *pProfileAlpha;
	int	nFeatureGlobalPosition[FD_RIP30_FEATURE_SIZE_TOTAL];
	int *pFeatureGlobalPosition = nFeatureGlobalPosition;	

	nDataStep=FD_RIP30_WINDOW_WIDTH+nResizeW;
	nFaceW = (FD_RIP30_WINDOW_WIDTH*nRatioSrcPerResize)>>FD_BIT_SHIFT;
	nFaceH = (FD_RIP30_WINDOW_HEIGHT*nRatioSrcPerResize)>>FD_BIT_SHIFT;

	pFeatureY = m_nRIP30FeatureY;
	if(nLRMode) { pFeatureX = m_nRRIP30FeatureX; pProfileAlpha = m_nRRIP30Alpha; }
	else { pFeatureX = m_nLRIP30FeatureX; pProfileAlpha = m_nLRIP30Alpha; }

	for(t=0; t<FD_RIP30_FEATURE_SIZE_TOTAL;) {
		*pFeatureGlobalPosition++ = *pFeatureY++ * nResizeW + *pFeatureX++;
		*pFeatureGlobalPosition++ = *pFeatureY++ * nResizeW + *pFeatureX++;		
		t+=2;
	}

	for(y=0; y<nSearchH; y+=2) {		
		for(x=0; x<nSearchW; x++) {
			nAlphaCount = 0;			
			pFeatureGlobalPosition = nFeatureGlobalPosition;
			for(c=0; c<FD_RIP30_CASCADE_SIZE; c++) { 
				nConfidence = 0;
				bFind = false;
				nOneCascadeFeatureSize = m_nRIP30FeatureSize[c];								
				for(t=0; t<nOneCascadeFeatureSize; ) {
					nConfidence += pProfileAlpha[(nAlphaCount + *(pData + *pFeatureGlobalPosition++))];
					nAlphaCount += FD_RIP30_VALUE_RANGE;

					nConfidence += pProfileAlpha[(nAlphaCount + *(pData + *pFeatureGlobalPosition++))];
					nAlphaCount += FD_RIP30_VALUE_RANGE;

					t+=2;
				}
				if(nConfidence > m_nRIP30Threshold[c]) break;
				//TRACE("nConfidence: %d, m_nRIP30Threshold[%d]: %d \n", nConfidence, c, m_nRIP30Threshold[c]);
				bFind = true;
			}

			//TRACE("nConfidence: %d, m_nRIP30Threshold[%d]: %d \n", nConfidence, c, m_nRIP30Threshold[c]);
			if(bFind) {
				nFaceX = (x*nRatioSrcPerResize)>>FD_BIT_SHIFT;
				nFaceY = (y*nRatioSrcPerResize)>>FD_BIT_SHIFT;
				if(nLRMode == 0) fd_enroll_RIP30L(nFaceX, nFaceY, nFaceW, nFaceH, nConfidence);
				else fd_enroll_RIP30R(nFaceX, nFaceY, nFaceW, nFaceH, nConfidence);
			}
			pData++;
		}
		pData += nDataStep;
	}
}

void fd_scan_Profile(int nResizeW, int nResizeH, int nSrcW, int nSrcH, unsigned char* nData, int nLRMode) 
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
				//TRACE("nConfidence: %d, m_nRIP30Threshold[%d]: %d \n", nConfidence, c, m_nRIP30Threshold[c]);
				bFind = true;
			}

			//TRACE("nConfidence: %d, m_nRIP30Threshold[%d]: %d \n", nConfidence, c, m_nRIP30Threshold[c]);
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

	//\B1\E2\C1\B8\BF\A1 \B5\EE\B7ϵ\C8 \BE\F3\B1\BC \C1\C2ǥ\BF\CD \B1\B3\C1\FD\C7\D5(IsInterction())\C0\CC \C1\B8\C0\E7\C7ϴ\C2\C1\F6\B8\A6 Ȯ\C0\CE\C7ϰ\ED,
	//\B8\B8\C0\CF \C1\B8\C0\E7\C7ϸ\E9 m_nFaceCandidateIntersectionCount\B8\A6 \C7ϳ\AA \C1\F5\B0\A1\C7ϰ\ED
	//\B1\E2\C1\B8 \C1\C2ǥ\BF\CD \C7\F6\C0\E7 \C1\C2ǥ \BB\E7\C0̿\A1\BC\AD nConfidence\B0\A1 \C0\DB\C0\BA \B0\CD\C0\B8\B7\CE \B1\B3ü\B8\A6 \C7Ѵ\D9.
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

	//\B1\E2\C1\B8\BF\A1 \B5\EE\B7ϵ\C8 \BE\F3\B1\BC \C1\C2ǥ\BF\CD \B1\B3\C1\FD\C7\D5\C0\CC \C1\B8\C0\E7\C7ϴ\C2 \B0\CD\C0\CC \BE\F8\C0\B8\B8\E9
	//\BB\F5\B7ο\EE \BE󱼷\CE \BA\B8\B0\ED, \C3߰\A1\B7\CE \B5\EE\B7\CF\C7Ѵ\D9.
	if(bIntersection == false) {
		m_rcFaceCandidateFrontal[m_nFaceCandidateCountFrontal] = rcFace; 
		m_rcFaceCandidateFrontalSum[m_nFaceCandidateCountFrontal] = rcFace;
		m_nFaceCandidateIntersectionCountFrontal[m_nFaceCandidateCountFrontal]++;
		m_nFaceCandidateConfidenceFrontal[m_nFaceCandidateCountFrontal] = nConfidence;
		m_bFaceCandidateValidFrontal[m_nFaceCandidateCountFrontal] = true;
		m_nFaceCandidateCountFrontal++;
	}

	//\B1\E2\C1\B8\BF\A1 \B5\EE\B7ϵ\C8 \B0͵\E9 \C1߿\A1 \C7\C1\B7α׷\A5\C0\CC \C1\F8\C7\E0\C0\CC \B5Ǹ鼭 \BB\F3ȣ\B0\A3\BF\A1 \B1\B3\C1\FD\C7\D5\C0\CC \C1\B8\C0\E7\C7ϴ\C2 \B0\CD\C0\CC \B9߻\FD\C7\D2 \BC\F6 \C0ִµ\A5,
	//\C0̶\A7 \B5\D1 \BB\E7\C0̿\A1\BC\AD \B1\B3\C1\FD\C7\D5\C0\CC \B9߻\FD\C0\BB \C7ϸ\E9, nConfidence\B0\A1 \C0\DB\C0\BA \B0\CD\C0\BB \B1\E2\C1\D8\C0\B8\B7\CE \C0\E7ó\B8\AE\B8\A6 \C7Ѵ\D9.
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

void fd_enroll_HalfProfileL(int nFaceX, int nFaceY, int nFaceW, int nFaceH, int nConfidence)
{
	int i, j;
	bool bIntersection = false;
	struct SMyRect rcFace;
	if(FD_MAX_FIND <= m_nFaceCandidateCountHalfProfileL) {		
		return;
	}
	rcFace.left = nFaceX;
	rcFace.top = nFaceY;
	rcFace.right = nFaceX + nFaceW;
	rcFace.bottom = nFaceY + nFaceH;	

	//\B1\E2\C1\B8\BF\A1 \B5\EE\B7ϵ\C8 \BE\F3\B1\BC \C1\C2ǥ\BF\CD \B1\B3\C1\FD\C7\D5(IsInterction())\C0\CC \C1\B8\C0\E7\C7ϴ\C2\C1\F6\B8\A6 Ȯ\C0\CE\C7ϰ\ED,
	//\B8\B8\C0\CF \C1\B8\C0\E7\C7ϸ\E9 m_nFaceCandidateIntersectionCount\B8\A6 \C7ϳ\AA \C1\F5\B0\A1\C7ϰ\ED
	//\B1\E2\C1\B8 \C1\C2ǥ\BF\CD \C7\F6\C0\E7 \C1\C2ǥ \BB\E7\C0̿\A1\BC\AD nConfidence\B0\A1 \C0\DB\C0\BA \B0\CD\C0\B8\B7\CE \B1\B3ü\B8\A6 \C7Ѵ\D9.
	for(i=0; i<m_nFaceCandidateCountHalfProfileL; i++) {
		if(m_bFaceCandidateValidHalfProfileL[i] == false) continue;
		if(fd_isintersection(m_rcFaceCandidateHalfProfileL[i], rcFace)) {			
			bIntersection = true;
			m_nFaceCandidateIntersectionCountHalfProfileL[i]++;
			m_rcFaceCandidateHalfProfileLSum[i].left += rcFace.left;
			m_rcFaceCandidateHalfProfileLSum[i].top += rcFace.top;
			m_rcFaceCandidateHalfProfileLSum[i].right += rcFace.right;
			m_rcFaceCandidateHalfProfileLSum[i].bottom += rcFace.bottom;	

			if(m_nFaceCandidateConfidenceHalfProfileL[i] > nConfidence) {
				m_rcFaceCandidateHalfProfileL[i] = rcFace;
				m_nFaceCandidateConfidenceHalfProfileL[i] = nConfidence;
			}
			break;
		}
	}

	//\B1\E2\C1\B8\BF\A1 \B5\EE\B7ϵ\C8 \BE\F3\B1\BC \C1\C2ǥ\BF\CD \B1\B3\C1\FD\C7\D5\C0\CC \C1\B8\C0\E7\C7ϴ\C2 \B0\CD\C0\CC \BE\F8\C0\B8\B8\E9
	//\BB\F5\B7ο\EE \BE󱼷\CE \BA\B8\B0\ED, \C3߰\A1\B7\CE \B5\EE\B7\CF\C7Ѵ\D9.
	if(bIntersection == false) {
		m_rcFaceCandidateHalfProfileL[m_nFaceCandidateCountHalfProfileL] = rcFace; 
		m_rcFaceCandidateHalfProfileLSum[m_nFaceCandidateCountHalfProfileL] = rcFace; 
		m_nFaceCandidateIntersectionCountHalfProfileL[m_nFaceCandidateCountHalfProfileL]++;
		m_nFaceCandidateConfidenceHalfProfileL[m_nFaceCandidateCountHalfProfileL] = nConfidence;
		m_bFaceCandidateValidHalfProfileL[m_nFaceCandidateCountHalfProfileL] = true;
		m_nFaceCandidateCountHalfProfileL++;
	}

	//\B1\E2\C1\B8\BF\A1 \B5\EE\B7ϵ\C8 \B0͵\E9 \C1߿\A1 \C7\C1\B7α׷\A5\C0\CC \C1\F8\C7\E0\C0\CC \B5Ǹ鼭 \BB\F3ȣ\B0\A3\BF\A1 \B1\B3\C1\FD\C7\D5\C0\CC \C1\B8\C0\E7\C7ϴ\C2 \B0\CD\C0\CC \B9߻\FD\C7\D2 \BC\F6 \C0ִµ\A5,
	//\C0̶\A7 \B5\D1 \BB\E7\C0̿\A1\BC\AD \B1\B3\C1\FD\C7\D5\C0\CC \B9߻\FD\C0\BB \C7ϸ\E9, nConfidence\B0\A1 \C0\DB\C0\BA \B0\CD\C0\BB \B1\E2\C1\D8\C0\B8\B7\CE \C0\E7ó\B8\AE\B8\A6 \C7Ѵ\D9.
	for(i=0; i<m_nFaceCandidateCountHalfProfileL; i++) {
		if(m_bFaceCandidateValidHalfProfileL[i] == false) continue;
		for(j=i+1; j<m_nFaceCandidateCountHalfProfileL; j++) {
			if(m_bFaceCandidateValidHalfProfileL[j] == false) continue;
			if(fd_isintersection(m_rcFaceCandidateHalfProfileL[i], m_rcFaceCandidateHalfProfileL[j]) == false) continue;
			if(m_nFaceCandidateConfidenceHalfProfileL[i] > m_nFaceCandidateConfidenceHalfProfileL[j]) {	//valid is j
				m_nFaceCandidateIntersectionCountHalfProfileL[j] += m_nFaceCandidateIntersectionCountHalfProfileL[i];
				m_rcFaceCandidateHalfProfileLSum[j].left += m_rcFaceCandidateHalfProfileLSum[i].left;
				m_rcFaceCandidateHalfProfileLSum[j].top += m_rcFaceCandidateHalfProfileLSum[i].top;
				m_rcFaceCandidateHalfProfileLSum[j].right += m_rcFaceCandidateHalfProfileLSum[i].right;
				m_rcFaceCandidateHalfProfileLSum[j].bottom += m_rcFaceCandidateHalfProfileLSum[i].bottom;
				m_bFaceCandidateValidHalfProfileL[i] = false;
			}
			else { //valid is i
				m_nFaceCandidateIntersectionCountHalfProfileL[i] += m_nFaceCandidateIntersectionCountHalfProfileL[j];
				m_rcFaceCandidateHalfProfileLSum[i].left += m_rcFaceCandidateHalfProfileLSum[j].left;
				m_rcFaceCandidateHalfProfileLSum[i].top += m_rcFaceCandidateHalfProfileLSum[j].top;
				m_rcFaceCandidateHalfProfileLSum[i].right += m_rcFaceCandidateHalfProfileLSum[j].right;
				m_rcFaceCandidateHalfProfileLSum[i].bottom += m_rcFaceCandidateHalfProfileLSum[j].bottom;
				m_bFaceCandidateValidHalfProfileL[j] = false;
			}					
		}
	}
}

void fd_enroll_HalfProfileR(int nFaceX, int nFaceY, int nFaceW, int nFaceH, int nConfidence)
{
	int i, j;
	bool bIntersection = false;
	struct SMyRect rcFace;
	if(FD_MAX_FIND <= m_nFaceCandidateCountHalfProfileR) {		
		return;
	}
	rcFace.left = nFaceX;
	rcFace.top = nFaceY;
	rcFace.right = nFaceX + nFaceW;
	rcFace.bottom = nFaceY + nFaceH;	

	//\B1\E2\C1\B8\BF\A1 \B5\EE\B7ϵ\C8 \BE\F3\B1\BC \C1\C2ǥ\BF\CD \B1\B3\C1\FD\C7\D5(IsInterction())\C0\CC \C1\B8\C0\E7\C7ϴ\C2\C1\F6\B8\A6 Ȯ\C0\CE\C7ϰ\ED,
	//\B8\B8\C0\CF \C1\B8\C0\E7\C7ϸ\E9 m_nFaceCandidateIntersectionCount\B8\A6 \C7ϳ\AA \C1\F5\B0\A1\C7ϰ\ED
	//\B1\E2\C1\B8 \C1\C2ǥ\BF\CD \C7\F6\C0\E7 \C1\C2ǥ \BB\E7\C0̿\A1\BC\AD nConfidence\B0\A1 \C0\DB\C0\BA \B0\CD\C0\B8\B7\CE \B1\B3ü\B8\A6 \C7Ѵ\D9.
	for(i=0; i<m_nFaceCandidateCountHalfProfileR; i++) {
		if(m_bFaceCandidateValidHalfProfileR[i] == false) continue;
		if(fd_isintersection(m_rcFaceCandidateHalfProfileR[i], rcFace)) {			
			bIntersection = true;
			m_nFaceCandidateIntersectionCountHalfProfileR[i]++;
			m_rcFaceCandidateHalfProfileRSum[i].left += rcFace.left;
			m_rcFaceCandidateHalfProfileRSum[i].top += rcFace.top;
			m_rcFaceCandidateHalfProfileRSum[i].right += rcFace.right;
			m_rcFaceCandidateHalfProfileRSum[i].bottom += rcFace.bottom;	

			if(m_nFaceCandidateConfidenceHalfProfileR[i] > nConfidence) {
				m_rcFaceCandidateHalfProfileR[i] = rcFace;
				m_nFaceCandidateConfidenceHalfProfileR[i] = nConfidence;
			}
			break;
		}
	}

	//\B1\E2\C1\B8\BF\A1 \B5\EE\B7ϵ\C8 \BE\F3\B1\BC \C1\C2ǥ\BF\CD \B1\B3\C1\FD\C7\D5\C0\CC \C1\B8\C0\E7\C7ϴ\C2 \B0\CD\C0\CC \BE\F8\C0\B8\B8\E9
	//\BB\F5\B7ο\EE \BE󱼷\CE \BA\B8\B0\ED, \C3߰\A1\B7\CE \B5\EE\B7\CF\C7Ѵ\D9.
	if(bIntersection == false) {
		m_rcFaceCandidateHalfProfileR[m_nFaceCandidateCountHalfProfileR] = rcFace; 
		m_rcFaceCandidateHalfProfileRSum[m_nFaceCandidateCountHalfProfileR] = rcFace; 
		m_nFaceCandidateIntersectionCountHalfProfileR[m_nFaceCandidateCountHalfProfileR]++;
		m_nFaceCandidateConfidenceHalfProfileR[m_nFaceCandidateCountHalfProfileR] = nConfidence;
		m_bFaceCandidateValidHalfProfileR[m_nFaceCandidateCountHalfProfileR] = true;
		m_nFaceCandidateCountHalfProfileR++;
	}

	//\B1\E2\C1\B8\BF\A1 \B5\EE\B7ϵ\C8 \B0͵\E9 \C1߿\A1 \C7\C1\B7α׷\A5\C0\CC \C1\F8\C7\E0\C0\CC \B5Ǹ鼭 \BB\F3ȣ\B0\A3\BF\A1 \B1\B3\C1\FD\C7\D5\C0\CC \C1\B8\C0\E7\C7ϴ\C2 \B0\CD\C0\CC \B9߻\FD\C7\D2 \BC\F6 \C0ִµ\A5,
	//\C0̶\A7 \B5\D1 \BB\E7\C0̿\A1\BC\AD \B1\B3\C1\FD\C7\D5\C0\CC \B9߻\FD\C0\BB \C7ϸ\E9, nConfidence\B0\A1 \C0\DB\C0\BA \B0\CD\C0\BB \B1\E2\C1\D8\C0\B8\B7\CE \C0\E7ó\B8\AE\B8\A6 \C7Ѵ\D9.
	for(i=0; i<m_nFaceCandidateCountHalfProfileR; i++) {
		if(m_bFaceCandidateValidHalfProfileR[i] == false) continue;
		for(j=i+1; j<m_nFaceCandidateCountHalfProfileR; j++) {
			if(m_bFaceCandidateValidHalfProfileR[j] == false) continue;
			if(fd_isintersection(m_rcFaceCandidateHalfProfileR[i], m_rcFaceCandidateHalfProfileR[j]) == false) continue;
			if(m_nFaceCandidateConfidenceHalfProfileR[i] > m_nFaceCandidateConfidenceHalfProfileR[j]) {	//valid is j
				m_nFaceCandidateIntersectionCountHalfProfileR[j] += m_nFaceCandidateIntersectionCountHalfProfileR[i];
				m_rcFaceCandidateHalfProfileRSum[j].left += m_rcFaceCandidateHalfProfileRSum[i].left;
				m_rcFaceCandidateHalfProfileRSum[j].top += m_rcFaceCandidateHalfProfileRSum[i].top;
				m_rcFaceCandidateHalfProfileRSum[j].right += m_rcFaceCandidateHalfProfileRSum[i].right;
				m_rcFaceCandidateHalfProfileRSum[j].bottom += m_rcFaceCandidateHalfProfileRSum[i].bottom;
				m_bFaceCandidateValidHalfProfileR[i] = false;
			}
			else { //valid is i
				m_nFaceCandidateIntersectionCountHalfProfileR[i] += m_nFaceCandidateIntersectionCountHalfProfileR[j];
				m_rcFaceCandidateHalfProfileRSum[i].left += m_rcFaceCandidateHalfProfileRSum[j].left;
				m_rcFaceCandidateHalfProfileRSum[i].top += m_rcFaceCandidateHalfProfileRSum[j].top;
				m_rcFaceCandidateHalfProfileRSum[i].right += m_rcFaceCandidateHalfProfileRSum[j].right;
				m_rcFaceCandidateHalfProfileRSum[i].bottom += m_rcFaceCandidateHalfProfileRSum[j].bottom;
				m_bFaceCandidateValidHalfProfileR[j] = false;
			}					
		}
	}
}

void fd_enroll_FullProfileL(int nFaceX, int nFaceY, int nFaceW, int nFaceH, int nConfidence)
{
	int i, j;
	bool bIntersection = false;
	struct SMyRect rcFace;
	if(FD_MAX_FIND <= m_nFaceCandidateCountFullProfileL) {		
		return;
	}
	rcFace.left = nFaceX;
	rcFace.top = nFaceY;
	rcFace.right = nFaceX + nFaceW;
	rcFace.bottom = nFaceY + nFaceH;	

	//\B1\E2\C1\B8\BF\A1 \B5\EE\B7ϵ\C8 \BE\F3\B1\BC \C1\C2ǥ\BF\CD \B1\B3\C1\FD\C7\D5(IsInterction())\C0\CC \C1\B8\C0\E7\C7ϴ\C2\C1\F6\B8\A6 Ȯ\C0\CE\C7ϰ\ED,
	//\B8\B8\C0\CF \C1\B8\C0\E7\C7ϸ\E9 m_nFaceCandidateIntersectionCount\B8\A6 \C7ϳ\AA \C1\F5\B0\A1\C7ϰ\ED
	//\B1\E2\C1\B8 \C1\C2ǥ\BF\CD \C7\F6\C0\E7 \C1\C2ǥ \BB\E7\C0̿\A1\BC\AD nConfidence\B0\A1 \C0\DB\C0\BA \B0\CD\C0\B8\B7\CE \B1\B3ü\B8\A6 \C7Ѵ\D9.
	for(i=0; i<m_nFaceCandidateCountFullProfileL; i++) {
		if(m_bFaceCandidateValidFullProfileL[i] == false) continue;
		if(fd_isintersection(m_rcFaceCandidateFullProfileL[i], rcFace)) {			
			bIntersection = true;
			m_nFaceCandidateIntersectionCountFullProfileL[i]++;
			m_rcFaceCandidateFullProfileLSum[i].left += rcFace.left;
			m_rcFaceCandidateFullProfileLSum[i].top += rcFace.top;
			m_rcFaceCandidateFullProfileLSum[i].right += rcFace.right;
			m_rcFaceCandidateFullProfileLSum[i].bottom += rcFace.bottom;	

			if(m_nFaceCandidateConfidenceFullProfileL[i] > nConfidence) {
				m_rcFaceCandidateFullProfileL[i] = rcFace;
				m_nFaceCandidateConfidenceFullProfileL[i] = nConfidence;
			}
			break;
		}
	}

	//\B1\E2\C1\B8\BF\A1 \B5\EE\B7ϵ\C8 \BE\F3\B1\BC \C1\C2ǥ\BF\CD \B1\B3\C1\FD\C7\D5\C0\CC \C1\B8\C0\E7\C7ϴ\C2 \B0\CD\C0\CC \BE\F8\C0\B8\B8\E9
	//\BB\F5\B7ο\EE \BE󱼷\CE \BA\B8\B0\ED, \C3߰\A1\B7\CE \B5\EE\B7\CF\C7Ѵ\D9.
	if(bIntersection == false) {
		m_rcFaceCandidateFullProfileL[m_nFaceCandidateCountFullProfileL] = rcFace; 
		m_rcFaceCandidateFullProfileLSum[m_nFaceCandidateCountFullProfileL] = rcFace; 
		m_nFaceCandidateIntersectionCountFullProfileL[m_nFaceCandidateCountFullProfileL]++;
		m_nFaceCandidateConfidenceFullProfileL[m_nFaceCandidateCountFullProfileL] = nConfidence;
		m_bFaceCandidateValidFullProfileL[m_nFaceCandidateCountFullProfileL] = true;
		m_nFaceCandidateCountFullProfileL++;
	}

	//\B1\E2\C1\B8\BF\A1 \B5\EE\B7ϵ\C8 \B0͵\E9 \C1߿\A1 \C7\C1\B7α׷\A5\C0\CC \C1\F8\C7\E0\C0\CC \B5Ǹ鼭 \BB\F3ȣ\B0\A3\BF\A1 \B1\B3\C1\FD\C7\D5\C0\CC \C1\B8\C0\E7\C7ϴ\C2 \B0\CD\C0\CC \B9߻\FD\C7\D2 \BC\F6 \C0ִµ\A5,
	//\C0̶\A7 \B5\D1 \BB\E7\C0̿\A1\BC\AD \B1\B3\C1\FD\C7\D5\C0\CC \B9߻\FD\C0\BB \C7ϸ\E9, nConfidence\B0\A1 \C0\DB\C0\BA \B0\CD\C0\BB \B1\E2\C1\D8\C0\B8\B7\CE \C0\E7ó\B8\AE\B8\A6 \C7Ѵ\D9.
	for(i=0; i<m_nFaceCandidateCountFullProfileL; i++) {
		if(m_bFaceCandidateValidFullProfileL[i] == false) continue;
		for(j=i+1; j<m_nFaceCandidateCountFullProfileL; j++) {
			if(m_bFaceCandidateValidFullProfileL[j] == false) continue;
			if(fd_isintersection(m_rcFaceCandidateFullProfileL[i], m_rcFaceCandidateFullProfileL[j]) == false) continue;
			if(m_nFaceCandidateConfidenceFullProfileL[i] > m_nFaceCandidateConfidenceFullProfileL[j]) {	//valid is j
				m_nFaceCandidateIntersectionCountFullProfileL[j] += m_nFaceCandidateIntersectionCountFullProfileL[i];
				m_rcFaceCandidateFullProfileLSum[j].left += m_rcFaceCandidateFullProfileLSum[i].left;
				m_rcFaceCandidateFullProfileLSum[j].top += m_rcFaceCandidateFullProfileLSum[i].top;
				m_rcFaceCandidateFullProfileLSum[j].right += m_rcFaceCandidateFullProfileLSum[i].right;
				m_rcFaceCandidateFullProfileLSum[j].bottom += m_rcFaceCandidateFullProfileLSum[i].bottom;
				m_bFaceCandidateValidFullProfileL[i] = false;
			}
			else { //valid is i
				m_nFaceCandidateIntersectionCountFullProfileL[i] += m_nFaceCandidateIntersectionCountFullProfileL[j];
				m_rcFaceCandidateFullProfileLSum[i].left += m_rcFaceCandidateFullProfileLSum[j].left;
				m_rcFaceCandidateFullProfileLSum[i].top += m_rcFaceCandidateFullProfileLSum[j].top;
				m_rcFaceCandidateFullProfileLSum[i].right += m_rcFaceCandidateFullProfileLSum[j].right;
				m_rcFaceCandidateFullProfileLSum[i].bottom += m_rcFaceCandidateFullProfileLSum[j].bottom;
				m_bFaceCandidateValidFullProfileL[j] = false;
			}					
		}
	}
}

void fd_enroll_FullProfileR(int nFaceX, int nFaceY, int nFaceW, int nFaceH, int nConfidence)
{
	int i, j;
	bool bIntersection = false;
	struct SMyRect rcFace;
	if(FD_MAX_FIND <= m_nFaceCandidateCountFullProfileR) {		
		return;
	}
	rcFace.left = nFaceX;
	rcFace.top = nFaceY;
	rcFace.right = nFaceX + nFaceW;
	rcFace.bottom = nFaceY + nFaceH;	

	//\B1\E2\C1\B8\BF\A1 \B5\EE\B7ϵ\C8 \BE\F3\B1\BC \C1\C2ǥ\BF\CD \B1\B3\C1\FD\C7\D5(IsInterction())\C0\CC \C1\B8\C0\E7\C7ϴ\C2\C1\F6\B8\A6 Ȯ\C0\CE\C7ϰ\ED,
	//\B8\B8\C0\CF \C1\B8\C0\E7\C7ϸ\E9 m_nFaceCandidateIntersectionCount\B8\A6 \C7ϳ\AA \C1\F5\B0\A1\C7ϰ\ED
	//\B1\E2\C1\B8 \C1\C2ǥ\BF\CD \C7\F6\C0\E7 \C1\C2ǥ \BB\E7\C0̿\A1\BC\AD nConfidence\B0\A1 \C0\DB\C0\BA \B0\CD\C0\B8\B7\CE \B1\B3ü\B8\A6 \C7Ѵ\D9.
	for(i=0; i<m_nFaceCandidateCountFullProfileR; i++) {
		if(m_bFaceCandidateValidFullProfileR[i] == false) continue;
		if(fd_isintersection(m_rcFaceCandidateFullProfileR[i], rcFace)) {			
			bIntersection = true;
			m_nFaceCandidateIntersectionCountFullProfileR[i]++;
			m_rcFaceCandidateFullProfileRSum[i].left += rcFace.left;
			m_rcFaceCandidateFullProfileRSum[i].top += rcFace.top;
			m_rcFaceCandidateFullProfileRSum[i].right += rcFace.right;
			m_rcFaceCandidateFullProfileRSum[i].bottom += rcFace.bottom;	

			if(m_nFaceCandidateConfidenceFullProfileR[i] > nConfidence) {
				m_rcFaceCandidateFullProfileR[i] = rcFace;
				m_nFaceCandidateConfidenceFullProfileR[i] = nConfidence;
			}
			break;
		}
	}

	//\B1\E2\C1\B8\BF\A1 \B5\EE\B7ϵ\C8 \BE\F3\B1\BC \C1\C2ǥ\BF\CD \B1\B3\C1\FD\C7\D5\C0\CC \C1\B8\C0\E7\C7ϴ\C2 \B0\CD\C0\CC \BE\F8\C0\B8\B8\E9
	//\BB\F5\B7ο\EE \BE󱼷\CE \BA\B8\B0\ED, \C3߰\A1\B7\CE \B5\EE\B7\CF\C7Ѵ\D9.
	if(bIntersection == false) {
		m_rcFaceCandidateFullProfileR[m_nFaceCandidateCountFullProfileR] = rcFace; 
		m_rcFaceCandidateFullProfileRSum[m_nFaceCandidateCountFullProfileR] = rcFace; 
		m_nFaceCandidateIntersectionCountFullProfileR[m_nFaceCandidateCountFullProfileR]++;
		m_nFaceCandidateConfidenceFullProfileR[m_nFaceCandidateCountFullProfileR] = nConfidence;
		m_bFaceCandidateValidFullProfileR[m_nFaceCandidateCountFullProfileR] = true;
		m_nFaceCandidateCountFullProfileR++;
	}

	//\B1\E2\C1\B8\BF\A1 \B5\EE\B7ϵ\C8 \B0͵\E9 \C1߿\A1 \C7\C1\B7α׷\A5\C0\CC \C1\F8\C7\E0\C0\CC \B5Ǹ鼭 \BB\F3ȣ\B0\A3\BF\A1 \B1\B3\C1\FD\C7\D5\C0\CC \C1\B8\C0\E7\C7ϴ\C2 \B0\CD\C0\CC \B9߻\FD\C7\D2 \BC\F6 \C0ִµ\A5,
	//\C0̶\A7 \B5\D1 \BB\E7\C0̿\A1\BC\AD \B1\B3\C1\FD\C7\D5\C0\CC \B9߻\FD\C0\BB \C7ϸ\E9, nConfidence\B0\A1 \C0\DB\C0\BA \B0\CD\C0\BB \B1\E2\C1\D8\C0\B8\B7\CE \C0\E7ó\B8\AE\B8\A6 \C7Ѵ\D9.
	for(i=0; i<m_nFaceCandidateCountFullProfileR; i++) {
		if(m_bFaceCandidateValidFullProfileR[i] == false) continue;
		for(j=i+1; j<m_nFaceCandidateCountFullProfileR; j++) {
			if(m_bFaceCandidateValidFullProfileR[j] == false) continue;
			if(fd_isintersection(m_rcFaceCandidateFullProfileR[i], m_rcFaceCandidateFullProfileR[j]) == false) continue;
			if(m_nFaceCandidateConfidenceFullProfileR[i] > m_nFaceCandidateConfidenceFullProfileR[j]) {	//valid is j
				m_nFaceCandidateIntersectionCountFullProfileR[j] += m_nFaceCandidateIntersectionCountFullProfileR[i];
				m_rcFaceCandidateFullProfileRSum[j].left += m_rcFaceCandidateFullProfileRSum[i].left;
				m_rcFaceCandidateFullProfileRSum[j].top += m_rcFaceCandidateFullProfileRSum[i].top;
				m_rcFaceCandidateFullProfileRSum[j].right += m_rcFaceCandidateFullProfileRSum[i].right;
				m_rcFaceCandidateFullProfileRSum[j].bottom += m_rcFaceCandidateFullProfileRSum[i].bottom;
				m_bFaceCandidateValidFullProfileR[i] = false;
			}
			else { //valid is i
				m_nFaceCandidateIntersectionCountFullProfileR[i] += m_nFaceCandidateIntersectionCountFullProfileR[j];
				m_rcFaceCandidateFullProfileRSum[i].left += m_rcFaceCandidateFullProfileRSum[j].left;
				m_rcFaceCandidateFullProfileRSum[i].top += m_rcFaceCandidateFullProfileRSum[j].top;
				m_rcFaceCandidateFullProfileRSum[i].right += m_rcFaceCandidateFullProfileRSum[j].right;
				m_rcFaceCandidateFullProfileRSum[i].bottom += m_rcFaceCandidateFullProfileRSum[j].bottom;
				m_bFaceCandidateValidFullProfileR[j] = false;
			}					
		}
	}
}

void fd_enroll_RIP30L(int nFaceX, int nFaceY, int nFaceW, int nFaceH, int nConfidence)
{
	int i, j;
	bool bIntersection = false;
	struct SMyRect rcFace;
	if(FD_MAX_FIND <= m_nFaceCandidateCountRIP30L) {		
		return;
	}
	rcFace.left = nFaceX;
	rcFace.top = nFaceY;
	rcFace.right = nFaceX + nFaceW;
	rcFace.bottom = nFaceY + nFaceH;	

	//\B1\E2\C1\B8\BF\A1 \B5\EE\B7ϵ\C8 \BE\F3\B1\BC \C1\C2ǥ\BF\CD \B1\B3\C1\FD\C7\D5(IsInterction())\C0\CC \C1\B8\C0\E7\C7ϴ\C2\C1\F6\B8\A6 Ȯ\C0\CE\C7ϰ\ED,
	//\B8\B8\C0\CF \C1\B8\C0\E7\C7ϸ\E9 m_nFaceCandidateIntersectionCount\B8\A6 \C7ϳ\AA \C1\F5\B0\A1\C7ϰ\ED
	//\B1\E2\C1\B8 \C1\C2ǥ\BF\CD \C7\F6\C0\E7 \C1\C2ǥ \BB\E7\C0̿\A1\BC\AD nConfidence\B0\A1 \C0\DB\C0\BA \B0\CD\C0\B8\B7\CE \B1\B3ü\B8\A6 \C7Ѵ\D9.
	for(i=0; i<m_nFaceCandidateCountRIP30L; i++) {
		if(m_bFaceCandidateValidRIP30L[i] == false) continue;
		if(fd_isintersection(m_rcFaceCandidateRIP30L[i], rcFace)) {			
			bIntersection = true;
			m_nFaceCandidateIntersectionCountRIP30L[i]++;
			m_rcFaceCandidateRIP30LSum[i].left += rcFace.left;
			m_rcFaceCandidateRIP30LSum[i].top += rcFace.top;
			m_rcFaceCandidateRIP30LSum[i].right += rcFace.right;
			m_rcFaceCandidateRIP30LSum[i].bottom += rcFace.bottom;	

			if(m_nFaceCandidateConfidenceRIP30L[i] > nConfidence) {
				m_rcFaceCandidateRIP30L[i] = rcFace;
				m_nFaceCandidateConfidenceRIP30L[i] = nConfidence;
			}
			break;
		}
	}

	//\B1\E2\C1\B8\BF\A1 \B5\EE\B7ϵ\C8 \BE\F3\B1\BC \C1\C2ǥ\BF\CD \B1\B3\C1\FD\C7\D5\C0\CC \C1\B8\C0\E7\C7ϴ\C2 \B0\CD\C0\CC \BE\F8\C0\B8\B8\E9
	//\BB\F5\B7ο\EE \BE󱼷\CE \BA\B8\B0\ED, \C3߰\A1\B7\CE \B5\EE\B7\CF\C7Ѵ\D9.
	if(bIntersection == false) {
		m_rcFaceCandidateRIP30L[m_nFaceCandidateCountRIP30L] = rcFace; 
		m_rcFaceCandidateRIP30LSum[m_nFaceCandidateCountRIP30L] = rcFace; 
		m_nFaceCandidateIntersectionCountRIP30L[m_nFaceCandidateCountRIP30L]++;
		m_nFaceCandidateConfidenceRIP30L[m_nFaceCandidateCountRIP30L] = nConfidence;
		m_bFaceCandidateValidRIP30L[m_nFaceCandidateCountRIP30L] = true;
		m_nFaceCandidateCountRIP30L++;
	}

	//\B1\E2\C1\B8\BF\A1 \B5\EE\B7ϵ\C8 \B0͵\E9 \C1߿\A1 \C7\C1\B7α׷\A5\C0\CC \C1\F8\C7\E0\C0\CC \B5Ǹ鼭 \BB\F3ȣ\B0\A3\BF\A1 \B1\B3\C1\FD\C7\D5\C0\CC \C1\B8\C0\E7\C7ϴ\C2 \B0\CD\C0\CC \B9߻\FD\C7\D2 \BC\F6 \C0ִµ\A5,
	//\C0̶\A7 \B5\D1 \BB\E7\C0̿\A1\BC\AD \B1\B3\C1\FD\C7\D5\C0\CC \B9߻\FD\C0\BB \C7ϸ\E9, nConfidence\B0\A1 \C0\DB\C0\BA \B0\CD\C0\BB \B1\E2\C1\D8\C0\B8\B7\CE \C0\E7ó\B8\AE\B8\A6 \C7Ѵ\D9.
	for(i=0; i<m_nFaceCandidateCountRIP30L; i++) {
		if(m_bFaceCandidateValidRIP30L[i] == false) continue;
		for(j=i+1; j<m_nFaceCandidateCountRIP30L; j++) {
			if(m_bFaceCandidateValidRIP30L[j] == false) continue;
			if(fd_isintersection(m_rcFaceCandidateRIP30L[i], m_rcFaceCandidateRIP30L[j]) == false) continue;
			if(m_nFaceCandidateConfidenceRIP30L[i] > m_nFaceCandidateConfidenceRIP30L[j]) {	//valid is j
				m_nFaceCandidateIntersectionCountRIP30L[j] += m_nFaceCandidateIntersectionCountRIP30L[i];
				m_rcFaceCandidateRIP30LSum[j].left += m_rcFaceCandidateRIP30LSum[i].left;
				m_rcFaceCandidateRIP30LSum[j].top += m_rcFaceCandidateRIP30LSum[i].top;
				m_rcFaceCandidateRIP30LSum[j].right += m_rcFaceCandidateRIP30LSum[i].right;
				m_rcFaceCandidateRIP30LSum[j].bottom += m_rcFaceCandidateRIP30LSum[i].bottom;
				m_bFaceCandidateValidRIP30L[i] = false;
			}
			else { //valid is i
				m_nFaceCandidateIntersectionCountRIP30L[i] += m_nFaceCandidateIntersectionCountRIP30L[j];
				m_rcFaceCandidateRIP30LSum[i].left += m_rcFaceCandidateRIP30LSum[j].left;
				m_rcFaceCandidateRIP30LSum[i].top += m_rcFaceCandidateRIP30LSum[j].top;
				m_rcFaceCandidateRIP30LSum[i].right += m_rcFaceCandidateRIP30LSum[j].right;
				m_rcFaceCandidateRIP30LSum[i].bottom += m_rcFaceCandidateRIP30LSum[j].bottom;
				m_bFaceCandidateValidRIP30L[j] = false;
			}					
		}
	}
}

void fd_enroll_RIP30R(int nFaceX, int nFaceY, int nFaceW, int nFaceH, int nConfidence)
{
	int i, j;
	bool bIntersection = false;
	struct SMyRect rcFace;
	if(FD_MAX_FIND <= m_nFaceCandidateCountRIP30R) {		
		return;
	}
	rcFace.left = nFaceX;
	rcFace.top = nFaceY;
	rcFace.right = nFaceX + nFaceW;
	rcFace.bottom = nFaceY + nFaceH;	

	//\B1\E2\C1\B8\BF\A1 \B5\EE\B7ϵ\C8 \BE\F3\B1\BC \C1\C2ǥ\BF\CD \B1\B3\C1\FD\C7\D5(IsInterction())\C0\CC \C1\B8\C0\E7\C7ϴ\C2\C1\F6\B8\A6 Ȯ\C0\CE\C7ϰ\ED,
	//\B8\B8\C0\CF \C1\B8\C0\E7\C7ϸ\E9 m_nFaceCandidateIntersectionCount\B8\A6 \C7ϳ\AA \C1\F5\B0\A1\C7ϰ\ED
	//\B1\E2\C1\B8 \C1\C2ǥ\BF\CD \C7\F6\C0\E7 \C1\C2ǥ \BB\E7\C0̿\A1\BC\AD nConfidence\B0\A1 \C0\DB\C0\BA \B0\CD\C0\B8\B7\CE \B1\B3ü\B8\A6 \C7Ѵ\D9.
	for(i=0; i<m_nFaceCandidateCountRIP30R; i++) {
		if(m_bFaceCandidateValidRIP30R[i] == false) continue;
		if(fd_isintersection(m_rcFaceCandidateRIP30R[i], rcFace)) {			
			bIntersection = true;
			m_nFaceCandidateIntersectionCountRIP30R[i]++;
			m_rcFaceCandidateRIP30RSum[i].left += rcFace.left;
			m_rcFaceCandidateRIP30RSum[i].top += rcFace.top;
			m_rcFaceCandidateRIP30RSum[i].right += rcFace.right;
			m_rcFaceCandidateRIP30RSum[i].bottom += rcFace.bottom;	

			if(m_nFaceCandidateConfidenceRIP30R[i] > nConfidence) {
				m_rcFaceCandidateRIP30R[i] = rcFace;
				m_nFaceCandidateConfidenceRIP30R[i] = nConfidence;
			}
			break;
		}
	}

	//\B1\E2\C1\B8\BF\A1 \B5\EE\B7ϵ\C8 \BE\F3\B1\BC \C1\C2ǥ\BF\CD \B1\B3\C1\FD\C7\D5\C0\CC \C1\B8\C0\E7\C7ϴ\C2 \B0\CD\C0\CC \BE\F8\C0\B8\B8\E9
	//\BB\F5\B7ο\EE \BE󱼷\CE \BA\B8\B0\ED, \C3߰\A1\B7\CE \B5\EE\B7\CF\C7Ѵ\D9.
	if(bIntersection == false) {
		m_rcFaceCandidateRIP30R[m_nFaceCandidateCountRIP30R] = rcFace; 
		m_rcFaceCandidateRIP30RSum[m_nFaceCandidateCountRIP30R] = rcFace; 
		m_nFaceCandidateIntersectionCountRIP30R[m_nFaceCandidateCountRIP30R]++;
		m_nFaceCandidateConfidenceRIP30R[m_nFaceCandidateCountRIP30R] = nConfidence;
		m_bFaceCandidateValidRIP30R[m_nFaceCandidateCountRIP30R] = true;
		m_nFaceCandidateCountRIP30R++;
	}

	//\B1\E2\C1\B8\BF\A1 \B5\EE\B7ϵ\C8 \B0͵\E9 \C1߿\A1 \C7\C1\B7α׷\A5\C0\CC \C1\F8\C7\E0\C0\CC \B5Ǹ鼭 \BB\F3ȣ\B0\A3\BF\A1 \B1\B3\C1\FD\C7\D5\C0\CC \C1\B8\C0\E7\C7ϴ\C2 \B0\CD\C0\CC \B9߻\FD\C7\D2 \BC\F6 \C0ִµ\A5,
	//\C0̶\A7 \B5\D1 \BB\E7\C0̿\A1\BC\AD \B1\B3\C1\FD\C7\D5\C0\CC \B9߻\FD\C0\BB \C7ϸ\E9, nConfidence\B0\A1 \C0\DB\C0\BA \B0\CD\C0\BB \B1\E2\C1\D8\C0\B8\B7\CE \C0\E7ó\B8\AE\B8\A6 \C7Ѵ\D9.
	for(i=0; i<m_nFaceCandidateCountRIP30R; i++) {
		if(m_bFaceCandidateValidRIP30R[i] == false) continue;
		for(j=i+1; j<m_nFaceCandidateCountRIP30R; j++) {
			if(m_bFaceCandidateValidRIP30R[j] == false) continue;
			if(fd_isintersection(m_rcFaceCandidateRIP30R[i], m_rcFaceCandidateRIP30R[j]) == false) continue;
			if(m_nFaceCandidateConfidenceRIP30R[i] > m_nFaceCandidateConfidenceRIP30R[j]) {	//valid is j
				m_nFaceCandidateIntersectionCountRIP30R[j] += m_nFaceCandidateIntersectionCountRIP30R[i];
				m_rcFaceCandidateRIP30RSum[j].left += m_rcFaceCandidateRIP30RSum[i].left;
				m_rcFaceCandidateRIP30RSum[j].top += m_rcFaceCandidateRIP30RSum[i].top;
				m_rcFaceCandidateRIP30RSum[j].right += m_rcFaceCandidateRIP30RSum[i].right;
				m_rcFaceCandidateRIP30RSum[j].bottom += m_rcFaceCandidateRIP30RSum[i].bottom;
				m_bFaceCandidateValidRIP30R[i] = false;
			}
			else { //valid is i
				m_nFaceCandidateIntersectionCountRIP30R[i] += m_nFaceCandidateIntersectionCountRIP30R[j];
				m_rcFaceCandidateRIP30RSum[i].left += m_rcFaceCandidateRIP30RSum[j].left;
				m_rcFaceCandidateRIP30RSum[i].top += m_rcFaceCandidateRIP30RSum[j].top;
				m_rcFaceCandidateRIP30RSum[i].right += m_rcFaceCandidateRIP30RSum[j].right;
				m_rcFaceCandidateRIP30RSum[i].bottom += m_rcFaceCandidateRIP30RSum[j].bottom;
				m_bFaceCandidateValidRIP30R[j] = false;
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

	//\B1\E2\C1\B8\BF\A1 \B5\EE\B7ϵ\C8 \BE\F3\B1\BC \C1\C2ǥ\BF\CD \B1\B3\C1\FD\C7\D5(IsInterction())\C0\CC \C1\B8\C0\E7\C7ϴ\C2\C1\F6\B8\A6 Ȯ\C0\CE\C7ϰ\ED,
	//\B8\B8\C0\CF \C1\B8\C0\E7\C7ϸ\E9 m_nFaceCandidateIntersectionCount\B8\A6 \C7ϳ\AA \C1\F5\B0\A1\C7ϰ\ED
	//\B1\E2\C1\B8 \C1\C2ǥ\BF\CD \C7\F6\C0\E7 \C1\C2ǥ \BB\E7\C0̿\A1\BC\AD nConfidence\B0\A1 \C0\DB\C0\BA \B0\CD\C0\B8\B7\CE \B1\B3ü\B8\A6 \C7Ѵ\D9.
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

	//\B1\E2\C1\B8\BF\A1 \B5\EE\B7ϵ\C8 \BE\F3\B1\BC \C1\C2ǥ\BF\CD \B1\B3\C1\FD\C7\D5\C0\CC \C1\B8\C0\E7\C7ϴ\C2 \B0\CD\C0\CC \BE\F8\C0\B8\B8\E9
	//\BB\F5\B7ο\EE \BE󱼷\CE \BA\B8\B0\ED, \C3߰\A1\B7\CE \B5\EE\B7\CF\C7Ѵ\D9.
	if(bIntersection == false) {
		m_rcFaceCandidateProfileL[m_nFaceCandidateCountProfileL] = rcFace; 
		m_rcFaceCandidateProfileLSum[m_nFaceCandidateCountProfileL] = rcFace; 
		m_nFaceCandidateIntersectionCountProfileL[m_nFaceCandidateCountProfileL]++;
		m_nFaceCandidateConfidenceProfileL[m_nFaceCandidateCountProfileL] = nConfidence;
		m_bFaceCandidateValidProfileL[m_nFaceCandidateCountProfileL] = true;
		m_nFaceCandidateCountProfileL++;
	}

	//\B1\E2\C1\B8\BF\A1 \B5\EE\B7ϵ\C8 \B0͵\E9 \C1߿\A1 \C7\C1\B7α׷\A5\C0\CC \C1\F8\C7\E0\C0\CC \B5Ǹ鼭 \BB\F3ȣ\B0\A3\BF\A1 \B1\B3\C1\FD\C7\D5\C0\CC \C1\B8\C0\E7\C7ϴ\C2 \B0\CD\C0\CC \B9߻\FD\C7\D2 \BC\F6 \C0ִµ\A5,
	//\C0̶\A7 \B5\D1 \BB\E7\C0̿\A1\BC\AD \B1\B3\C1\FD\C7\D5\C0\CC \B9߻\FD\C0\BB \C7ϸ\E9, nConfidence\B0\A1 \C0\DB\C0\BA \B0\CD\C0\BB \B1\E2\C1\D8\C0\B8\B7\CE \C0\E7ó\B8\AE\B8\A6 \C7Ѵ\D9.
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

	//\B1\E2\C1\B8\BF\A1 \B5\EE\B7ϵ\C8 \BE\F3\B1\BC \C1\C2ǥ\BF\CD \B1\B3\C1\FD\C7\D5(IsInterction())\C0\CC \C1\B8\C0\E7\C7ϴ\C2\C1\F6\B8\A6 Ȯ\C0\CE\C7ϰ\ED,
	//\B8\B8\C0\CF \C1\B8\C0\E7\C7ϸ\E9 m_nFaceCandidateIntersectionCount\B8\A6 \C7ϳ\AA \C1\F5\B0\A1\C7ϰ\ED
	//\B1\E2\C1\B8 \C1\C2ǥ\BF\CD \C7\F6\C0\E7 \C1\C2ǥ \BB\E7\C0̿\A1\BC\AD nConfidence\B0\A1 \C0\DB\C0\BA \B0\CD\C0\B8\B7\CE \B1\B3ü\B8\A6 \C7Ѵ\D9.
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

	//\B1\E2\C1\B8\BF\A1 \B5\EE\B7ϵ\C8 \BE\F3\B1\BC \C1\C2ǥ\BF\CD \B1\B3\C1\FD\C7\D5\C0\CC \C1\B8\C0\E7\C7ϴ\C2 \B0\CD\C0\CC \BE\F8\C0\B8\B8\E9
	//\BB\F5\B7ο\EE \BE󱼷\CE \BA\B8\B0\ED, \C3߰\A1\B7\CE \B5\EE\B7\CF\C7Ѵ\D9.
	if(bIntersection == false) {
		m_rcFaceCandidateProfileR[m_nFaceCandidateCountProfileR] = rcFace; 
		m_rcFaceCandidateProfileRSum[m_nFaceCandidateCountProfileR] = rcFace; 
		m_nFaceCandidateIntersectionCountProfileR[m_nFaceCandidateCountProfileR]++;
		m_nFaceCandidateConfidenceProfileR[m_nFaceCandidateCountProfileR] = nConfidence;
		m_bFaceCandidateValidProfileR[m_nFaceCandidateCountProfileR] = true;
		m_nFaceCandidateCountProfileR++;
	}

	//\B1\E2\C1\B8\BF\A1 \B5\EE\B7ϵ\C8 \B0͵\E9 \C1߿\A1 \C7\C1\B7α׷\A5\C0\CC \C1\F8\C7\E0\C0\CC \B5Ǹ鼭 \BB\F3ȣ\B0\A3\BF\A1 \B1\B3\C1\FD\C7\D5\C0\CC \C1\B8\C0\E7\C7ϴ\C2 \B0\CD\C0\CC \B9߻\FD\C7\D2 \BC\F6 \C0ִµ\A5,
	//\C0̶\A7 \B5\D1 \BB\E7\C0̿\A1\BC\AD \B1\B3\C1\FD\C7\D5\C0\CC \B9߻\FD\C0\BB \C7ϸ\E9, nConfidence\B0\A1 \C0\DB\C0\BA \B0\CD\C0\BB \B1\E2\C1\D8\C0\B8\B7\CE \C0\E7ó\B8\AE\B8\A6 \C7Ѵ\D9.
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
	//\B1\E2\C1\B8\BF\A1 \B5\EE\B7ϵ\C8 \B0͵\E9 \C1߿\A1 \C7\C1\B7α׷\A5\C0\CC \C1\F8\C7\E0\C0\CC \B5Ǹ鼭 \BB\F3ȣ\B0\A3\BF\A1 \B1\B3\C1\FD\C7\D5\C0\CC \C1\B8\C0\E7\C7ϴ\C2 \B0\CD\C0\CC \B9߻\FD\C7\D2 \BC\F6 \C0ִµ\A5,
	//\C0̶\A7 \B5\D1 \BB\E7\C0̿\A1\BC\AD \B1\B3\C1\FD\C7\D5\C0\CC \B9߻\FD\C0\BB \C7ϸ\E9, nConfidence\B0\A1 \C0\DB\C0\BA \B0\CD\C0\BB \B1\E2\C1\D8\C0\B8\B7\CE \C0\E7ó\B8\AE\B8\A6 \C7Ѵ\D9.
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
