//#include "stdafx.h"
#include "../inc/ImageProcessing.h"
#include <math.h>

using namespace std;

CImageProcessing::CImageProcessing()
{
}

CImageProcessing::~CImageProcessing()
{
}






bool CImageProcessing::load(const char* path, cv::Mat& image)
{
	//image = imread(path);
	return true;
}



bool CImageProcessing::save(const char* path, cv::Mat& image)
{
	//if(image.dims != 0) return imwrite(path, image);
	if( image.cols != 0 && image.rows != 0 ) return imwrite(path, image);
	return false;
}




bool CImageProcessing::clone(cv::Mat& src_image, cv::Mat& des_image)
{
	des_image = src_image;
	//if(src_image.GetImage()->roi == NULL) create(des_image, src_image.Width(), src_image.Height(), src_image.GetImage()->nChannels);
	//else create(des_image, src_image.GetImage()->roi->width, src_image.GetImage()->roi->height, src_image.GetImage()->nChannels);

	//des_image.CopyOf(src_image.GetImage());
	return true;
}

void CImageProcessing::size(cv::Mat& image, int& w, int& h, int& channel)
{
	w = h = channel = 0;

	w = image.cols;
	h = image.rows;
	channel = image.channels();
}




bool CImageProcessing::getraw_notnull(cv::Mat& image, unsigned char* pdata)
{
	int i, w, h, channel;
	size(image, w, h, channel);

	memcpy(pdata, image.data, w * h * channel);

	return true;
}




bool CImageProcessing::setraw(cv::Mat& image, int w, int h, int channel, unsigned char* pdata)
{
	//destroy(image);
	image.create(h, w, CV_8UC(channel));
	memcpy(image.data, pdata, sizeof(unsigned char) * w * h * channel);

	return true;
}

bool CImageProcessing::draw_rect(cv::Mat& image, MyRect rc, int r, int g, int b, int thick)
{
	return draw_rect(image, rc.left, rc.top, rc.Width(), rc.Height(), r, g, b, thick);
}

bool CImageProcessing::draw_rect(cv::Mat& image, MyRect rc, int bright, int thick)
{
	return draw_rect(image, rc.left, rc.top, rc.Width(), rc.Height(), bright, thick);
}

bool CImageProcessing::draw_rect(cv::Mat& image, int x, int y, int w, int h, int r, int g, int b, int thick)
{
	if(thick <= 0) 
		rectangle(image, cvPoint(x, y), cvPoint(x+w, y+h), CV_RGB(r, g, b), CV_FILLED);
	else 
		rectangle(image, cvPoint(x, y), cvPoint(x+w, y+h), CV_RGB(r, g, b), thick);
	return true;
}

bool CImageProcessing::draw_rect(cv::Mat& image, int x, int y, int w, int h, int bright, int thick)
{
	if(image.channels() == 3) 
		draw_rect(image, x, y, w, h, bright, bright, bright, thick);
	else {
		if(thick <= 0) 
			rectangle(image, cvPoint(x, y), cvPoint(x+w, y+h), cvScalar(bright), CV_FILLED);
		else 
			rectangle(image, cvPoint(x, y), cvPoint(x+w, y+h), cvScalar(bright), thick);
	}
	return true;
}


bool CImageProcessing::draw_circle(cv::Mat& image, MyPoint center, int radius, int r, int g, int b, int thick) 
{	
	return draw_circle(image, center.x, center.y, radius, r, g, b, thick);	
}

bool CImageProcessing::draw_circle(cv::Mat& image, MyPoint center, int radius, int bright, int thick)
{
	return draw_circle(image, center.x, center.y, radius, bright, thick);
}

bool CImageProcessing::draw_circle(cv::Mat& image, int center_x, int center_y, int radius, int r, int g, int b, int thick)
{	
	if(thick <= 0) 
		circle(image, cvPoint(center_x, center_y), radius, CV_RGB(r, g, b), CV_FILLED);
	else 
		circle(image, cvPoint(center_x, center_y), radius, CV_RGB(r, g, b), thick);
	return true;
}

bool CImageProcessing::draw_circle(cv::Mat& image, int center_x, int center_y, int radius, int bright, int thick)
{
	if(image.channels() == 3) 
		draw_circle(image, center_x, center_y, radius, bright, bright, bright, thick);
	else {
		if(thick <= 0) 
			circle(image, cvPoint(center_x, center_y), radius, cvScalar(bright), CV_FILLED);
		else 
			circle(image, cvPoint(center_x, center_y), radius, cvScalar(bright), thick);
	}
	return true;
}

