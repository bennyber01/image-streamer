/*
 * main.cpp
 *
 *  Created on: 28 בספט 2014
 *      Author: benny
 */

#include "FramesTransmitter.h"
#include "ImageGenerator.h"
#include <string>
#include <algorithm>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>  // cvWaitKey

int main()
{
	FramesTransmitter framesTransmitter;
	ImageGenerator imageGenerator;

	int imageWidth, imageHeight, imageBPP;

	imageGenerator.GetImageParams(imageWidth, imageHeight, imageBPP);

	framesTransmitter.Init("127.0.0.1", "5541", imageWidth, imageHeight, imageBPP);

	//for (int i = 0; i < 100; ++i)
	//{
	//	unsigned char * buff = imageGenerator.GenerateImage();

	//	framesTransmitter.Transmit(buff);

	//	// save image on HDD
	//	bitmap_image image(imageWidth, imageHeight);
	//	int sizeToCopy = sizeof(char) * imageWidth * imageHeight * imageBPP / 8;
	//	std::copy(buff, buff + sizeToCopy, image.data());
	//	char filePath[200];
	//	sprintf(filePath, "D:/eclipse_workspace/FramesTransmitter/FramesToSend/img%04d.bmp", i);
	//	image.save_image(filePath);
	//}

	int frameCounter = 0;

	int channels = imageBPP / 8;
	IplImage * image = cvCreateImage(cvSize(imageWidth, imageHeight), IPL_DEPTH_8U, channels);
	
	while(cvWaitKey(40) != 113)	// press 'q'
	{
		unsigned char * buff = imageGenerator.GenerateImage();

		int sizeToCopy = sizeof(char) * imageWidth * imageHeight * channels;
		std::copy(buff, buff + sizeToCopy, image -> imageData);

		char str[100];   
		sprintf(str,"[%04d]", frameCounter++ );
		CvFont font;
		cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 1, 1, 0, 2, 8);
		cvPutText(image, str, cvPoint(10,40), &font, cvScalar(255,255,255)); 

		cvShowImage("Display window", image);

		framesTransmitter.Transmit((unsigned char*) image -> imageData);
	}
	
	cvDestroyWindow("Display window");
	cvReleaseImage(&image);

	return 0;
}


