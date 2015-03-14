#include "GStreamerFramesReceiver.h"

#include <stdio.h>		// sprintf_s
#include <string.h>		// strcat_s
#include <algorithm>	// std::copy

#include <boost/assert.hpp>

#pragma warning(disable:4996)	// warning C4996: 'sprintf'

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                   G S T R E A M E R    C A L L B A C K S                   //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// possible values in GstCaps are:
//    width
//    height
//    format
//    framerate
void ExtractImageParams(GstCaps *caps, int & width, int & height, int & bpp)
{
	width = height = bpp = 0;

	char text[4000];
	text[0] = 0;

	for (unsigned int j = 0; j < gst_caps_get_size(caps); ++j)
	{
		GstStructure * structure = gst_caps_get_structure(caps, j);

		for (int i = 0; i < gst_structure_n_fields(structure); ++i)
		{
			if (i != 0)
				strcat_s(text, ", ");

			const char * name = gst_structure_nth_field_name(structure, i);
			GType type = gst_structure_get_field_type(structure, name);
			const GValue * value = gst_structure_get_value(structure, name);

			if (strcmp("width", name) == 0)
			{
				width = value->data->v_int;
			}
			if (strcmp("height", name) == 0)
			{
				height = value->data->v_int;
			}
			if (strcmp("format", name) == 0)
			{
				const gchar * format = g_value_get_string(value);
				if (strcmp(format, "RGB") == 0 || strcmp(format, "BGR") == 0)
				{
					bpp = 24;
				}
			}

			strcat_s(text, name);
			strcat_s(text, "[");
			strcat_s(text,  g_type_name(type));
			strcat_s(text, ":");

			if (g_type_is_a(type, G_TYPE_STRING))
				strcat_s(text, g_value_get_string(value));
			else if (GST_VALUE_HOLDS_FRACTION(&type))
			{
				char size[100];
				sprintf_s(size, "%d/%d", value->data[0].v_int, value->data[1].v_int);
				strcat_s(text, size);
			}
			else
			{
				char size[100];
				sprintf_s(size, "%d", value->data->v_int);
				strcat_s(text, size);
			}
			strcat(text ,"]");

		}
		printf(text);
	}
}

GstBusSyncReply GStreamerFramesReceiver::BusCallback(GstBus * bus, GstMessage * message, gpointer data)
{
	BOOST_ASSERT(bus);
	BOOST_ASSERT(message);
	BOOST_ASSERT(data);

	GStreamerFramesReceiver * pClass = (GStreamerFramesReceiver*) data;

	g_print("Got %s buss message\r\n", GST_MESSAGE_TYPE_NAME(message));

	switch (GST_MESSAGE_TYPE (message))
	{
	case GST_MESSAGE_ERROR:
		{
			GError *err;
			gchar *debug;
      
			gst_message_parse_error (message, &err, &debug);
			g_free (debug);

			g_printerr ("Error: %s\n", err->message);
			g_error_free (err);

			g_main_loop_quit (pClass -> mainLoop);
			break;
		}
	case GST_MESSAGE_EOS:
		{
			/* end-of-stream */
			g_main_loop_quit (pClass -> mainLoop);
			break;
		}
	case GST_MESSAGE_BUFFERING:
		{
			gint percent = 0;
      
			/* If the stream is live, we do not care about buffering. */
//			if (data->is_live) break;
      
			gst_message_parse_buffering (message, &percent);
			g_print ("Buffering (%3d%%)", percent);

			///* Wait until buffering is complete before start/resume playing */
			//if (percent == 100)
			//	gst_element_set_state (pClass -> pipeline, GST_STATE_PLAYING);
			//else
			//	gst_element_set_state (pClass -> pipeline, GST_STATE_PAUSED);

			break;
		}
	case GST_MESSAGE_CLOCK_LOST:
		{
			/* Get a new clock */
			gst_element_set_state (pClass -> pipeline, GST_STATE_PAUSED);
			gst_element_set_state (pClass -> pipeline, GST_STATE_PLAYING);
			break;
		}
	default:
		{
			/* Unhandled message */
			break;
		}
	}

	/* remove message from the queue */
//	gst_message_unref (message);
	return GST_BUS_DROP;
}

