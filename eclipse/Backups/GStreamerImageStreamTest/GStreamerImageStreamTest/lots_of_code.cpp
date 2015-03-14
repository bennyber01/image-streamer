#include <stdio.h>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>
#include <gst/rtsp-server/rtsp-server.h>

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

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>  // cvWaitKey

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
	
	printf ("This program is linked against GStreamer %d.%d.%d %s\n", major, minor, micro, nano_str);
}

static void media_configure(GstRTSPMediaFactory * factory, GstRTSPMedia * media)
{
    gst_rtsp_media_use_time_provider(media, TRUE);
} 

int main (int argc, char *argv[])
{
	//GstElement *pipeline, *source, *jpg_decoder, *freeze, *colorspace, *sink;
	//
	//char * image1 = "d:/dalmation.jpg";
	//char * image2 = "d:/dalmation.jpg";

	///* Initialisation */
	//gst_init (&argc, &argv);
	
	GMainLoop *loop = g_main_loop_new (NULL, FALSE);

	///* Create gstreamer elements */
	//pipeline = gst_pipeline_new ("image-player");
	//if(!pipeline)
	//{
	//	g_printerr ("Pipeline could not be created. Exiting.\n");
	//	return -1;
	//}

	//source   = gst_element_factory_make ("filesrc",       "file-source");
	////set the location of the file to the argv[1]
	//g_object_set (G_OBJECT (source), "location", image1, NULL);
	//if(!source)
	//{
	//	g_printerr ("File could not be created. Exiting.\n");
	//	return -1;
	//}

	//jpg_decoder  = gst_element_factory_make ("jpegdec", "jpg-decoder");
	//if(!jpg_decoder)
	//{
	//	g_printerr ("Jpg Decoder could not be created. Exiting.\n");
	//	return -1;
	//}  

	//freeze = gst_element_factory_make("imagefreeze", "freeze");
	//if(!freeze)
	//{
	//	g_printerr ("ImageFreeze could not be created. Exiting.\n");
	//	return -1;
	//}  

	//colorspace = gst_element_factory_make("videoconvert", "colorspace");
	//if(!colorspace)
	//{
	//	g_printerr ("Colorspace could not be created. Exiting.\n");
	//	return -1;
	//}  

	//sink     = gst_element_factory_make ("ximagesink", "imagesink");
	//if(!sink)
	//{
	//	g_printerr ("Image sink could not be created. Exiting.\n");
	//	return -1;
	//}

	
    /* create a server instance */ 
	GstRTSPServer *server = gst_rtsp_server_new ();

//	gst_rtsp_server_set_service (server, "5458");		// set port
	
    /* get the mount points for this server, every server has a default object
    * that be used to map uri mount points to media factories */ 
	GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points (server);
	
	/* make a media factory for a test stream. The default media factory can use
    * gst-launch syntax to create pipelines.
    * any launch line works as long as it contains elements named pay%d. Each
    * element with pay%d names will be a stream */
	GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new ();


	gchar *str;
	str = g_strdup_printf("( "
		"filesrc location=D:/larvasessi.mp4 ! qtdemux name=d "
		"d. ! queue ! rtph264pay pt=96 name=pay0 "
		"d. ! queue ! rtpmp4apay pt=97 name=pay1 " ")");
	gst_rtsp_media_factory_set_launch(factory, str);
	g_free(str);

	//gst_rtsp_media_factory_set_launch (factory,	"( videotestsrc is-live=1 ! x264enc ! rtph264pay )");
	
	gst_rtsp_media_factory_set_shared (factory, TRUE);

    g_signal_connect(factory, "media-configure", (GCallback)media_configure, NULL);
	
    /* attach the test factory to the /test url */
    gst_rtsp_mount_points_add_factory(mounts, "/test", factory);

	/* don't need the ref to the mapper anymore */ 
	g_object_unref (mounts);

	/* attach the server to the default maincontext */ 
	gst_rtsp_server_attach (server, NULL);

	gst_rtsp_server_set_address(server, "127.0.0.1");
	
    /* start serving */
    g_print("stream ready at rtsp://127.0.0.1:8554/test\n"); 
	printf("server address: %s\r\n", gst_rtsp_server_get_address(server));
	printf("server port: %d\r\n", gst_rtsp_server_get_bound_port(server));

	g_main_loop_run (loop);

	///* file-source | jpg-decoder | image-freeze | colorspace | sink */
	//gst_bin_add_many (GST_BIN (pipeline), source, jpg_decoder, freeze, colorspace, sink, NULL);
	//gst_element_link_many (source, jpg_decoder, freeze, colorspace, sink, NULL);

	///* Set the pipeline to "playing" state*/
	//g_print ("Now playing: %s\n", image1);
	//gst_element_set_state (pipeline, GST_STATE_PLAYING);

	//getchar();
	//gst_element_set_state (pipeline, GST_STATE_READY);
	//g_object_set (G_OBJECT (source), "location", image2, NULL);
	//gst_element_set_state (pipeline, GST_STATE_PLAYING);
	//getchar();

	///* Out of the main loop, clean up nicely */
	//g_print ("Returned, stopping playback\n");
	//gst_element_set_state (pipeline, GST_STATE_NULL);

	//g_print ("Deleting pipeline\n");
	//gst_object_unref (GST_OBJECT (pipeline));

	return 0;
}

