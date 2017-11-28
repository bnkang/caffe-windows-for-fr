#include "stdafx.h"
#include <atltypes.h>
#include <string.h>

#include "CvImage.h"
#include "ImageProcessing.h"
#include <math.h>

CImageProcess::CImageProcess()
{
}

CImageProcess::~CImageProcess()
{
}

void CImageProcess::size(CvImage& image, int& w, int& h, int& channel)
{
	w = h = channel = 0;
	if(image.GetImage() == NULL) return;

	w = image.Width();
	h = image.Height();
	channel = image.GetImage()->nChannels;
}

bool CImageProcess::create(CvImage& image, int w, int h, int channel)
{
	if(w <= 0 || h <= 0 || !(channel==1 || channel == 3)) return false;
	destroy(image);
	image.Create(w, h, channel*8);
	if(image.GetImage() == NULL) return false;
	memset(image.GetImage()->imageData, 0, image.GetImage()->imageSize);
	return true;
}

void CImageProcess::destroy(CvImage& image)
{
	if(image.GetImage()) image.Destroy();
}

bool CImageProcess::load(const char* path, CvImage& image)
{
	destroy(image);
	return image.Load(path);
}

bool CImageProcess::save(const char* path, CvImage& image)
{
	if(image.GetImage()) return image.Save(path);
	return false;
}

bool CImageProcess::clone(CvImage& src_image, CvImage& des_image)
{
	if(src_image.GetImage() == NULL) return false;
	if(src_image.GetImage()->roi == NULL) create(des_image, src_image.Width(), src_image.Height(), src_image.GetImage()->nChannels);
	else create(des_image, src_image.GetImage()->roi->width, src_image.GetImage()->roi->height, src_image.GetImage()->nChannels);
	des_image.CopyOf(src_image);
	return true;
}

bool CImageProcess::getraw(CvImage& image, unsigned char* pdata)
{
	if(image.GetImage() == NULL) return false;
	int i, w, h, channel;
	size(image, w, h, channel);
	
	int step = w*channel;
	int dumy = step%4;
	if(dumy) dumy = 4 - dumy;
	if(!dumy) memcpy(pdata, image.GetImage()->imageData, image.GetImage()->imageSize);
	else {
		for(i=0; i<h; i++) memcpy(pdata+i*step, image.GetImage()->imageData+i*(step+dumy), step);		
	}	
	return true;
}

bool CImageProcess::getraw_notnull(CvImage& image, unsigned char* pdata)
{
	if(image.GetImage() == NULL) return false;
	int i, w, h, channel;
	size(image, w, h, channel);

	int step = w*channel;
	int dumy = step%4;
	if(dumy) dumy = 4 - dumy;
	if(!dumy) memcpy(pdata, image.GetImage()->imageData, image.GetImage()->imageSize);
	else {
		for(i=0; i<h; i++) memcpy(pdata+i*step, image.GetImage()->imageData+i*(step+dumy), step);		
	}	
	return true;
}

bool CImageProcess::setraw(CvImage& image, int w, int h, int channel, const unsigned char* pdata)
{
	destroy(image);
	create(image, w, h, channel);

	int i;
	int step = w*channel;
	int dumy = step%4;
	if(dumy) dumy = 4 - dumy;
	if(!dumy) memcpy(image.GetImage()->imageData, pdata, image.GetImage()->imageSize);
	else {
		for(i=0; i<h; i++) memcpy(image.GetImage()->imageData+i*(step+dumy), pdata+i*step, step);		
	}
	return true;
}

#ifdef _WIN32
bool CImageProcess::draw(HDC hDCDst, CvImage& image, int screen_x, int screen_y)
{
	if(image.GetImage() == NULL) return false;
	CRect rc = CRect(screen_x, screen_y, screen_x+image.Width(), screen_y+image.Height());
	image.DrawToHDC(hDCDst, &rc);
	return true;
}

bool CImageProcess::draw(HDC hDCDst, CvImage& image, int screen_x, int screen_y, MyRect rcROI)
{
	if(image.GetImage() == NULL) return false;
	if(rcROI.left + rcROI.Width() > image.Width() || rcROI.top + rcROI.Height() > image.Height()) return false;
	cvSetImageROI(image.GetImage(), cvRect(rcROI.left, rcROI.top, rcROI.Width(), rcROI.Height()));
	CRect rc = CRect(screen_x, screen_y, screen_x+rcROI.Width(), screen_y+rcROI.Height());
	image.DrawToHDC(hDCDst, &rc);
	cvResetImageROI(image.GetImage());	
	return true;
}
#endif

bool CImageProcess::draw_line(CvImage& image, MyPoint pt1, MyPoint pt2, int r, int g, int b, int thick)
{	
	return draw_line(image, pt1.x, pt1.y, pt2.x, pt2.y, r, g, b, thick);
}

bool CImageProcess::draw_line(CvImage& image, MyPoint pt1, MyPoint pt2, int bright, int thick)
{
	return draw_line(image, pt1.x, pt1.y, pt2.x, pt2.y, bright, thick);
}

bool CImageProcess::draw_line(CvImage& image, int x1, int y1, int x2, int y2, int r, int g, int b, int thick)
{
	if(image.GetImage() == NULL) return false;
	if(image.GetImage()->nChannels == 3) cvLine(image.GetImage(), cvPoint(x1, y1), cvPoint(x2, y2), CV_RGB(r, g, b), thick, 8);
	else draw_line(image, x1, y1, x2, y2, (r+g+b)/3, thick);
	return true;
}

bool CImageProcess::draw_line(CvImage& image, int x1, int y1, int x2, int y2, int bright, int thick)
{
	if(image.GetImage() == NULL) return false;
	if(image.GetImage()->nChannels == 3) draw_line(image, x1, y1, x2, y2, bright, bright, bright, thick);
	else cvLine(image.GetImage(), cvPoint(x1, y1), cvPoint(x2, y2), cvScalar(bright), thick, 8);
	return true;
}

bool CImageProcess::draw_rect(CvImage& image, MyRect rc, int r, int g, int b, int thick)
{
	return draw_rect(image, rc.left, rc.top, rc.Width(), rc.Height(), r, g, b, thick);
}

bool CImageProcess::draw_rect(CvImage& image, MyRect rc, int bright, int thick)
{
	return draw_rect(image, rc.left, rc.top, rc.Width(), rc.Height(), bright, thick);
}

bool CImageProcess::draw_rect(CvImage& image, int x, int y, int w, int h, int r, int g, int b, int thick)
{
	if(image.GetImage() == NULL) return false;
	if(thick <= 0) cvRectangle(image.GetImage(), cvPoint(x, y), cvPoint(x+w, y+h), CV_RGB(r, g, b), CV_FILLED);
	else cvRectangle(image.GetImage(), cvPoint(x, y), cvPoint(x+w, y+h), CV_RGB(r, g, b), thick);
	return true;
}

bool CImageProcess::draw_rect(CvImage& image, int x, int y, int w, int h, int bright, int thick)
{
	if(image.GetImage() == NULL) return false;
	if(image.GetImage()->nChannels == 3) draw_rect(image, x, y, w, h, bright, bright, bright, thick);
	else {
		if(thick <= 0) cvRectangle(image.GetImage(), cvPoint(x, y), cvPoint(x+w, y+h), cvScalar(bright), CV_FILLED);
		else cvRectangle(image.GetImage(), cvPoint(x, y), cvPoint(x+w, y+h), cvScalar(bright), thick);
	}
	return true;
}

