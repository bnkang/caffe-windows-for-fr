#ifndef _CAMEAR_H_
#define _CAMEAR_H_

#include "CvImage.h"
#include "opencv2/videoio.hpp"


class CCamera  
{ 
public:    
    CCamera();
	virtual ~CCamera();

	bool  Initialize( int ww, int hh, int wIndex );
	void  Uninitialize();
	bool  IsInitialized();
	void  GetFrame(CvImage &img);
	bool  IsRunning() { return m_isRunning; };
	void  Start();
	void  Stop();
        
protected:
	//CvCapture *capture;
	cv::VideoCapture capture;
	IplImage *image;
	bool	m_isRunning;
};

#endif