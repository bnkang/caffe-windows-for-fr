#ifndef _IMAGEPROCESSING_H_
#define _IMAGEPROCESSING_H_

#pragma once

#include "../../../exp/PIM_Define_op.h"
#include "cv.h"
#include "cxcore.h"
#include "highgui.h"


class CImageProcessing
{
private:
	void ClipHistogram (unsigned long* pulHistogram, unsigned int
		uiNrGreylevels, unsigned long ulClipLimit);
	void MakeHistogram (unsigned char* pImage, unsigned int uiXRes,
		unsigned int uiSizeX, unsigned int uiSizeY,
		unsigned long* pulHistogram,
		unsigned int uiNrGreylevels, unsigned char* pLookupTable);
	void MapHistogram (unsigned long* pulHistogram, unsigned char Min, unsigned char Max,
		unsigned int uiNrGreylevels, unsigned long ulNrOfPixels);
	void MakeLut (unsigned char * pLUT, unsigned char Min, unsigned char Max, unsigned int uiNrBins);
	void Interpolate (unsigned char * pImage, int uiXRes, unsigned long * pulMapLU,
		unsigned long * pulMapRU, unsigned long * pulMapLB,  unsigned long * pulMapRB,
		unsigned int uiXSize, unsigned int uiYSize, unsigned char * pLUT);

public:
	CImageProcessing();
	virtual ~CImageProcessing();

	void size(cv::Mat& image, int& w, int& h, int& channel);

	bool split(cv::Mat& src_image, cv::Mat& r, cv::Mat& g, cv::Mat& b);

	//file
	bool load(const char* path, cv::Mat& image);
	bool save(const char* path, cv::Mat& image);
	bool clone(cv::Mat& src_image, cv::Mat& des_image);

	//raw	
	bool getraw_notnull(cv::Mat& image, unsigned char* pdata);
	bool setraw(cv::Mat& image, int w, int h, int channel, unsigned char* pdata);	

	bool draw_rect(cv::Mat& image, MyRect rc, int r, int g, int b, int thick);
	bool draw_rect(cv::Mat& image, MyRect rc, int bright, int thick);
	bool draw_rect(cv::Mat& image, int x, int y, int w, int h, int r, int g, int b, int thick);
	bool draw_rect(cv::Mat& image, int x, int y, int w, int h, int bright, int thick);



	bool draw_circle(cv::Mat& image, MyPoint center, int radius, int r, int g, int b, int thick);
	bool draw_circle(cv::Mat& image, MyPoint center, int radius, int bright, int thick);
	bool draw_circle(cv::Mat& image, int center_x, int center_y, int radius, int r, int g, int b, int thick);
	bool draw_circle(cv::Mat& image, int center_x, int center_y, int radius, int bright, int thick);

	bool draw_line(cv::Mat& image, MyPoint pt1, MyPoint pt2, int r, int g, int b, int thick);
	bool draw_line(cv::Mat& image, MyPoint pt1, MyPoint pt2, int bright, int thick);
	bool draw_line(cv::Mat& image, int x1, int y1, int x2, int y2, int r, int g, int b, int thick);
	bool draw_line(cv::Mat& image, int x1, int y1, int x2, int y2, int bright, int thick);



	bool draw_text(cv::Mat& image, char* text, MyPoint pt, int r, int g, int b, int thick);
	bool draw_text(cv::Mat& image, char* text, int x, int y, int r, int g, int b, int thick);
	bool draw_text(cv::Mat& image, char* text, MyPoint pt, int bright, int thick);
	bool draw_text(cv::Mat& image, char* text, int x, int y, int bright, int thick);


	//color conversion

	bool bgr2gray(cv::Mat& src_image, cv::Mat& des_image);

	bool gray2bgr(cv::Mat& src_image, cv::Mat& des_image);


