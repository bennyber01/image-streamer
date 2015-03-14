#ifndef GSTREAMER_FRAMES_RECEIVER_H_
#define GSTREAMER_FRAMES_RECEIVER_H_

#include "FramesReceiverInterface.h"

#include <gst/gst.h>
#include <gst/app/gstappsink.h>

class GStreamerFramesReceiver : public FramesReceiverInterface
{
public:
	GStreamerFramesReceiver(void);
	~GStreamerFramesReceiver(void);

	bool IsFrameFready() { return isNewFrameReady; }
	unsigned char * GetFrameData();

	bool LoadVideo(char * URL);

	bool GetFrameInfo(int & frameWidth, int & frameHeight, int & frameBPP);

	int & FrameWidth()  { return frameWidth;  }
	int & FrameHeight() { return frameHeight; }
	int & FrameBPP()    { return frameBPP;    }

	bool CopyFrameData(unsigned char * data, int size);

private:
	GstElement * pipeline;
	GThread * mainLoopThread;
	GMainLoop * mainLoop;
	GstElement * sink;

	unsigned char * buff;
	int buffSize;
	int frameWidth;
	int frameHeight;
	int frameBPP;

	bool isNewFrameReady;

	static gpointer MainLoopThreadFunction(gpointer data);
};

#endif /* GSTREAMER_FRAMES_RECEIVER_H_ */
