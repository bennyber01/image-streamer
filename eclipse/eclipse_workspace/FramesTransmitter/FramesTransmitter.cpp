/*
 * FramesTransmitter.cpp
 *
 *  Created on: 28 בספט 2014
 *      Author: benny
 */

#include "FramesTransmitter.h"

#include <boost/assert.hpp>

void PrintVersion()
{
	const gchar *nano_str;
	guint major, minor, micro, nano;

	gst_version (&major, &minor, &micro, &nano);
	
	if (nano == 1)
		nano_str = "(CVS)";
	else if (nano == 2)
		nano_str = "(Prerelease)";
	else
		nano_str = "";
	
	g_print("This program is linked against GStreamer %d.%d.%d %s\n", major, minor, micro, nano_str);
}

//============================================================================================================

struct MyContext
{
	FramesTransmitter* framesTransmitter;
	GstClockTime timestamp;
};

/* called when we need to give data to appsrc */
static void need_data(GstElement * appsrc, guint unused, MyContext * ctx)
{
	unsigned int size;
	GstFlowReturn ret;

	GstBuffer * buffer = ctx->framesTransmitter->buffer;

	unsigned char * buff = ctx->framesTransmitter->framesBuffer.Pop(size);

	gst_buffer_fill(buffer, 0, buff, size);

	GST_BUFFER_PTS(buffer) = ctx -> timestamp;
	GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale_int(1, GST_SECOND, 25);
	ctx -> timestamp += GST_BUFFER_DURATION(buffer);

	g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);
}

/* called when a new media pipeline is constructed. We can query the
* pipeline and configure our appsrc */
static void	media_configure (GstRTSPMediaFactory * factory, GstRTSPMedia * media, gpointer user_data)
{
	GstElement *element, *appsrc;
	MyContext *ctx;

	FramesTransmitter* framesTransmitter = (FramesTransmitter*)user_data;

	/* get the element used for providing the streams of the media */
	element = gst_rtsp_media_get_element (media);

	/* get our appsrc, we named it 'mysrc' with the name property */
	appsrc = gst_bin_get_by_name_recurse_up (GST_BIN (element), "mysrc");

	/* this instructs appsrc that we will be dealing with timed buffer */
	gst_util_set_object_arg (G_OBJECT (appsrc), "format", "time");
	/* configure the caps of the video */
	int frameWidth  = framesTransmitter -> framesBuffer.GetFrameWidth();
	int frameHeight = framesTransmitter -> framesBuffer.GetFrameHeight();
	g_object_set (G_OBJECT (appsrc), "caps",
		gst_caps_new_simple ("video/x-raw",
		"format", G_TYPE_STRING, "BGR",
		"width", G_TYPE_INT, frameWidth,
		"height", G_TYPE_INT, frameHeight,
		"framerate", GST_TYPE_FRACTION, 0, 1, NULL), NULL);

	ctx = g_new0 (MyContext, 1);
	ctx->framesTransmitter = framesTransmitter;
	ctx->timestamp = 0;

	/* make sure ther datais freed when the media is gone */
	g_object_set_data_full (G_OBJECT (media), "my-extra-data", ctx,	(GDestroyNotify) g_free);

	/* install the callback that will be called when a buffer is needed */
	g_signal_connect (appsrc, "need-data", (GCallback) need_data, ctx);
	gst_object_unref (appsrc);
	gst_object_unref (element);
}

//============================================================================================================

FramesTransmitter::FramesTransmitter()
{
	mainLoopThread = NULL;
	mainLoop = NULL;

	framesBuffer.Reset();
	buffer  = NULL;
	server  = NULL;
	factory = NULL;
}

FramesTransmitter::~FramesTransmitter()
{
	if (mainLoop)
		g_main_loop_quit (mainLoop);

	if (mainLoopThread)
		g_thread_join(mainLoopThread);
	mainLoopThread = NULL;

	framesBuffer.Reset();

	if (buffer)
		gst_buffer_unref(buffer);
	buffer = NULL;

	if (factory)
		g_object_unref (factory);
	factory = NULL;

	if (server)
		g_object_unref (server);
	server = NULL;
}

gpointer MainLoopThreadFunction(gpointer data)
{
	FramesTransmitter* framesTransmitter = (FramesTransmitter*)data;
	
	g_main_loop_run (framesTransmitter -> mainLoop);

	/* Free resources */
	g_main_loop_unref (framesTransmitter -> mainLoop);
	framesTransmitter -> mainLoop  = NULL;

	return NULL;
}

bool FramesTransmitter::Init(char * url, char * port, int frameWidth, int frameHeight, int frameBPP)
{
	gst_init (NULL, NULL);

	PrintVersion();

	if (!framesBuffer.Init(frameWidth, frameHeight, frameBPP))
		return false;

	unsigned size = frameWidth * frameHeight * frameBPP / 8;
	buffer = gst_buffer_new_allocate(NULL, size, NULL);

	/* create a server instance */
	server = gst_rtsp_server_new ();
	
	/* set server url and port number */
	gst_rtsp_server_set_service (server, port);
	gst_rtsp_server_set_address(server, url);

	/* get the mount points for this server, every server has a default object
	* that be used to map uri mount points to media factories */
	GstRTSPMountPoints * mounts = gst_rtsp_server_get_mount_points (server);

	/* make a media factory for a test stream. The default media factory can use
	* gst-launch syntax to create pipelines.
	* any launch line works as long as it contains elements named pay%d. Each
	* element with pay%d names will be a stream */
	factory = gst_rtsp_media_factory_new ();
	gst_rtsp_media_factory_set_launch (factory,	"( appsrc name=mysrc ! videoconvert ! x264enc tune=zerolatency ! rtph264pay name=pay0 pt=96 )");

	/* notify when our media is ready, This is called whenever someone asks for
	* the media and a new pipeline with our appsrc is created */
	g_signal_connect (factory, "media-configure", (GCallback) media_configure, this);

	/* attach the test factory to the /test url */
	gst_rtsp_mount_points_add_factory (mounts, "/test", factory);

	/* don't need the ref to the mounts anymore */
	g_object_unref (mounts);

	/* attach the server to the default maincontext */
	gst_rtsp_server_attach (server, NULL);

	mainLoop = g_main_loop_new (NULL, FALSE);

	/* start serving */
	mainLoopThread = g_thread_new("mainLoopThread", MainLoopThreadFunction, this);

	g_print ("stream ready at rtsp://%s:%d/test\n", gst_rtsp_server_get_address(server), gst_rtsp_server_get_bound_port(server));

	return true;
}

bool FramesTransmitter::Transmit(unsigned char * buff)
{
	return framesBuffer.Push(buff);
}
