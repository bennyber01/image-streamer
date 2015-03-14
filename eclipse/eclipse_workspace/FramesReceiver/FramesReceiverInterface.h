/*
 * FramesReceiverInterface.h
 *
 *  Created on: 28 בספט 2014
 *      Author: benny
 */

#ifndef FRAMESRECEIVERINTERFACE_H_
#define FRAMESRECEIVERINTERFACE_H_

#include <stddef.h>

class FramesReceiverInterface
{
public:
	virtual ~FramesReceiverInterface() {}
	virtual bool LoadVideo(char * URL) = 0;
	virtual bool IsFrameFready() = 0;
	virtual unsigned char * GetFrameData() = 0;
	virtual bool GetFrameInfo(int & frameWidth, int & frameHeight, int & frameBPP) = 0;
};

#endif /* FRAMESRECEIVERINTERFACE_H_ */
