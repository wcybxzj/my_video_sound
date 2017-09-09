/**
 * 最简单的基于FFmpeg的AVFilter例子（叠加水印）
 * Simplest FFmpeg AVfilter Example (Watermark)
 *
 * 本程序使用FFmpeg的AVfilter实现了视频的水印叠加功能。
 * 可以将一张PNG图片作为水印叠加到视频上。
 * 是最简单的FFmpeg的AVFilter方面的教程。
 * 适合FFmpeg的初学者。
 */
#include <stdio.h>

#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
#define snprintf _snprintf
//Windows
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavfilter/avfiltergraph.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavutil/avutil.h"
#include "libswscale/swscale.h"
#include "SDL/SDL.h"
};
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <SDL/SDL.h>
#ifdef __cplusplus
};
#endif
#endif

//注：SDL显示和输出YUV可以通过程序最前面的宏控制
//Enable SDL?
#define ENABLE_SDL 1
//Output YUV data?
#define ENABLE_YUVFILE 1

const char *filter_descr = "movie=my_logo.png[wm];[in][wm]overlay=5:5[out]";

static AVFormatContext *pFormatCtx;
static AVCodecContext *pCodecCtx;
AVFilterContext *buffersink_ctx;
AVFilterContext *buffersrc_ctx;
AVFilterGraph *filter_graph;
static int video_stream_index = -1;

static int open_input_file(const char *filename)
{
	int ret;
	AVCodec *dec;
	//1.打开输入文件,并且初始化AVFormatContext
	if ((ret = avformat_open_input(&pFormatCtx, filename, NULL, NULL)) < 0) {
		printf( "Cannot open input file\n");
		return ret;
	}

	//2.找出文件中的音视频流信息,保存在容器中，后边解码时使用
	if ((ret = avformat_find_stream_info(pFormatCtx, NULL)) < 0) {
		printf( "Cannot find stream information\n");
		return ret;
	}

	/*3. select the video stream  并且 回填对应的解码器*/
	ret = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
	if (ret < 0) {
		printf( "Cannot find a video stream in the input file\n");
		return ret;
	}
	video_stream_index = ret;
	pCodecCtx = pFormatCtx->streams[video_stream_index]->codec;

	/*4. init the video decoder */
	if ((ret = avcodec_open2(pCodecCtx, dec, NULL)) < 0) {
		printf( "Cannot open video decoder\n");
		return ret;
	}

	return 0;
}

static int init_filters(const char *filters_descr)
{
	char args[512];
	int ret;
	//1.获取两个filter
	AVFilter *buffersrc  = avfilter_get_by_name("buffer");
	AVFilter *buffersink = avfilter_get_by_name("buffersink");

	//2.创建graph
	filter_graph = avfilter_graph_alloc();

	//3.相当于 outputs = malloc(sizeof(AVFilterInOut));
	AVFilterInOut *outputs = avfilter_inout_alloc();
	AVFilterInOut *inputs  = avfilter_inout_alloc();
	enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };
	AVBufferSinkParams *buffersink_params;

	/*4. buffer video source: the decoded frames from the decoder will be inserted here. */
	snprintf(args, sizeof(args),
			"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
			pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
			pCodecCtx->time_base.num, pCodecCtx->time_base.den,
			pCodecCtx->sample_aspect_ratio.num, pCodecCtx->sample_aspect_ratio.den);

	//5.创建源AVFilterContext并且添加到graph
	ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in", args, NULL, filter_graph);
	if (ret < 0) {
		printf("Cannot create buffer source\n");
		return ret;
	}

	//6.bufersink的格式参数为YUV
	/* buffer video sink: to terminate the filter chain. */
	//Create an AVBufferSinkParams structure.
	buffersink_params = av_buffersink_params_alloc();
	buffersink_params->pixel_fmts = pix_fmts;

	//7.创建 sink AVFilterContext并且添加到graph
	ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out", NULL, buffersink_params, filter_graph);
	av_free(buffersink_params);
	if (ret < 0) {
		printf("Cannot create buffer sink\n");
		printf("ret:%d\n", ret);
		return ret;
	}

	//8.填充AVFilterInOut
	/* Endpoints for the filter graph. */
	outputs->name       = av_strdup("in");
	outputs->filter_ctx = buffersrc_ctx;
	outputs->pad_idx    = 0;
	outputs->next       = NULL;

	inputs->name       = av_strdup("out");
	inputs->filter_ctx = buffersink_ctx;
	inputs->pad_idx    = 0;
	inputs->next       = NULL;

	//9.解析输入的filter_descr,请总movie是filter,在avfilter_register_all()中注册
	//const char *filter_descr = "movie=my_logo.png[wm];[in][wm]overlay=5:5[out]";
	if ((ret = avfilter_graph_parse_ptr(filter_graph, filters_descr, &inputs, &outputs, NULL)) < 0)
		return ret;

	if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
		return ret;
	return 0;
}

