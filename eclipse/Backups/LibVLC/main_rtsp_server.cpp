#include <stdio.h>
#include <stdlib.h>
#include <vlc/vlc.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>  // cvWaitKey
#include <string>
#include <algorithm>

#include <windows.h>		// Sleep

const int IMAGE_WIDTH = 320;
const int IMAGE_HEIGHT = 240;
const int IMAGE_BPP = 24;
const int IMAGE_COLOR_CHANNELS = IMAGE_BPP / 8;

struct MyImemData
{
    MyImemData()
	{
		image = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, IMAGE_COLOR_CHANNELS);
	
		buffSize = sizeof(char) * IMAGE_WIDTH * IMAGE_HEIGHT * IMAGE_COLOR_CHANNELS;

		mDts = mPts = 0;
	}

	char * GenerateImage()
	{
		memset(image -> imageData, 0, buffSize);

		static int frameCounter = 0;

		char str[100];   
		sprintf(str,"[%04d]", frameCounter++);
		CvFont font;
		cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 1, 1, 0, 2, 8);
		cvPutText(image, str, cvPoint(10,40), &font, cvScalar(255,255,0)); 

		return image -> imageData;
	}

    ~MyImemData()
	{
		cvReleaseImage(&image);
	}

	IplImage * image;
	int buffSize;

    int64_t mDts;
    int64_t mPts;
};


/**
    \brief Callback method triggered by VLC to get image data from
    a custom memory source. This is used to tell VLC where the 
    data is and to allocate buffers as needed.

    To set this callback, use the "--imem-get=<memory_address>" 
    option, with memory_address the address of this function in memory.

    When using IMEM, be sure to indicate the format for your data
    using "--imem-cat=2" where 2 is video. Other options for categories are
    0 = Unknown,
    1 = Audio,
    2 = Video,
    3 = Subtitle,
    4 = Data

    When creating your media instance, use libvlc_media_new_location and
    set the location to "imem:/" and then play.

    \param[in] data Pointer to user-defined data, this is your data that
    you set by passing the "--imem-data=<memory_address>" option when
    initializing VLC instance.
    \param[in] cookie A user defined string. This works the same way as
    data, but for string. You set it by adding the "--imem-cookie=<your_string>"
    option when you initialize VLC. Use this when multiple VLC instances are
    running.
    \param[out] dts The decode timestamp, value is in microseconds. This value
    is the time when the frame was decoded/generated. For example:
	30 fps : video would be every 33 ms, so values would be 0, 33333, 66666, 99999, etc.
	25 fps : video would be every 40 ms, so values would be 0, 40000, 80000, 120000, etc.
    \param[out] pts The presentation timestamp, value is in microseconds. This
    value tells the receiver when to present the frame. For example:
	30 fps : video would be every 33 ms, so values would be 0, 33333, 66666, 99999, etc.
	25 fps : video would be every 40 ms, so values would be 0, 40000, 80000, 120000, etc.
    \param[out] flags Unused,ignore.
    \param[out] bufferSize Use this to set the size of the buffer in bytes.
    \param[out] buffer Change to point to your encoded frame/audio/video data. 
        The codec format of the frame is user defined and set using the
        "--imem-codec=<four_letter>," where 4 letter is the code for your
        codec of your source data.
*/
int MyImemGetCallback (void *data, 
                       const char *cookie, 
                       int64_t *dts, 
                       int64_t *pts, 
                       unsigned *flags, 
                       size_t * bufferSize,
                       void ** buffer)
{
    MyImemData * imem = (MyImemData*) data;

    if(imem == NULL)
        return 1;

    // Changing this value will impact the playback
    // rate on the receiving end (if they use the dts and pts values).
    //int64_t uS = 33333; // 30 fps
    int64_t uS = 40000; // 25 fps

    *bufferSize = imem -> buffSize;
    *buffer = imem -> GenerateImage();
    *dts = *pts = imem -> mDts = imem -> mPts = imem -> mPts + uS;

	cvShowImage("Display window", imem -> image);
	cvWaitKey(1);

    return 0;
}


