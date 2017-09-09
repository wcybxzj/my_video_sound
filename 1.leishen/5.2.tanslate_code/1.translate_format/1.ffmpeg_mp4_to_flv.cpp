#define __STDC_CONSTANT_MACROS

#ifdef __cplusplus
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}
#endif

#include <stdio.h>

//mp4格式--->flv格式,但是里面的视频h264和音频aac没变化
//可以用windows media info查看
int main(int argc, const char *argv[]){
	int frame_index = 0;
	int ret, i;
	AVPacket pkt;
	AVOutputFormat *ofmt = NULL;
	AVFormatContext *ifmt_ctx=NULL, *ofmt_ctx=NULL;
	const char *in_filename, *out_filename;

	in_filename = "/root/www/video_sound_data/3.h264_data/h288_w512.mp4";
	out_filename = "h288_w512.flv";

	av_register_all();
	//输入
	//1.打开输入文件，初始化输入视频码流的AVFormatContext。
	ret = avformat_open_input(&ifmt_ctx, in_filename,0 , 0);
	if (ret < 0) {
		printf("avformat_open_input error!\n");
		goto end;
	}
	//2.读取输入文件信息
	ret = avformat_find_stream_info(ifmt_ctx, 0);
	if (ret < 0) {
		printf("avformat_find_stream_info error!\n");
		goto end;
	}
	av_dump_format(ifmt_ctx,0 ,in_filename, 0);

	//输出
	//4.用输出文件,初始化输出AVFormatContext
	avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
	if (!ofmt_ctx) {
		printf("avformat_alloc_output_context2() error!\n");
		ret =AVERROR_UNKNOWN;
		goto end;
	}
	ofmt = ofmt_ctx->oformat;
	for (i = 0; i < ifmt_ctx->nb_streams; i++) {
		//5.根据输入流codec创建输出码流的AVStream。
		AVStream *in_stream = ifmt_ctx->streams[i];
		AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
		if (!out_stream) {
			printf("fail to new output stream\n");
			ret = AVERROR_UNKNOWN;
			goto end;
		}
		//6.把输入文件的AVCodecContext的设置信息,复制到输出文件的AVCodecContext
		ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
		if (ret < 0) {
			printf("fail to copy AVCodecContext! \n");
			goto end;
		}
		out_stream->codec->codec_tag = 0;//视频类型
		//需要?
		if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
			out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
		}
	}

	av_dump_format(ofmt_ctx, 0, out_filename, 1);

	//7.打开输出文件
	//如果输出文件不是特殊设备,打开输出文件到ofmt_ctx->pb
	if (!(ofmt_ctx->flags & AVFMT_NOFILE)) {
		ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
		if (ret < 0) {
			printf("avio_open error\n");
			goto end;
		}
	}

	//8.写输出文件头
	ret = avformat_write_header(ofmt_ctx, NULL);
	if (ret < 0) {
		printf("avformat_write_header() error\n");
		goto end;
	}

	while (1) {
		AVStream *in_stream, *out_stream;
		//10.从输入文件中读取一个AVPacket。
		ret = av_read_frame(ifmt_ctx, &pkt);
		if (ret<0) {
			break;
		}
		in_stream = ifmt_ctx->streams[pkt.stream_index];
		out_stream = ofmt_ctx->streams[pkt.stream_index];
		//11.pts/dts
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base,
				(AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base,
				(AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;
		//12.写入输出文件体
		ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
		if (ret < 0) {
			printf("error mixing pakcet\n");
			break;
		}
		printf("write %8d frames to output file\n", frame_index);
		av_free_packet(&pkt);
		frame_index++;
	}

	//13.写输出文件尾
	av_write_trailer(ofmt_ctx);
end:
	avformat_close_input(&ifmt_ctx);
	if (ofmt_ctx && !(ofmt_ctx->flags & AVFMT_NOFILE)) {
		avio_close(ofmt_ctx->pb);
	}
	avformat_free_context(ofmt_ctx);
	
	if (ret < 0 && ret != AVERROR_EOF) {
		printf("如果出了错误并且错误不是读取到文件末尾就报错\n");
		return -1;
	}

	return 0;
}
