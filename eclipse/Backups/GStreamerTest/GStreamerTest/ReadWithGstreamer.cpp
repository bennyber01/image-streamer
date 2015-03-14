#include "ReadWithGstreamer.h"

#include <boost/assert.hpp>

#pragma warning(disable:4996)	// warning C4996: 'sprintf'

ReadWithGstreamer *g_this = NULL;

static void on_eos(GstAppSink * sink, gpointer user_data)
{
	int gg=0;
}

static GstBusSyncReply my_bus_callback (GstBus *bus, GstMessage *message, gpointer user_data)
{
	BOOST_ASSERT(bus);
	BOOST_ASSERT(message);
	BOOST_ASSERT(user_data);

	GstElement* pipeline = (GstElement*) user_data;

	//g_print("Got %s message\r\n", GST_MESSAGE_TYPE_NAME(message));

	switch (GST_MESSAGE_TYPE (message))
	{
	case GST_MESSAGE_ERROR:
		{
			GError *err;
			gchar *debug;
      
			gst_message_parse_error (message, &err, &debug);
			g_print ("Error: %s\n", err->message);
			g_error_free (err);
			g_free (debug);
      
			gst_element_set_state (pipeline, GST_STATE_READY);
//			g_main_loop_quit (data->loop);
			break;
		}
	case GST_MESSAGE_EOS:
		{
			/* end-of-stream */
			gst_element_set_state (pipeline, GST_STATE_READY);
//			g_main_loop_quit (data->loop);
			break;
		}
	case GST_MESSAGE_BUFFERING:
		{
			gint percent = 0;
      
			/* If the stream is live, we do not care about buffering. */
//			if (data->is_live) break;
      
			gst_message_parse_buffering (message, &percent);
			g_print ("Buffering (%3d%%)\r", percent);

			/* Wait until buffering is complete before start/resume playing */
//			if (percent < 100)
//				gst_element_set_state (g_this->pipeline, GST_STATE_PAUSED);
//			else
//				gst_element_set_state (g_this->pipeline, GST_STATE_PLAYING);

			break;
		}
	case GST_MESSAGE_CLOCK_LOST:
		{
			/* Get a new clock */
			gst_element_set_state (pipeline, GST_STATE_PAUSED);
			gst_element_set_state (pipeline, GST_STATE_PLAYING);
			break;
		}
	default:
		{
			/* Unhandled message */
			break;
		}
	}

	/* remove message from the queue */
	gst_message_unref (message);
	return GST_BUS_DROP;
}

static void my_bus_destroy_notify(gpointer data)
{
	int tt=0;
}

void print_buffer(GstSample *sample, const char *title)
{
	char text[4000];
	text[0] = 0;

	GstCaps *caps = gst_sample_get_caps(sample);
	for (unsigned int j = 0; j < gst_caps_get_size(caps); ++j)
	{
		GstStructure *structure = gst_caps_get_structure(caps, j);
		strcat_s(text, "\n");
		strcat_s(text, title);
		strcat_s(text, "-");
		strcat_s(text, gst_structure_get_name(structure));
		strcat_s(text, " ");

		for (int i = 0; i < gst_structure_n_fields(structure); ++i)
		{
			if (i != 0)
				strcat_s(text, ", ");

			const char *name = gst_structure_nth_field_name(structure, i);
			GType type = gst_structure_get_field_type(structure, name);
			const GValue *value = gst_structure_get_value(structure, name);

			if (strcmp("width", name) == 0)
			{
				g_this -> m_iWidth = value->data->v_int;
			}
			if (strcmp("height", name) == 0)
			{
				g_this -> m_iHeight = value->data->v_int;
			}
			if (strcmp("format", name) == 0)
			{
				 const gchar * image_format = g_value_get_string(value);
				 if (strcmp(image_format, "RGB") == 0 || strcmp(image_format, "BGR") == 0)
					 g_this -> m_iBPP = 24;
				 else
					 g_this -> m_iBPP = -1;
			}

			strcat_s(text, name);
			strcat_s(text, "[");
			strcat_s(text, g_type_name(type));
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
			strcat_s(text, "]");
		}
	}
	gst_caps_unref(caps);
	
	strcat_s(text, "\n");
	printf(text);
}