bool CImageProcessing::draw_line(cv::Mat& image, MyPoint pt1, MyPoint pt2, int r, int g, int b, int thick)
{
	return draw_line(image, pt1.x, pt1.y, pt2.x, pt2.y, r, g, b, thick);
}

bool CImageProcessing::draw_line(cv::Mat& image, MyPoint pt1, MyPoint pt2, int bright, int thick)
{
	return draw_line(image, pt1.x, pt1.y, pt2.x, pt2.y, bright, thick);
}

bool CImageProcessing::draw_line(cv::Mat& image, int x1, int y1, int x2, int y2, int r, int g, int b, int thick)
{
	line(image, cvPoint(x1, y1), cvPoint(x2, y2), CV_RGB(r, g, b), thick, 8);
	return true;
}

bool CImageProcessing::draw_line(cv::Mat& image, int x1, int y1, int x2, int y2, int bright, int thick)
{
	if (image.channels() == 3) draw_line(image, x1, y1, x2, y2, bright, bright, bright, thick);
	else line(image, cvPoint(x1, y1), cvPoint(x2, y2), cvScalar(bright), thick, 8);
	return true;
}



bool CImageProcessing::bgr2gray(cv::Mat& src_image, cv::Mat& des_image)
{
	if(src_image.channels() != 3) return false;
	cvtColor(src_image, des_image, CV_BGR2GRAY);
	return true;
}

bool CImageProcessing::split(cv::Mat& src_image, cv::Mat& r, cv::Mat& g, cv::Mat& b)
{
	if (src_image.channels() != 3){ r = src_image;  return false; };

	vector<cv::Mat> bgr;

	//r.create(src_image.rows, src_image.cols, CV_8UC1);
	//g.create(src_image.rows, src_image.cols, CV_8UC1);
	//b.create(src_image.rows, src_image.cols, CV_8UC1);
	cv::split(src_image, bgr);
	b = bgr[0];
	g = bgr[1];
	r = bgr[2];
	
	return true;
}


bool CImageProcessing::gray2bgr(cv::Mat& src_image, cv::Mat& des_image)
{
	if( src_image.cols == 0 || src_image.rows == 0 || src_image.channels() != 1 ) return false;
	cvtColor(src_image, des_image, CV_GRAY2BGR);
	return true;
}

/*
bool CImageProcessing::rotation(CmyImage& srcImage, CmyImage& rotationImage, int cx, int cy, double radian)
{
	int w, h, ch, src_size;
	size(srcImage, w, h, ch);
	src_size = w*h;
	unsigned char* srcData = (unsigned char*)malloc(src_size);
	unsigned char* desData = (unsigned char*)malloc(src_size);
	getraw_notnull(srcImage, srcData);
	
	rotation(srcData, w, h, desData, cx, cy, radian);

	setraw(rotationImage, w, h, 1, desData);
	free(srcData);
	free(desData);

	return true;
}

void CImageProcessing::rotation(unsigned char* srcData, int src_w, int src_h, unsigned char*desData, int cx, int cy, double radian)
{
	int srcSize = src_w*src_h;
	memset(desData, 0, srcSize);
	double c = cos(-radian), s = sin(-radian);

	int x, y;
	double x_, y_;
	int l, k, src, src_l, src_k, src_lk, reg, des_w_step = 0;
	int end_w = src_w-1, end_h = src_h-1;
	double a, b, y1, y2;
	for(y=0; y<src_h; y++) {
		y1 = -s*(y-cy) + cx;
		y2 = c*(y-cy) + cy;
		for(x=0; x<src_w; x++) {
			reg = x-cx;
			x_ = c*reg + y1;
			y_ = s*reg + y2;

			if(x_ < 0 || x_ > end_w) continue;
			if(y_ < 0 || y_ > end_h) continue;
			
			l = (int)(x_);
			k = (int)(y_);
			a = x_ - l;
			b = y_ - k;
			reg = k*src_w+l;
			src = srcData[reg];
			src_l = srcData[reg+1];
			reg += src_w; 
			src_k = srcData[reg];
			src_lk = srcData[reg+1];

			desData[des_w_step+x] = (unsigned char)(src + a*(src_l-src) + b*(src_k-src) +
					a*b*(src+src_lk-src_l-src_k));
		}
		des_w_step += src_w;
	}
}

bool CImageProcessing::resize(CmyImage& src_image, CmyImage& des_image, int des_w, int des_h)
{
	if(src_image.GetImage() == NULL) return false;
	destroy(des_image);
	create(des_image, des_w, des_h, src_image.GetImage()->nChannels);
	cvResize(src_image.GetImage(), des_image.GetImage(), CV_INTER_LINEAR);
	return true;
}

bool CImageProcessing::combine_image(CmyImage& src_image1, CmyImage& src_image2, int x, int y, CmyImage& des_image)
{
	if(src_image1.GetImage() == NULL || src_image2.GetImage() == NULL) return false;
	int w1, h1, c1, w2, h2, c2;
	
	size(src_image1, w1, h1, c1);
	size(src_image2, w2, h2, c2);
	if(w1 < (x+w2) || h1 < (y+h2) || c1 != c2) return false;

	unsigned char* src_data1 = new unsigned char[w1*h1*c1];
	unsigned char* src_data2 = new unsigned char[w2*h2*c2];

	getraw_notnull(src_image1, src_data1);
	getraw_notnull(src_image2, src_data2);

	if(c1 == 1) {
		int sp=y, ep=y+h2, p1=(y*w1)+x, p2=0;
		for(sp=y; sp<ep; sp++) {
			memcpy(src_data1+p1, src_data2+p2, w2);
			p1 += w1; p2 += w2;		
		}
	}
	else {
		int sp=y, ep=y+h2, p1=((y*w1)+x)*3, p2=0;
		for(sp=y; sp<ep; sp++) {
			memcpy(src_data1+p1, src_data2+p2, w2*3);
			p1 += (w1*3); p2 += (w2*3);		
		}
	}

	setraw(des_image, w1, h1, c1, src_data1);
	delete[] src_data1;
	delete[] src_data2;
	return true;
}*/