bool CImageProcess::draw_corner_rect(CvImage& image, int x, int y, int w, int h, int r, int g, int b, int thick)
{
	int nMargin = w/6;

	draw_line(image, x, y, x + nMargin, y, r, g, b, thick);
	draw_line(image, x, y, x, y + nMargin, r, g, b, thick);
	draw_line(image, x + w, y, x + w - nMargin, y, r, g, b, thick);
	draw_line(image, x + w, y, x + w, y + nMargin, r, g, b, thick);

	draw_line(image, x, y + h, x + nMargin, y + h, r, g, b, thick);
	draw_line(image, x, y + h, x, y +h - nMargin, r, g, b, thick);
	draw_line(image, x + w, y + h, x + w - nMargin, y + h, r, g, b, thick);
	draw_line(image, x + w, y + h, x + w, y + h - nMargin, r, g, b, thick);

	return true;
}

bool CImageProcess::draw_cross(CvImage& image, int x, int y, int r, int g, int b, int thick) 
{	
	int nMargin = 3;
	draw_line(image, x - nMargin , y, x + nMargin, y, r, g, b, thick);
	draw_line(image, x , y - nMargin, x, y + nMargin, r, g, b, thick);

	return true;
}

bool CImageProcess::draw_cross(CvImage& image, MyPoint center, int r, int g, int b, int thick) 
{	
	int nMargin = 3;
	draw_line(image, center.x - nMargin , center.y, center.x + nMargin, center.y, r, g, b, thick);
	draw_line(image, center.x , center.y - nMargin, center.x, center.y + nMargin, r, g, b, thick);

	return true;
}

bool CImageProcess::draw_circle(CvImage& image, MyPoint center, int radius, int r, int g, int b, int thick) 
{	
	return draw_circle(image, center.x, center.y, radius, r, g, b, thick);	
}

bool CImageProcess::draw_circle(CvImage& image, MyPoint center, int radius, int bright, int thick)
{
	return draw_circle(image, center.x, center.y, radius, bright, thick);
}

bool CImageProcess::draw_circle(CvImage& image, int center_x, int center_y, int radius, int r, int g, int b, int thick)
{	
	if(image.GetImage() == NULL) return false;
	if(thick <= 0) cvCircle(image.GetImage(), cvPoint(center_x, center_y), radius, CV_RGB(r, g, b), CV_FILLED);
	else cvCircle(image.GetImage(), cvPoint(center_x, center_y), radius, CV_RGB(r, g, b), thick);
	return true;
}

bool CImageProcess::draw_circle(CvImage& image, int center_x, int center_y, int radius, int bright, int thick)
{
	if(image.GetImage() == NULL) return false;
	if(image.GetImage()->nChannels == 3) draw_circle(image, center_x, center_y, radius, bright, bright, bright, thick);
	else {
		if(thick <= 0) cvCircle(image.GetImage(), cvPoint(center_x, center_y), radius, cvScalar(bright), CV_FILLED);
		else cvCircle(image.GetImage(), cvPoint(center_x, center_y), radius, cvScalar(bright), thick);
	}
	return true;
}

bool CImageProcess::draw_text(CvImage& image, char* text, MyPoint pt, int r, int g, int b, int thick)
{	
	return draw_text(image, text, pt.x, pt.y, r, g, b, thick);
}

bool CImageProcess::draw_text(CvImage& image, char* text, int x, int y, int r, int g, int b, int thick)
{
	if(image.GetImage() == NULL) return false;
	CvFont font;
	cvInitFont(&font, CV_FONT_VECTOR0, 0.5, 0.5, 0, thick);
	cvPutText(image.GetImage(), text, cvPoint(x, y), &font, CV_RGB(r, g, b));
	return true;
}

bool CImageProcess::draw_text(CvImage& image, char* text, MyPoint pt, int bright, int thick)
{	
	return draw_text(image, text, pt.x, pt.y, bright, bright, bright, thick);
}

bool CImageProcess::draw_text(CvImage& image, char* text, int x, int y, int bright, int thick)
{
	return draw_text(image, text, x, y, bright, bright, bright, thick);
}


bool CImageProcess::draw_rich_text(CClientDC& dc, CvImage& image, char* text, int x, int y, int r, int g, int b, int font_size, int font_num)
{
	if(text == NULL) return false;
	if(image.GetImage() == NULL) return false;

	int				w, h, c, data_size;
	unsigned char	*bitmap_raw_data, *bitmap_raw_data_ptr;
	unsigned char	*raw_data, *raw_data_ptr;
	CString			font_style;
	CFont			font, *pOldFont;
	CBitmap			bitmap, *pOldBitmap;
	CDC				memDC;

	// 메모리 DC 생성
	memDC.CreateCompatibleDC(0);

	// 이미지의 raw data얻어오기 
	// bitmap raw_data생성 & raw_data 복사
	size(image, w, h, c);

	data_size = w * h * 4;

	raw_data = new unsigned char [w * h * c];
	memset(raw_data, 0, sizeof(unsigned char) * w * h * c);

	bitmap_raw_data = new unsigned char [data_size];
	memset(bitmap_raw_data, 0, sizeof(unsigned char) * data_size);

	getraw(image, raw_data);

	raw_data_ptr = raw_data;
	bitmap_raw_data_ptr = bitmap_raw_data;
	for(int i = 0; i < w * h; i++){
		if(c == 3){
			memcpy(bitmap_raw_data_ptr, raw_data_ptr, sizeof(unsigned char) * 3);
			raw_data_ptr += 3;
			bitmap_raw_data_ptr += 4;
		}
		else{
			*bitmap_raw_data_ptr++ = *raw_data_ptr;
			*bitmap_raw_data_ptr++ = *raw_data_ptr;
			*bitmap_raw_data_ptr++ = *raw_data_ptr++;
			bitmap_raw_data_ptr++;
		}
	}

	bitmap.CreateBitmap(w, h, 1, 32, bitmap_raw_data);

	pOldBitmap = (CBitmap *)memDC.SelectObject(&bitmap);

	// font 설정
	switch(font_num)
	{
	case 0:
		font_style.Format("Britannic Bold");
		break;
	case 1:
		font_style.Format("Times New Roman");
		break;
	case 2:
		font_style.Format("Franklin Gothic Demi");
		break;
	default:
		return false;
	}
	font.CreatePointFont(font_size * 10, font_style);   

	pOldFont = (CFont *)memDC.SelectObject(&font);

	memDC.SetTextColor(RGB(r, g, b));
	memDC.SetBkMode(TRANSPARENT);
	memDC.TextOut(x, y, text);

	memset(bitmap_raw_data, 0, sizeof(unsigned char) * data_size);
	bitmap.GetBitmapBits(data_size, bitmap_raw_data); 

	memset(raw_data, 0, sizeof(unsigned char) * w * h * c);

	raw_data_ptr = raw_data;
	bitmap_raw_data_ptr = bitmap_raw_data;
	for(int i = 0; i < w * h; i++){
		if(c == 3){
			memcpy(raw_data_ptr, bitmap_raw_data_ptr, sizeof(unsigned char) * 3);
			raw_data_ptr += 3;
			bitmap_raw_data_ptr += 4;
		}
		else{
			*raw_data_ptr++ = *bitmap_raw_data_ptr * 0.3 + *(bitmap_raw_data_ptr+1) * 0.59 + *(bitmap_raw_data_ptr+2) * 0.11;
			*bitmap_raw_data_ptr += 4;
		}
	}
	setraw(image, w, h, c, raw_data);	

	delete[] raw_data;
	delete[] bitmap_raw_data;

	bitmap.DeleteObject();
	DeleteDC(memDC);
	dc.SelectObject(pOldBitmap);
	dc.SelectObject(pOldFont);

	return true;
}