/**
    \brief Callback method triggered by VLC to release memory allocated
    during the GET callback.

    To set this callback, use the "--imem-release=<memory_address>" 
    option, with memory_address the address of this function in memory.

    \param[in] data Pointer to user-defined data, this is your data that
    you set by passing the "--imem-data=<memory_address>" option when
    initializing VLC instance.
    \param[in] cookie A user defined string. This works the same way as
    data, but for string. You set it by adding the "--imem-cookie=<your_string>"
    option when you initialize VLC. Use this when multiple VLC instances are
    running.
    \param[int] bufferSize The size of the buffer in bytes.
    \param[out] buffer Pointer to data you allocated or set during the GET
    callback to handle  or delete as needed.
*/
int MyImemReleaseCallback (void *data, 
                           const char *cookie, 
                           size_t bufferSize, 
                           void * buffer)
{
    // Since I did not allocate any new memory, I don't need
    // to delete it here. However, if you did in your get method, you
    // should delete/free it here.
    return 0;
}


int main(int argc, char* argv[])
{
	// rtsp://192.168.1.62:5544/

	//////const char *media_name = "video";

	//////libvlc_instance_t *vlc;
	//////const char *url = "imem://";
	//////const char *sout = "#transcode{venc=x264{preset=ultrafast},vcodec=h264,vb=0,vbv-bufsize=1200,bframes=0,acodec=none}:rtp{sdp=rtsp://:5544/}";

	MyImemData data;

	char imemDataArg[256];
    sprintf(imemDataArg, "--imem-data=%#p", &data);

    char imemGetArg[256];
    sprintf(imemGetArg, "--imem-get=%#p", MyImemGetCallback);

    char imemReleaseArg[256];
    sprintf(imemReleaseArg, "--imem-release=%#p", MyImemReleaseCallback);

    // If using RAW image data, like RGB24, then you
    // must specify the width, height, and number of channels
    // to IMEM. Other codes may have that information within
    // the data buffer, but RAW will not.
    char imemWidthArg[256];
    sprintf(imemWidthArg, "--imem-width=%d", IMAGE_WIDTH);

    char imemHeightArg[256];
    sprintf(imemHeightArg, "--imem-height=%d", IMAGE_HEIGHT);

    char imemChannelsArg[256];
    sprintf(imemChannelsArg, "--imem-channels=%d", IMAGE_COLOR_CHANNELS);

	const char* const vlc_args[] = {
									//"--no-audio",			// make problems
									"--ignore-config",
									//"--sout-avcodec-strict=-2",
									"--no-video-title-show",
									imemDataArg,
									imemGetArg,
									imemReleaseArg,
									"--imem-cookie=\"IMEM\"",
									"--imem-codec=RV24",
									"--imem-cat=2",
									imemWidthArg,
									imemHeightArg,
									imemChannelsArg,
									};

	libvlc_instance_t * vlc = libvlc_new(sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);

	// Create a media item from file
    libvlc_media_t * media = libvlc_media_new_location (vlc, "imem://");

    // Stream as MPEG2 via RTSP
    //options.push_back(":sout=#transcode{venc=ffmpeg{keyint=1,min-keyint=1,tune=zerolatency,bframes=0,vbv-bufsize=1200}, vcodec=mp2v,vb=800}:rtp{sdp=rtsp://:1234/BigDog}");

    // Stream as MJPEG (Motion JPEG) to http destination. MJPEG encoder
    // does not currently support RTSP
    //options.push_back(":sout=#transcode{vcodec=MJPG,vb=800,scale=1,acodec=none}:duplicate{dst=std{access=http,mux=mpjpeg,noaudio,dst=:1234/BigDog.mjpg}");

    // Convert to H264 and stream via RTSP
    //options.push_back(":sout=#transcode{vcodec=h264,venc=x264,vb=0,vbv-bufsize=1200,bframes=0,scale=0,acodec=none}:rtp{sdp=rtsp://:1234/BigDog}");

    //// Set media options
    //for(option = options.begin(); option != options.end(); option++)
    //{
    //    libvlc_media_add_option(media, *option);
    //}
	
	const char *option = ":sout=#transcode{venc=x264{preset=ultrafast},vcodec=h264,vb=0,bframes=0,keyint=1,acodec=none}:rtp{sdp=rtsp://:5544/}";
	libvlc_media_add_option(media, option);

    // Create a media player playing environment 
    libvlc_media_player_t * mediaPlayer = libvlc_media_player_new_from_media (media);

    // No need to keep the media now
    libvlc_media_release (media);

    // play the media_player
    libvlc_media_player_play (mediaPlayer);
	
	Sleep (120000); /* Let it play a bit */

    // Stop playing 
    libvlc_media_player_stop (mediaPlayer);

    // Free the media_player 
    libvlc_media_player_release (mediaPlayer);

    // Free vlc
    libvlc_release (vlc);

	cvDestroyWindow("Display window");

	return 0;

	//////vlc = libvlc_new(sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);

	//////libvlc_vlm_add_broadcast(vlc, media_name, url, sout, 0, NULL, true, false);
	//////libvlc_vlm_play_media(vlc, media_name);

	//////Sleep (120000); /* Let it play a bit */

	//////libvlc_vlm_stop_media(vlc, media_name);
	//////libvlc_vlm_release(vlc);

	//////cvDestroyWindow("Display window");

	//////return 0;
}

