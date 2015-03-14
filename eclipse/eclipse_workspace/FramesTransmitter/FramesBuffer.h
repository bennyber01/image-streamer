#pragma once

#include <time.h>			// time

struct FrameData
{
	FrameData() : buff(0) { Reset(); }
	
	~FrameData() { Reset(); }

	void Reset()
	{
		if (buff)
			delete [] buff;
		buff = 0;
		size = 0;
	}

	bool Init(unsigned int size)
	{
		Reset();

		if ((buff = new unsigned char[size]) == 0)
			return false;

		for (unsigned int i = 0; i < size; ++i)
			buff[i] = 0;
		
		this -> size = size;
		return true;
	}

	bool IsInit() { return buff && size > 0; }

	unsigned char * buff;
	unsigned int size;
};

enum UpdateFramesMode
{
	UFM__NO_UPDATE,
	UFM__NEED_TO_UPDATE,
	UFM__READY_TO_SWAP_BUFFERS
};

class FramesBuffer
{
public:
	FramesBuffer(void)  { Reset(); }
	~FramesBuffer(void) { Reset(); }
	
	bool Init(int frameWidth, int frameHeight, int frameBPP);

	bool isInit() { return frames[0].IsInit() && frames[1].IsInit() && popFrame.IsInit(); }

	void Reset()
	{
		frameWidth  = 0;
		frameHeight = 0;
		frameBPP    = 0;
	
		frames[0].Reset();
		frames[1].Reset();
		popFrame.Reset();
		popPlace = 0;
		updateFramesMode = UFM__NO_UPDATE;
		inputFramesCounter = 0;
		timeDelta = 0;
		framesDelta = 0;
		prevTime = 0;
	}
	
	bool Push(unsigned char * data);
	unsigned char * Pop(unsigned int & size, int * passedTime = NULL, int * framesCounter = NULL);
	
	int GetFrameWidth()  { return frameWidth;  }
	int GetFrameHeight() { return frameHeight; }
	int GetFrameBPP()    { return frameBPP;    }

private:
	int frameWidth;
	int frameHeight;
	int frameBPP;

	int inputFramesCounter;
	time_t timeDelta;
	int framesDelta;
	time_t prevTime;

	FrameData frames[2];
	FrameData popFrame;
	int popPlace;
	UpdateFramesMode updateFramesMode;
};