bool CImageProcess::bgr2rgb(CvImage& src_image, CvImage& des_image)
{
	if(src_image.GetImage() == NULL || src_image.GetImage()->nChannels != 3) return false;
	destroy(des_image);
	create(des_image, src_image.Width(), src_image.Height(), src_image.GetImage()->nChannels);
	cvCvtColor(src_image.GetImage(), des_image.GetImage(), CV_BGR2RGB);
	return true;
}

bool CImageProcess::rgb2bgr(CvImage& src_image, CvImage& des_image)
{
	if(src_image.GetImage() == NULL || src_image.GetImage()->nChannels != 3) return false;
	destroy(des_image);
	create(des_image, src_image.Width(), src_image.Height(), src_image.GetImage()->nChannels);
	cvCvtColor(src_image.GetImage(), des_image.GetImage(), CV_RGB2BGR);
	return true;
}

bool CImageProcess::bgr2gray(CvImage& src_image, CvImage& des_image)
{
	if(src_image.GetImage() == NULL || src_image.GetImage()->nChannels != 3) return false;
	destroy(des_image);
	create(des_image, src_image.Width(), src_image.Height(), 1);
	cvCvtColor(src_image.GetImage(), des_image.GetImage(), CV_BGR2GRAY);
	return true;
}

bool CImageProcess::rgb2gray(CvImage& src_image, CvImage& des_image)
{
	if(src_image.GetImage() == NULL || src_image.GetImage()->nChannels != 3) return false;
	destroy(des_image);
	create(des_image, src_image.Width(), src_image.Height(), 1);
	cvCvtColor(src_image.GetImage(), des_image.GetImage(), CV_RGB2GRAY);
	return true;
}

bool CImageProcess::gray2bgr(CvImage& src_image, CvImage& des_image)
{
	if(src_image.GetImage() == NULL || src_image.GetImage()->nChannels != 1) return false;
	destroy(des_image);
	create(des_image, src_image.Width(), src_image.Height(), 3);
	cvCvtColor(src_image.GetImage(), des_image.GetImage(), CV_GRAY2BGR);
	return true;
}

bool CImageProcess::gray2rgb(CvImage& src_image, CvImage& des_image)
{
	if(src_image.GetImage() == NULL || src_image.GetImage()->nChannels != 1) return false;
	destroy(des_image);
	create(des_image, src_image.Width(), src_image.Height(), 3);
	cvCvtColor(src_image.GetImage(), des_image.GetImage(), CV_GRAY2RGB);
	return true;
}

bool CImageProcess::resize(CvImage& src_image, CvImage& des_image, int des_w, int des_h)
{
	if(src_image.GetImage() == NULL) return false;
	destroy(des_image);
	create(des_image, des_w, des_h, src_image.GetImage()->nChannels);
	cvResize(src_image.GetImage(), des_image.GetImage(), CV_INTER_LINEAR);
	return true;
}

void CImageProcess::resize(unsigned char *srcData, int nSrcW, int nSrcH, unsigned char *desData, int nDesW, int nDesH)
{
	/* < 10 */
	int udtx=0, udty=0, udtx_=0, udty_=0;
	/* < 10 */
	int uda_11;
	/* < 10 */
	int uda_22;
	int i, j;
	int utx=0, uty=0;
	int locgray4 = 0;
	int ucgray1 = 0, ucgray2 = 0, ucgray3 = 0, ucgray4 = 0;
	int nSrcIndex=0;
	unsigned char *pucsrcp = 0;
	unsigned char *pucdstp = desData;
	unsigned int aa, ab, ac, ad;
	
	/* Affine transformation */	
	uda_11 = (nSrcW << 10) / nDesW;
	uda_22 = (nSrcH << 10) / nDesH;

	for(j = 0; j < nDesH; j++) {
		udtx = 0;
		udtx_ = 0;
		utx = 0;
		for(i = 0; i < nDesW; i++) {
			locgray4 = 0;						
			pucsrcp = srcData + (nSrcIndex + utx);
			ucgray1 = *pucsrcp;

			if(i >= nDesW - 1) {
				ucgray2 = *pucsrcp;
				udtx = 1 << 10;
			}
			else {
				ucgray2 = *(pucsrcp + 1);
				locgray4 += 1;
			}

			if (j >= nDesH - 1) {
				ucgray3 = *pucsrcp;
				udty = 1 << 10;
			}
			else {
				ucgray3 = *( pucsrcp + nSrcW );
				locgray4 += nSrcW;
			}
			ucgray4 = *( pucsrcp + locgray4 );

			aa = ucgray1 << 20;
			ab = ((ucgray2 - ucgray1) * (udtx << 10));
			ac = ((ucgray3 - ucgray1) * (udty << 10));
			ad = (ucgray1 + ucgray4 - ucgray3 - ucgray2) * udtx * udty;
			*(pucdstp + i) = (unsigned char)((aa + ab + ac + ad)>> 20);

			udtx_ += uda_11;
			udtx = udtx_;			
			utx = udtx >> 10;			
			udtx -= ( utx << 10 );
		}
		
		udty_ += uda_22;
		udty = udty_;
		uty = udty >> 10;
		udty -= ( uty << 10 );
		nSrcIndex = uty * nSrcW;
		pucdstp += nDesW;
	}
}

void CImageProcess::image2MCT(unsigned char* srcData, int nSrcW, int nSrcH, int* mct)
{
	int x, y, w=nSrcW, h=nSrcH-1, src_size=nSrcW*nSrcH;
	int m1, m2, m3, m4, m5, m6, m7, m8, m9;
	int mu1, mu2, mu3, mu4, mu5, mu6, mu7, mu8, mu9;
	int data_mean;
	int data_mct;	
	unsigned char *pt1, *pt2=srcData+nSrcW, *pt3;
	int *pmct = mct+nSrcW+1;

	memset(mct, 0, sizeof(int)*src_size);
	pt1 = pt2-nSrcW;
	pt3 = pt2+nSrcW;
	for(y=1; y<h; y++) {
		m1 = *pt1++;
		m4 = *pt2++;
	    m7 = *pt3++;
		m2 = *pt1++;
		m5 = *pt2++;
		m8 = *pt3++;		
		for(x=2; x<w; x++) {
			m3 = *pt1++;
			m6 = *pt2++;
			m9 = *pt3++;
			data_mean = ((m1+m2+m3+m4+m5+m6+m7+m8+m9));
			
			mu1 = data_mean - (( m1 << 3 ) + m1);
			mu2 = data_mean - (( m2 << 3 ) + m2);
			mu3 = data_mean - (( m3 << 3 ) + m3);
			mu4 = data_mean - (( m4 << 3 ) + m4);
			mu5 = data_mean - (( m5 << 3 ) + m5);
			mu6 = data_mean - (( m6 << 3 ) + m6);
			mu7 = data_mean - (( m7 << 3 ) + m7);
			mu8 = data_mean - (( m8 << 3 ) + m8);
			mu9 = data_mean - (( m9 << 3 ) + m9);

			/*
			data_mct = 0;			
			if(mu1 < 0) data_mct |= 1;
			if(mu2 < 0) data_mct |= 2;
			if(mu3 < 0) data_mct |= 4;
			if(mu4 < 0) data_mct |= 8;
			if(mu5 < 0) data_mct |= 16;
			if(mu6 < 0) data_mct |= 32;
			if(mu7 < 0) data_mct |= 64;
			if(mu8 < 0) data_mct |= 128;
			if(mu9 < 0) data_mct |= 256;
			*/
			
			data_mct = ((mu1>>31)&1) + (((mu2>>31)&1)<<1) + (((mu3>>31)&1)<<2) +
			   (((mu4>>31)&1)<<3) + (((mu5>>31)&1)<<4) + (((mu6>>31)&1)<<5) +
			   (((mu7>>31)&1)<<6) + (((mu8>>31)&1)<<7) + (((mu9>>31)&1)<<8);

			m1 = m2;
			m4 = m5;
			m7 = m8;
			m2 = m3;
			m5 = m6;
			m8 = m9;
			
			*pmct++ = data_mct;
		}
		pmct++;
		pmct++;
	}
}

