#ifndef _VIDEO_CAPTURE_H_
#define _VIDEO_CAPTURE_H_

#include <linux/videodev2.h>

#include "h264encoder.h"

struct buffer {
	void *start;
	size_t length;
};

typedef struct v4lCamera
{
    char                *device_name;
    int                 fd;
    int                 width;
    int                 height;
    int                 display_depth;

    unsigned char       *h264_buf;      //encoded buffer
    unsigned int        encodedLength;  //encoded length

    int                 buff_num;

    struct              buffer *buffers;

    struct              v4l2_capability v4l2_cap;
    struct              v4l2_cropcap v4l2_cropcap;
    struct              v4l2_format v4l2_fmt;
    struct              v4l2_crop crop;

    X264Encoder             encoder;

}IOTC_Camera;

void camera_init(IOTC_Camera *cam);
int  camera_open(IOTC_Camera *cam);

void camera_capturing_start(IOTC_Camera *cam);
void camera_capturing_stop(IOTC_Camera *cam);

int  read_and_encode_frame(IOTC_Camera *cam);

void camera_uninit(IOTC_Camera *cam);
int  camera_close(IOTC_Camera *cam);

#endif
