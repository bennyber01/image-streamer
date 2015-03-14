#pragma once

#include <gst/gst.h>
#include <gst/app/gstappsink.h>

class  ReadWithGstreamer
{
public:
	ReadWithGstreamer();
	~ReadWithGstreamer();
	int InitAndBuildPipline(char *url, int bufferSize);
	
	void StartReadingStream();
	void GetImageData(bool &haveData, unsigned char** buffer, int &width, int &height, int &bpp);
	bool DoesHaveImageInfo() { return m_bHaveImageData; }

//private:
	GstElement *pipeline;

	// image info
	int m_iWidth;
	int m_iHeight;
	int m_iBPP;

	// image data
	unsigned char* m_cImageBuffer;
	int            m_iImageBufferSize;

	// shows if new frame arrived
	bool m_bHaveImageData;

public:
	void CopyDataToClass(unsigned char *buffer, int width, int height, int bpp);
};

