// tutorial02.c
// A pedagogical video player that will stream through every video frame as fast as it can.
//
// Code based on FFplay, Copyright (c) 2003 Fabrice Bellard,
// and a tutorial by Martin Bohme (boehme@inb.uni-luebeckREMOVETHIS.de)
// Tested on Gentoo, CVS version 5/01/07 compiled with GCC 4.1.1
// With updates from https://github.com/chelyaev/ffmpeg-tutorial
// Updates tested on:
// LAVC 54.59.100, LAVF 54.29.104, LSWS 2.1.101, SDL 1.2.15
// on GCC 4.7.2 in Debian February 2015
//
// Use
//
// gcc -o tutorial02 tutorial02.c -lavformat -lavcodec -lswscale -lz -lm `sdl-config --cflags --libs`
// to build (assuming libavformat and libavcodec are correctly installed,
// and assuming you have sdl-config. Please refer to SDL docs for your installation.)
//
// Run using
// tutorial02 myvideofile.mpg
//
// to play the video stream on your screen.

#define __STDC_CONSTANT_MACROS

#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus
};
#endif

#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>

#ifdef __MINGW32__
#undef main /* Prevents SDL from overriding main() */
#endif

#include <stdio.h>

// compatibility with newer API
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc avcodec_alloc_frame
#define av_frame_free avcodec_free_frame
#endif

//SDL1 vs SDL2
//SDL_SetVideoMode()————SDL_CreateWindow()
//SDL_Surface————SDL_Window
//SDL_CreateYUVOverlay()————SDL_CreateTexture()
//SDL_Overlay————SDL_Texture

//程序2:视频播放特别快
//./tutorial02 /root/www/video_sound_data/3.h264_data/408.mp4
int main(int argc, char *argv[]) {
	AVFormatContext *pFormatCtx = NULL;
	int             i, videoStream;
	AVCodecContext  *pCodecCtxOrig = NULL;
	AVCodecContext  *pCodecCtx = NULL;
	AVCodec         *pCodec = NULL;
	AVFrame         *pFrame = NULL;
	AVPacket        packet;
	int             frameFinished;
	float           aspect_ratio;
	struct SwsContext *sws_ctx = NULL;

	SDL_Overlay     *bmp;
	SDL_Surface     *screen;
	SDL_Rect        rect;
	SDL_Event       event;

	if(argc < 2) {
		fprintf(stderr, "Usage: test <file>\n");
		exit(1);
	}
	// Register all formats and codecs
	av_register_all();

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
		exit(1);
	}

	// Open video file
	if(avformat_open_input(&pFormatCtx, argv[1], NULL, NULL)!=0)
		return -1; // Couldn't open file

	// Retrieve stream information
	if(avformat_find_stream_info(pFormatCtx, NULL)<0)
		return -1; // Couldn't find stream information

	// Dump information about file onto standard error
	av_dump_format(pFormatCtx, 0, argv[1], 0);

	// Find the first video stream
	videoStream=-1;
	for(i=0; i<pFormatCtx->nb_streams; i++)
		if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
			videoStream=i;
			break;
		}
	if(videoStream==-1)
		return -1; // Didn't find a video stream

	// Get a pointer to the codec context for the video stream
	pCodecCtxOrig=pFormatCtx->streams[videoStream]->codec;
	// Find the decoder for the video stream
	pCodec=avcodec_find_decoder(pCodecCtxOrig->codec_id);
	if(pCodec==NULL) {
		fprintf(stderr, "Unsupported codec!\n");
		return -1; // Codec not found
	}

	// Copy context
	pCodecCtx = avcodec_alloc_context3(pCodec);
	if(avcodec_copy_context(pCodecCtx, pCodecCtxOrig) != 0) {
		fprintf(stderr, "Couldn't copy codec context");
		return -1; // Error copying codec context
	}

	// Open codec
	if(avcodec_open2(pCodecCtx, pCodec, NULL)<0)
		return -1; // Could not open codec

	// Allocate video frame
	pFrame=av_frame_alloc();

	// Make a screen to put our video
#ifndef __DARWIN__
	printf("__DARWIN__\n");
	screen = SDL_SetVideoMode(pCodecCtx->width, pCodecCtx->height, 0, 0);
