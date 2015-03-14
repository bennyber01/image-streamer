#include "FramesBuffer.h"

#include <algorithm>		// std::copy

bool FramesBuffer::Init(int frameWidth, int frameHeight, int frameBPP)
{
	unsigned size = frameWidth * frameHeight * frameBPP / 8;

	if (!frames[0].Init(size))
		return false;

	if (!frames[1].Init(size))
		return false;

	if (!popFrame.Init(size))
		return false;

	this -> frameWidth  = frameWidth;
	this -> frameHeight = frameHeight;
	this -> frameBPP    = frameBPP;

	popPlace = 0;

	return true;
}

bool FramesBuffer::Push(unsigned char * data)
{
	if (updateFramesMode == UFM__READY_TO_SWAP_BUFFERS)
		popPlace = !popPlace;

	int pushPlace = !popPlace;
	FrameData & currFrame = frames[pushPlace];
    std::copy(data, data + currFrame.size, currFrame.buff);
	updateFramesMode = UFM__NEED_TO_UPDATE;

	++inputFramesCounter;

	return true;
}

unsigned char* FramesBuffer::Pop(unsigned int & size, int * passedTime, int * framesCounter)
{
	if (updateFramesMode == UFM__NEED_TO_UPDATE)
	{
		FrameData & currFrame = frames[popPlace];
		std::copy(currFrame.buff, currFrame.buff + currFrame.size, popFrame.buff);
		updateFramesMode = UFM__READY_TO_SWAP_BUFFERS;
	}

	time_t currTime;
	time(&currTime);	// The value returned represents the number of seconds since 00:00 hours, Jan 1, 1970 UTC
	if (prevTime == 0)
		prevTime = currTime;

	timeDelta = currTime - prevTime;
	if (timeDelta)
	{
		framesDelta = inputFramesCounter;
		inputFramesCounter = 0;
		prevTime = currTime;
	}
	else if (!timeDelta || !framesDelta)
		timeDelta = framesDelta = 1;
	
	if (passedTime) *passedTime = (int) timeDelta;
	if (framesCounter) *framesCounter = framesDelta;

	size = popFrame.size;
	return popFrame.buff;
}