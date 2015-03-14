#include "stdafx.h"
#include "ReadWithGstreamer.h"
#include <stdio.h>

void* lock(void* data, void** p_pixels)
{
	ReadWithGstreamer * ctx = (ReadWithGstreamer*)data;

	*p_pixels = ctx->buff;

	return NULL;
}

void unlock(void* data, void* id, void* const* p_pixels)
{
	ReadWithGstreamer * ctx = (ReadWithGstreamer *)data;

	unsigned char* pixels = (unsigned char*)*p_pixels;

	ctx->CopyDataToClass(pixels, ctx->buffSize);
}

unsigned setup(void **opaque, char *chroma, unsigned *width, unsigned *height, unsigned *pitches, unsigned *lines)
{
	ReadWithGstreamer * readWithGstreamer = static_cast<ReadWithGstreamer*>(*opaque);
	
	*width  /= readWithGstreamer -> imageResizeX;
	*height /= readWithGstreamer -> imageResizeY;

	readWithGstreamer -> m_iWidth = *width;
	readWithGstreamer -> m_iheight = *height;
	readWithGstreamer -> m_iBpp = 24;
	readWithGstreamer -> m_iCompression = 0;
	*pitches = readWithGstreamer -> m_iWidth * 3;
	*lines = readWithGstreamer -> m_iheight;
	chroma[0] = 'R';
	chroma[1] = 'V';
	chroma[2] = '2';
	chroma[3] = '4';
	chroma[4] = 0;
	
	int buffSize = readWithGstreamer -> m_iWidth * readWithGstreamer -> m_iheight * readWithGstreamer -> m_iBpp / 8;
	readWithGstreamer -> buff = new unsigned char[buffSize];

	if (readWithGstreamer -> buff)
	{
		readWithGstreamer -> buffSize = buffSize;
		return 1;
	}

	readWithGstreamer -> buffSize = 0;
	return 0;
}

ReadWithGstreamer::ReadWithGstreamer()
{
	m_iWidth       = -1;
	m_iheight      = -1;
	m_iCompression = -1;
	m_iBpp         = -1;

	buff = NULL;
	buffSize = 0;

	isNewFrame = false;

	inst = NULL;
	mp = NULL;
}

ReadWithGstreamer::~ReadWithGstreamer()
{
	/* Stop playing */
	libvlc_media_player_stop (mp);

	/* Free the media_player */
	libvlc_media_player_release (mp);

	libvlc_release (inst);

	inst = NULL;
	mp = NULL;

	if (buff)
		delete [] buff;
	buff = NULL;
	buffSize = 0;

	isNewFrame = false;
}

bool ReadWithGstreamer::InitAndBuildPipline(char* url, double resizeX, double resizeY)
{
	const char* const vlc_args[] = {
								//	"--no-audio",		// make problems
									//"-q",
									//"--rtsp-tcp",		// use RTP over TCP : --rtsp-tcp, --no-rtsp-tcp Use RTP over RTSP (TCP) (default disabled)
								//	"--rtsp-frame-buffer-size=2000000",
									"--no-spu",
									"--no-osd",
									"--ignore-config",
								//	"--sout-udp-caching=1",				// ms
								//	"--sout-rtp-caching=1",				// ms
								//	"--sout-bridge-in-delay=0",			// ms
									"--live-caching=200",					// ms
									"--network-caching=300",				// ms	- make problems
								//	"--sout-display-delay=1",			// ms
								//	"--sout-ts-dts-delay=100000"			// ms
								//	"--plugin-path=C:\\Program Files\\VideoLAN\\VLC\\plugins",							// D:\\Program Files\\VideoLAN\\VLC\\plugins"
								//	"--no-network-synchronisation",
								//	"--input-timeshift-granularity=200",
									};

	/* Load the VLC engine */
	if ((inst = libvlc_new (sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args)) == NULL)
		return false;
	
	libvlc_media_t *m;

	/* Create a new item */
	if ((m = libvlc_media_new_location (inst, url)) == NULL)
		return false;

	/* Create a media player playing environement */
	if ((mp = libvlc_media_player_new_from_media (m)) == NULL)
		return false;

	libvlc_video_set_format_callbacks(mp, setup, NULL);

	libvlc_video_set_callbacks(mp, lock, unlock, NULL, this);

	/* No need to keep the media now */
	libvlc_media_release (m);

	imageResizeX = resizeX;
	imageResizeY = resizeY;

	return true;
}

bool ReadWithGstreamer::StartReadingStream()
{
	/* play the media_player */
	bool isOK = libvlc_media_player_play (mp) == 0;

	return isOK;
}

void ReadWithGstreamer::CopyDataToClass(unsigned char *buffer, int imageSize)
{
	//if (!buff)
	//{
	//	buff = new unsigned char[imageSize];
	//	buffSize = imageSize;
	//}
	//memcpy(buff, buffer, imageSize);
	//return;

	if (!_picBufferHolder.IsInit())
		_picBufferHolder.CreateImageBuffers(imageSize);

	_picBufferHolder.PutNewImage(buffer, imageSize);

	isNewFrame = true;
}

bool ReadWithGstreamer::GetImageData(unsigned char** buffer, int &width, int &height, int &compression, int &imageSize, int &bpp)
{
	*buffer = NULL;

	if (m_iWidth < 0)
		return false;

	if (!isNewFrame)
		return false;

	FrameImageInfo* frameGrabberInfo = _picBufferHolder.GetNextFrame();

	if(!frameGrabberInfo)
		return false;

	*buffer     = frameGrabberInfo -> _buffer;
	imageSize   = frameGrabberInfo -> _imageSize;

	//if (!buff) return false; *buffer = buff;

	width       = m_iWidth;
	height      = m_iheight;
	compression = m_iCompression;
	bpp         = m_iBpp;

	isNewFrame = false;

	return true;
}
