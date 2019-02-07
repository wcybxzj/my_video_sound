#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "h264encoder.h"

static int pts_time=0;

void h264_encoder_init(X264Encoder * encoder, int width, int height)
{

    encoder->param = (x264_param_t *) malloc(sizeof(x264_param_t));
    encoder->picture = (x264_picture_t *) malloc(sizeof(x264_picture_t));
    x264_param_default(encoder->param);

    x264_param_default_preset(encoder->param, "veryfast", "zerolatency");
    encoder->param->i_width = width;
    encoder->param->i_height = height;
    encoder->param->rc.i_lookahead = 0;	//i帧向前缓冲区
    encoder->param->i_fps_num = 15;
    encoder->param->i_fps_den = 1;
    encoder->param->b_annexb = 1;
    encoder->param->i_keyint_max=100;
    encoder->param->i_keyint_min=90;
    encoder->param->i_bframe=3;
    encoder->param->b_repeat_headers=1;


    x264_param_apply_profile(encoder->param, "baseline");	//使用baseline

    if ((encoder->handle = x264_encoder_open(encoder->param)) == 0) {
		printf("x264_encoder_open error!\n");
		return;
	}

    x264_picture_alloc(encoder->picture, X264_CSP_I420, encoder->param->i_width,encoder->param->i_height);
    encoder->picture->img.i_csp = X264_CSP_I420;
    encoder->picture->img.i_plane = 3;


	return;
}

int h264_compress_frame(X264Encoder * encoder, int type, uint8_t * in, uint8_t * out)
{
	x264_picture_t pic_out;
	int nNal = -1;
	int result = 0;
    //int i = 0;
	uint8_t *p_out = out;


    unsigned int i,j;
    unsigned int base_h;

    char *y = encoder->picture->img.plane[0];
    char *u = encoder->picture->img.plane[1];
    char *v = encoder->picture->img.plane[2];

    int is_y = 1, is_u = 1;
    int y_index = 0, u_index = 0, v_index = 0;
    int yuv422_length = 2 * encoder->param->i_width * encoder->param->i_height;

    for(i=0; i<yuv422_length; i+=2){
        *(y+y_index) = *(in+i);
        y_index++;
    }

    for(i=0; i<encoder->param->i_height; i+=2){
        base_h = i*encoder->param->i_width*2;
        for(j=base_h+1; j<base_h+encoder->param->i_width*2; j+=2){
            if(is_u){
                *(u+u_index) = *(in+j);
                u_index++;
                is_u = 0;
            }
            else{
                *(v+v_index) = *(in+j);
                v_index++;
                is_u = 1;
            }
        }
    }

	switch (type) {
	case 0:
        encoder->picture->i_type = X264_TYPE_P;
		break;
	case 1:
        encoder->picture->i_type = X264_TYPE_IDR;
		break;
	case 2:
        encoder->picture->i_type = X264_TYPE_I;
		break;
	default:
        encoder->picture->i_type = X264_TYPE_AUTO;
		break;
	}

    encoder->picture->i_pts=pts_time;

    if (x264_encoder_encode(encoder->handle, &(encoder->nal), &nNal, encoder->picture,&pic_out) < 0) {
            printf("x264_encoder_encode error,type:%08x!\n",encoder->picture->img.i_csp);
		return -1;
	}

	for (i = 0; i < nNal; i++) {
        memcpy(p_out, encoder->nal[i].p_payload, encoder->nal[i].i_payload);
        p_out += encoder->nal[i].i_payload;
        result += encoder->nal[i].i_payload;
	}

    pts_time=pts_time+1;

	return result;
}

void h264_encoder_uninit(X264Encoder * encoder)
{
    if (encoder->picture) {
        x264_picture_clean(encoder->picture);
        free(encoder->picture);
        encoder->picture = 0;
	}
    if (encoder->param) {
        free(encoder->param);
        encoder->param = 0;
	}
    if (encoder->handle) {
        x264_encoder_close(encoder->handle);
	}


	return;
}