#else
	printf("NOT __DARWIN__\n");
	screen = SDL_SetVideoMode(pCodecCtx->width, pCodecCtx->height, 24, 0);
#endif
	if(!screen) {
		fprintf(stderr, "SDL: could not set video mode - exiting\n");
		exit(1);
	}

	//YV12 SDL Texture
	// Allocate a place to put our YUV image on that screen
	bmp = SDL_CreateYUVOverlay(pCodecCtx->width,
			pCodecCtx->height,
			SDL_YV12_OVERLAY,
			screen);

	//获取按照YUV420P来获取
	// initialize SWS context for software scaling
	sws_ctx = sws_getContext(pCodecCtx->width,
			pCodecCtx->height,
			pCodecCtx->pix_fmt,
			pCodecCtx->width,
			pCodecCtx->height,
			AV_PIX_FMT_YUV420P,
			SWS_BILINEAR,
			NULL,
			NULL,
			NULL
			);

	// Read frames and save first five frames to disk
	i=0;
	while(av_read_frame(pFormatCtx, &packet)>=0) {
		// Is this a packet from the video stream?
		if(packet.stream_index==videoStream) {
			// Decode video frame
			avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
			// Did we get a video frame?
			if(frameFinished) {
				SDL_LockYUVOverlay(bmp);
				//yv12转成yuv420p
				//我们锁定这个覆盖，因为我们将要去改写它
				//这个 AVPicture 结构体有一个数据指针指向一个有 4 个元素的指针数组。
				//由于我们处理的是 YUV420P，所以我们只需要 3 个通道即只要三组数据。
				//其它的格式可能需要第四个指针来表示 alpha 通道或者其它参数。
				//在 YUV 覆盖中相同功能的结构体是像素（ pixel）和间距（ pitch）。
				//（“间距”是在 SDL 里用来表示指定行数据宽度的值）。
				//所以我们现在做的是让我们的 pict.data 中的三个数组指针指向我们的覆盖，
				//这样当我们写（数据）到 pict 的时候，实际上是写入到我们的覆盖中，当然要先申请必要的空间。
				//类似的，我们可以直接从覆盖中得到行尺寸信息。
				//像前面一样我们使用 img_convert 来把格式转换成 PIX_FMT_YUV420P
				AVPicture pict;
				pict.data[0] = bmp->pixels[0];
				pict.data[1] = bmp->pixels[2];
				pict.data[2] = bmp->pixels[1];
				pict.linesize[0] = bmp->pitches[0];
				pict.linesize[1] = bmp->pitches[2];
				pict.linesize[2] = bmp->pitches[1];
				// Convert the image into YUV format that SDL uses
				sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data,
						pFrame->linesize, 0, pCodecCtx->height,
						pict.data, pict.linesize);
				SDL_UnlockYUVOverlay(bmp);
				rect.x = 0;
				rect.y = 0;
				rect.w = pCodecCtx->width;
				rect.h = pCodecCtx->height;
				SDL_DisplayYUVOverlay(bmp, &rect);
			}
		}

		// Free the packet that was allocated by av_read_frame
		av_free_packet(&packet);
		//SDL事件（ event）驱动系统。
		//SDL 被设置成当你在 SDL 程序中点击或移动鼠标，或者向它发送一个信号，它都将产生一个事件的驱动方式。
		//如果你的程序想要处理用户输入的话，就检测这些事件。你的程序也可以产生事件并且传递给 SDL 事件系统。
		//当使用 SDL 进行多线程编程的时候，这相当有用，这方面代码我们可以在教程4中看到。
		//在这个程序中，我们将在处理完包以后就立即轮询事 件。
		//现在而言，我们将处理 SDL_QUIT 事件以便于我们退出
		SDL_PollEvent(&event);
		switch(event.type) {
			case SDL_QUIT:
				printf("用户在SDL播放窗口按下x来关闭\n");
				SDL_Quit();
				exit(0);
				break;
			default:
				break;
		}
	}

	// Free the YUV frame
	av_frame_free(&pFrame);

	// Close the codec
	avcodec_close(pCodecCtx);
	avcodec_close(pCodecCtxOrig);

	// Close the video file
	avformat_close_input(&pFormatCtx);

	return 0;
}
