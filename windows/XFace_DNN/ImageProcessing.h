#ifndef _IMAGEPROCESS_H_
#define _IMAGEPROCESS_H_

#include "Define.h"
#include "CvImage.h"
//#include "Define_op.h"


class CImageProcess  
{
public:
	CImageProcess();
	virtual ~CImageProcess();

	void size(CvImage& image, int& w, int& h, int& channel);
	bool create(CvImage& image, int w, int h, int nChannel);
	void destroy(CvImage& image);

	bool load(const char* path, CvImage& image);
	bool save(const char* path, CvImage& image);
	bool clone(CvImage& src_image, CvImage& des_image);

	bool getraw(CvImage& image, unsigned char* pdata);
	bool getraw_notnull(CvImage& image, unsigned char* pdata);
	bool setraw(CvImage& image, int w, int h, int channel, const unsigned char* pdata);	

#ifdef _WIN32
	bool draw(HDC hDCDst, CvImage& image, int screen_x, int screen_y);
	bool draw(HDC hDCDst, CvImage& image, int screen_x, int screen_y, MyRect rcROI);
#endif
	bool draw_line(CvImage& image, MyPoint pt1, MyPoint pt2, int r, int g, int b, int thick);
	bool draw_line(CvImage& image, MyPoint pt1, MyPoint pt2, int bright, int thick);
	bool draw_line(CvImage& image, int x1, int y1, int x2, int y2, int r, int g, int b, int thick);
	bool draw_line(CvImage& image, int x1, int y1, int x2, int y2, int bright, int thick);

	bool draw_cross(CvImage& image, int x, int y, int r, int g, int b, int thick);
	bool draw_cross(CvImage& image, MyPoint center, int r, int g, int b, int thick);
	bool draw_corner_rect(CvImage& image, int x, int y, int w, int h, int r, int g, int b, int thick);

	bool draw_rect(CvImage& image, MyRect rc, int r, int g, int b, int thick);
	bool draw_rect(CvImage& image, MyRect rc, int bright, int thick);
	bool draw_rect(CvImage& image, int x, int y, int w, int h, int r, int g, int b, int thick);
	bool draw_rect(CvImage& image, int x, int y, int w, int h, int bright, int thick);

	bool draw_circle(CvImage& image, MyPoint center, int radius, int r, int g, int b, int thick);
	bool draw_circle(CvImage& image, MyPoint center, int radius, int bright, int thick);
	bool draw_circle(CvImage& image, int center_x, int center_y, int radius, int r, int g, int b, int thick);
	bool draw_circle(CvImage& image, int center_x, int center_y, int radius, int bright, int thick);

	bool draw_text(CvImage& image, char* text, MyPoint pt, int r, int g, int b, int thick);
	bool draw_text(CvImage& image, char* text, int x, int y, int r, int g, int b, int thick);
	bool draw_text(CvImage& image, char* text, MyPoint pt, int bright, int thick);
	bool draw_text(CvImage& image, char* text, int x, int y, int bright, int thick);
	bool draw_rich_text(CClientDC& dc, CvImage& image, char* text, int x, int y, int r, int g, int b, int font_size, int font_num);

	bool bgr2rgb(CvImage& src_image, CvImage& des_image);
	bool rgb2bgr(CvImage& src_image, CvImage& des_image);

	bool bgr2gray(CvImage& src_image, CvImage& des_image);
	bool rgb2gray(CvImage& src_image, CvImage& des_image);

	bool gray2bgr(CvImage& src_image, CvImage& des_image);
	bool gray2rgb(CvImage& src_image, CvImage& des_image);

	bool resize(CvImage& src_image, CvImage& des_image, int des_w, int des_h);
	void resize(unsigned char *srcData, int nSrcW, int nSrcH, unsigned char *desData, int nDesW, int nDesH);
	void image2MCT(unsigned char* srcData, int nSrcW, int nSrcH, int* mct);
	
	void rotation(CvImage& srCvImage, CvImage& rotationImage, int cx, int cy, double radian);
	void rotation(unsigned char* srcData, int src_w, int src_h, unsigned char*desData, int cx, int cy, double radian);
	void inverse_rotation(int x, int y, int cx, int cy, double radian, int &ix, int &iy);

	bool region(CvImage& src_image, CvImage& des_image, MyRect roi);
	bool region(CvImage& src_image, CvImage& des_image, int roi_x, int roi_y, int roi_w, int roi_h);

	bool he(CvImage& src_image, CvImage& des_image);
	bool he(unsigned char *image, int w, int h);
	bool amhe(CvImage& src_image, CvImage& des_image);
	bool amhe(unsigned char *image, int w, int h);
	bool CLAHE(CvImage& srCvImage, float fCliplimit);
	int CLAHE(unsigned char* pImage, unsigned int uiXRes, unsigned int uiYRes,
		unsigned char Min, unsigned char Max, unsigned int uiNrX, unsigned int uiNrY, unsigned int uiNrBins, float fCliplimit);
	void CLAHEClipHistogram(unsigned long* pulHistogram, unsigned int uiNrGreylevels, unsigned long ulClipLimit);
	void CLAHEMakeHistogram(unsigned char* pImage, unsigned int uiXRes,
		unsigned int uiSizeX, unsigned int uiSizeY, 
		unsigned long* pulHistogram, unsigned int uiNrGreylevels, unsigned char* pLookupTable);
	void CLAHEMapHistogram(unsigned long* pulHistogram, unsigned char Min, unsigned char Max,
	    unsigned int uiNrGreylevels, unsigned long ulNrOfPixels);
	void CLAHEMakeLut(unsigned char* pLUT, unsigned char Min, unsigned char Max, unsigned int uiNrBins);
	void CLAHEInterpPIMte(unsigned char* pImage, int uiXRes, unsigned long* pulMapLU,
		unsigned long* pulMapRU, unsigned long* pulMapLB,  unsigned long* pulMapRB,
		unsigned int uiXSize, unsigned int uiYSize, unsigned char* pLUT);

	bool threshold_binary(CvImage& src_image, CvImage& des_image, int th, int max_val, int inv_mode);
	bool threshold_tozero(CvImage& src_image, CvImage& des_image, int th, int max_val, int inv_mode);
	bool threshold_trunc(CvImage& src_image, CvImage& des_image, int th, int max_val);

	bool erode(CvImage& src_image, CvImage& des_image, int iter);
	bool dilate(CvImage& src_image, CvImage& des_image, int iter);

	int connected_component(CvImage& src_image, MyRect** rc);

	bool affine_trans(unsigned char* src_data, int src_w, int src_h, MyPoint ptSrcLEye, MyPoint ptSrcREye,
		unsigned char* des_data, int des_w, int des_h, MyPoint ptDesLEye, MyPoint ptDesREye,
		double f1, double f2, double f3, double f4);
	bool affine_trans_fr_MMCT(unsigned char* src_data, int src_w, int src_h, 
		unsigned char* des_data, int lx, int ly, int rx, int ry);
	bool affine_trans_fr_MMCT(CvImage& srCvImage, CvImage& faceImage, MyPoint ptLEye, MyPoint ptREye);
	bool affine_trans_fr(CvImage& src_image, CvImage& des_image, MyPoint ptLeftEye, MyPoint ptRightEye);

	bool combine_image(CvImage& src_image1, CvImage& src_image2, int x, int y, CvImage& des_image);
	void PrewittGradientAngle(unsigned char* src_image, int w, int h, unsigned char* des_image);
};

#endif 