void CImageProcess::rotation(CvImage& srCvImage, CvImage& rotationImage, int cx, int cy, double radian)
{
	int w, h, ch, src_size;
	size(srCvImage, w, h, ch);
	src_size = w*h;
	unsigned char* srcData = (unsigned char*)malloc(src_size);
	unsigned char* desData = (unsigned char*)malloc(src_size);
	getraw(srCvImage, srcData);
	
	rotation(srcData, w, h, desData, cx, cy, radian);

	setraw(rotationImage, w, h, 1, desData);
	FREE(srcData);
	FREE(desData);
}

void CImageProcess::rotation(unsigned char* srcData, int src_w, int src_h, unsigned char*desData, int cx, int cy, double radian)
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

void CImageProcess::inverse_rotation(int x, int y, int cx, int cy, double radian, int &ix, int &iy)
{
	double c = cos(radian), s = sin(radian);

	ix = (int)(c*(x-cx) + s*(y-cy) + cx + 0.5);
	iy = (int)(-s*(x-cx) + c*(y-cy) + cy + 0.5);
}

bool CImageProcess::region(CvImage& src_image, CvImage& des_image, MyRect roi)
{
	return region(src_image, des_image, roi.left, roi.top, roi.Width(), roi.Height());
}

bool CImageProcess::region(CvImage& src_image, CvImage& des_image, int roi_x, int roi_y, int roi_w, int roi_h)
{
	TRACE("region_0\n");
	if(src_image.GetImage() == NULL) return false;
	if(roi_x < 0 || roi_y < 0) return false;
	if(roi_w >= src_image.Width() || roi_h >= src_image.Height()) return false;
	if(roi_x + roi_w >= src_image.Width() || roi_y + roi_h >= src_image.Height()) return false;
	TRACE("region_1\n");
	
	cvSetImageROI(src_image.GetImage(), cvRect(roi_x, roi_y, roi_w, roi_h));
	TRACE("region_2\n");
	clone(src_image, des_image);
	TRACE("region_3\n");
	cvResetImageROI(src_image.GetImage());
	TRACE("region_4\n");
	return true;
}

bool CImageProcess::he(CvImage& src_image, CvImage& des_image)
{
	int w, h, c;
	size(src_image, w, h, c);
	if(w <= 0 || h <= 0 || c != 1) return false;

	unsigned char* srcData = (unsigned char*)malloc(w*h);
	getraw(src_image, srcData);
	he(srcData, w, h);
	setraw(des_image, w, h, 1, srcData);
	FREE(srcData);
    return true;
}

bool CImageProcess::he(unsigned char *image, int w, int h)
{
	int i, size = w * h;
    int map_table[256];
    memset(map_table, 0, sizeof(int)*256);
    for(i=0; i<size; i++) map_table[image[i]]++;
    for(i=1; i<256; i++) map_table[i] += map_table[i-1];
	
	double hist_value = 255.0/map_table[255];	
    for(i=0; i<256; i++) map_table[i] = (int)(map_table[i]*hist_value);
    for(i=0; i<size; i++) image[i] = (unsigned char)map_table[image[i]];
    return true;
}

bool CImageProcess::amhe(CvImage& src_image, CvImage& des_image)
{
	int w, h, c;
	size(src_image, w, h, c);
	if(w <= 0 || h <= 0 || c != 1) return false;
	
	unsigned char* srcData = (unsigned char*)malloc(w*h);
	getraw(src_image, srcData);
	amhe(srcData, w, h);
	setraw(des_image, w, h, 1, srcData);
	FREE(srcData);
    return true;
}

bool CImageProcess::amhe(unsigned char *image, int w, int h)
{
	int i, size=w*h, map_table[256];
    memset(map_table, 0, sizeof(int)*256);
	unsigned char* pimage = image;
    for(i=0; i<size; i++) map_table[*pimage++]++;
	
	int p_max = 0, p_mid;
	for(i=0; i<256; i++) {
		if(p_max < map_table[i]) p_max = map_table[i];
	}
	p_mid = p_max / 2;

	double alpha_high = 0.75/(p_max-p_mid);
	double alpha_low = 0.75/(p_mid);
	for(i=0; i<256; i++) {
		if(map_table[i] > p_mid) {
			map_table[i] = p_mid + int(alpha_high*(map_table[i]-p_mid)*(map_table[i]-p_mid)+0.5);
		}
		else {
			map_table[i] = p_mid - int(alpha_low*(p_mid-map_table[i])*(p_mid-map_table[i])+0.5);
		}
	}

    for(i=1; i<256; i++) map_table[i] += map_table[i-1];
	double hist_value = 255.0/map_table[255];	
    for(i=0; i<256; i++) map_table[i] = (int)(map_table[i]*hist_value);

	pimage = image;
    for(i=0; i<size; i++) {
		*pimage = (unsigned char)map_table[*pimage];
		pimage++;
	}
	return true;
}

bool CImageProcess::CLAHE(CvImage& srCvImage, float fCliplimit)
{
	int w, h, c;
	size(srCvImage, w, h, c);
	if(c != 1) return false;

	unsigned char* srcData = (unsigned char*)malloc(w*h);
	if(getraw(srCvImage, srcData) == false) {
		FREE(srcData);
		return false;
	}
	if(CLAHE(srcData, w, h, 0, 255, 2, 2, 128, fCliplimit) != 0) {
		FREE(srcData);
		return false;
	}
	setraw(srCvImage, w, h, 1, srcData);
	FREE(srcData);
    return true;
}

int CImageProcess::CLAHE(unsigned char* pImage, unsigned int uiXRes, unsigned int uiYRes,
	 unsigned char Min, unsigned char Max, unsigned int uiNrX, unsigned int uiNrY, unsigned int uiNrBins, float fCliplimit)