bool CImageProcessing::draw_text(cv::Mat& image, char* text, MyPoint pt, int r, int g, int b, int thick)
{	
	return draw_text(image, text, pt.x, pt.y, r, g, b, thick);
}

bool CImageProcessing::draw_text(cv::Mat& image, char* text, int x, int y, int r, int g, int b, int thick)
{
	CvFont font;
	cvInitFont(&font, CV_FONT_VECTOR0, 0.5, 0.5, 0, thick);
	putText(image, text, cvPoint(x, y), CV_FONT_VECTOR0, 0.5, CV_RGB(r, g, b), thick);
	return true;
}

bool CImageProcessing::draw_text(cv::Mat& image, char* text, MyPoint pt, int bright, int thick)
{	
	return draw_text(image, text, pt.x, pt.y, bright, bright, bright, thick);
}

bool CImageProcessing::draw_text(cv::Mat& image, char* text, int x, int y, int bright, int thick)
{
	return draw_text(image, text, x, y, bright, bright, bright, thick);
}

/*
bool CImageProcessing::filt_gaussian(CmyImage& src_image, CmyImage& des_image, int mask_w, int mask_h)
{
	if(src_image.GetImage() == NULL) return false;
	destroy(des_image);
	create(des_image, src_image.Width(), src_image.Height(), src_image.GetImage()->nChannels);
	cvSmooth(src_image.GetImage(), des_image.GetImage(), CV_GAUSSIAN, mask_w, mask_h);
	return true;
}


bool CImageProcessing::erode(CmyImage& src_image, CmyImage& des_image, int iter)
{
	if(src_image.GetImage() == NULL) return false;
	destroy(des_image);
	create(des_image, src_image.Width(), src_image.Height(), src_image.GetImage()->nChannels);
	cvErode(src_image.GetImage(), des_image.GetImage(), 0, iter);
	return true;
}

bool CImageProcessing::dilate(CmyImage& src_image, CmyImage& des_image, int iter)
{
	if(src_image.GetImage() == NULL) return false;
	destroy(des_image);
	create(des_image, src_image.Width(), src_image.Height(), src_image.GetImage()->nChannels);
	cvDilate(src_image.GetImage(), des_image.GetImage(), 0, iter);
	return true;
}

void CImageProcessing::flipx(CmyImage& src_image, CmyImage& des_image)
{
	cvFlip(src_image.GetImage(), des_image.GetImage(), 1);
}*/

void CImageProcessing::flipx(unsigned char* src_data, int src_w, int src_h, unsigned char* des_data)
{
	int x, y, k;
	unsigned char* raw_data = new unsigned char[src_w];

	for(k=0, y=0; y<src_h; y++) {
		memcpy(raw_data, src_data+k, src_w);
		for(x=src_w-1; x>=0; x--) des_data[k++] = raw_data[x];
	}
	delete[] raw_data;
}