//================================================================================================

//// Local file/media source.
//std::string IMEM_SOURCE_FOLDER = "settings/rvideo/samples/bigdog";
//
//class MyImemData
//{
//public:
//    MyImemData() : mFrame(0), mDts(0), mPts(0) {}
//    ~MyImemData() {}
//    std::vector<cv::Mat> mImages;
//    std::size_t mFrame;
//    int64_t mDts;
//    int64_t mPts;
//};
//
//
///**
//    \brief Callback method triggered by VLC to get image data from
//    a custom memory source. This is used to tell VLC where the 
//    data is and to allocate buffers as needed.
//
//    To set this callback, use the "--imem-get=<memory_address>" 
//    option, with memory_address the address of this function in memory.
//
//    When using IMEM, be sure to indicate the format for your data
//    using "--imem-cat=2" where 2 is video. Other options for categories are
//    0 = Unknown,
//    1 = Audio,
//    2 = Video,
//    3 = Subtitle,
//    4 = Data
//
//    When creating your media instance, use libvlc_media_new_location and
//    set the location to "imem:/" and then play.
//
//    \param[in] data Pointer to user-defined data, this is your data that
//    you set by passing the "--imem-data=<memory_address>" option when
//    initializing VLC instance.
//    \param[in] cookie A user defined string. This works the same way as
//    data, but for string. You set it by adding the "--imem-cookie=<your_string>"
//    option when you initialize VLC. Use this when multiple VLC instances are
//    running.
//    \param[out] dts The decode timestamp, value is in microseconds. This value
//    is the time when the frame was decoded/generated. For example, 30 fps 
//    video would be every 33 ms, so values would be 0, 33333, 66666, 99999, etc.
//    \param[out] pts The presentation timestamp, value is in microseconds. This
//    value tells the receiver when to present the frame. For example, 30 fps 
//    video would be every 33 ms, so values would be 0, 33333, 66666, 99999, etc.
//    \param[out] flags Unused,ignore.
//    \param[out] bufferSize Use this to set the size of the buffer in bytes.
//    \param[out] buffer Change to point to your encoded frame/audio/video data. 
//        The codec format of the frame is user defined and set using the
//        "--imem-codec=<four_letter>," where 4 letter is the code for your
//        codec of your source data.
//*/
//int MyImemGetCallback (void *data, 
//                       const char *cookie, 
//                       int64_t *dts, 
//                       int64_t *pts, 
//                       unsigned *flags, 
//                       size_t * bufferSize,
//                       void ** buffer)
//{
//    MyImemData* imem = (MyImemData*)data;
//
//    if(imem == NULL)
//        return 1;
//    // Loop...
//    if(imem->mFrame >= imem->mImages.size())
//    {
//        imem->mFrame = 0;
//    }
//    // Changing this value will impact the playback
//    // rate on the receiving end (if they use the dts and pts values).
//    int64_t uS = 33333; // 30 fps
//
//    cv::Mat img = imem->mImages[imem->mFrame++];
//    *bufferSize = img.rows*img.cols*img.channels();
//    *buffer = img.data;
//    *dts = *pts = imem->mDts = imem->mPts = imem->mPts + uS;
//
//    return 0;
//}
//
//
///**
//    \brief Callback method triggered by VLC to release memory allocated
//    during the GET callback.
//
//    To set this callback, use the "--imem-release=<memory_address>" 
//    option, with memory_address the address of this function in memory.
//
//    \param[in] data Pointer to user-defined data, this is your data that
//    you set by passing the "--imem-data=<memory_address>" option when
//    initializing VLC instance.
//    \param[in] cookie A user defined string. This works the same way as
//    data, but for string. You set it by adding the "--imem-cookie=<your_string>"
//    option when you initialize VLC. Use this when multiple VLC instances are
//    running.
//    \param[int] bufferSize The size of the buffer in bytes.
//    \param[out] buffer Pointer to data you allocated or set during the GET
//    callback to handle  or delete as needed.
//*/
//int MyImemReleaseCallback (void *data, 
//                           const char *cookie, 
//                           size_t bufferSize, 
//                           void * buffer)
//{
//    // Since I did not allocate any new memory, I don't need
//    // to delete it here. However, if you did in your get method, you
//    // should delete/free it here.
//    return 0;
//}
//
//
///**
//    \brief Method to load a series of images to use as raw image data
//    for the network stream.
//
//    \param[in] sourceFolder Path to folder containing jpeg or png images.
//*/
//std::vector<cv::Mat> GetRawImageData(const std::string& sourceFolder)
//{
//    namespace fs = boost::filesystem;
//    std::vector<cv::Mat> result;
//    std::vector<std::string> filenames;
//    if( fs::exists(sourceFolder) && fs::is_directory(sourceFolder) )
//    {
//        for(fs::directory_iterator dir(sourceFolder);
//            dir != fs::directory_iterator();
//            dir++)
//        {
//            std::string ext = dir->path().extension().string();
//            if( fs::is_regular_file( dir->status() ) &&
//                (dir->path().extension() == ".jpeg" ||
//                 dir->path().extension() == ".png") )
//            {
//                filenames.push_back(dir->path().string());
//            }
//        }
//    }
//
//    if(filenames.size() > 0)
//    {
//        // Sort from 0 to N
//        std::sort(filenames.begin(), filenames.end());
//        std::vector<std::string>::iterator filename;
//        for(filename = filenames.begin();
//            filename != filenames.end();
//            filename++)
//        {
//            cv::Mat img = cv::imread(*filename);
//            result.push_back(img);
//        }
//    }
//    return result;
//}
//
//
//int main(int argc, char* argv[])
//{
//
//    // Load images first since we need to know
//    // the size of the image data for IMEM
//    MyImemData data;
//    data.mImages = 
//        GetRawImageData(IMEM_SOURCE_FOLDER);
//
//    if(data.mImages.size() == 0)
//    {
//        std::cout << "No images found to render/stream.";
//        return 0;
//    }
//
//    int w, h, channels;
//    w = data.mImages.front().cols;
//    h = data.mImages.front().rows;
//    channels = data.mImages.front().channels();
//
//    // You must create an instance of the VLC Library
//    libvlc_instance_t * vlc;
//    // You need a player to play media
//    libvlc_media_player_t *mediaPlayer;
//    // Media object to play.
//    libvlc_media_t *media;
//
//    // Configure options for this instance of VLC (global settings).
//    // See VLC command line documentation for options.
//    std::vector<const char*> options;
//    std::vector<const char*>::iterator option;
//    options.push_back("--no-video-title-show");
//
//    char imemDataArg[256];
//    sprintf(imemDataArg, "--imem-data=%#p", &data);
//    options.push_back(imemDataArg);
//
//    char imemGetArg[256];
//    sprintf(imemGetArg, "--imem-get=%#p", MyImemGetCallback);
//    options.push_back(imemGetArg);
//
//    char imemReleaseArg[256];
//    sprintf(imemReleaseArg, "--imem-release=%#p", MyImemReleaseCallback);
//    options.push_back(imemReleaseArg);
//
//    options.push_back("--imem-cookie=\"IMEM\"");
//    // Codec of data in memory for IMEM, raw 3 channel RGB images is RV24
//    options.push_back("--imem-codec=RV24");
//    // Video data.
//    options.push_back("--imem-cat=2");
//
//    // If using RAW image data, like RGB24, then you
//    // must specify the width, height, and number of channels
//    // to IMEM. Other codes may have that information within
//    // the data buffer, but RAW will not.
//    char imemWidthArg[256];
//    sprintf(imemWidthArg, "--imem-width=%d", w);
//    options.push_back(imemWidthArg);
//
//    char imemHeightArg[256];
//    sprintf(imemHeightArg, "--imem-height=%d", h);
//    options.push_back(imemHeightArg);
//
//    char imemChannelsArg[256];
//    sprintf(imemChannelsArg, "--imem-channels=%d", channels);
//    options.push_back(imemChannelsArg);
//
//    //options.push_back("--verbose=2");
//
//    // Load the VLC engine
//    vlc = libvlc_new (int(options.size()), options.data());
//
//    // Create a media item from file
//    media = libvlc_media_new_location (vlc, "imem://");
//
//    // Configure any transcoding or streaming
//    // options for the media source.
//    options.clear();
//
//    // Stream as MPEG2 via RTSP
//    //options.push_back(":sout=#transcode{venc=ffmpeg{keyint=1,min-keyint=1,tune=zerolatency,bframes=0,vbv-bufsize=1200}, vcodec=mp2v,vb=800}:rtp{sdp=rtsp://:1234/BigDog}");
//
//    // Stream as MJPEG (Motion JPEG) to http destination. MJPEG encoder
//    // does not currently support RTSP
//    //options.push_back(":sout=#transcode{vcodec=MJPG,vb=800,scale=1,acodec=none}:duplicate{dst=std{access=http,mux=mpjpeg,noaudio,dst=:1234/BigDog.mjpg}");
//
//    // Convert to H264 and stream via RTSP
//    options.push_back(":sout=#transcode{vcodec=h264,venc=x264,vb=0,vbv-bufsize=1200,bframes=0,scale=0,acodec=none}:rtp{sdp=rtsp://:1234/BigDog}");
//
//    // Set media options
//    for(option = options.begin(); option != options.end(); option++)
//    {
//        libvlc_media_add_option(media, *option);
//    }
//
//    // Create a media player playing environment 
//    mediaPlayer = libvlc_media_player_new_from_media (media);
//
//    // No need to keep the media now
//    libvlc_media_release (media);
//
//    // play the media_player
//    libvlc_media_player_play (mediaPlayer);
//
//    boost::this_thread::sleep(boost::posix_time::milliseconds(60000));
//
//    // Stop playing 
//    libvlc_media_player_stop (mediaPlayer);
//
//    // Free the media_player 
//    libvlc_media_player_release (mediaPlayer);
//    // Free vlc
//    libvlc_release (vlc);
//
//    return 0;
//}

