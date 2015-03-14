/*
 * FramesTransmitter.h
 *
 *  Created on: 28 בספט 2014
 *      Author: benny
 */

#ifndef FRAMESTRANSMITTER_H_
#define FRAMESTRANSMITTER_H_

#include "FramesTransmitterInterface.h"
#include "FramesBuffer.h"

#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

class FramesTransmitter: public FramesTransmitterInterface
{
public:
	FramesTransmitter();
	~FramesTransmitter();

	bool Init(char * url, char * port, int frameWidth, int frameHeight, int frameBPP);
	bool Transmit(unsigned char * buff);

	GMainLoop * mainLoop;

	GThread * mainLoopThread;

	FramesBuffer framesBuffer;

	GstRTSPMediaFactory * factory;
	GstRTSPServer * server;
	GstBuffer * buffer;
};

#endif /* FRAMESTRANSMITTER_H_ */