static void on_eos(GstAppSink * sink, gpointer user_data)
{
}

GstFlowReturn frame_handler(GstSample * sample, GStreamerFramesReceiver * pClass)
{
	GstBuffer * buffer = gst_sample_get_buffer(sample);
	GstMapInfo info; 

	gst_buffer_map(buffer, &info, GST_MAP_READ);

	if (pClass)
	{
		if (pClass -> FrameWidth() == 0)
		{
			int width, height, bpp;
			GstCaps *caps = gst_sample_get_caps(sample);
			ExtractImageParams(caps, width, height, bpp);
			pClass -> FrameWidth()  = width;
			pClass -> FrameHeight() = height;
			pClass -> FrameBPP()    = bpp;
		}
	
		BOOST_ASSERT(pClass -> FrameWidth() * pClass -> FrameHeight()  * pClass -> FrameBPP() / 8 == info.size);

		pClass -> CopyFrameData(info.data, info.size);
	}

	gst_buffer_unmap (buffer, &info);
	gst_buffer_unref(buffer);

	return GST_FLOW_OK;
}

static GstFlowReturn new_preroll(GstAppSink * sink, gpointer data)
{
	GStreamerFramesReceiver * pClass = (GStreamerFramesReceiver*) data;

	GstSample * sample = gst_app_sink_pull_preroll(sink);

	GstFlowReturn res = frame_handler(sample, pClass);

	gst_sample_unref(sample);

	return res;
}

