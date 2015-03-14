#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>  // cvWaitKey

#include <boost/assert.hpp>

// buffer size: replace the GST_BUFFER_SIZE() to gst_buffer_get_size().

// buffer data: replace the GST_BUFFER_DATA() by using the gst_buffer_map to access the data.
//    GstBuffer *buffer = gst_app_sink_pull_preroll(sink);
//    GstMapInfo mapinfo = {0, };
//    gst_buffer_map(buffer, &mapinfo, GST_MAP_READ);
//    /* access the buffer by mapinfo.data and mapinfo.size */
//    gst_buffer_unmap(buffer, &mapinfo);

//  * g_thread_create remove, use g_thread_new -> check further.
//  * gst_app_sink_set_callbacks() -> callbacks structure is change.
//  * ways change to handle sink callback functions.
//    #if 0
//      GstBuffer *buffer = gst_app_sink_pull_preroll(sink);
//    #else
//      GstSample *sample = gst_app_sink_pull_preroll(sink);
//      GstBuffer *buffer = gst_sampe_get_buffer(sample);
//    #endif

//  * The GstRTSPMediaMapping rename to be GstRTSPMountPoints, we can compare by the examples.
//  * gst_rtsp_server_get_media_mapping() become gst_rtsp_server_get_mount_points()
//  * gst_rtsp_media_mapping_add_factory() become gst_rtsp_mount_points_add_factory()

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

struct MyContext
{
	gboolean white;
	GstClockTime timestamp;
};

/* called when we need to give data to appsrc */
static void	need_data (GstElement * appsrc, guint unused, MyContext * ctx)
{
	GstBuffer *buffer;
	guint size;
	GstFlowReturn ret;

	size = 385 * 288 * 2;

	buffer = gst_buffer_new_allocate (NULL, size, NULL);

	/* this makes the image black/white */
	gst_buffer_memset (buffer, 0, ctx->white ? 0xff : 0x0, size);

	ctx->white = !ctx->white;

	/* increment the timestamp every 1/2 second */
	GST_BUFFER_PTS (buffer) = ctx->timestamp;
	GST_BUFFER_DURATION (buffer) = gst_util_uint64_scale_int (1, GST_SECOND, 2);
	ctx->timestamp += GST_BUFFER_DURATION (buffer);

	g_signal_emit_by_name (appsrc, "push-buffer", buffer, &ret);
}

/* called when a new media pipeline is constructed. We can query the
* pipeline and configure our appsrc */
static void	media_configure (GstRTSPMediaFactory * factory, GstRTSPMedia * media, gpointer user_data)
{
	GstElement *element, *appsrc;
	MyContext *ctx;

	/* get the element used for providing the streams of the media */
	element = gst_rtsp_media_get_element (media);

	/* get our appsrc, we named it 'mysrc' with the name property */
	appsrc = gst_bin_get_by_name_recurse_up (GST_BIN (element), "mysrc");

	/* this instructs appsrc that we will be dealing with timed buffer */
	gst_util_set_object_arg (G_OBJECT (appsrc), "format", "time");
	/* configure the caps of the video */
	g_object_set (G_OBJECT (appsrc), "caps",
		gst_caps_new_simple ("video/x-raw",
		"format", G_TYPE_STRING, "RGB16",
		"width", G_TYPE_INT, 384,
		"height", G_TYPE_INT, 288,
		"framerate", GST_TYPE_FRACTION, 0, 1, NULL), NULL);

	ctx = g_new0 (MyContext, 1);
	ctx->white = FALSE;
	ctx->timestamp = 0;
	/* make sure ther datais freed when the media is gone */
	g_object_set_data_full (G_OBJECT (media), "my-extra-data", ctx,
		(GDestroyNotify) g_free);

	/* install the callback that will be called when a buffer is needed */
	g_signal_connect (appsrc, "need-data", (GCallback) need_data, ctx);
	gst_object_unref (appsrc);
	gst_object_unref (element);
}

int	main (int argc, char *argv[])
{
	GMainLoop *loop;
	GstRTSPServer *server;
	GstRTSPMountPoints *mounts;
	GstRTSPMediaFactory *factory;

	gst_init (&argc, &argv);

	PrintVersion();

	loop = g_main_loop_new (NULL, FALSE);

	/* create a server instance */
	server = gst_rtsp_server_new ();
	
//	gst_rtsp_server_set_service (server, "5458");		// set port
	gst_rtsp_server_set_address(server, "127.0.0.1");

	/* get the mount points for this server, every server has a default object
	* that be used to map uri mount points to media factories */
	mounts = gst_rtsp_server_get_mount_points (server);

	/* make a media factory for a test stream. The default media factory can use
	* gst-launch syntax to create pipelines.
	* any launch line works as long as it contains elements named pay%d. Each
	* element with pay%d names will be a stream */
	factory = gst_rtsp_media_factory_new ();
	gst_rtsp_media_factory_set_launch (factory,
		"( appsrc name=mysrc ! videoconvert ! x264enc ! rtph264pay name=pay0 pt=96 )");

	/* notify when our media is ready, This is called whenever someone asks for
	* the media and a new pipeline with our appsrc is created */
	g_signal_connect (factory, "media-configure", (GCallback) media_configure,
		NULL);

	/* attach the test factory to the /test url */
	gst_rtsp_mount_points_add_factory (mounts, "/test", factory);

	/* don't need the ref to the mounts anymore */
	g_object_unref (mounts);

	/* attach the server to the default maincontext */
	gst_rtsp_server_attach (server, NULL);

	g_print ("stream ready at rtsp://%s:%d/test\n", gst_rtsp_server_get_address(server), gst_rtsp_server_get_bound_port(server));

	/* start serving */
	g_main_loop_run (loop);

	//IplImage * image = cvLoadImage("d:/dalmation.bmp");
	//
	//while(cvWaitKey(10) != 113)	// press 'q'
	//{
	//	cvShowImage("Display window", image);
	//}
	//
	//cvDestroyWindow("Display window");
	//cvReleaseImage(&image);

	return 0;
}