//================================================================================================

//#include <vlc/vlc.h>
//void *vlc_lock_fn(void *data)
//{
//
//   pthread_mutex_lock(&blurred_mutex);
//   return p_rgb_blurred;
//}
//
//void vlc_unlock_fn(void *data)
//{
//   pthread_mutex_unlock(&blurred_mutex);
//}
//
//
//libvlc_instance_t* vlc_inst;
//static const char* vlc_args[] = {
//                                   "--codec", "invmem",
//                                   "-vvvvv",
//                                   "-I", "dummy",
//                                   "--no-ipv6",
//                                   "--ipv4",
//                                   "--file-logging",
//                                   "--drop-late-frames",
//                                   "--skip-frames", "--nocolor", "--rtsp-caching=100",
//                                   "--logfile", "/tmp/mylogfile",":no-sout-rtp-sap", ":no-sout-standard-sap", ":sout-keep",
//                                   };
//
//int init_video_out() {
//   vlc_inst = libvlc_new(sizeof(vlc_args)/sizeof(vlc_args[0]), vlc_args);
//   if(vlc_inst == NULL) {
//     //your error handler
//   }
//   
//   char swidth[50], sheight[50], slock[50], sunlock[50], sdata[50];
//   char sinput[10];
//   
//   sprintf(swidth, ":invmem-width=%d", 640);
//   sprintf(sheight, ":invmem-height=%d", 480);
//   sprintf(slock, ":invmem-lock=%ld", (intptr_t)vlc_lock_fn);
//   sprintf(sunlock, ":invmem-unlock=%ld", (intptr_t)vlc_unlock_fn);
//   sprintf(sdata, ":invmem-data=%ld", (uintptr_t)NULL);
//   
//   const char* argvideo[] = {
//                                       swidth,
//                                       sheight,
//                                       slock,
//                                       sunlock,
//                                       sdata
//                                   };
//   
//   sprintf(sinput, "fake:// :input-slave=alsa://");
//   
//   libvlc_vlm_add_broadcast(vlc_inst,
//                                        "myapp",
//                                         sinput,
//                                         "#transcode{vcodec=mp2v,vb=1200,scale=1,acodec=mpga,ab=128,channels=2,samplerate=44100}:duplicate{dst=rtp{sdp=rtsp://:5544/}}",
//                                         sizeof(argvideo)/sizeof(argvideo[0]),
//                                         argvideo,
//                                         1,
//                                         0);
//
//   libvlc_vlm_play_media (vlc_inst, "myappt");
//
//}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

