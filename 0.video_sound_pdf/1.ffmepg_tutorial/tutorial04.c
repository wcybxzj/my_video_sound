// tutorial04.c
// A pedagogical video player that will stream through every video frame as fast as it can,
// and play audio (out of sync).
//
// Code based on FFplay, Copyright (c) 2003 Fabrice Bellard,
// and a tutorial by Martin Bohme (boehme@inb.uni-luebeckREMOVETHIS.de)
// Tested on Gentoo, CVS version 5/01/07 compiled with GCC 4.1.1
// With updates from https://github.com/chelyaev/ffmpeg-tutorial
// Updates tested on:
// LAVC 54.59.100, LAVF 54.29.104, LSWS 2.1.101, SDL 1.2.15
// on GCC 4.7.2 in Debian February 2015
// Use
//
// gcc -o tutorial04 tutorial04.c -lavformat -lavcodec -lswscale -lz -lm `sdl-config --cflags --libs`
// to build (assuming libavformat and libavcodec are correctly installed,
// and assuming you have sdl-config. Please refer to SDL docs for your installation.)
//
// Run using
// tutorial04 myvideofile.mpg
//
// to play the video stream on your screen.

#define __STDC_CONSTANT_MACROS

//Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/avstring.h>
#ifdef __cplusplus
};
#endif

#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>

#ifdef __MINGW32__
#undef main /* Prevents SDL from overriding main() */
#endif

#include <stdio.h>
#include <assert.h>
#include <math.h>

// compatibility with newer API
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc avcodec_alloc_frame
#define av_frame_free avcodec_free_frame
#endif

#define SDL_AUDIO_BUFFER_SIZE 1024
//#define MAX_AUDIO_FRAME_SIZE 192000
#define MAX_AUDIO_FRAME_SIZE 22050

#define MAX_AUDIOQ_SIZE (5 * 16 * 1024)
#define MAX_VIDEOQ_SIZE (5 * 256 * 1024)

#define FF_REFRESH_EVENT (SDL_USEREVENT)
#define FF_QUIT_EVENT (SDL_USEREVENT + 1)

#define VIDEO_PICTURE_QUEUE_SIZE 1

typedef struct PacketQueue {
	AVPacketList *first_pkt, *last_pkt;
	int nb_packets;
	int size;
	SDL_mutex *mutex;
	SDL_cond *cond;
} PacketQueue;

typedef struct VideoPicture {
	SDL_Overlay *bmp;
	int width, height; /* source height & width */
	int allocated;
} VideoPicture;

