#include <stdio.h>
#include <stdlib.h>
#include <vlc/vlc.h>

#include <windows.h>		// Sleep

int main(int argc, char* argv[])
{
	libvlc_instance_t * inst;
	libvlc_media_player_t *mp;
	libvlc_media_t *m;

	/* Load the VLC engine */
	inst = libvlc_new (0, NULL);

	/* Create a new item */
	m = libvlc_media_new_location (inst, "rtsp://admin:9999@10.200.1.222:554/live/stream1");
	//m = libvlc_media_new_location (inst, "rtsp://localhost:554");

	/* Create a media player playing environement */
	mp = libvlc_media_player_new_from_media (m);

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


//stream.h:
//
//#include <vlc/vlc.h>
//#include <vlc/libvlc.h>
//
//struct ctx {
//   uchar* frame;
//};
////=============================================================
//
//stream.cpp:
//
//void* lock(void* data, void** p_pixels){
//  struct ctx* ctx = (struct ctx*)data;
//  *p_pixels = ctx->frame;
//  return NULL;
//}
//
//void unlock(void* data, void* id, void* const* p_pixels){
//  struct ctx* ctx = (struct ctx*)data;
//  uchar* pixels = (uchar*)*p_pixels;
//  assert(id == NULL);
//}
////=============================================================
//
//struct ctx* context = (struct ctx*)malloc(sizeof(*context));
//const char* const vlc_args[] = {"-vvv",
//                                 "-q",
//                                 "--no-audio"};
//libvlc_media_t* media = NULL;
//libvlc_media_player_t* media_player = NULL;
//libvlc_instance_t* instance = libvlc_new(sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);
//
//media = libvlc_media_new_location(instance, "udp://@123.123.123.123:1000");
//media_player = libvlc_media_player_new(instance);
//libvlc_media_player_set_media(media_player, media);
//libvlc_media_release(media);
//context->frame = new uchar[height * width * 3];
//libvlc_video_set_callbacks(media_player, lock, unlock, NULL, context);
//libvlc_video_set_format(media_player, "RV24", VIDEOWIDTH, VIDEOHEIGHT, VIDEOWIDTH * 3);
//libvlc_media_player_play(media_player);
////=============================================================