static GstFlowReturn new_buffer(GstAppSink * sink, gpointer data)
{
	GStreamerFramesReceiver * pClass = (GStreamerFramesReceiver*) data;

	GstSample * sample = gst_app_sink_pull_preroll(sink);

	GstFlowReturn res = frame_handler(sample, pClass);

	//gst_sample_unref(sample);

	return res;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//              M A I N    L O O P    T H R E A D    M E T H O D              //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

gpointer GStreamerFramesReceiver::MainLoopThreadFunction(gpointer data)
{
	GStreamerFramesReceiver * pClass = (GStreamerFramesReceiver*) data;

	pClass -> mainLoop = g_main_loop_new(NULL, FALSE);

	g_main_loop_run (pClass -> mainLoop);

	// clean the program vars after we exit the main loop
	gst_app_sink_set_emit_signals(GST_APP_SINK(pClass -> sink), false);
	gst_bus_remove_signal_watch(GST_BUS(pClass -> bus));

	/* Move to paused */
	gst_element_set_state(pClass -> pipeline, GST_STATE_PAUSED);

	/* iterate mainloop to process pending stuff */
	while (g_main_context_iteration(NULL, FALSE));

	gst_bus_set_flushing(pClass -> bus, TRUE);
	gst_element_set_state(pClass -> pipeline, GST_STATE_READY);

	gst_bus_set_flushing(pClass -> bus, FALSE);
	gst_object_unref (pClass -> bus);

	/* Free resources */
	gst_element_set_state (pClass -> pipeline, GST_STATE_NULL);
	gst_object_unref (pClass -> pipeline);
	g_main_loop_unref (pClass -> mainLoop);

	pClass -> sink     = NULL;
	pClass -> pipeline = NULL;
	pClass -> bus      = NULL;
	pClass -> mainLoop = NULL;

	return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                M E T H O D S    I M P L E M E N T A T I O N                //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

GStreamerFramesReceiver::GStreamerFramesReceiver(void) : buff(NULL)
{
	buffSize    = 0;
	frameWidth  = 0;
	frameHeight = 0;
	frameBPP    = 0;

	isNewFrameReady = false;

	pipeline = NULL;
	mainLoopThread = NULL;
	mainLoop = NULL;
	bus = NULL;
	sink = NULL;
}


GStreamerFramesReceiver::~GStreamerFramesReceiver(void)
{
	if (mainLoop)
		g_main_loop_quit (mainLoop);

	if (mainLoopThread)
		g_thread_join(mainLoopThread);
	mainLoopThread = NULL;

	if (buff)
		delete [] buff;
	buff = NULL;
	buffSize    = 0;
	frameWidth  = 0;
	frameHeight = 0;
	frameBPP    = 0;

	isNewFrameReady = false;

	pipeline = NULL;
	mainLoopThread = NULL;
	mainLoop = NULL;
	bus = NULL;
	sink = NULL;
}

bool GStreamerFramesReceiver::LoadVideo(char * URL)
{
	/* Initialize GStreamer */
	gst_init(NULL, NULL);

	/* Build the pipeline */
	GError *error = NULL;
	char * init_str = g_strdup_printf("rtspsrc location=%s latency=1000 drop-on-latency=false ! queue ! rtph264depay ! queue ! avdec_h264 ! queue ! videoconvert ! appsink name=mysink", URL);
	pipeline = gst_parse_launch(init_str, &error);
	g_free(init_str);
	
	if (error)
	{
		gchar * message = g_strdup_printf("Unable to build pipeline: %s", error -> message);
		g_clear_error(&error);
		g_free(message);
		return false;
	}

	bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
	gst_bus_add_signal_watch (bus);
	g_signal_connect (bus, "message", G_CALLBACK (BusCallback), this);

	sink = gst_bin_get_by_name(GST_BIN(pipeline), "mysink");
	gst_app_sink_set_emit_signals(GST_APP_SINK(sink), true);
	gst_app_sink_set_drop(GST_APP_SINK(sink), true);
	gst_app_sink_set_max_buffers(GST_APP_SINK(sink), 1);		//number of recived buffers in appsink before updating new frame
	g_object_set(G_OBJECT(sink), "sync", TRUE, NULL);			// GST_OBJECT

	//set requierd caps
	GstCaps* video_caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "BGR", NULL);
	gst_app_sink_set_caps(GST_APP_SINK(sink), video_caps);
	gst_caps_unref(video_caps);

	// Registering callbacks to appsink element
	GstAppSinkCallbacks callbacks = { on_eos, new_preroll, new_buffer, NULL };
	gst_app_sink_set_callbacks(GST_APP_SINK(sink), &callbacks, this, NULL);

	GstStateChangeReturn res = gst_element_set_state (pipeline, GST_STATE_PLAYING);

	if (res == GST_STATE_CHANGE_FAILURE)
	{
		g_printerr ("Unable to set the pipeline to the playing state.\n");
		gst_object_unref (pipeline);
		pipeline = NULL;
		return false;
	}
	else if (res == GST_STATE_CHANGE_NO_PREROLL)
	{
//		streamerData.is_live = true;
	}
	else if (res == GST_STATE_CHANGE_ASYNC)
	{
		// can happen when buffering occurs
		GstState current, pending;
		res = gst_element_get_state(GST_ELEMENT(pipeline), &current, &pending, GST_CLOCK_TIME_NONE);
		if(res == GST_STATE_CHANGE_FAILURE || res == GST_STATE_CHANGE_ASYNC)
		{
			g_printerr ("Unable to set the pipeline to the playing state.\n");
			gst_object_unref (pipeline);
			pipeline = NULL;
			return false;
		}
	}
	
	mainLoopThread = g_thread_new("mainLoopThread", MainLoopThreadFunction, this);

	return true;
}

bool GStreamerFramesReceiver::GetFrameInfo(int & frameWidth, int & frameHeight, int & frameBPP)
{
	frameWidth  = this -> frameWidth;
	frameHeight = this -> frameHeight;
	frameBPP    = this -> frameBPP;

	bool isOK = frameWidth > 0 && frameHeight > 0 && frameBPP > 0;
	BOOST_ASSERT(isOK);
	return isOK;
}

unsigned char* GStreamerFramesReceiver::GetFrameData()
{
	if (isNewFrameReady)
	{
		isNewFrameReady = false;
		return buff;
	}

	return NULL;
}

bool GStreamerFramesReceiver::CopyFrameData(unsigned char * data, int size)
{
	if (!buff || buffSize < size)
	{
		if (buff)
			delete [] buff;

		if ((buff = new unsigned char[size]) == NULL)
		{
			buffSize = 0;
			return false;
		}

		buffSize = size;
	}
	
	std::copy(data, data + size, buff);

	isNewFrameReady = true;

	return true;
}
