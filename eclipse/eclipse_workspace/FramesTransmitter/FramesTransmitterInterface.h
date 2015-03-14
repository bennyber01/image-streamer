/*
 * FramesTransmitterInterface.h
 *
 *  Created on: 28 בספט 2014
 *      Author: benny
 */

#ifndef FRAMESTRANSMITTERINTERFACE_H_
#define FRAMESTRANSMITTERINTERFACE_H_

class FramesTransmitterInterface
{
public:
	virtual ~FramesTransmitterInterface() {}
	virtual bool Init(char * url, char * port, int frameWidth, int frameHeight, int frameBPP) = 0;
	virtual bool Transmit(unsigned char * buff) = 0;
};

#endif /* FRAMESTRANSMITTERINTERFACE_H_ */




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