/*   pImage - Pointer to the input/output image
 *   uiXRes - Image resolution in the X direction
 *   uiYRes - Image resolution in the Y direction
 *   Min - Minimum greyvalue of input image (also becomes minimum of output image)
 *   Max - Maximum greyvalue of input image (also becomes maximum of output image)
 *   uiNrX - Number of contextial regions in the X direction (min 2, max 16)
 *   uiNrY - Number of contextial regions in the Y direction (min 2, max 16)
 *   uiNrBins - Number of greybins for histogram ("dynamic range")
 *   float fCliplimit - Normalized cliplimit (higher values give more contrast)
 * The number of "effective" greylevels in the output image is set by uiNrBins; selecting
 * a small value (eg. 128) speeds up processing and still produce an output image of
 * good quality. The output image will have the same minimum and maximum value as the input
 * image. A clip limit smaller than 1 results in standard (non-contrast limited) AHE.
 */
{
    unsigned int uiX=0, uiY=0;								/* counters */
    unsigned int uiXSize=0, uiYSize=0, uiSubX=0, uiSubY=0;	/* size of context. reg. and subimages */
    unsigned int uiXL=0, uiXR=0, uiYU=0, uiYB=0;			/* auxiliary variables interpPIMtion routine */
    unsigned long ulClipLimit=0, ulNrPixels=0;				/* clip limit and region pixel count */
    unsigned char* pImPointer=NULL;							/* pointer to image */
    unsigned char aLUT[256];								/* lookup table used for scaling of input image */
	memset(aLUT, 0, sizeof(unsigned char)*256);
    unsigned long* pulHist=NULL, *pulMapArray=NULL;			/* pointer to histogram and mappings*/
    unsigned long* pulLU=NULL, *pulLB=NULL, *pulRU=NULL, *pulRB=NULL; /* auxiliary pointers interpPIMtion */

    if (uiNrX > 16) return -1;					/* # of regions x-direction too large */
    if (uiNrY > 16) return -2;					/* # of regions y-direction too large */
//	if (uiXRes % uiNrX) return -3;				/* x-resolution no multiple of uiNrX */
//	if (uiYRes % uiNrY) return -4;				/* y-resolution no multiple of uiNrY */
    if (Max >= 256) return -5;					/* maximum too large */
    if (Min >= Max) return -6;					/* minimum equal or larger than maximum */
    if (uiNrX < 2 || uiNrY < 2) return -7;		/* at least 4 contextual regions required */
    if (fCliplimit == 1.0) return 0;			/* is OK, immediately returns original image. */
    if (uiNrBins == 0) uiNrBins = 128;			/* default value when not specified */

    pulMapArray = (unsigned long *)malloc(sizeof(unsigned long)*uiNrX*uiNrY*uiNrBins);
	memset(pulMapArray, 0, sizeof(unsigned long)*uiNrX*uiNrY*uiNrBins);

    uiXSize = uiXRes/uiNrX; uiYSize = uiYRes/uiNrY;			/* Actual size of contextual regions */
    ulNrPixels = (unsigned long)uiXSize * (unsigned long)uiYSize;

    if(fCliplimit > 0.0) {									/* Calculate actual cliplimit	 */
       ulClipLimit = (unsigned long) (fCliplimit * (uiXSize * uiYSize) / uiNrBins);
       ulClipLimit = (ulClipLimit < 1UL) ? 1UL : ulClipLimit;
    }
    else ulClipLimit = 1UL<<14;								/* Large value, do not clip (AHE) */
    CLAHEMakeLut(aLUT, Min, Max, uiNrBins);					/* Make lookup table for mapping of greyvalues */

    /* Calculate greylevel mappings for each contextual region */
    for (uiY = 0, pImPointer = pImage; uiY < uiNrY; uiY++) {
		for (uiX = 0; uiX < uiNrX; uiX++, pImPointer += uiXSize) {
			pulHist = &pulMapArray[uiNrBins * (uiY * uiNrX + uiX)];
			CLAHEMakeHistogram(pImPointer,uiXRes,uiXSize,uiYSize,pulHist,uiNrBins,aLUT);
			CLAHEClipHistogram(pulHist, uiNrBins, ulClipLimit);
			CLAHEMapHistogram(pulHist, Min, Max, uiNrBins, ulNrPixels);
		}
		pImPointer += (uiYSize - 1) * uiXRes;				/* skip lines, set pointer */
    }

    /* InterpPIMte greylevel mappings to get CLAHE image */
    for (pImPointer = pImage, uiY = 0; uiY <= uiNrY; uiY++) {
		if (uiY == 0) {										/* special case: top row */
			uiSubY = uiYSize >> 1;  uiYU = 0; uiYB = 0;
		}
		else {
			if (uiY == uiNrY) {								/* special case: bottom row */
				uiSubY = uiYSize >> 1;	uiYU = uiNrY-1;	 uiYB = uiYU;
			}
			else {											/* default values */
				uiSubY = uiYSize; uiYU = uiY - 1; uiYB = uiYU + 1;
			}
		}
		for (uiX = 0; uiX <= uiNrX; uiX++) {
			if (uiX == 0) {									/* special case: left column */
				uiSubX = uiXSize >> 1; uiXL = 0; uiXR = 0;
			}
			else {
				if (uiX == uiNrX) {							/* special case: right column */
					uiSubX = uiXSize >> 1;  uiXL = uiNrX - 1; uiXR = uiXL;
				}
				else {										/* default values */
					uiSubX = uiXSize; uiXL = uiX - 1; uiXR = uiXL + 1;
				}
			}
			
			pulLU = &pulMapArray[uiNrBins * (uiYU * uiNrX + uiXL)];
			pulRU = &pulMapArray[uiNrBins * (uiYU * uiNrX + uiXR)];
			pulLB = &pulMapArray[uiNrBins * (uiYB * uiNrX + uiXL)];
			pulRB = &pulMapArray[uiNrBins * (uiYB * uiNrX + uiXR)];
			CLAHEInterpPIMte(pImPointer,uiXRes,pulLU,pulRU,pulLB,pulRB,uiSubX,uiSubY,aLUT);
			pImPointer += uiSubX;							/* set pointer on next matrix */
		}
		pImPointer += (uiSubY - 1) * uiXRes;
    }
		
	FREE(pulMapArray);										/* free space for histograms */
    return 0;												/* return status OK */
}

void CImageProcess::CLAHEClipHistogram(unsigned long* pulHistogram, unsigned int uiNrGreylevels, unsigned long ulClipLimit)
/* This function performs clipping of the histogram and redistribution of bins.
 * The histogram is clipped and the number of excess pixels is counted. Afterwards
 * the excess pixels are equally redistributed across the whole histogram (providing
 * the bin count is smaller than the cliplimit).
 */
{
    unsigned long* pulBinPointer=NULL, *pulEndPointer=NULL, *pulHisto=NULL;
    unsigned long ulNrExcess=0UL, ulUpper=0UL, ulBinIncr=0UL, ulStepSize=0UL, i=0UL;
    long lBinExcess=0L;

    ulNrExcess = 0;  pulBinPointer = pulHistogram;
    for (i = 0; i < uiNrGreylevels; i++) {					/* calculate total number of excess pixels */
		lBinExcess = (long) pulBinPointer[i] - (long) ulClipLimit;
		if (lBinExcess > 0) ulNrExcess += lBinExcess;		/* excess in current bin */
    }

    /* Second part: clip histogram and redistribute excess pixels in each bin */
    ulBinIncr = ulNrExcess / uiNrGreylevels;				/* average binincrement */
    ulUpper =  ulClipLimit - ulBinIncr;						/* Bins larger than ulUpper set to cliplimit */
    for (i = 0; i < uiNrGreylevels; i++) {
		if (pulHistogram[i] > ulClipLimit) pulHistogram[i] = ulClipLimit; /* clip bin */
		else {
			if (pulHistogram[i] > ulUpper) {				/* high bin count */
				ulNrExcess -= pulHistogram[i] - ulUpper; pulHistogram[i]=ulClipLimit;
			}
			else {											/* low bin count */
				ulNrExcess -= ulBinIncr; pulHistogram[i] += ulBinIncr;
			}
		}
    }

    while (ulNrExcess) {									/* Redistribute remaining excess  */
		pulEndPointer = &pulHistogram[uiNrGreylevels]; pulHisto = pulHistogram;
		
		while (ulNrExcess && pulHisto < pulEndPointer) {
			ulStepSize = uiNrGreylevels / ulNrExcess;
			if (ulStepSize < 1) ulStepSize = 1;				/* stepsize at least 1 */
			for (pulBinPointer=pulHisto; pulBinPointer < pulEndPointer && ulNrExcess; pulBinPointer += ulStepSize) {
				if (*pulBinPointer < ulClipLimit) {
					(*pulBinPointer)++;	 ulNrExcess--;		/* reduce excess */
				}
			}
			pulHisto++;										/* restart redistributing on other bin location */
		}
    }
}

