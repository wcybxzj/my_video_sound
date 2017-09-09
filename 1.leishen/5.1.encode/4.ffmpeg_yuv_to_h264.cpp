/**
 * 最简单的基于FFmpeg的视频编码器
 * Simplest FFmpeg Video Encoder
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 本程序实现了YUV像素数据编码为视频码流（H264，MPEG2，VP8等等）。
 * 是最简单的FFmpeg视频编码方面的教程。
 * 通过学习本例子可以了解FFmpeg的编码流程。
 * This software encode YUV420P data to H.264 bitstream.
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
#include "libavformat/avformat.h"
};
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#ifdef __cplusplus
};
#endif
#endif

//http://blog.csdn.net/leixiaohua1020/article/details/19016109
//继续调用avcodec_decode_video2()把之前那些没有成功解码的帧都解出来。调用的次数就是之前main 中got_picture返回0的次数
int flush_encoder(AVFormatContext *fmt_ctx,unsigned int stream_index){
	printf("only work once!!!!!\n");
	int ret;
	int got_frame;
	AVPacket enc_pkt;
	//http://bbs.csdn.net/topics/390692774
	//CODEC_CAP_DELAY.那么应该(或者是必须)在解码完所有帧后,继续循环调用解码接口,
	//同时把输入包AVPacket的数据指针置为NULL,大小置为0.直到返回got_pictrue为0为止.这样才能保证得到与输入帧对应数量的所有的图像.
	if (!(fmt_ctx->streams[stream_index]->codec->codec->capabilities & CODEC_CAP_DELAY))
		return 0;
	while (1) {
		enc_pkt.data = NULL;
		enc_pkt.size = 0;
		av_init_packet(&enc_pkt);
		ret = avcodec_encode_video2 (fmt_ctx->streams[stream_index]->codec, &enc_pkt, NULL, &got_frame);
		av_frame_free(NULL);
		if (ret < 0)
			break;
		if (!got_frame){
			ret=0;
			break;
		}
		printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n",enc_pkt.size);
		/* mux encoded frame */
		ret = av_write_frame(fmt_ctx, &enc_pkt);
		if (ret < 0)
			break;
	}
	return ret;
}