static GstFlowReturn new_preroll(GstAppSink *sink, gpointer user_data)
{
	GstSample *sample = gst_app_sink_pull_preroll(sink);
	if (sample)
		print_buffer(sample, "preroll");

	gst_sample_unref(sample);
	return GST_FLOW_OK;
}

void GetBufferCaps(GstSample *sample, int* width, int* height, int* compression, int* imageSize, int* frameRate,int* bpp)
{
	GstBuffer * buffer = gst_sample_get_buffer(sample);

	GstMapInfo info; 

	gst_buffer_map(buffer, &info, GST_MAP_READ);

	char text[4000];
	text[0] = 0;

	GstCaps *caps = gst_sample_get_caps(sample);
	for (unsigned int j = 0; j < gst_caps_get_size(caps); ++j)
	{
		GstStructure *structure = gst_caps_get_structure(caps, j);
		for (int i = 0; i < gst_structure_n_fields(structure); ++i)
		{
			if (i != 0)
				strcat_s(text, ", ");

			const char *name = gst_structure_nth_field_name(structure, i);
			GType type = gst_structure_get_field_type(structure, name);
			const GValue *value = gst_structure_get_value(structure, name);

			if (strcmp("width", name) == 0)
			{
				*width = value->data->v_int;
			}
			if (strcmp("height", name) == 0)
			{
				*height = value->data->v_int;
			}

			*imageSize = info.size;//GST_BUFFER_SIZE(buffer);
			if (strcmp("format", name) == 0)
			{
				*compression = value->data->v_int;

				 const gchar * image_format = g_value_get_string(value);
				 if (strcmp(image_format, "RGB") == 0 || strcmp(image_format, "BGR") == 0)
					 *bpp = 24;
				 else
					 *bpp = -1;
			}
			if (strcmp("framerate", name) == 0)
			{
				*frameRate = value->data->v_int;
			}
			strcat_s(text, name);
			strcat_s(text, "[");
			strcat_s(text, g_type_name(type));
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
			strcat_s(text, "]");
		}
	}
	
	strcat_s(text, "\n");
	printf(text);

	gst_buffer_unmap (buffer, &info);
	gst_buffer_unref(buffer);

}

static GstFlowReturn new_buffer(GstAppSink *sink, gpointer user_data)
{
	GstMapInfo info;

	GstSample* sample = gst_app_sink_pull_sample(sink);
	GstBuffer* buffer = gst_sample_get_buffer(sample);
	gst_buffer_map(buffer, &info, GST_MAP_READ);

//#ifdef _DEBUG
//	int size = info.size;
//
//	int width, height, compression, imageSize, frameRate, bpp;
//	width = height = compression = imageSize = frameRate = bpp = -1;
//	if (g_this)
//		GetBufferCaps(sample, &width, &height, &compression, &imageSize, &frameRate, &bpp);
//
//	int computedImageSize = width * height * bpp / 8;
//
//	BOOST_ASSERT(width == g_this -> m_iWidth && height == g_this -> m_iHeight && bpp == g_this -> m_iBPP && computedImageSize == size);
//#endif

	if (g_this)
		g_this->CopyDataToClass(info.data, g_this -> m_iWidth, g_this -> m_iHeight, g_this -> m_iBPP);
	
	gst_buffer_unmap (buffer, &info);
	gst_buffer_unref(buffer);
//	gst_sample_unref(sample);

	return GST_FLOW_OK;
}

ReadWithGstreamer::ReadWithGstreamer()
{
	g_this = this;
	m_cImageBuffer = NULL;
	m_iImageBufferSize = 0;
	m_iWidth = m_iHeight = m_iBPP = -1;
	m_bHaveImageData = false;
}


ReadWithGstreamer::~ReadWithGstreamer()
{	
	/* Stop playing */
	gst_element_set_state(pipeline, GST_STATE_NULL);
	gst_object_unref(pipeline);

	if (m_cImageBuffer)
		delete [] m_cImageBuffer;
	m_cImageBuffer = NULL;
	m_iImageBufferSize = 0;

	m_iWidth = m_iHeight = m_iBPP = -1;

	m_bHaveImageData = false;

	g_this = NULL;
}

static gboolean my_bus_callback2 (GstBus *bus, GstMessage *message, gpointer user_data)
{
	BOOST_ASSERT(bus);
	BOOST_ASSERT(message);
	BOOST_ASSERT(user_data);

	g_print("Got %s message\r\n", GST_MESSAGE_TYPE_NAME(message));

	my_bus_callback (bus, message, user_data);

	return TRUE;
}