void CImageProcess::CLAHEMakeHistogram(unsigned char* pImage, unsigned int uiXRes,
		unsigned int uiSizeX, unsigned int uiSizeY, 
		unsigned long* pulHistogram, unsigned int uiNrGreylevels, unsigned char* pLookupTable)
/* This function classifies the greylevels present in the array image into
 * a greylevel histogram. The pLookupTable specifies the relationship
 * between the greyvalue of the pixel (typically between 0 and 4095) and
 * the corresponding bin in the histogram (usually containing only 128 bins).
 */
{
    unsigned char* pImagePointer=NULL;
    unsigned int i=0;

    for (i = 0; i < uiNrGreylevels; i++) pulHistogram[i] = 0L; /* clear histogram */
    for (i = 0; i < uiSizeY; i++) {
		pImagePointer = &pImage[uiSizeX];
		while (pImage < pImagePointer) pulHistogram[pLookupTable[*pImage++]]++;
		pImagePointer += uiXRes;
		pImage = &pImagePointer[uiSizeX*-1];
    }
}

void CImageProcess::CLAHEMapHistogram(unsigned long* pulHistogram, unsigned char Min, unsigned char Max,
	    unsigned int uiNrGreylevels, unsigned long ulNrOfPixels)
/* This function calculates the equalized lookup table (mapping) by
 * cumulating the input histogram. Note: lookup table is rescaled in range [Min..Max].
 */
{
    unsigned int i;  unsigned long ulSum = 0;
    const float fScale = ((float)(Max - Min)) / ulNrOfPixels;
    const unsigned long ulMin = (unsigned long) Min;

    for (i = 0; i < uiNrGreylevels; i++) {
		ulSum += pulHistogram[i]; pulHistogram[i]=(unsigned long)(ulMin+ulSum*fScale);
		if (pulHistogram[i] > Max) pulHistogram[i] = Max;
    }
}

void CImageProcess::CLAHEMakeLut(unsigned char* pLUT, unsigned char Min, unsigned char Max, unsigned int uiNrBins)
/* To speed up histogram clipping, the input image [Min,Max] is scaled down to
 * [0,uiNrBins-1]. This function calculates the LUT.
 */
{
    int i;
    const unsigned char BinSize = (unsigned char) (1 + (Max - Min) / uiNrBins);
    for (i = Min; i <= Max; i++)  pLUT[i] = (i - Min) / BinSize;
}

void CImageProcess::CLAHEInterpPIMte(unsigned char* pImage, int uiXRes, unsigned long* pulMapLU,
		unsigned long* pulMapRU, unsigned long* pulMapLB,  unsigned long* pulMapRB,
		unsigned int uiXSize, unsigned int uiYSize, unsigned char* pLUT)
/* pImage      - pointer to input/output image
 * uiXRes      - resolution of image in x-direction
 * pulMap*     - mappings of greylevels from histograms
 * uiXSize     - uiXSize of image submatrix
 * uiYSize     - uiYSize of image submatrix
 * pLUT	       - lookup table containing mapping greyvalues to bins
 * This function calculates the new greylevel assignments of pixels within a submatrix
 * of the image with size uiXSize and uiYSize. This is done by a bilinear interpPIMtion
 * between four different mappings in order to eliminate boundary artifacts.
 * It uses a division; since division is often an expensive operation, I added code to
 * perform a logical shift instead when feasible.
 */
{
    const unsigned int uiIncr = uiXRes-uiXSize;						/* Pointer increment after processing row */
    unsigned char GreyValue; unsigned int uiNum = uiXSize*uiYSize;	/* Normalization factor */
    unsigned int uiXCoef, uiYCoef, uiXInvCoef, uiYInvCoef, uiShift = 0;

    if (uiNum & (uiNum - 1)) {										/* If uiNum is not a power of two, use division */
		for (uiYCoef = 0, uiYInvCoef = uiYSize; uiYCoef < uiYSize; uiYCoef++, uiYInvCoef--,pImage+=uiIncr) {
			for (uiXCoef = 0, uiXInvCoef = uiXSize; uiXCoef < uiXSize; uiXCoef++, uiXInvCoef--) {
				GreyValue = pLUT[*pImage];							/* get histogram bin value */
				*pImage++ = (unsigned char) ((uiYInvCoef * (uiXInvCoef*pulMapLU[GreyValue]
					+ uiXCoef * pulMapRU[GreyValue])
					+ uiYCoef * (uiXInvCoef * pulMapLB[GreyValue] + uiXCoef * pulMapRB[GreyValue])) / uiNum);
			}
		}
	}
    else {															/* avoid the division and use a right shift instead */
		while (uiNum >>= 1) uiShift++;								/* Calculate 2log of uiNum */
		for (uiYCoef = 0, uiYInvCoef = uiYSize; uiYCoef < uiYSize; uiYCoef++, uiYInvCoef--,pImage+=uiIncr) {
			for(uiXCoef = 0, uiXInvCoef = uiXSize; uiXCoef < uiXSize; uiXCoef++, uiXInvCoef--) {
				GreyValue = pLUT[*pImage];							/* get histogram bin value */
				*pImage++ = (unsigned char)((uiYInvCoef* (uiXInvCoef * pulMapLU[GreyValue]
					+ uiXCoef * pulMapRU[GreyValue])
					+ uiYCoef * (uiXInvCoef * pulMapLB[GreyValue] + uiXCoef * pulMapRB[GreyValue])) >> uiShift);
			}
		}
    }
}

bool CImageProcess::threshold_binary(CvImage& src_image, CvImage& des_image, int th, int max_val, int inv_mode)
{
	if(src_image.GetImage() == NULL) return false;
	destroy(des_image);
	create(des_image, src_image.Width(), src_image.Height(), src_image.GetImage()->nChannels);
	if(inv_mode == 0) cvThreshold(src_image.GetImage(), des_image.GetImage(), (float)th, (float)max_val, CV_THRESH_BINARY);
	else cvThreshold(src_image.GetImage(), des_image.GetImage(), (float)th, (float)max_val, CV_THRESH_BINARY_INV);
	return true;
}

bool CImageProcess::threshold_tozero(CvImage& src_image, CvImage& des_image, int th, int max_val, int inv_mode)
{
	if(src_image.GetImage() == NULL) return false;
	destroy(des_image);
	create(des_image, src_image.Width(), src_image.Height(), src_image.GetImage()->nChannels);
	if(inv_mode == 0) cvThreshold(src_image.GetImage(), des_image.GetImage(), (float)th, (float)max_val, CV_THRESH_TOZERO);
	else cvThreshold(src_image.GetImage(), des_image.GetImage(), (float)th, (float)max_val, CV_THRESH_TOZERO_INV);
	return true;
}

bool CImageProcess::threshold_trunc(CvImage& src_image, CvImage& des_image, int th, int max_val)
{
	if(src_image.GetImage() == NULL) return false;
	destroy(des_image);
	create(des_image, src_image.Width(), src_image.Height(), src_image.GetImage()->nChannels);
	cvThreshold(src_image.GetImage(), des_image.GetImage(), (float)th, (float)max_val, CV_THRESH_TRUNC);	
	return true;
}

bool CImageProcess::erode(CvImage& src_image, CvImage& des_image, int iter)
{
	if(src_image.GetImage() == NULL) return false;
	destroy(des_image);
	create(des_image, src_image.Width(), src_image.Height(), src_image.GetImage()->nChannels);
	cvErode(src_image.GetImage(), des_image.GetImage(), 0, iter);
	return true;
}