//int main (int argc, char *argv[])
//{
//	GMainLoop *loop;
//	GstRTSPServer *server;
//	GstRTSPMountPoints *mapping;
//	GstRTSPMediaFactory *factory;
//
//	gst_init (&argc, &argv);
//	loop = g_main_loop_new (NULL, FALSE);
//	server = gst_rtsp_server_new ();
//	mapping = gst_rtsp_server_get_mount_points (server);
//	factory = gst_rtsp_media_factory_new ();
//	gst_rtsp_media_factory_set_launch (factory,	"( videotestsrc is-live=1 ! x264enc ! rtph264pay )");
//
//	gst_rtsp_server_set_address(server, "127.0.0.1");
//
//	gst_rtsp_media_factory_set_shared (factory, TRUE);
//	gst_rtsp_mount_points_add_factory (mapping, "D:/larvasessi.mp4", factory);
//	g_object_unref (mapping);
//	gst_rtsp_server_attach (server, NULL);
//
//	printf("server address: %s\r\n", gst_rtsp_server_get_address(server));
//	printf("server port: %d\r\n", gst_rtsp_server_get_bound_port(server));
//
//	g_main_loop_run (loop);
//
//	return 0;
//}

////typedef struct _App App;
////struct _App
////{
////	GstElement *pipeline;
////	GstElement *appsrc;
////
////	GMainLoop *loop;
////	guint sourceid;
////	GTimer *timer;
////};
////
////App s_app;
////IplImage * image;
////
////static gboolean read_data(App *app)
////{
////	GstFlowReturn ret;
////	//GstBuffer *buffer = gst_buffer_new();
////	//GST_BUFFER_DATA(buffer) = (uchar*)image->imageData;
////	//GST_BUFFER_SIZE(buffer) = image->width*image->height*sizeof(uchar*);
////	GstBuffer *buffer = gst_buffer_new_wrapped((uchar*)image->imageData, image->width*image->height*sizeof(uchar*));
////	g_signal_emit_by_name(app->appsrc,"push-buffer",buffer,&ret);
////	//gst_buffer_unref(buffer);
////	if(ret != GST_FLOW_OK){
////		GST_DEBUG("Error al alimentar buffer");
////		return FALSE;
////	}
////	return TRUE;
////}
////
////static void start_feed(GstElement* pipeline,guint size, App* app){
////	if(app->sourceid == 0){
////		GST_DEBUG("Alimentando");
////		app->sourceid = g_idle_add((GSourceFunc) read_data, app);
////	}
////}
////
////static void stop_feed(GstElement* pipeline, App* app){
////	if(app->sourceid !=0 ){
////		GST_DEBUG("Stop feeding");
////		g_source_remove(app->sourceid);
////		app->sourceid = 0;
////	}
////}
////
////static gboolean
////	bus_message (GstBus * bus, GstMessage * message, App * app)
////{
////	GST_DEBUG ("got message %s",
////		gst_message_type_get_name (GST_MESSAGE_TYPE (message)));
////
////	switch (GST_MESSAGE_TYPE (message)) {
////	case GST_MESSAGE_ERROR: {
////		GError *err = NULL;
////		gchar *dbg_info = NULL;
////		gst_message_parse_error (message, &err, &dbg_info);
////		g_printerr ("ERROR from element %s: %s\n",
////			GST_OBJECT_NAME (message->src), err->message);
////		g_printerr ("Debugging info: %s\n", (dbg_info) ? dbg_info : "none");
////		g_error_free (err);
////		g_free (dbg_info);
////		g_main_loop_quit (app->loop);
////		break;
////							}
////	case GST_MESSAGE_EOS:
////		g_main_loop_quit (app->loop);
////		break;
////	default:
////		break;
////	}
////	return TRUE;
////}
////
////int main(int argc, char* argv[])
////{
////	App *app = &s_app;
////	GError *error = NULL;
////	GstBus *bus;
////	GstCaps *caps;
////	
////	image = cvLoadImage("d:/dalmation.bmp");
////
////	gst_init(&argc,&argv);
////
////	/* create a mainloop to get messages and to handle the idle handler that will feed data to appsrc. */
////	app->loop = g_main_loop_new (NULL, TRUE);
////	app->timer = g_timer_new();
////
////	// -> tcp://127.0.0.1:5000
////	app->pipeline = gst_parse_launch("appsrc name=mysource ! video/x-raw,format=RGB,width=640,height=480 ! ffmpegcolorspace ! videoscale method=1 ! theoraenc bitrate=150 ! tcpserversink host=127.0.0.1 port=5000", NULL);
////	g_assert (app->pipeline);
////	bus = gst_pipeline_get_bus (GST_PIPELINE (app->pipeline));
////	g_assert(bus);
////
////	/* add watch for messages */
////	gst_bus_add_watch (bus, (GstBusFunc) bus_message, app);
////	
////	/* get the appsrc */
////	app->appsrc = gst_bin_get_by_name (GST_BIN(app->pipeline), "mysource");
////	g_assert(app->appsrc);
////	g_assert(GST_IS_APP_SRC(app->appsrc));
////	g_signal_connect (app->appsrc, "need-data", G_CALLBACK (start_feed), app);
////	g_signal_connect (app->appsrc, "enough-data", G_CALLBACK (stop_feed), app);
////
////	/* set the caps on the source */	
////	caps = gst_caps_new_simple("video/x-raw",
////							   "format", G_TYPE_STRING, "RGB",
////							   "width",  G_TYPE_INT, image -> width,
////							   "height", G_TYPE_INT, image -> height,
////							   NULL);
////
////	gst_app_src_set_caps(GST_APP_SRC(app->appsrc), caps);
////
////	/* go to playing and wait in a mainloop. */
////	GstStateChangeReturn res = gst_element_set_state (app->pipeline, GST_STATE_PLAYING);
////
////	if (res == GST_STATE_CHANGE_FAILURE)
////	{
////		g_printerr ("Unable to set the pipeline to the playing state.\n");
////	}
////	else if (res == GST_STATE_CHANGE_NO_PREROLL)
////	{
////		//data.is_live = TRUE;
////	}
////	else if (res == GST_STATE_CHANGE_ASYNC)
////	{
////		// can happen when buffering occurs
////		GstState current, pending;
////		res = gst_element_get_state(GST_ELEMENT (app->pipeline), &current, NULL/*&pending*/, GST_CLOCK_TIME_NONE);
////		if(res == GST_STATE_CHANGE_FAILURE || res == GST_STATE_CHANGE_ASYNC)
////		{
////			g_printerr ("Unable to set the pipeline to the playing state.\n");
////		}
////	}
////
////	/* this mainloop is stopped when we receive an error or EOS */
////	g_main_loop_run (app->loop);
////	
////	GST_DEBUG ("stopping");
////	gst_element_set_state (app->pipeline, GST_STATE_NULL);
////	gst_object_unref (bus);
////	g_main_loop_unref (app->loop);
////	cvReleaseImage(&image);
////	return 0;
////}

//int main (int argc, char *argv[])
//{
//	gst_init (&argc, &argv);
//
//	gboolean isInit = gst_is_initialized();
//
//	PrintVersion();
//
//	IplImage * image = cvLoadImage("d:/dalmation.bmp");
//
//	while(cvWaitKey(10) != 113)	// press 'q'
//	{
//		cvShowImage("Display window", image);
//	}
//
//	cvDestroyWindow("Display window");
//	cvReleaseImage(&image);
//	
//	return 0;
//}