int ReadWithGstreamer::InitAndBuildPipline(char* url,int bufferSize)
{
	//char s[4000];
	// buffer-duration=10
	//sprintf(s, "playbin uri=%s buffer-size=%d max-size-time=10 use-rate-estimate=25", url, bufferSize);
	
	GError *error = NULL;
	char * playbin_init_str = g_strdup_printf("rtspsrc location=%s ! rtph264depay ! avdec_h264 ! videoconvert ! appsink name=mysink",url);
	pipeline = gst_parse_launch(playbin_init_str, &error);
	g_free(playbin_init_str);
	if (error) {
		gchar* message = g_strdup_printf("Unable to build pipeline: %s", error->message);
		g_clear_error (&error);
		g_free (message);
		return 1;
	}

	GstBus* bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
	gst_bus_set_sync_handler (bus, my_bus_callback, pipeline, my_bus_destroy_notify);
	gst_bus_add_signal_watch(bus);
	gst_bus_add_watch (bus, my_bus_callback2, pipeline);
	gst_object_unref (bus);

	//GstElement* sink = gst_element_factory_make("appsink", "video-sink");
	GstElement* sink = gst_bin_get_by_name(GST_BIN(pipeline), "mysink");
	gst_app_sink_set_emit_signals((GstAppSink*)sink, true);
	gst_app_sink_set_drop((GstAppSink*)sink, true);
	gst_app_sink_set_max_buffers((GstAppSink*)sink, 1);
	//g_object_set(GST_OBJECT(sink), "sync", TRUE, NULL);	// make appsink respect the playout time of the buffers arriving on it.

	//set requierd caps with have color convert to rgb so we would retrive the rgb image instead of yuv.
	GstCaps* video_caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "BGR", NULL);
	gst_app_sink_set_caps((GstAppSink*)sink, video_caps);
	gst_caps_unref(video_caps);

//	/* Set playbin's video sink to be our sink bin */
//	g_object_set(GST_OBJECT(pipeline), "video-sink", sink, NULL);

	// Registering callbacks to appsink element
	GstAppSinkCallbacks callbacks = { on_eos, new_preroll, new_buffer, NULL };
	gst_app_sink_set_callbacks(GST_APP_SINK(sink), &callbacks, this, NULL);

	return 0;
}

void ReadWithGstreamer::StartReadingStream()
{
	GstStateChangeReturn res = gst_element_set_state(pipeline, GST_STATE_PLAYING);

	if (res == GST_STATE_CHANGE_FAILURE)
	{
		g_printerr ("Unable to set the pipeline to the playing state.\n");
		gst_object_unref (pipeline);
	}
	else if (res == GST_STATE_CHANGE_NO_PREROLL)
	{
		//data.is_live = TRUE;
	}
	else if (res == GST_STATE_CHANGE_ASYNC)
	{
		// can happen when buffering occurs
		GstState current, pending;
		res = gst_element_get_state(GST_ELEMENT (pipeline), &current, NULL/*&pending*/, GST_CLOCK_TIME_NONE);
		if(res == GST_STATE_CHANGE_FAILURE || res == GST_STATE_CHANGE_ASYNC)
		{
			g_printerr ("Unable to set the pipeline to the playing state.\n");
			gst_object_unref (pipeline);
		}
	}

	//BOOST_ASSERT(res == GST_STATE_CHANGE_SUCCESS);
}

void ReadWithGstreamer::CopyDataToClass(unsigned char *buffer, int width, int height, int bpp)
{
	int imageSize = width * height * bpp / 8;

	if (!m_cImageBuffer || imageSize > m_iImageBufferSize)
	{
		if (m_cImageBuffer)
			delete [] m_cImageBuffer;

		m_cImageBuffer = new unsigned char[imageSize];
		m_iImageBufferSize = imageSize;
	}

	//copy data
	memcpy(m_cImageBuffer, buffer, imageSize);

	m_bHaveImageData = true;
}

void ReadWithGstreamer::GetImageData(bool &haveData, unsigned char** buffer, int &width, int &height, int &bpp)
{
	haveData = m_bHaveImageData;
	*buffer  = m_cImageBuffer;
	width    = m_iWidth;
	height   = m_iHeight;
	bpp      = m_iBPP;

	m_bHaveImageData = false;
}