bool CImageProcess::dilate(CvImage& src_image, CvImage& des_image, int iter)
{
	if(src_image.GetImage() == NULL) return false;
	destroy(des_image);
	create(des_image, src_image.Width(), src_image.Height(), src_image.GetImage()->nChannels);
	cvDilate(src_image.GetImage(), des_image.GetImage(), 0, iter);
	return true;
}

int CImageProcess::connected_component(CvImage& srCvImage, MyRect** rc)
{
	if(srCvImage.GetImage() == NULL) return 0;
	int w, h, c;
	size(srCvImage, w, h, c);
	if(c != 1) return 0;

	CvImage cCvImage;
	clone(srCvImage, cCvImage);

	CvMemStorage* storage = cvCreateMemStorage(0);
    CvSeq *contour1 = NULL, *contour2 = NULL;      
	cvFindContours(cCvImage.GetImage(), storage, &contour1, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	
	int nRCCount = 0;	
	contour2 = contour1;	
    for(; contour2 !=0 ; contour2=contour2->h_next) nRCCount++;
	if(nRCCount == 0) {
		cvReleaseMemStorage(&storage);
		FREE(contour1);
		destroy(cCvImage);
		return 0;
	}
	
	int i=0;
	CvRect rect;	
	(*rc) = (MyRect*)malloc(sizeof(MyRect)*nRCCount);	
	contour2 = contour1;
	for(; contour2!=0; contour2=contour2->h_next) {	
		rect = cvBoundingRect(contour2);
		(*rc+i)->left = rect.x;
		(*rc+i)->top = rect.y;
		(*rc+i)->right = rect.x + rect.width;
		(*rc+i)->bottom = rect.y + rect.height;	
		i++;
    }	
	
	cvReleaseMemStorage(&storage);
//	FREE(contour1);
	destroy(cCvImage);
	return nRCCount;
}

bool CImageProcess::affine_trans(unsigned char* src_data, int src_w, int src_h, MyPoint ptSrcLEye, MyPoint ptSrcREye,
								 unsigned char* des_data, int des_w, int des_h, MyPoint ptDesLEye, MyPoint ptDesREye,
								 double f1, double f2, double f3, double f4)
{
	int des_x, des_y;
	int des_x2=ptDesLEye.x, des_y2=ptDesLEye.y;
	int des_x3=ptDesREye.x, des_y3=ptDesREye.y;
	int src_size = src_w*src_h;

	int des_size = des_w*des_h;
	memset(des_data, 0, des_size);

	int x2 = ptSrcLEye.x, y2 = ptSrcLEye.y;
	int x3 = ptSrcREye.x, y3 = ptSrcREye.y;
	double dx1 = double(x3-x2)*des_x2/double(des_x3-des_x2);
	double dy1 = dx1*(y3-y2)/double(x3-x2);
	if(dy1 < 0) dy1 = -dy1;
	double dy2 = dx1*des_y2/(double)des_x2;
	double dx2 = dy1*dy2/dx1;

	int x1, y1;
	if(y2 > y3)	{
		x1 = (int)(x2 - dx1 - dx2 + 0.5);
		y1 = (int)(y2 + dy1 - dy2 + 0.5);
	}
	else {
		x1 = (int)(x2 - dx1 + dx2 + 0.5);
		y1 = (int)(y2 - dy1 - dy2 + 0.5);
	}
	
	double src_x, src_y;
	double a11 = f1*(x3-x2), a12 = f2*x1 + f3*x2 + f4*x3;
	double a21 = f1*(y3-y2), a22 = f2*y1 + f3*y2 + f4*y3;
	int l, k, src, src_l, src_k, src_lk, src_w_step, des_w_step = 0;
	double a, b;
	for(des_y=0; des_y<des_h; des_y++) {				
		for(des_x=0; des_x<des_w; des_x++) {
			src_x = a11*des_x + a12*des_y + x1;
			src_y = a21*des_x + a22*des_y + y1;

			if(src_x < 0) { src_x = 0; return false; }
			if(src_x >= src_w) { src_x = src_w-1; return false; }
			if(src_y < 0) { src_y = 0; return false; }
			if(src_y >= src_h) { src_y = src_h-1; return false; }

			l = (int)(src_x);
			k = (int)(src_y);
			a = src_x - l;
			b = src_y - k;
			src_w_step = k*src_w;
			src = src_data[src_w_step+l];
			src_l = src_data[src_w_step+l+1];
			src_k = src_data[src_w_step+src_w+l];
			src_lk = src_data[src_w_step+src_w+l+1];

			if(src_w_step+src_w+l+1 < src_size) {
				des_data[des_w_step+des_x] = (unsigned char)(src + a*(src_l-src) + b*(src_k-src) +
					a*b*(src+src_lk-src_l-src_k));
			}
		}
		des_w_step += des_w;
	}
	return true;
}

bool CImageProcess::affine_trans_fr_MMCT(unsigned char* src_data, int src_w, int src_h, unsigned char* des_data, int lx, int ly, int rx, int ry)
{
	int des_x, des_y;
	int des_w=40, des_h=58;
	int des_x2=9, des_y2=28;
	int des_x3=31, des_y3=des_y2;
	int src_size = src_w*src_h;

	int des_size = des_w*des_h;
	memset(des_data, 0, des_size);

	int x2 = lx, y2 = ly;
	int x3 = rx, y3 = ry;
	double dx1 = double(x3-x2)*des_x2/double(des_x3-des_x2);
	double dy1 = dx1*(y3-y2)/double(x3-x2);
	if(dy1 < 0) dy1 = -dy1;
	double dy2 = dx1*des_y2/(double)des_x2;
	double dx2 = dy1*dy2/dx1;

	int x1, y1;
	if(y2 > y3)	{
		x1 = (int)(x2 - dx1 - dx2 + 0.5);
		y1 = (int)(y2 + dy1 - dy2 + 0.5);
	}
	else {
		x1 = (int)(x2 - dx1 + dx2 + 0.5);
		y1 = (int)(y2 - dy1 - dy2 + 0.5);
	}
	
	double src_x, src_y;
	double a11 = 0.04545454545455*(x3-x2), a12 = -0.03571428571429*x1 + 0.05032467532468*x2 -0.01461038961039*x3;
	double a21 = 0.04545454545455*(y3-y2), a22 = -0.03571428571429*y1 + 0.05032467532468*y2 -0.01461038961039*y3;
	int l, k, src, src_l, src_k, src_lk, src_w_step, des_w_step = 0;
	double a, b;
	for(des_y=0; des_y<des_h; des_y++) {				
		for(des_x=0; des_x<des_w; des_x++) {
			src_x = a11*des_x + a12*des_y + x1;
			src_y = a21*des_x + a22*des_y + y1;

			if(src_x < 0) { src_x = 0; return false; }
			if(src_x >= src_w) { src_x = src_w-1; return false; }
			if(src_y < 0) { src_y = 0; return false; }
			if(src_y >= src_h) { src_y = src_h-1; return false; }

			l = (int)(src_x);
			k = (int)(src_y);
			a = src_x - l;
			b = src_y - k;
			src_w_step = k*src_w;
			src = src_data[src_w_step+l];
			src_l = src_data[src_w_step+l+1];
			src_k = src_data[src_w_step+src_w+l];
			src_lk = src_data[src_w_step+src_w+l+1];

			if(src_w_step+src_w+l+1 < src_size) {
				des_data[des_w_step+des_x] = (unsigned char)(src + a*(src_l-src) + b*(src_k-src) +
					a*b*(src+src_lk-src_l-src_k));
			}
		}
		des_w_step += des_w;
	}
	return true;
}

bool CImageProcess::affine_trans_fr_MMCT(CvImage& srCvImage, CvImage& faceImage, MyPoint ptLEye, MyPoint ptREye)
{
	int src_w, src_h, src_c;
	size(srCvImage, src_w, src_h, src_c);

	int des_w = 40, des_h = 58;
	unsigned char* src_data = (unsigned char*)malloc(src_w*src_h);	
	unsigned char* des_data = (unsigned char*)malloc(des_w*des_h);
	getraw(srCvImage, src_data);

	if(affine_trans_fr_MMCT(src_data, src_w, src_h, des_data, ptLEye.x, ptLEye.y, ptREye.x, ptREye.y) == false) {
		FREE(src_data);
		FREE(des_data);
		return false;
	}
	else {
		setraw(faceImage, des_w, des_h, 1, des_data);
		FREE(src_data);
		FREE(des_data);
		return true;
	}
}

bool CImageProcess::combine_image(CvImage& src_image1, CvImage& src_image2, int x, int y, CvImage& des_image)
{
	if(src_image1.GetImage() == NULL || src_image2.GetImage() == NULL) return false;
	int w1, h1, c1, w2, h2, c2;
	
	size(src_image1, w1, h1, c1);
	size(src_image2, w2, h2, c2);
	if(w1 < (x+w2) || h1 < (y+h2) || c1 != c2) return false;

	unsigned char* src_data1 = (unsigned char*)malloc(w1*h1*c1);
	unsigned char* src_data2 = (unsigned char*)malloc(w2*h2*c2);
	getraw(src_image1, src_data1);
	getraw(src_image2, src_data2);

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
	FREE(src_data1);
	FREE(src_data2);
	return true;
}

bool CImageProcess::affine_trans_fr(CvImage& src_image, CvImage& des_image, MyPoint ptLeftEye, MyPoint ptRightEye)
{
	if(src_image.GetImage() == NULL) return false;
	int src_w, src_h, c;
	int des_x, des_y;
	int des_w=40, des_h=50;
	int des_x2=10, des_y2=15;
	int des_x3=30, des_y3=des_y2;

	size(src_image, src_w, src_h, c);
	if(src_w < des_w || src_h < des_h || c != 1) return false;
	int src_size = src_w*src_h;
	int des_size = des_w*des_h;

	unsigned char* src_data = (unsigned char*)malloc(src_size);	
	unsigned char* des_data = (unsigned char*)malloc(des_size);
	memset(des_data, 0, des_size);
	if(getraw(src_image, src_data) == false) {
		free(src_data); free(des_data); return false;
	}

	int x2 = ptLeftEye.x,  y2 = ptLeftEye.y;
	int x3 = ptRightEye.x, y3 = ptRightEye.y;
	double dx1 = double(x3-x2)*des_x2/double(des_x3-des_x2);
	double dy1 = dx1*(y3-y2)/double(x3-x2);
	if(dy1 < 0) dy1 = -dy1;
	double dy2 = dx1*des_y2/(double)des_x2;
	double dx2 = dy1*dy2/dx1;

	int x1, y1;
	if(y2 > y3)	{
		x1 = (int)(x2 - dx1 - dx2 + 0.5);
		y1 = (int)(y2 + dy1 - dy2 + 0.5);
	}
	else {
		x1 = (int)(x2 - dx1 + dx2 + 0.5);
		y1 = (int)(y2 - dy1 - dy2 + 0.5);
	}
	
	double src_x, src_y;
	double a11 = 0.05*(x3-x2), a12 = -0.0667*x1 + 0.1*x2 -0.0333*x3;
	double a21 = 0.05*(y3-y2), a22 = -0.0667*y1 + 0.1*y2 -0.0333*y3;
	int l, k, src, src_l, src_k, src_lk, src_w_step, des_w_step = 0;
	double a, b;
	for(des_y=0; des_y<des_h; des_y++) {				
		for(des_x=0; des_x<des_w; des_x++) {
			src_x = a11*des_x + a12*des_y + x1;
			src_y = a21*des_x + a22*des_y + y1;

			if(src_x < 0) src_x = 0;
			if(src_x >= src_w) src_x = src_w-1;
			if(src_y < 0) src_y = 0;
			if(src_y >= src_h) src_y = src_h-1;

			l = (int)(src_x);
			k = (int)(src_y);
			a = src_x - l;
			b = src_y - k;
			src_w_step = k*src_w;
			src = src_data[src_w_step+l];
			src_l = src_data[src_w_step+l+1];
			src_k = src_data[src_w_step+src_w+l];
			src_lk = src_data[src_w_step+src_w+l+1];

			if(src_w_step+src_w+l+1 < src_size) {
				des_data[des_w_step+des_x] = (unsigned char)(src + a*(src_l-src) + b*(src_k-src) +
					a*b*(src+src_lk-src_l-src_k));
			}
		}
		des_w_step += des_w;
	}

	setraw(des_image, des_w, des_h, 1, des_data);
	free(src_data);
	free(des_data);
	return true;
}

void CImageProcess::PrewittGradientAngle(unsigned char* src_image, int w, int h, unsigned char* des_image)
{ 
	int i, j, size = w*h;
	double* PImage = (double*)malloc(sizeof(double)*size);
	double* QImage = (double*)malloc(sizeof(double)*size);
	memset(PImage, 0, sizeof(double)*(size));
	memset(QImage, 0, sizeof(double)*(size));

	//prewitt operator	
	int w_step, w_pre_step, w_next_step;
	for(i = 1; i<h-1; i++) {
		w_pre_step = (i-1)*w;	w_step = w_pre_step + w;	w_next_step = w_step + w;
		for(j = 1; j<w-1; j++) {
			PImage[w_step + j] = (double)((-1)*src_image[w_pre_step+ (j-1)] +   src_image[w_pre_step + (j+1)] + 
						  	 (-1)*src_image[w_step + (j-1)] +   src_image[w_step + (j+1)] +
							 (-1)*src_image[w_next_step + (j-1)] +   src_image[w_next_step + (j+1)]);			
		}
	}

	for( i = 1; i < h-1; i++) {
		w_pre_step = (i-1)*w;	w_step = w_pre_step + w;	w_next_step = w_step + w;
		for( j = 1; j < w-1; j++) {
			QImage[w_step+j] = (double)(src_image[w_pre_step + j-1] + src_image[w_pre_step + j] + src_image[w_pre_step + j+1] +
				        (-1)*src_image[w_next_step + j-1] + (-1)*src_image[w_next_step + j] + (-1)*src_image[w_next_step + j+1]);			 
		}
	}
	
	for (i = 0 ; i < w ; i++) {
		PImage[(h - 1)*w+i] = src_image[(h - 1)*w+i];
    	QImage[(h - 1)*w+i] = src_image[(h - 1)*w+i];
	}

	for (i = 0 ; i < h ; i++) {
		PImage[i*w + (w-1)] = src_image[i*w + (w-1)];
		QImage[i*w + (w-1)] = src_image[i*w + (w-1)];
	}

	/////////////////converting 0-360 angle to 0-255 angle//////////////////	
	double angle;
	for(i=0; i<size ; i++)	{
		angle = 180.0+(double)atan2(QImage[i], PImage[i])*360.0/(2.0*CV_PI);	
		angle =(255.0*angle/360.0);
		if(angle < 0) angle = 0; 
		des_image[i] = (unsigned char)angle;
	}

	FREE(PImage);
	FREE(QImage);
}