//所有关于电影的信息
typedef struct VideoState {
	AVFormatContext *pFormatCtx;//封装上下文
	int             videoStream, audioStream;
	AVStream        *audio_st;
	AVCodecContext  *audio_ctx;
	PacketQueue     audioq;//音频队列
	uint8_t         audio_buf[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
	unsigned int    audio_buf_size;
	unsigned int    audio_buf_index;
	AVFrame         audio_frame;
	AVPacket        audio_pkt;
	uint8_t         *audio_pkt_data;
	int             audio_pkt_size;
	AVStream        *video_st;
	AVCodecContext  *video_ctx;
	PacketQueue     videoq;//视频队列
	struct SwsContext *sws_ctx;
	VideoPicture    pictq[VIDEO_PICTURE_QUEUE_SIZE];
	int             pictq_size, pictq_rindex, pictq_windex;
	SDL_mutex       *pictq_mutex;
	SDL_cond        *pictq_cond;
	SDL_Thread      *parse_tid;
	SDL_Thread      *video_tid;
	char            filename[1024];
	int             quit;
} VideoState;

SDL_Surface     *screen;
SDL_mutex       *screen_mutex;

/* Since we only have one decoding thread, the Big Struct
   can be global in case we need it. */
VideoState *global_video_state;

void packet_queue_init(PacketQueue *q) {
	memset(q, 0, sizeof(PacketQueue));
	q->mutex = SDL_CreateMutex();
	q->cond = SDL_CreateCond();
}

int packet_queue_put(PacketQueue *q, AVPacket *pkt) {
	AVPacketList *pkt1;
	if(av_dup_packet(pkt) < 0) {
		return -1;
	}
	pkt1 = (AVPacketList*)av_malloc(sizeof(AVPacketList));
	if (!pkt1)
		return -1;
	pkt1->pkt = *pkt;
	pkt1->next = NULL;
	SDL_LockMutex(q->mutex);
	if (!q->last_pkt)
		q->first_pkt = pkt1;
	else
		q->last_pkt->next = pkt1;
	q->last_pkt = pkt1;
	q->nb_packets++;
	q->size += pkt1->pkt.size;
	SDL_CondSignal(q->cond);
	SDL_UnlockMutex(q->mutex);
	return 0;
}

static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block)
{
	AVPacketList *pkt1;
	int ret;
	SDL_LockMutex(q->mutex);
	for(;;) {
		if(global_video_state->quit) {
			ret = -1;
			break;
		}
		pkt1 = q->first_pkt;
		if (pkt1) {
			q->first_pkt = pkt1->next;
			if (!q->first_pkt)
				q->last_pkt = NULL;
			q->nb_packets--;
			q->size -= pkt1->pkt.size;
			*pkt = pkt1->pkt;
			av_free(pkt1);
			ret = 1;
			break;
		} else if (!block) {
			ret = 0;
			break;
		} else {
			SDL_CondWait(q->cond, q->mutex);
		}
	}
	SDL_UnlockMutex(q->mutex);
	return ret;
}//end packet_queue_get()

//is->audio_pkt_size:46, len1:46
//is->audio_pkt_size:0
//is->audio_pkt_size:6, len1:6
//is->audio_pkt_size:0
//is->audio_pkt_size:14, len1:14
//is->audio_pkt_size:0
//is->audio_pkt_size:610, len1:610
//is->audio_pkt_size:0
//is->audio_pkt_size:557, len1:557
//is->audio_pkt_size:0

/*
while (1) {
	while(已经读取到audio packet的长度){
		获取到的话尝试进行解码
	}
	没获取到的话从音频队列获取
}
*/
//audio_buf:回填的音频buf
//buf_size:回填音频buf的大小
int audio_decode_frame(VideoState *is, uint8_t *audio_buf, int buf_size) {
	int len1, data_size = 0;
	AVPacket *pkt = &is->audio_pkt;
	for(;;) {
		while(is->audio_pkt_size > 0) {
			int got_frame = 0;
			len1 = avcodec_decode_audio4(is->audio_ctx, &is->audio_frame, &got_frame, pkt);
			//解码出现错误跳过这个frame
			if(len1 < 0) {
				/* if error, skip frame */
				is->audio_pkt_size = 0;
				break;
			}
			data_size = 0;
			//获取到一个完整frame
			if(got_frame) {
				data_size = av_samples_get_buffer_size(NULL,
						is->audio_ctx->channels,
						is->audio_frame.nb_samples,
						is->audio_ctx->sample_fmt,
						1);
				assert(data_size <= buf_size);
				memcpy(audio_buf, is->audio_frame.data[0], data_size);
			}
			//如果一个packet解码后还剩余数据还可以再次解码
			is->audio_pkt_data += len1;
			is->audio_pkt_size -= len1;
			//没能获取当一个完整的frame数据,要继续获取packet然后解码
			if(data_size <= 0) {
				/* No data yet, get more frames */
				continue;
			}
			/* We have data, return it and come back for more later */
			return data_size;
		}
		//对应上边while中解码出现问题的情况
		//在这里需要将这种frame释放掉
		if(pkt->data){
			av_free_packet(pkt);
		}
		if(is->quit) {
			return -1;
		}
		/* next packet */
		if(packet_queue_get(&is->audioq, pkt, 1) < 0) {
			return -1;
		}
		is->audio_pkt_data = pkt->data;
		is->audio_pkt_size = pkt->size;
	}
}//end audio_decode_frame()


