/**
 * 最简单的基于FFmpeg的视频解码器（纯净版）
 * Simplest FFmpeg Decoder Pure
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 *
 * 本程序实现了视频码流(支持HEVC，H.264，MPEG2等)解码为YUV数据。
 * 它仅仅使用了libavcodec（而没有使用libavformat）。
 * 是最简单的FFmpeg视频解码方面的教程。
 * 通过学习本例子可以了解FFmpeg的解码流程。
 * This software is a simplest decoder based on FFmpeg.
 * It decode bitstreams to YUV pixel data.
 * It just use libavcodec (do not contains libavformat).
 * Suitable for beginner of FFmpeg.
 */

#include <stdio.h>

#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
//Windows
extern "C"
{
#include "libavcodec/avcodec.h"
};
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#ifdef __cplusplus
};
#endif
#endif


//test different codec
#define TEST_H264  1
#define TEST_HEVC  0

//最简单的基于FFmpeg的解码器-纯净版（不包含libavformat）
//./4.ffmpeg_without_libavformat >/tmp/123
int main(int argc, char* argv[])
{
	int num, y_size;
	AVCodec *pCodec;
	AVCodecContext *pCodecCtx= NULL;
	AVCodecParserContext *pCodecParserCtx=NULL;

	FILE *fp_in;
	FILE *fp_out;
	AVFrame *pFrame;

	const int in_buffer_size=4096;
	uint8_t in_buffer[in_buffer_size + FF_INPUT_BUFFER_PADDING_SIZE]={0};
	uint8_t *cur_ptr;
	int cur_size;
	AVPacket packet;
	int ret, got_picture;

#if TEST_HEVC
	enum AVCodecID codec_id=AV_CODEC_ID_HEVC;
	char filepath_in[]="bigbuckbunny_480x272.hevc";
#elif TEST_H264
	AVCodecID codec_id=AV_CODEC_ID_H264;
	char filepath_in[]="/root/www/video_sound_data/3.h264_data/out.h264";
#else
	AVCodecID codec_id=AV_CODEC_ID_MPEG2VIDEO;
	char filepath_in[]="bigbuckbunny_480x272.m2v";
#endif

	char filepath_out[]="292x240.yuv";
	int first_time=1;


	//av_log_set_level(AV_LOG_DEBUG);

	//只注册编解码器有关的组件。比如说编码器、解码器、比特流滤镜等，
	//但是不注册复用/解复用器这些和编解码器无关的组件。
	avcodec_register_all();

	pCodec = avcodec_find_decoder(codec_id);
	if (!pCodec) {
		printf("Codec not found\n");
		return -1;
	}

	//创建AVCodecContext结构体。
	pCodecCtx = avcodec_alloc_context3(pCodec);
	if (!pCodecCtx){
		printf("Could not allocate video codec context\n");
		return -1;
	}

	//初始化AVCodecParserContext结构体。
	pCodecParserCtx=av_parser_init(codec_id);
	if (!pCodecParserCtx){
		printf("Could not allocate video parser context\n");
		return -1;
	}

	//if(pCodec->capabilities&CODEC_CAP_TRUNCATED)
	//    pCodecCtx->flags|= CODEC_FLAG_TRUNCATED;

	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		printf("Could not open codec\n");
		return -1;
	}

	//Input File
	fp_in = fopen(filepath_in, "rb");
	if (!fp_in) {
		printf("Could not open input stream\n");
		printf("filename:%s\n", filepath_in);
		return -1;
	}

	//Output File
	fp_out = fopen(filepath_out, "wb");
	if (!fp_out) {
		printf("Could not open output YUV file\n");
		return -1;
	}

	pFrame = av_frame_alloc();
	av_init_packet(&packet);

	while (1) {
		printf("========================fread()=======================\n");
		//读取数据
		cur_size = fread(in_buffer, 1, in_buffer_size, fp_in);
		if (cur_size == 0)
			break;
		cur_ptr=in_buffer;

		while (cur_size>0){
			printf("-----------av_parser_parser2()-----------------\n");
			//len是本次成功解析的大小
			int len = av_parser_parse2(
					pCodecParserCtx, pCodecCtx,
					&packet.data, &packet.size,
					cur_ptr , cur_size ,
					AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);

			//printf("cur_size是fread读取到的大小:%d\n",cur_size);
			cur_ptr += len;
			cur_size -= len;
			//printf("len是本次成功解析的大小:%d\n",len);
			//printf("cur_size是fread读取到的大小:%d\n",cur_size);
			//printf("packet.size的大小:%d\n",packet.size);

			if(packet.size==0){
				printf("packet.size ==0\n");
				continue;
			}

			//Some Info from AVCodecParserContext
			printf("[Packet]Size:%6d\t",packet.size);
			switch(pCodecParserCtx->pict_type){
				case AV_PICTURE_TYPE_I: printf("Type:I\t");break;
				case AV_PICTURE_TYPE_P: printf("Type:P\t");break;
				case AV_PICTURE_TYPE_B: printf("Type:B\t");break;
				default: printf("Type:Other\t");break;
			}
			printf("Number:%4d\n",pCodecParserCtx->output_picture_number);

			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);
			if (ret < 0) {
				printf("Decode Error.\n");
				return ret;
			}
			if (got_picture) {
				if(first_time){
					printf("\nCodec Full Name:%s\n",pCodecCtx->codec->long_name);
					printf("width:%d\nheight:%d\n\n",pCodecCtx->width,pCodecCtx->height);//width:292  height:240
					printf("linesize[0]:%d\n",pFrame->linesize[0]);//320
					printf("linesize[1]:%d\n",pFrame->linesize[1]);//160
					printf("linesize[2]:%d\n",pFrame->linesize[2]);//160
					printf("pFrame->width:%d, pFrame->height:%d\n", pFrame->width, pFrame->height);//292 240
					first_time=0;
				}
				/*
				y_size=pCodecCtx->width*pCodecCtx->height;
				fwrite(pFrame->data[0],1,y_size,  fp_out);    //Y
				fwrite(pFrame->data[1],1,y_size/4,fp_out);  //U
				fwrite(pFrame->data[2],1,y_size/4,fp_out);  //V
				printf("Succeed to decode 1 frame!\n");
				*/

				//Y, U, V
				for(int i=0;i<pFrame->height;i++){
					num = fwrite(pFrame->data[0]+pFrame->linesize[0]*i,1,pFrame->width,fp_out);
					//printf("y num:%d\n", num);
				}
				for(int i=0;i<pFrame->height/2;i++){
					num = fwrite(pFrame->data[1]+pFrame->linesize[1]*i,1,pFrame->width/2,fp_out);
					//printf("u num:%d\n", num);
				}
				for(int i=0;i<pFrame->height/2;i++){
					num = fwrite(pFrame->data[2]+pFrame->linesize[2]*i,1,pFrame->width/2,fp_out);
					//printf("v num:%d\n", num);
				}
				printf("Succeed to decode 1 frame!\n");
			}
		}
	}

	//printf("+++++++++++++++++++++++++++++++++++++++++++\n");
	//printf("cur_size:%d\n", cur_size);//0
	//printf("+++++++++++++++++++++++++++++++++++++++++++\n");

	//Flush Decoder
	//http://blog.csdn.net/leixiaohua1020/article/details/38868499
	packet.data = NULL;
	packet.size = 0;
	while(1){
		ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);
		if (ret < 0) {
			printf("Decode Error.\n");
			return ret;
		}
		if (!got_picture){
			printf("!got_picture\n");
			break;
		}else {
			/*
			y_size=pCodecCtx->width*pCodecCtx->height;
			fwrite(pFrame->data[0],1,y_size,  fp_out);    //Y
			fwrite(pFrame->data[1],1,y_size/4,fp_out);  //U
			fwrite(pFrame->data[2],1,y_size/4,fp_out);  //V
			printf("Flush Decoder: Succeed to decode 1 frame!\n");
			*/

			//Y, U, V
			for(int i=0;i<pFrame->height;i++){
				fwrite(pFrame->data[0]+pFrame->linesize[0]*i,1,pFrame->width,fp_out);
			}
			for(int i=0;i<pFrame->height/2;i++){
				fwrite(pFrame->data[1]+pFrame->linesize[1]*i,1,pFrame->width/2,fp_out);
			}
			for(int i=0;i<pFrame->height/2;i++){
				fwrite(pFrame->data[2]+pFrame->linesize[2]*i,1,pFrame->width/2,fp_out);
			}
			printf("Flush Decoder: Succeed to decode 1 frame!\n");
		}
	}

	fclose(fp_in);
	fclose(fp_out);


	av_parser_close(pCodecParserCtx);

	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	av_free(pCodecCtx);

	return 0;
}
