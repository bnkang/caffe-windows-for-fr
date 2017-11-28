#include "stdafx.h"
#include "Camera.h"
#include "opencv2/core/mat.hpp"
CCamera::CCamera()
{
	//capture = new cv::VideoCapture();
	m_isRunning = true;
	image = NULL;
}

CCamera::~CCamera()
{
    Uninitialize();
}

// Initialize camera input
bool  CCamera::Initialize( int ww, int hh, int wIndex )
{
	capture.open(0);
	m_isRunning = true;
	if (!(capture.isOpened()))
	{
		return false;
	}

	capture.set(CV_CAP_PROP_FRAME_WIDTH, 640.0);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, 480.0);
	capture.set(CV_CAP_PROP_FPS, 60.0);
// 	capture = cvCaptureFromCAM(wIndex);
// 	if( capture == NULL )
// 		return false;
// 
// 	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, ww);
// 	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, hh);
// 	cvSetCaptureProperty(capture, CV_CAP_PROP_FPS, 60);

	return true;
}

bool CCamera::IsInitialized()
{
	if(capture.isOpened()) 
		return true;

	return false;
}

// Uninitialize camera input
void  CCamera::Uninitialize()
{
	if(!capture.isOpened()) 
		return;
	
// 	cvReleaseCapture(&capture);
// 	capture = NULL;

	capture.release();
	
}

void CCamera::GetFrame(CvImage &img)
{
	if(!capture.isOpened()) 
		return;

	cv::Mat temp;
	capture.read(temp);
	image = &IplImage(temp);

	//image = cvQueryFrame(capture);

	//printf("width = %d, height = %d\n", image->width, image->height);
	img.CopyOf(image);
	
	
}

// Start capture
void  CCamera::Start()
{
	m_isRunning = true;
}

// Stop capture
void  CCamera::Stop()
{
	m_isRunning = false;
}