//通过回调将数据写入stream
//len指明stream的大小
void audio_callback(void *userdata, Uint8 *stream, int len) {
	VideoState *is = (VideoState *)userdata;
	int len1, audio_size;
	while(len > 0) {
		if(is->audio_buf_index >= is->audio_buf_size) {
			/* We have already sent all our data; get more */
			audio_size = audio_decode_frame(is, is->audio_buf, sizeof(is->audio_buf));
			if(audio_size < 0) {
				/* If error, output silence */
				is->audio_buf_size = 1024;
				memset(is->audio_buf, 0, is->audio_buf_size);
			} else {
				is->audio_buf_size = audio_size;
			}
			is->audio_buf_index = 0;
		}
		len1 = is->audio_buf_size - is->audio_buf_index;
		if(len1 > len)
			len1 = len;
		memcpy(stream, (uint8_t *)is->audio_buf + is->audio_buf_index, len1);
		len -= len1;
		stream += len1;
		is->audio_buf_index += len1;
	}
}

static Uint32 sdl_refresh_timer_cb(Uint32 interval, void *opaque) {
	SDL_Event event;
	event.type = FF_REFRESH_EVENT;
	event.user.data1 = opaque;
	SDL_PushEvent(&event);
	return 0; /* 0 means stop timer */
}

/* schedule a video refresh in 'delay' ms */
static void schedule_refresh(VideoState *is, int delay) {
	SDL_AddTimer(delay, sdl_refresh_timer_cb, is);
}

//screen640x480
void video_display(VideoState *is) {
	SDL_Rect rect;
	VideoPicture *vp;
	float aspect_ratio;
	int w, h, x, y;
	int i;
	vp = &is->pictq[is->pictq_rindex];
	if(vp->bmp) {
		if(is->video_ctx->sample_aspect_ratio.num == 0) {
			aspect_ratio = 0;
		} else {
			aspect_ratio = av_q2d(is->video_ctx->sample_aspect_ratio) *
				is->video_ctx->width / is->video_ctx->height;
		}
		if(aspect_ratio <= 0.0) {
			aspect_ratio = (float)is->video_ctx->width /
				(float)is->video_ctx->height;
		}
		h = screen->h;//480
		w = ((int)rint(h * aspect_ratio)) & -3;//584
		if(w > screen->w) {//584 < 640
			w = screen->w;
			h = ((int)rint(w / aspect_ratio)) & -3;
		}
		x = (screen->w - w) / 2;
		y = (screen->h - h) / 2;
		rect.x = x;
		rect.y = y;
		rect.w = w;
		rect.h = h;
		SDL_LockMutex(screen_mutex);
		SDL_DisplayYUVOverlay(vp->bmp, &rect);
		SDL_UnlockMutex(screen_mutex);
	}
}//end video_display()

void video_refresh_timer(void *userdata) {
	VideoState *is = (VideoState *)userdata;
	VideoPicture *vp;
	if(is->video_st) {
		if(is->pictq_size == 0) {
			schedule_refresh(is, 1);
		} else {
			vp = &is->pictq[is->pictq_rindex];
			/* Now, normally here goes a ton of code
			   about timing, etc. we're just going to
			   guess at a delay for now. You can
			   increase and decrease this value and hard code
			   the timing - but I don't suggest that ;)
			   We'll learn how to do it for real later.
			   */
			//先写死这里，后边介绍如何确定refresh的时间
			schedule_refresh(is, 40);
			/* show the picture! */
			video_display(is);
			/* update queue for next picture! */
			if(++is->pictq_rindex == VIDEO_PICTURE_QUEUE_SIZE) {
				is->pictq_rindex = 0;
			}
			SDL_LockMutex(is->pictq_mutex);
			is->pictq_size--;
			SDL_CondSignal(is->pictq_cond);
			SDL_UnlockMutex(is->pictq_mutex);
		}
	} else {
		schedule_refresh(is, 100);
	}
}//end video_refresh_timer()

