#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <vlc/vlc.h>

#include "PicBufferHolder.h"

struct ctx
{
   unsigned char * frame;
};

class  ReadWithGstreamer
{
public:
	ReadWithGstreamer();
	~ReadWithGstreamer();
	bool InitAndBuildPipline(char *url, double resizeX = 1.0, double resizeY = 1.0);
	
	bool StartReadingStream();
	bool GetImageData(unsigned char** buffer, int &width, int &height, int &compression, int &imageSize, int &bpp);
	bool IsImageParametersInit() { return m_iWidth > 0; }

//private:
	unsigned int m_iWidth;
	unsigned int m_iheight;
	unsigned int m_iCompression;
	//int m_iImageSize;
	unsigned int m_iBpp;

	CPicBufferHolder _picBufferHolder;

public:

	unsigned char * buff;
	int buffSize;

	bool isNewFrame;

	libvlc_instance_t * inst;
	libvlc_media_player_t *mp;
	
	double imageResizeX;
	double imageResizeY;

	void CopyDataToClass(unsigned char *buffer, int imageSize);
};

