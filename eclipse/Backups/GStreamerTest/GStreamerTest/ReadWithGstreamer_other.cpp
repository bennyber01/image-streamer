#include "stdafx.h"
#include "ReadWithGstreamer.h"
#include <stdio.h>


void GetBufferCaps(GstCaps *caps, int & width, int & height, int & compression, int & frameRate, int & bpp)
{
	width = height = compression = frameRate = bpp = -1;

	char text[4000];
	ZeroMemory(text,4000);
	for (int j = 0; j < gst_caps_get_size(caps); ++j) {

		GstStructure *structure = gst_caps_get_structure(caps, j);
		for (int i = 0; i < gst_structure_n_fields(structure); ++i) {
			if (i != 0)
				strcat(text,", ");
			const char *name = gst_structure_nth_field_name(structure, i);
			GType type = gst_structure_get_field_type(structure, name);
			const GValue *value = gst_structure_get_value(structure, name);
			//serialized.append(QString("%1[%2]").arg(name).arg(g_type_name(type)));
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
				const gchar * image_format = g_value_get_string(value);
				if (strcmp(image_format, "RGB") == 0 || strcmp(image_format, "BGR") == 0)
				{
					bpp = 24;
					compression = 0;
				}
			}
			if (strcmp("framerate", name) == 0)
			{
				frameRate = value->data->v_int;
			}

			strcat(text, name);
			strcat(text, "[");
			strcat(text,  g_type_name(type));
			strcat(text, ":");

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

static void cb_message (GstBus *bus, GstMessage *msg, CustomData *data)
{
	g_print("Got %s message\r\n", GST_MESSAGE_TYPE_NAME(msg));
	char s[1000];
	sprintf_s(s, "Got %s message\r\n", GST_MESSAGE_TYPE_NAME(msg));
	OutputDebugString(s);

	switch (GST_MESSAGE_TYPE (msg)) {
	case GST_MESSAGE_ERROR: {
		GError *err;
		gchar *debug;

		gst_message_parse_error (msg, &err, &debug);
		g_print ("Error: %s\n", err->message);
		sprintf_s(s, "Error: %s\n", err->message);
		OutputDebugString(s);
		g_error_free (err);
		g_free (debug);

		gst_element_set_state (data->pipeline, GST_STATE_READY);
		g_main_loop_quit (data->main_loop);
		break;
							}
	case GST_MESSAGE_EOS:
		/* end-of-stream */
		gst_element_set_state (data->pipeline, GST_STATE_READY);
		g_main_loop_quit (data->main_loop);
		break;
	case GST_MESSAGE_BUFFERING: {
		gint percent = 0;

		/* If the stream is live, we do not care about buffering. */
		if (data->is_live) break;

		gst_message_parse_buffering (msg, &percent);
		g_print ("Buffering (%3d%%)\r", percent);

		sprintf_s(s, "Buffering (%3d%%)\r", percent);
		OutputDebugString(s);

		///* Wait until buffering is complete before start/resume playing */
		//if (percent < 100)
		//	gst_element_set_state (data->pipeline, GST_STATE_PAUSED);
		//else
		//	gst_element_set_state (data->pipeline, GST_STATE_PLAYING);
		break;
								}
	case GST_MESSAGE_CLOCK_LOST:
		/* Get a new clock */
		gst_element_set_state (data->pipeline, GST_STATE_PAUSED);
		gst_element_set_state (data->pipeline, GST_STATE_PLAYING);
		break;
	default:
		/* Unhandled message */
		break;
	}
}

static void on_eos(GstAppSink * sink, gpointer user_data)
{
	int gg=0;
}

GstFlowReturn frame_handler(GstSample* sample, ReadWithGstreamer* pClass)
{
	GstBuffer * buffer = gst_sample_get_buffer(sample);
	GstMapInfo info; 

	gst_buffer_map(buffer, &info, GST_MAP_READ);

	if (pClass)
	{
		if (pClass->IsImageParametersInit() == false)
		{
			int width, height, compression, imageSize, frameRate, bpp;
			GstCaps *caps = gst_sample_get_caps(sample);
			GetBufferCaps(caps, width, height, compression, frameRate, bpp);
			pClass->SetImageParameters(width, height, compression, frameRate, bpp);
		}
	
		pClass->CopyDataToClass(info.data, info.size);
	}

	gst_buffer_unmap (buffer, &info);
	gst_buffer_unref(buffer);

	return GST_FLOW_OK;
}

static GstFlowReturn new_preroll(GstAppSink *sink, gpointer user_data)
{
	ReadWithGstreamer* pClass = (ReadWithGstreamer*)user_data;

	GstSample* sample = gst_app_sink_pull_preroll(sink);

	GstFlowReturn res = frame_handler(sample, pClass);

	gst_sample_unref(sample);

	return res;
}

static GstFlowReturn new_buffer(GstAppSink *sink, gpointer user_data)
{
	ReadWithGstreamer* pClass = (ReadWithGstreamer*)user_data;

	GstSample* sample = gst_app_sink_pull_preroll(sink);

	GstFlowReturn res = frame_handler(sample, pClass);

	//gst_sample_unref(sample);

	return res;
}

ReadWithGstreamer::ReadWithGstreamer()
{
	m_iWidth       = -1;
	m_iheight      = -1;
	m_iCompression = -1;
	m_iBpp         = -1;

	main_loop_thread = NULL;
}

ReadWithGstreamer::~ReadWithGstreamer()
{
	if (streamerData.main_loop)
		g_main_loop_quit (streamerData.main_loop);

	if (main_loop_thread)
		g_thread_join(main_loop_thread);
	main_loop_thread = NULL;
}

bool ReadWithGstreamer::InitAndBuildPipline(char* url)
{
	/* Initialize GStreamer */
	gst_init(NULL, NULL);

	/* Build the pipeline */
	// encoding-name=(string)H264 
	//char * playbin_init_str = g_strdup_printf("playbin uri=%s buffer-size=1000 max-size-time=10 use-rate-estimate=25 -vvv",url);
	char * playbin_init_str = g_strdup_printf("rtspsrc location=%s ! rtph264depay ! avdec_h264 ! videoconvert ! appsink name=mysink",url);
	streamerData.pipeline = gst_parse_launch(playbin_init_str, NULL);
	g_free(playbin_init_str);
	
	//streamerData.video_sink = gst_element_factory_make("appsink", "video-sink");
	streamerData.video_sink = gst_bin_get_by_name(GST_BIN(streamerData.pipeline), "mysink");
	gst_app_sink_set_emit_signals((GstAppSink*)streamerData.video_sink, true);
	gst_app_sink_set_drop((GstAppSink*)streamerData.video_sink, true);
	gst_app_sink_set_max_buffers((GstAppSink*)streamerData.video_sink, 1);		//number of recived buffers in appsink before updating new frame
	//g_object_set((GstAppSink*)streamerData.video_sink, "sync", FALSE/*TRUE*/, NULL);

	//set requierd caps
	GstCaps* video_caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "BGR", NULL);
	gst_app_sink_set_caps((GstAppSink*)streamerData.video_sink, video_caps);
	gst_caps_unref(video_caps);

	//next step - crate bin that would have appsink and converting of image to rgb.
	/* Set playbin's video sink to be our sink bin */
	//g_object_set(GST_OBJECT(streamerData.pipeline), "mysink", streamerData.video_sink, NULL);  //next step instead of attaching the sink itself create a bin that would have color convert to rgb and appsink so we would retrive the rgb image instead of yuv.

	// Registering callbacks to appsink element
	GstAppSinkCallbacks callbacks = { on_eos, new_preroll, new_buffer, NULL };
	gst_app_sink_set_callbacks(GST_APP_SINK(streamerData.video_sink), &callbacks, this, NULL);

	return true;
}

void ReadStreamReadStreamThread(gpointer p)
{
	ReadWithGstreamer* pReader = (ReadWithGstreamer*)p;
	
	g_main_loop_run (pReader -> streamerData.main_loop);

	gst_app_sink_set_emit_signals((GstAppSink*)pReader -> streamerData.video_sink, false);

	/* Free resources */
	gst_element_set_state (pReader -> streamerData.pipeline, GST_STATE_NULL);
	gst_object_unref (pReader -> streamerData.pipeline);
	gst_object_unref (pReader -> streamerData.bus);
	g_main_loop_unref (pReader -> streamerData.main_loop);

	pReader -> streamerData.video_sink = NULL;
	pReader -> streamerData.pipeline   = NULL;
	pReader -> streamerData.bus        = NULL;
	pReader -> streamerData.main_loop  = NULL;
}

bool ReadWithGstreamer::StartReadingStream()
{
	streamerData.is_live = false;

	GstStateChangeReturn res = gst_element_set_state (streamerData.pipeline, GST_STATE_PLAYING);

	if (res == GST_STATE_CHANGE_FAILURE)
	{
		g_printerr ("Unable to set the pipeline to the playing state.\n");
		gst_object_unref (streamerData.pipeline);
		streamerData.pipeline = NULL;
		return false;
	}
	else if (res == GST_STATE_CHANGE_NO_PREROLL)
	{
		streamerData.is_live = true;
	}
	else if (res == GST_STATE_CHANGE_ASYNC)
	{
		// can happen when buffering occurs
		GstState current, pending;
		res = gst_element_get_state(GST_ELEMENT (streamerData.pipeline), &current, &pending, GST_CLOCK_TIME_NONE);
		if(res == GST_STATE_CHANGE_FAILURE || res == GST_STATE_CHANGE_ASYNC)
		{
			g_printerr ("Unable to set the pipeline to the playing state.\n");
			gst_object_unref (streamerData.pipeline);
			streamerData.pipeline = NULL;
			return false;
		}
	}

	streamerData.bus = gst_element_get_bus(streamerData.pipeline);

	streamerData.main_loop = g_main_loop_new (NULL, FALSE);

	gst_bus_add_signal_watch (streamerData.bus);
	g_signal_connect (streamerData.bus, "message", G_CALLBACK (cb_message), &streamerData);

	main_loop_thread = g_thread_create((GThreadFunc)ReadStreamReadStreamThread, this, true, NULL);

	return true;
}

void ReadWithGstreamer::SetImageParameters(int width, int height, int compression, int frameRate,int bpp)
{
	m_iheight = height;
	m_iWidth = width;
	if (compression == -1)
		compression = 0;
	m_iCompression = compression;
	m_iBpp = bpp;
}

void ReadWithGstreamer::CopyDataToClass(unsigned char *buffer, int imageSize)
{
	if (!_picBufferHolder.IsInit())
		_picBufferHolder.CreateImageBuffers(imageSize);

	_picBufferHolder.PutNewImage(buffer, imageSize);
}

bool ReadWithGstreamer::GetImageData(unsigned char** buffer, int &width, int &height, int &compression, int &imageSize, int &bpp)
{
	*buffer = NULL;

	if (m_iWidth < 0)
		return false;

	FrameImageInfo* frameGrabberInfo = _picBufferHolder.GetNextFrame();

	if(!frameGrabberInfo)
		return false;

	*buffer     = frameGrabberInfo -> _buffer;
	imageSize   = frameGrabberInfo -> _imageSize;

	width       = m_iWidth;
	height      = m_iheight;
	compression = m_iCompression;
	bpp         = m_iBpp;

	return true;
}