/*
 简单介绍一下流程中各个函数的意义：
 av_register_all()：注册FFmpeg所有编解码器。
 avformat_alloc_output_context2()：初始化输出码流的AVFormatContext。
 avio_open()：打开输出文件。
 av_new_stream()：创建输出码流的AVStream。
 avcodec_find_encoder()：查找编码器。
 avcodec_open2()：打开编码器。
 avformat_write_header()：写文件头（对于某些没有文件头的封装格式，不需要此函数。比如说MPEG2TS）。
 avcodec_encode_video2()：编码一帧视频。即将AVFrame（存储YUV像素数据）编码为AVPacket（存储H.264等格式的码流数¤´的封装格式，不需要此函数。比如说MPEG2TS
*/
int main(int argc, char* argv[])
{
	AVFormatContext* pFormatCtx;
	AVOutputFormat* fmt;
	AVStream* video_st;
	AVCodecContext* pCodecCtx;
	AVCodec* pCodec;
	AVPacket pkt;
	uint8_t* picture_buf;
	AVFrame* pFrame;
	int picture_size;
	int y_size;
	int framecnt=0;
	FILE *in_file = fopen("/root/www/video_sound_data/3.h264_data/408.yuv", "rb");   //Input raw YUV data
	int in_w=292,in_h=240;                              //Input data's width and height
	int framenum=100;                                   //Frames to encode 只转换100帧，决定输出文件的大小
	const char* out_file = "4.h264";

	av_register_all();

	//Method1.
	pFormatCtx = avformat_alloc_context();
	//Guess Format
	fmt = av_guess_format(NULL, out_file, NULL);
	pFormatCtx->oformat = fmt;

	//Method 2.
	//avformat_alloc_output_context2(&pFormatCtx, NULL, NULL, out_file);
	//fmt = pFormatCtx->oformat;

	//Open output URL
	if (avio_open(&pFormatCtx->pb,out_file, AVIO_FLAG_READ_WRITE) < 0){
		printf("Failed to open output file! \n");
		return -1;
	}

	video_st = avformat_new_stream(pFormatCtx, 0);
	if (video_st==NULL){
		return -1;
	}

	//Param that must set
	pCodecCtx = video_st->codec;
	pCodecCtx->codec_id = fmt->video_codec;
	pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	pCodecCtx->width = in_w;
	pCodecCtx->height = in_h;
	pCodecCtx->bit_rate = 400000;
	pCodecCtx->gop_size=250;
	pCodecCtx->time_base.num = 1;
	pCodecCtx->time_base.den = 25;
	//H264
	//pCodecCtx->me_range = 16;
	//pCodecCtx->max_qdiff = 4;
	//pCodecCtx->qcompress = 0.6;
	pCodecCtx->qmin = 10;
	pCodecCtx->qmax = 51;
	//Optional Param
	pCodecCtx->max_b_frames=3;

	// Set Option
	AVDictionary *param = 0;
	//H.264
	if(pCodecCtx->codec_id == AV_CODEC_ID_H264) {
		av_dict_set(&param, "preset", "slow", 0);
		av_dict_set(&param, "tune", "zerolatency", 0);
	}
	//H.265
	if(pCodecCtx->codec_id == AV_CODEC_ID_H265){
		av_dict_set(&param, "preset", "ultrafast", 0);
		av_dict_set(&param, "tune", "zero-latency", 0);
	}

	//Show some Information
	av_dump_format(pFormatCtx, 0, out_file, 1);

	pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
	if (!pCodec){
		printf("Can not find encoder! \n");
		return -1;
	}
	if (avcodec_open2(pCodecCtx, pCodec,&param) < 0){
		printf("Failed to open encoder! \n");
		return -1;
	}

	//给AVFrame申请一块buf
	pFrame = av_frame_alloc();
	picture_size = avpicture_get_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
	picture_buf = (uint8_t *)av_malloc(picture_size);
	avpicture_fill((AVPicture *)pFrame, picture_buf, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);

	//Write File Header
	avformat_write_header(pFormatCtx,NULL);
	av_new_packet(&pkt,picture_size);
	y_size = pCodecCtx->width * pCodecCtx->height;

	for (int i=0; i<framenum; i++){
		//Read raw YUV data
		if (fread(picture_buf, 1, y_size*3/2,in_file) <= 0){
			printf("Failed to read raw data! \n");
			return -1;
		}else if(feof(in_file)){
			break;
		}
		pFrame->data[0] = picture_buf;              // Y
		pFrame->data[1] = picture_buf+ y_size;      // U
		pFrame->data[2] = picture_buf+ y_size*5/4;  // V
		//PTS
		//pFrame->pts=i;
		pFrame->pts=i*(video_st->time_base.den)/((video_st->time_base.num)*25);
		printf("i:%d, pts:%d\n", i, pFrame->pts);
		int got_picture=0;
		//Encode
		int ret = avcodec_encode_video2(pCodecCtx, &pkt,pFrame, &got_picture);
		if(ret < 0){
			printf("Failed to encode! \n");
			return -1;
		}
		printf("got_picture:%d\n", got_picture);
		if (got_picture==1){
			printf("Succeed to encode frame: %5d\tsize:%5d\n",framecnt,pkt.size);
			framecnt++;
			pkt.stream_index = video_st->index;//0
			//把Packet写入到AVFormatContext
			ret = av_write_frame(pFormatCtx, &pkt);
			av_free_packet(&pkt);
		}
	}

	//Flush Encoder
	int ret = flush_encoder(pFormatCtx,0);
	if (ret < 0) {
		printf("Flushing encoder failed\n");
		return -1;
	}

	//Write file trailer
	av_write_trailer(pFormatCtx);

	//Clean
	if (video_st){
		avcodec_close(video_st->codec);
		av_free(pFrame);
		av_free(picture_buf);
	}
	avio_close(pFormatCtx->pb);
	avformat_free_context(pFormatCtx);

	fclose(in_file);

	return 0;
}
