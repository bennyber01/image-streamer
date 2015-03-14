/*
 * FramesReceiver.cpp
 *
 *  Created on: 28 בספט 2014
 *      Author: benny
 */

#include "OpenCVFramesReceiver.h"

#include <opencv2/opencv.hpp>

OpenCVFramesReceiver::OpenCVFramesReceiver()
{
	_captureSource = NULL;
	_image = NULL;
}

OpenCVFramesReceiver::~OpenCVFramesReceiver()
{
	cvReleaseCapture(&_captureSource);
}


bool OpenCVFramesReceiver::LoadCamera(int source)
{
	_captureSource = cvCreateCameraCapture(source);
	return _captureSource != NULL;
}

bool OpenCVFramesReceiver::LoadVideo(char * URL)
{
	_captureSource = cvCaptureFromFile(URL);
	return _captureSource != NULL;
}

bool OpenCVFramesReceiver::GetFrameInfo(int & frameWidth, int & frameHeight, int & frameBPP)
{
	frameWidth = frameHeight = frameBPP = 0;

	if(_captureSource)
	{
		//imageWidth  = cvGetCaptureProperty(_captureSource, CV_CAP_PROP_FRAME_WIDTH);
		//imageHeight = cvGetCaptureProperty(_captureSource, CV_CAP_PROP_FRAME_HEIGHT);

		while (_image == NULL)
			_image = cvQueryFrame(_captureSource);

		frameWidth  = _image -> width;
		frameHeight = _image -> height;
		frameBPP    = _image -> nChannels * 8;
	}

	return frameWidth != 0 && frameHeight != 0 && frameBPP != 0;
}

bool OpenCVFramesReceiver::IsFrameFready()
{
	_image = cvQueryFrame(_captureSource);
	return _image != NULL;
}

unsigned char * OpenCVFramesReceiver::GetFrameData()
{
	if (_image)
		return (unsigned char*) _image -> imageData;
	return NULL;
}
