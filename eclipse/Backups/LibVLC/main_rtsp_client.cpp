#include <stdio.h>
#include <stdlib.h>
#include <vlc/vlc.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>  // cvWaitKey
#include <string>
#include <algorithm>

#include <windows.h>		// Sleep

struct MyImemData
{
    MyImemData()
	{
		image = NULL;
		buffSize = 0;
	}

    ~MyImemData()
	{
		cvReleaseImage(&image);
		buffSize = 0;
	}
	
	bool InitImage(int imageWidth, int imageHeight, int imageBPP)
	{
		image = cvCreateImage(cvSize(imageWidth, imageHeight), IPL_DEPTH_8U, imageBPP / 8);

		if (image)
			buffSize = sizeof(char) * imageWidth * imageHeight * imageBPP / 8;
		else
			buffSize = 0;

		return image != NULL;
	}

	IplImage * image;
	int buffSize;
};

MyImemData myFrame;

void* lock(void* data, void** p_pixels)
{
	*p_pixels = myFrame.image->imageData;

	return NULL;
}

void unlock(void* data, void* id, void* const* p_pixels)
{
	unsigned char* pixels = (unsigned char*)*p_pixels;

	cvShowImage("Display window", myFrame.image);
	cvWaitKey(1);
}

unsigned setup(void **opaque, char *chroma, unsigned *width, unsigned *height, unsigned *pitches, unsigned *lines)
{
	if (!myFrame.InitImage(*width, *height, 24))
		return 0;

	*pitches = (*width) * 3;
	*lines = (*height);
	chroma[0] = 'R';
	chroma[1] = 'V';
	chroma[2] = '2';
	chroma[3] = '4';
	chroma[4] = 0;

	return 1;
}

int main(int argc, char* argv[])
{
	libvlc_instance_t * inst;
	libvlc_media_player_t *mp;
	libvlc_media_t *m;

	/* Load the VLC engine */
	inst = libvlc_new (0, NULL);
	
	// Samples in the wild
	// http://wiki.multimedia.cx/index.php?title=RTSP

	//char * URL = "rtsp://184.72.239.149/vod/mp4:BigBuckBunny_175k.mov";		// 240x160

	//char * URL = "rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mov";
	//char * URL = "rtsp://127.0.0.1:5541/test";

	//char * URL = "rtsp://46.249.213.87/broadcast/bollywoodhungama-tablet.3gp";					// 480x360
	char * URL = "rtsp://videocdn-us.geocdn.scaleengine.net/jblive/live/jblive.stream";		//1280x720

	//char * URL = "rtsp://localhost:554";

	//char * URL = "rtsp://admin:9999@10.200.1.222:554/live/stream1";

	/* Create a new item */
	m = libvlc_media_new_location (inst, URL);

	/* Create a media player playing environement */
	mp = libvlc_media_player_new_from_media (m);

    libvlc_video_set_format_callbacks(mp, setup, NULL);

	libvlc_video_set_callbacks(mp, lock, unlock, NULL, NULL);

	/* No need to keep the media now */
	libvlc_media_release (m);

#if 0
	/* This is a non working code that show how to hooks into a window,
	* if we have a window around */
	libvlc_media_player_set_xwindow (mp, xid);
	/* or on windows */
	libvlc_media_player_set_hwnd (mp, hwnd);
	/* or on mac os */
	libvlc_media_player_set_nsobject (mp, view);
#endif

	/* play the media_player */
	libvlc_media_player_play (mp);
	
	Sleep (30000); /* Let it play a bit */

	/* Stop playing */
	libvlc_media_player_stop (mp);

	/* Free the media_player */
	libvlc_media_player_release (mp);

	libvlc_release (inst);

	return 0;
}