void alloc_picture(void *userdata) {
	VideoState *is = (VideoState *)userdata;
	VideoPicture *vp;
	vp = &is->pictq[is->pictq_windex];
	if(vp->bmp) {
		// we already have one make another, bigger/smaller
		SDL_FreeYUVOverlay(vp->bmp);
	}
	// Allocate a place to put our YUV image on that screen
	SDL_LockMutex(screen_mutex);
	//printf("is->video_ctx->width:%d\n", is->video_ctx->width);//292
	//printf("is->video_ctx->height:%d\n", is->video_ctx->height);//240
	vp->bmp = SDL_CreateYUVOverlay(is->video_ctx->width,
			is->video_ctx->height,
			SDL_YV12_OVERLAY,
			screen);
	SDL_UnlockMutex(screen_mutex);
	vp->width = is->video_ctx->width;
	vp->height = is->video_ctx->height;
	vp->allocated = 1;
}//end alloc_picture()

int queue_picture(VideoState *is, AVFrame *pFrame) {
	VideoPicture *vp;
	int dst_pix_fmt;
	AVPicture pict;
	//先要等待缓冲清空以便于有位置来保存我们的 VideoPicture
	/* wait until we have space for a new pic */
	SDL_LockMutex(is->pictq_mutex);
	while(is->pictq_size >= VIDEO_PICTURE_QUEUE_SIZE && !is->quit) {
		SDL_CondWait(is->pictq_cond, is->pictq_mutex);
	}
	SDL_UnlockMutex(is->pictq_mutex);
	if(is->quit)
		return -1;
	// windex is set to 0 initially
	vp = &is->pictq[is->pictq_windex];
	/* allocate or resize the buffer! */
	//如果vp中没有纹理,创建vp中的纹理
	if(!vp->bmp ||
			vp->width != is->video_ctx->width ||
			vp->height != is->video_ctx->height) {
		SDL_Event event;
		vp->allocated = 0;
		alloc_picture(is);
		if(is->quit) {
			return -1;
		}
	}

	//frame->pict->bmp
	/* We have a place to put our picture on the queue */
	if(vp->bmp) {
		SDL_LockYUVOverlay(vp->bmp);
		dst_pix_fmt = AV_PIX_FMT_YUV420P;

		//pict->bmp:
		//linesize:一行屏幕像素对应的数据量
		//pixels:像素
		//pitches:间距
		/* point pict at the queue */
		pict.data[0] = vp->bmp->pixels[0];//Y一行的数据 字符串
		pict.data[1] = vp->bmp->pixels[2];//U一行的数据 字符串
		pict.data[2] = vp->bmp->pixels[1];//V一行的数据 字符串
		pict.linesize[0] = vp->bmp->pitches[0];//292
		pict.linesize[1] = vp->bmp->pitches[2];//146
		pict.linesize[2] = vp->bmp->pitches[1];//146

		//printf("linesize[0]:%d\n", pict.linesize[0]);//292
		//printf("linesize[1]:%d\n", pict.linesize[1]);//146
		//printf("linesize[2]:%d\n", pict.linesize[2]);//146
		//printf("pict.data[0]:%s\n",pict.data[0]);
		//printf("---------------------------------\n");
		//printf("pict.data[1]:%s\n",pict.data[1]);
		//printf("---------------------------------\n");
		//printf("pict.data[2]:%s\n",pict.data[2]);
		//printf("---------------------------------\n");

		// Convert the image into YUV format that SDL uses
		sws_scale(is->sws_ctx, (const uint8_t * const *)pFrame->data,
				pFrame->linesize, 0, is->video_ctx->height,
				pict.data, pict.linesize);
		SDL_UnlockYUVOverlay(vp->bmp);
		//通知展示线程 pictq就绪
		/* now we inform our display thread that we have a pic ready */
		if(++is->pictq_windex == VIDEO_PICTURE_QUEUE_SIZE) {
			is->pictq_windex = 0;
		}
		SDL_LockMutex(is->pictq_mutex);
		is->pictq_size++;
		SDL_UnlockMutex(is->pictq_mutex);
	}
	return 0;
}//end queue_picture()