//int main(int argc, char* argv[])
//{
//	libvlc_instance_t * inst;
//	libvlc_media_player_t *mp;
//	libvlc_media_t *m;
//
//	/* Load the VLC engine */
//	inst = libvlc_new (0, NULL);
//
//	
//	// Samples in the wild
//	// http://wiki.multimedia.cx/index.php?title=RTSP
//
//	char * URL = "rtsp://184.72.239.149/vod/mp4:BigBuckBunny_175k.mov";
//
//	//char * URL = "rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mov";
//	//char * URL = "rtsp://127.0.0.1:5541/test";
//
//	//char * URL = "rtsp://46.249.213.87/broadcast/bollywoodhungama-tablet.3gp";
//	//char * URL = "rtsp://218.248.64.82.554:/";
//	//char * URL = "rtsp://videocdn-us.geocdn.scaleengine.net/jblive/live/jblive.stream";
//
//	//char * URL = "rtsp://localhost:554";
//
//	//char * URL = "rtsp://admin:9999@10.200.1.222:554/live/stream1";
//
//	/* Create a new item */
//	m = libvlc_media_new_location (inst, URL);
//	//m = libvlc_media_new_location (inst, "rtsp://localhost:554");
//
//	/* Create a media player playing environement */
//	mp = libvlc_media_player_new_from_media (m);
//
//	/* No need to keep the media now */
//	libvlc_media_release (m);
//
//#if 0
//	/* This is a non working code that show how to hooks into a window,
//	* if we have a window around */
//	libvlc_media_player_set_xwindow (mp, xid);
//	/* or on windows */
//	libvlc_media_player_set_hwnd (mp, hwnd);
//	/* or on mac os */
//	libvlc_media_player_set_nsobject (mp, view);
//#endif
//
//	/* play the media_player */
//	libvlc_media_player_play (mp);
//
//	Sleep (30000); /* Let it play a bit */
//
//	/* Stop playing */
//	libvlc_media_player_stop (mp);
//
//	/* Free the media_player */
//	libvlc_media_player_release (mp);
//
//	libvlc_release (inst);
//
//	return 0;
//}


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
