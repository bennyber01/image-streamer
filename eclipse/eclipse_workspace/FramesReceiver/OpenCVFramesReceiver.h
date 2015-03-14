/*
 * FramesReceiver.h
 *
 *  Created on: 28 בספט 2014
 *      Author: benny
 */

#ifndef OPENCV_FRAMES_RECEIVER_H_
#define OPENCV_FRAMES_RECEIVER_H_

#include "FramesReceiverInterface.h"
#include <opencv2/core/types_c.h>

struct CvCapture;

class OpenCVFramesReceiver: public FramesReceiverInterface
{
public:
	OpenCVFramesReceiver();
	~OpenCVFramesReceiver();

	bool IsFrameFready();
	unsigned char * GetFrameData();

	bool LoadVideo(char * URL);
	bool LoadCamera(int source);

	bool GetFrameInfo(int & frameWidth, int & frameHeight, int & frameBPP);

private:
	CvCapture * _captureSource;
	IplImage * _image;
};

#endif /* OPENCV_FRAMES_RECEIVER_H_ */