int video_thread(void *arg) {
	VideoState *is = (VideoState *)arg;
	AVPacket pkt1, *packet = &pkt1;
	int frameFinished;
	AVFrame *pFrame;
	pFrame = av_frame_alloc();
	for(;;) {
		//从videoq阻塞获取video packet
		if(packet_queue_get(&is->videoq, packet, 1) < 0) {
			// means we quit getting packets
			break;
		}
		//解码video packet成frame
		// Decode video frame
		avcodec_decode_video2(is->video_ctx, pFrame, &frameFinished, packet);
		//获取一个完整的video frame
		// Did we get a video frame?
		if(frameFinished) {
			if(queue_picture(is, pFrame) < 0) {
				break;
			}
		}
		av_free_packet(packet);
	}
	av_frame_free(&pFrame);
	return 0;
}//video_thread()

int stream_component_open(VideoState *is, int stream_index) {
	AVFormatContext *pFormatCtx = is->pFormatCtx;
	AVCodecContext *codecCtx = NULL;
	AVCodec *codec = NULL;
	SDL_AudioSpec wanted_spec, spec;
	//判断流索引是否正常
	if(stream_index < 0 || stream_index >= pFormatCtx->nb_streams) {
		return -1;
	}
	codec = avcodec_find_decoder(pFormatCtx->streams[stream_index]->codec->codec_id);
	if(!codec) {
		fprintf(stderr, "Unsupported codec!\n");
		return -1;
	}
	//写法1
	codecCtx = avcodec_alloc_context3(codec);
	if(avcodec_copy_context(codecCtx, pFormatCtx->streams[stream_index]->codec) != 0) {
		fprintf(stderr, "Couldn't copy codec context");
		return -1; // Error copying codec context
	}
	//写法2
	//codecCtx =  pFormatCtx->streams[stream_index]->codec;
	if(codecCtx->codec_type == AVMEDIA_TYPE_AUDIO) {
		// Set audio settings from codec info
		wanted_spec.freq = codecCtx->sample_rate;
		wanted_spec.format = AUDIO_S16SYS;
		wanted_spec.channels = codecCtx->channels;
		wanted_spec.silence = 0;
		wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
		wanted_spec.callback = audio_callback;
		wanted_spec.userdata = is;
		if(SDL_OpenAudio(&wanted_spec, &spec) < 0) {
			fprintf(stderr, "SDL_OpenAudio: %s\n", SDL_GetError());
			return -1;
		}
	}
	if(avcodec_open2(codecCtx, codec, NULL) < 0) {
		fprintf(stderr, "Unsupported codec!\n");
		return -1;
	}
	switch(codecCtx->codec_type) {
		case AVMEDIA_TYPE_AUDIO:
			is->audioStream = stream_index;
			is->audio_st = pFormatCtx->streams[stream_index];//音频AVStream
			is->audio_ctx = codecCtx;
			is->audio_buf_size = 0;
			is->audio_buf_index = 0;
			memset(&is->audio_pkt, 0, sizeof(is->audio_pkt));
			packet_queue_init(&is->audioq);
			SDL_PauseAudio(0);
			break;
		case AVMEDIA_TYPE_VIDEO:
			is->videoStream = stream_index;
			is->video_st = pFormatCtx->streams[stream_index];//视频AVStream
			is->video_ctx = codecCtx;
			packet_queue_init(&is->videoq);
			is->video_tid = SDL_CreateThread(video_thread, is);
			is->sws_ctx = sws_getContext(is->video_ctx->width, is->video_ctx->height,
					is->video_ctx->pix_fmt, is->video_ctx->width,
					is->video_ctx->height, AV_PIX_FMT_YUV420P,
					SWS_BILINEAR, NULL, NULL, NULL
					);
			break;
		default:
			break;
	}
}//stream_component_open()

