#ifndef GSTREAMER_FRAMES_RECEIVER_H_
#define GSTREAMER_FRAMES_RECEIVER_H_

#include "FramesReceiverInterface.h"

#include <opencv2/opencv.hpp>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>

enum PixelFormat
{
	PF__RGB,
	PF__BGR,
	PF__I420,
	PF__UNKNOWN
};

class GStreamerFramesReceiver : public FramesReceiverInterface
{
public:
	GStreamerFramesReceiver(void);
	~GStreamerFramesReceiver(void);

	bool IsFrameFready() { return isNewFrameReady; }
	unsigned char * GetFrameData();

	bool LoadVideo(char * URL);

	bool GetFrameInfo(int & frameWidth, int & frameHeight, int & frameBPP);

	int & InputFrameWidth()          { return inpFrameWidth;  }
	int & InputFrameHeight()         { return inpFrameHeight; }
	PixelFormat & InputPixelFormat() { return inpPixelFormat; }

	bool CopyFrameData(unsigned char * data, int size);

private:
	GstElement * pipeline;
	GThread * mainLoopThread;
	GMainLoop * mainLoop;
	GstElement * sink;

	int inpFrameWidth;
	int inpFrameHeight;
	PixelFormat inpPixelFormat;

	IplImage * imageFromStream;
	IplImage * imageRGB;

	bool isNewFrameReady;

	static gpointer MainLoopThreadFunction(gpointer data);
};

#endif /* GSTREAMER_FRAMES_RECEIVER_H_ */