int main(int argc, char* argv[])
{
	int ret;
	AVPacket packet;
	AVFrame *pFrame;
	AVFrame *pFrame_out;
	int got_frame;

	//printf("ffmpeg version:%d\n", avcodec_version());
	//1.初始化
	av_register_all();
	avfilter_register_all();

#if ENABLE_YUVFILE
	//2.打开输出文件
	FILE *fp_yuv=fopen("1.yuv","wb+");
	if (fp_yuv==NULL) {
		printf("fopen error 1.yuv\n");
		exit(1);
	}
#endif

	//3.打开输入文件
	if ((ret = open_input_file("/root/www/video_sound_data/5.flv_data/w512_h288.flv")) < 0)
		goto end;

	//4.初始化avfilter
	if ((ret = init_filters(filter_descr)) < 0)
		goto end;

#if ENABLE_SDL
	SDL_Surface *screen;
	SDL_Overlay *bmp;
	SDL_Rect rect;
	//
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		printf( "Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	}
	//
	screen = SDL_SetVideoMode(pCodecCtx->width, pCodecCtx->height, 0, 0);
	if(!screen) {
		printf("SDL: could not set video mode - exiting\n");
		return -1;
	}
	//
	bmp = SDL_CreateYUVOverlay(pCodecCtx->width, pCodecCtx->height,SDL_YV12_OVERLAY, screen);
	SDL_WM_SetCaption("Simplest FFmpeg Video Filter",NULL);
#endif

	pFrame=av_frame_alloc();
	pFrame_out=av_frame_alloc();

	/* read all packets */
	while (1) {
		ret = av_read_frame(pFormatCtx, &packet);
		if (ret< 0){
			break;
		}
		//视频
		if (packet.stream_index == video_stream_index) {
			got_frame = 0;
			//解码
			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_frame, &packet);
			if (ret < 0) {
				printf( "Error decoding video\n");
				break;
			}
			if (got_frame) {
				//从流中的时间为基础预估的时间戳
				pFrame->pts = av_frame_get_best_effort_timestamp(pFrame);
				//将解码后的数据（一个AVFrame）送至AVFilterContext
				/* push the decoded frame into the filtergraph */
				if (av_buffersrc_add_frame(buffersrc_ctx, pFrame) < 0) {
					printf( "Error while feeding the filtergraph\n");
					break;
				}
				/* pull filtered pictures from the filtergraph */
				while (1) {
					ret = av_buffersink_get_frame(buffersink_ctx, pFrame_out);
					if (ret < 0){
						break;
					}
					printf("Process 1 frame!\n");
					if (pFrame_out->format==AV_PIX_FMT_YUV420P) {
#if ENABLE_YUVFILE
						//Y, U, V
						for(int i=0;i<pFrame_out->height;i++){
							fwrite(pFrame_out->data[0]+pFrame_out->linesize[0]*i, 1, pFrame_out->width, fp_yuv);
						}
						for(int i=0;i<pFrame_out->height/2;i++){
							fwrite(pFrame_out->data[1]+pFrame_out->linesize[1]*i, 1, pFrame_out->width/2, fp_yuv);
						}
						for(int i=0;i<pFrame_out->height/2;i++){
							fwrite(pFrame_out->data[2]+pFrame_out->linesize[2]*i, 1, pFrame_out->width/2, fp_yuv);
						}
#endif
#if ENABLE_SDL
						SDL_LockYUVOverlay(bmp);
						int y_size=pFrame_out->width*pFrame_out->height;
						memcpy(bmp->pixels[0],pFrame_out->data[0],y_size);   //Y
						memcpy(bmp->pixels[2],pFrame_out->data[1],y_size/4); //U
						memcpy(bmp->pixels[1],pFrame_out->data[2],y_size/4); //V
						bmp->pitches[0]=pFrame_out->linesize[0];
						bmp->pitches[2]=pFrame_out->linesize[1];
						bmp->pitches[1]=pFrame_out->linesize[2];
						SDL_UnlockYUVOverlay(bmp);
						rect.x = 0;
						rect.y = 0;
						rect.w = pFrame_out->width;
						rect.h = pFrame_out->height;
						SDL_DisplayYUVOverlay(bmp, &rect);
						//Delay 40ms
						SDL_Delay(40);
#endif
					}
					av_frame_unref(pFrame_out);
				}
			}
			av_frame_unref(pFrame);
		}
		av_free_packet(&packet);
	}
#if ENABLE_YUVFILE
	fclose(fp_yuv);
#endif

end:
	avfilter_graph_free(&filter_graph);
	if (pCodecCtx)
		avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);


	if (ret < 0 && ret != AVERROR_EOF) {
		char buf[1024];
		av_strerror(ret, buf, sizeof(buf));
		printf("Error occurred: %s\n", buf);
		return -1;
	}

	return 0;
}



