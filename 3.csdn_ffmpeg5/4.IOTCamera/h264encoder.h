#ifndef _H264ENCODER_H_
#define _H264ENCODER_H_

#include <stdint.h>
#include <stdio.h>

#include "x264.h"

typedef struct {
    x264_param_t        *param;
    x264_t              *handle;
    x264_picture_t      *picture;	//一个视频序列中每帧特点
    x264_nal_t          *nal;
} X264Encoder;

void h264_encoder_init(X264Encoder * encoder, int width, int height);
int  h264_compress_frame(X264Encoder * encoder, int type, uint8_t * in, uint8_t * out);
void h264_encoder_uninit(X264Encoder * encoder);

#endif