	/*
	//filtering
	bool filt_min(CmyImage& src_image, CmyImage& des_image, int mask_w, int mask_h);
	bool filt_mean(CmyImage& src_image, CmyImage& des_image , int mask_w, int mask_h);
	bool filt_median(CmyImage& src_image, CmyImage& des_image, int mask_w, int mask_h);
	bool filt_gaussian(CmyImage& src_image, CmyImage& des_image, int mask_w, int mask_h);
	bool filt_bilateral(CmyImage& src_image, CmyImage& des_image, int mask_w, int mask_h);

	//resize
	bool resize_bi(CmyImage& src_image, CmyImage& des_image, int resize_w, int resize_h);
	void resize_bi(unsigned char *srcData, int nSrcW, int nSrcH, unsigned char *desData, int nDesW, int nDesH);
	void resize_nn(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data, int des_w, int des_h);

	//rotation
	bool rotation(CmyImage& src_image, CmyImage& des_image, int center_x, int center_y, double radian);
	void rotation(unsigned char* srcData, int src_w, int src_h, unsigned char*desData, int cx, int cy, double radian);

	//region image
	bool region(CmyImage& src_image, CmyImage& des_image, MyRect roi);
	bool region(CmyImage& src_image, CmyImage& des_image, int roi_x, int roi_y, int roi_w, int roi_h);

	//threshold
	//inv_mode = 0 -> val = (val > th ? max_val : 0)
	//inv_mode = 1 -> val = (val > th ? 0 : max_val)
	bool threshold_binary(CmyImage& src_image, CmyImage& des_image, int th, int max_val = 255, int inv_mode = 0);
	//inv_mode = 0 -> val = (val > th ? val : 0)
	//inv_mode = 1 -> val = (val > th ? 0 : val)
	bool threshold_tozero(CmyImage& src_image, CmyImage& des_image, int th, int max_val = 255, int inv_mode = 0);
	//val = (val > th ? th : max_val)
	bool threshold_trunc(CmyImage& src_image, CmyImage& des_image, int th, int max_val = 255);

	//edge
	//The smallest of threshold1 and threshold2 is used for edge linking, 
	//the largest - to find initial segments of strong edges.
	bool canny(CmyImage& src_image, CmyImage& edge_image, unsigned char* angle, double low_th, double high_th, int apertureSize=3);
	bool canny(CmyImage& src_image, CmyImage& edge_image, double low_th, double high_th, int apertureSize=3);

	// Histogram Equalization
	bool he_w160_h120(unsigned char *image);
	bool he(CmyImage& src_image, CmyImage& des_image);
	bool he(unsigned char *image, int w, int h);
	void integral_image(unsigned char* data, int* ii_image, int w, int h);	

	//Adaptive Histogram Equalization	
	bool CLAHE(CmyImage& srcImage, float fCliplimit);
	int CLAHE(unsigned char* pImage, unsigned int uiXRes, unsigned int uiYRes,
		unsigned char Min, unsigned char Max, unsigned int uiNrX, unsigned int uiNrY,
		unsigned int uiNrBins, float fCliplimit);	
	int CLAHE(CmyImage& src_image, unsigned int uiXRes, unsigned int uiYRes,
		unsigned char Min, unsigned char Max, unsigned int uiNrX, unsigned int uiNrY,
		unsigned int uiNrBins, float fCliplimit);	

	//angle
	void PrewittGradientAngle(unsigned char* src_image, int w, int h, unsigned char* des_image);

	//connect component
	int connected_component(CmyImage& src_image, MyRect** rc);

	bool dilate(CmyImage& src_image, CmyImage& des_image, int iter=1);
	bool erode(CmyImage& src_image, CmyImage& des_image, int iter=1);

	bool self_quotient1(CmyImage& src_image, CmyImage& sq_image);
	bool self_quotient2(CmyImage& src_image, CmyImage& sq_image);
	bool self_quotient3(CmyImage& src_image, CmyImage& sq_image);
	void matching_fun(double a, double b, int a_, int b_, int size, double* x, unsigned char* x_);
	void matching_fun_reverse(double a, double b, int a_, int b_, int size, double* x, unsigned char* x_);
	void matching_fun(int a1, int hist_index, int a1_, int b1_, int b2, int a2_, int b2_, 
		int size, unsigned char* x, unsigned char* x_);	
	bool BackgroundTH(const unsigned char* data, const int size, int& th);

	bool affine_trans(unsigned char* src_data, int src_w, int src_h, MyPoint ptSrcLEye, MyPoint ptSrcREye,
		unsigned char* des_data, int des_w, int des_h, MyPoint ptDesLEye, MyPoint ptDesREye,
		double f1, double f2, double f3, double f4);

	bool affine_trans_eye(CmyImage& src_image, CmyImage& des_image,	MyRect rcEye);
	bool affine_trans_fv(CmyImage& src_image, CmyImage& des_image, MyPoint ptLeftEye, MyPoint ptRightEye);
	bool affine_trans_fshow(CmyImage& src_image, CmyImage& des_image1, CmyImage& des_image2, CmyImage& des_image3, 
		MyPoint ptLeftEye, MyPoint ptRightEye);
	bool affine_trans_fr(CmyImage& src_image, CmyImage& des_image, MyPoint ptLeftEye, MyPoint ptRightEye);

	bool float_affine_trans(unsigned char* src_data, int src_w, int src_h, 
		unsigned char* des_data, int des_w, int des_h,
		int* x, int* y, double* z);
	bool fixed_affine_trans(unsigned char* src_data, int src_w, int src_h, 
		unsigned char* des_data, int des_w, int des_h,
		int* x, int* y, double* z);
	bool xchip_affine_trans(unsigned char* src_data, int src_w, int src_h, 
		unsigned char* des_data, int des_w, int des_h,
		int* x, int* y, int* z);
	*/

	void flipx(unsigned char* src_data, int src_w, int src_h, unsigned char* des_data);
	//void flipy(CmyImage& src_image, CmyImage& dst_image);

	/*int diff(CmyImage& src_image1, CmyImage& src_image2, CmyImage& diff_image);
	int diff_mean(CmyImage& src_image1, CmyImage& src_image2, CmyImage& diff_image);

	void resize( unsigned char *pecsrcimg, int euwid, int euhei, unsigned char *pecdstimg, int eutwid, int euthei);
	void image2MCT(unsigned char* src_data, int src_w, int src_h, int* mct);
	void image2LBP8_1(unsigned char* src_data, int src_w, int src_h, unsigned char* lbp_data);

	void flipx(CmyImage& src_image, CmyImage& des_image);
	void IterativeThreshold(unsigned char* src_data, int src_w, int src_h, int& th, unsigned char* des_data);

	bool combine_image(CmyImage& src_image1, CmyImage& src_image2, int x, int y, CmyImage& des_image);

	void image2LBP8_1_FR(unsigned char* src_data, int src_w, int src_h, unsigned char* lbp_data);
	void image2LBP8_2_FR(unsigned char* src_data, int src_w, int src_h, unsigned char* lbp_data);
	bool Region(unsigned char* src_data, int src_w, int src_h, unsigned char* des_data, struct SMyRect rcRegion);

	bool region_MD1(CmyImage& src_image, CmyImage& des_image, MyRect roi);
	bool region_MD1(CmyImage& src_image, CmyImage& des_image, int roi_x, int roi_y, int roi_w, int roi_h);
	bool draw_text_MD1(CmyImage& image, char* text, int x, int y, int r, int g, int b, int thick);

	bool resize(CmyImage& src_image, CmyImage& des_image, int des_w, int des_h);
	*/

};

#endif 
