#include <stdio.h>
#include <gst/gst.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>  // cvWaitKey

#include "ReadWithGstreamer.h"

void PrintVersion()
{
	const gchar *nano_str;
	guint major, minor, micro, nano;

	gst_version (&major, &minor, &micro, &nano);
	
	if (nano == 1)
		nano_str = "(CVS)";
	else if (nano == 2)
		nano_str = "(Prerelease)";
	else
		nano_str = "";
	
	printf ("This program is linked against GStreamer %d.%d.%d %s\n", major, minor, micro, nano_str);
}

int main (int argc, char *argv[])
{
	gst_init (&argc, &argv);

	gboolean isInit = gst_is_initialized();

	PrintVersion();

	ReadWithGstreamer readWithGstreamer;

	readWithGstreamer.InitAndBuildPipline("rtsp://184.72.239.149/vod/mp4:BigBuckBunny_175k.mov", 1000);

	readWithGstreamer.StartReadingStream();

	//IplImage * image = cvLoadImage("d:/dalmation.bmp");

	bool haveData;
	unsigned char* buffer = NULL;
	int width;
	int height;
	int bpp;

	while(cvWaitKey(10) != 113)	// press 'q'
	{
		readWithGstreamer.GetImageData(haveData, &buffer, width, height, bpp);

		if (haveData)
		{
			CvSize size = cvSize(width, height);
			int depth = IPL_DEPTH_8U;
			int channels = bpp / 8;
			IplImage * image = cvCreateImage(size, depth, channels);

			memcpy(image -> imageData, buffer, width * height * channels);

			cvShowImage("Display window", image);
			
			cvReleaseImage(&image);
		}
	}

	cvDestroyWindow("Display window");
	//cvReleaseImage(&image);
	
	return 0;
}