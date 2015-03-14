/*
 * main.cpp
 *
 *  Created on: 28 áñôè 2014
 *      Author: benny
 */

#include "OpenCVFramesReceiver.h"
#include "GStreamerFramesReceiver.h"
#include "bitmap_image.hpp"
#include <stdio.h>

#include <opencv2/highgui/highgui.hpp>  // cvWaitKey

int main()
{
	// Samples in the wild
	// http://wiki.multimedia.cx/index.php?title=RTSP

	//char * URL = "rtsp://184.72.239.149/vod/mp4:BigBuckBunny_175k.mov";

	//char * URL = "http://www.wowza.com/_h264/BigBuckBunny_115k.mov";
	//char * URL = "rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mov";
	//char * URL = "rtsp://127.0.0.1:5541/test";

	//char * URL = "rtsp://46.249.213.87/broadcast/bollywoodhungama-tablet.3gp";
	//char * URL = "rtsp://218.248.64.82.554:/";
	char * URL = "rtsp://videocdn-us.geocdn.scaleengine.net/jblive/live/jblive.stream";
	
	//FramesReceiverInterface * framesReceiver = new OpenCVFramesReceiver();
	FramesReceiverInterface * framesReceiver = new GStreamerFramesReceiver();

	int imageWidth;
	int imageHeight;
	int imageBPP;

	if (!framesReceiver -> LoadVideo(URL))
		return 1;

	framesReceiver -> GetFrameInfo(imageWidth, imageHeight, imageBPP);

	unsigned char * buff = NULL;

	//while(cvWaitKey(10) != atoi("q"))
	//{
	//	while (framesReceiver -> IsFrameFready())
	//	{
	//		buff = framesReceiver -> ReceiveFrame();

	//		if (1)
	//			framesReceiver.Show();
	//		else
	//		{
	//			// save image on HDD
	//			bitmap_image image(imageWidth, imageHeight);
	//			int sizeToCopy = sizeof(char) * imageWidth * imageHeight * imageBPP / 8;
	//			std::copy(buff, buff + sizeToCopy, image.data());
	//			char filePath[200];
	//			static int imageCounter = 0;
	//			_snprintf_s(filePath, sizeof(filePath), "D:/eclipse_workspace/FramesReceiver/ReceivedFrames/img%04d.bmp", imageCounter++);
	//			image.save_image(filePath);
	//		}
	//	}
	//}

	int channels = imageBPP / 8;
	IplImage * image = cvCreateImage(cvSize(imageWidth, imageHeight), IPL_DEPTH_8U, channels);
	
	while(cvWaitKey(10) != 113)	// press 'q'
	{
		if (!framesReceiver -> IsFrameFready())
		{
			//cvWaitKey(1);
			g_usleep(1);
			continue;
		}
		
		buff = framesReceiver -> GetFrameData();

		int sizeToCopy = sizeof(char) * imageWidth * imageHeight * channels;
		std::copy(buff, buff + sizeToCopy, image -> imageData);

		cvShowImage("Display window", image);
	}
	
	cvDestroyWindow("Display window");
	cvReleaseImage(&image);

	delete framesReceiver;

	return 0;
}


//void FramesReceiver::Show()
//{
////	void * hWnd = cvGetWindowHandle("Display window");
////	if (!hWnd)
////		cvNamedWindow("Display window", CV_WINDOW_AUTOSIZE | CV_GUI_NORMAL);// Create a window for display.
//
//	cvShowImage("Display window", _image);
//
//	cvWaitKey(1);	// needed to repaint the window
//}


//	cvDestroyWindow("Display window");


