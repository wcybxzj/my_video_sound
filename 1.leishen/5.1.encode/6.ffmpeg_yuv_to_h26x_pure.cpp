/**
 * 最简单的基于FFmpeg的视频编码器（纯净版）
 * Simplest FFmpeg Video Encoder Pure
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 本程序实现了YUV像素数据编码为视频码流（H264，MPEG2，VP8等等）。
 * 它仅仅使用了libavcodec（而没有使用libavformat）。
 * 是最简单的FFmpeg视频编码方面的教程。
 * 通过学习本例子可以了解FFmpeg的编码流程。
 * This software encode YUV420P data to video bitstream
 * (Such as H.264, H.265, VP8, MPEG2 etc).
 * It only uses libavcodec to encode video (without libavformat)
 * It's the simplest video encoding software based on FFmpeg.
 * Suitable for beginner of FFmpeg
 */


#include <stdio.h>

#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
//Windows
extern "C"
{
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavutil/imgutils.h"
};
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#ifdef __cplusplus
};
#endif
#endif

//test different codec
#define TEST_H264  0
#define TEST_HEVC  1

int main(int argc, char* argv[])
{
	AVCodec *pCodec;
	AVCodecContext *pCodecCtx= NULL;
	int i, ret, got_output;
	FILE *fp_in;
	FILE *fp_out;
	AVFrame *pFrame;
	AVPacket pkt;
	int y_size;
	int framecnt=0;
	//必须用例子中的yuv才行，不知道为什么
	//char filename_in[]= "/root/www/video_sound_data/3.h264_data/408.yuv";//我自己的YUV
	char filename_in[]= "/root/www/video_sound_data/3.h264_data/ds_480x272.yuv";//

#if TEST_HEVC
	AVCodecID codec_id=AV_CODEC_ID_HEVC;
	char filename_out[]="6.hevc";
#else
	AVCodecID codec_id=AV_CODEC_ID_H264;
	char filename_out[]="6.h264";
#endif

	int in_w=480,in_h=272;
	//int in_w=292,in_h=240;
	int framenum=100;

	//1.
	avcodec_register_all();

	//2.获取h264的编码器
	pCodec = avcodec_find_encoder(codec_id);
	if (!pCodec) {
		printf("Codec not found\n");
		return -1;
	}

	//3.获取h264编码器上下文
	pCodecCtx = avcodec_alloc_context3(pCodec);
	if (!pCodecCtx) {
		printf("Could not allocate video codec context\n");
		return -1;
	}
	pCodecCtx->bit_rate = 400000;
	pCodecCtx->width = in_w;
	pCodecCtx->height = in_h;
	pCodecCtx->time_base.num=1;
	pCodecCtx->time_base.den=25;
	pCodecCtx->gop_size = 10;
	pCodecCtx->max_b_frames = 1;
	pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

	//if (codec_id == AV_CODEC_ID_H264)
	//	av_opt_set(pCodecCtx->priv_data, "preset", "slow", 0);

	// Set Option
	AVDictionary *param = 0;
	//H.264
	if(codec_id == AV_CODEC_ID_H264) {
		av_dict_set(&param, "preset", "slow", 0);
		av_dict_set(&param, "tune", "zerolatency", 0);
	}
	//H.265
	if(codec_id == AV_CODEC_ID_H265){
		av_dict_set(&param, "preset", "ultrafast", 0);
		av_dict_set(&param, "tune", "zero-latency", 0);
	}

	//4.打开h264编码器
	if (avcodec_open2(pCodecCtx, pCodec, &param) < 0) {
		printf("Could not open codec\n");
		return -1;
	}

	//5.申请AVFrame
	pFrame = av_frame_alloc();
	if (!pFrame) {
		printf("Could not allocate video frame\n");
		return -1;
	}
	pFrame->format = pCodecCtx->pix_fmt;
	pFrame->width  = pCodecCtx->width;
	pFrame->height = pCodecCtx->height;

	//6.
	ret = av_image_alloc(pFrame->data, pFrame->linesize, pCodecCtx->width, pCodecCtx->height,
			pCodecCtx->pix_fmt, 16);
	if (ret < 0) {
		printf("Could not allocate raw picture buffer\n");
		return -1;
	}
	//7.Input raw data
	fp_in = fopen(filename_in, "rb");
	if (!fp_in) {
		printf("Could not open %s\n", filename_in);
		return -1;
	}
	//8.Output bitstream
	fp_out = fopen(filename_out, "wb");
	if (!fp_out) {
		printf("Could not open %s\n", filename_out);
		return -1;
	}
	y_size = pCodecCtx->width * pCodecCtx->height;
	//9.Encode
	for (i = 0; i < framenum; i++) {
		av_init_packet(&pkt);
		pkt.data = NULL;    // packet data will be allocated by the encoder
		pkt.size = 0;
		//Read raw YUV data
		if (fread(pFrame->data[0],1,y_size,fp_in)<= 0||		// Y
				fread(pFrame->data[1],1,y_size/4,fp_in)<= 0||	// U
				fread(pFrame->data[2],1,y_size/4,fp_in)<= 0){	// V
			return -1;
		}else if(feof(fp_in)){
			break;
		}
		pFrame->pts = i;
		/* encode the image */
		ret = avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_output);
		if (ret < 0) {
			printf("Error encoding frame\n");
			return -1;
		}
		if (got_output) {
			printf("Succeed to encode frame: %5d\tsize:%5d\n",framecnt,pkt.size);
			framecnt++;
			fwrite(pkt.data, 1, pkt.size, fp_out);
			av_free_packet(&pkt);
		}
	}
	//Flush Encoder
	for (got_output = 1; got_output; i++) {
		ret = avcodec_encode_video2(pCodecCtx, &pkt, NULL, &got_output);
		if (ret < 0) {
			printf("Error encoding frame\n");
			return -1;
		}
		if (got_output) {
			printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n",pkt.size);
			fwrite(pkt.data, 1, pkt.size, fp_out);
			av_free_packet(&pkt);
		}
	}

	fclose(fp_out);
	avcodec_close(pCodecCtx);
	av_free(pCodecCtx);
	av_freep(&pFrame->data[0]);
	av_frame_free(&pFrame);

	return 0;
}