int decode_thread(void *arg) {
	VideoState *is = (VideoState *)arg;
	AVFormatContext *pFormatCtx;
	AVPacket pkt1, *packet = &pkt1;
	int video_index = -1;
	int audio_index = -1;
	int i;
	is->videoStream=-1;
	is->audioStream=-1;
	global_video_state = is;
	// Open video file
	if(avformat_open_input(&pFormatCtx, is->filename, NULL, NULL)!=0)
		return -1; // Couldn't open file
	is->pFormatCtx = pFormatCtx;
	// Retrieve stream information
	if(avformat_find_stream_info(pFormatCtx, NULL)<0)
		return -1; // Couldn't find stream information
	// Dump information about file onto standard error
	av_dump_format(pFormatCtx, 0, is->filename, 0);
	// Find the first video stream
	for(i=0; i<pFormatCtx->nb_streams; i++) {
		if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO &&
				video_index < 0) {
			video_index=i;
		}
		if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO &&
				audio_index < 0) {
			audio_index=i;
		}
	}
	if(audio_index >= 0) {
		stream_component_open(is, audio_index);
	}
	if(video_index >= 0) {
		stream_component_open(is, video_index);
	}
	if(is->videoStream < 0 || is->audioStream < 0) {
		fprintf(stderr, "%s: could not open codecs\n", is->filename);
		goto fail;
	}
	// main decode loop
	for(;;) {
		if(is->quit) {
			break;
		}
		//音频或者视频队列已满
		// seek stuff goes here
		if(is->audioq.size > MAX_AUDIOQ_SIZE ||
				is->videoq.size > MAX_VIDEOQ_SIZE) {
			SDL_Delay(10);
			continue;
		}
		//从流中读取packet
		if(av_read_frame(is->pFormatCtx, packet) < 0) {
			if(is->pFormatCtx->pb->error == 0) {
				SDL_Delay(100); /* no error; wait for user input */
				continue;
			} else {
				break;
			}
		}
		// Is this a packet from the video stream?
		if(packet->stream_index == is->videoStream) {
			packet_queue_put(&is->videoq, packet);
		} else if(packet->stream_index == is->audioStream) {
			packet_queue_put(&is->audioq, packet);
		} else {
			av_free_packet(packet);
		}
	}
	/* all done - wait for it */
	while(!is->quit) {
		SDL_Delay(100);
	}

fail:
	if(1){
		SDL_Event event;
		event.type = FF_QUIT_EVENT;
		event.user.data1 = is;
		SDL_PushEvent(&event);
	}
	return 0;
}

//程序4：视频还算正常，音频很快
//./tutorial04 /root/www/video_sound_data/3.h264_data/408.mp4
int main(int argc, char *argv[]) {
	SDL_Event       event;
	VideoState      *is;
	is = (VideoState *)av_mallocz(sizeof(VideoState));
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
	// Make a screen to put our video
#ifndef __DARWIN__
	screen = SDL_SetVideoMode(640, 480, 0, 0);
#else
	screen = SDL_SetVideoMode(640, 480, 24, 0);
#endif
	if(!screen) {
		fprintf(stderr, "SDL: could not set video mode - exiting\n");
		exit(1);
	}

	//printf("111111111\n");
	//SDL_Delay(5000);//5秒
	//printf("222222222\n");

	screen_mutex = SDL_CreateMutex();
	av_strlcpy(is->filename, argv[1], sizeof(is->filename));
	is->pictq_mutex = SDL_CreateMutex();
	is->pictq_cond = SDL_CreateCond();
	schedule_refresh(is, 40);//视频定时刷新屏幕事件
	is->parse_tid = SDL_CreateThread(decode_thread, is);
	if(!is->parse_tid) {
		av_free(is);
		return -1;
	}
	for(;;) {
		SDL_WaitEvent(&event);
		switch(event.type) {
			case FF_QUIT_EVENT:
			case SDL_QUIT:
				is->quit = 1;
				SDL_Quit();
				return 0;
				break;
			case FF_REFRESH_EVENT:
				video_refresh_timer(event.user.data1);
				break;
			default:
				break;
		}
	}
	return 0;
}
