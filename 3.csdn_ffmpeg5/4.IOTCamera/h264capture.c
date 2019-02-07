#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <dirent.h>
#include <stdio.h>
#include <fcntl.h>				/* low-level i/o */
#include <asm/types.h>			/* for videodev2.h */
#include <linux/videodev2.h>

#include "h264capture.h"

static inline int camera_ioctl(int fd, int request, void *arg)
{
	int r = -1;

	do {
		r = ioctl(fd, request, arg);
	} while (r < 0 && EINTR == errno);

	return r;
}

int camera_open(IOTC_Camera* cam)
{
	struct stat st;

	if (stat(cam->device_name, &st) < 0) {
		printf("Cannot identify '%s': %d, %s\n", cam->device_name,
			   errno, strerror(errno));
		return -1;
	}

	if (!S_ISCHR(st.st_mode)) {
		printf("%s is no device\n", cam->device_name);
		return -1;
	}

	cam->fd = open(cam->device_name, O_RDWR, 0);	//  | O_NONBLOCK
	if (cam->fd < 0) {
		printf("Cannot open '%s': %d, %s\n", cam->device_name, errno,strerror(errno));
		return -1;
	}

	return 0;
}

int camera_close(IOTC_Camera* cam)
{
	if (cam->fd < 0)
		return -1;

	if (close(cam->fd) < 0) {
		printf("can not close camera!\n");
		return -1;
	}

	cam->fd = -1;

	return -1;
}

static void camera_init_mmap(IOTC_Camera* cam)
{
	struct v4l2_requestbuffers req;
	struct v4l2_buffer buf;

	memset(&buf, 0, sizeof(struct v4l2_buffer));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	memset(&req, 0, sizeof(struct v4l2_requestbuffers));
	req.count = 4;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	//分配内存
	if (camera_ioctl(cam->fd, VIDIOC_REQBUFS, &req) < 0) {
		if (EINVAL == errno) {
			printf("%s does not support memory mapping\n", cam->device_name);
			return;
		} else {
			printf("VIDIOC_REQBUFS\n");
			return;
		}
	}

	if (req.count < 2) {
		printf("Insufficient buffer memory on %s\n", cam->device_name);
		return;
	}

	cam->buffers = calloc(req.count, sizeof(*(cam->buffers)));
	if (!cam->buffers) {
		printf("Out of memory\n");
		return;
	}

	for (cam->buff_num = 0; cam->buff_num < req.count; cam->buff_num++) {

		buf.index = cam->buff_num;

		if (camera_ioctl(cam->fd, VIDIOC_QUERYBUF, &buf) < 0) {
			printf("VIDIOC_QUERYBUF\n");
			return;
		}

		cam->buffers[cam->buff_num].length = buf.length;
		cam->buffers[cam->buff_num].start = mmap(NULL /* start anywhere */ ,
												 buf.length,
												 PROT_READ | PROT_WRITE
												 /* required */ ,
												 MAP_SHARED /* recommended */ ,
												 cam->fd, buf.m.offset);

		if (MAP_FAILED == cam->buffers[cam->buff_num].start) {
			printf("mmap\n");
			return;
		}
	}

	return;
}

void camera_init(IOTC_Camera* cam)
{
	unsigned int min;
	struct v4l2_capability *cap = &(cam->v4l2_cap);
	struct v4l2_cropcap *cropcap = &(cam->v4l2_cropcap);
	struct v4l2_crop *crop = &(cam->crop);
	struct v4l2_format *fmt = &(cam->v4l2_fmt);

	if (camera_ioctl(cam->fd, VIDIOC_QUERYCAP, cap) < 0) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s is no V4L2 device\n", cam->device_name);
			return;
		} else {
			printf("VIDIOC_QUERYCAP\n");
			return;
		}
	}

	if (!(cap->capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		fprintf(stderr, "%s is no video capture device\n", cam->device_name);
		return;
	}

	if (!(cap->capabilities & V4L2_CAP_STREAMING)) {
		fprintf(stderr, "%s does not support streaming i/o\n",
				cam->device_name);
		return;
	}

	printf("\nVIDOOC_QUERYCAP\n");
	printf("the camera driver is %s\n", cap->driver);
	printf("the camera card is %s\n", cap->card);
	printf("the camera bus info is %s\n", cap->bus_info);
	printf("the version is %d\n", cap->version);

	/* Select video input, video standard and tune here. */
	cropcap->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	crop->c.width = cam->width;
	crop->c.height = cam->height;
	crop->c.left = 0;
	crop->c.top = 0;
	crop->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	fmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt->fmt.pix.width = cam->width;
	fmt->fmt.pix.height = cam->height;
    fmt->fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;	//yuv422
	fmt->fmt.pix.field = V4L2_FIELD_INTERLACED;	//隔行扫描

	if (camera_ioctl(cam->fd, VIDIOC_S_FMT, fmt) < 0) {
		printf("VIDIOC_S_FMT\n");
		return;
	}


	/* Buggy driver paranoia. */
	min = fmt->fmt.pix.width * 2;
	if (fmt->fmt.pix.bytesperline < min)
		fmt->fmt.pix.bytesperline = min;
	min = fmt->fmt.pix.bytesperline * fmt->fmt.pix.height;
	if (fmt->fmt.pix.sizeimage < min)
		fmt->fmt.pix.sizeimage = min;

	camera_init_mmap(cam);

	return;
}

void camera_uninit(IOTC_Camera* cam)
{
	uint32_t i;

	for (i = 0; i < cam->buff_num; ++i) {
		if (munmap(cam->buffers[i].start, cam->buffers[i].length) < 0) {
			printf("munmap\n");
			return;
		}
	}

	free(cam->buffers);

	return;
}

void camera_capturing_start(IOTC_Camera* cam)
{
	uint32_t i;
	struct v4l2_buffer buf;
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	memset(&buf, 0, sizeof(struct v4l2_buffer));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	for (i = 0; i < cam->buff_num; ++i) {
		buf.index = i;

		if (camera_ioctl(cam->fd, VIDIOC_QBUF, &buf) < 0) {
			printf("VIDIOC_DQBUF\n");
			return;
		}
	}

	if (camera_ioctl(cam->fd, VIDIOC_STREAMON, &type) < 0) {
		printf("VIDIOC_STREAMON\n");
		return;
	}

	return;
}

void camera_capturing_stop(IOTC_Camera* cam)
{
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (camera_ioctl(cam->fd, VIDIOC_STREAMOFF, &type) < 0) {
		printf("VIDIOC_STREAMOFF\n");
		return;
	}

	return;
}

static void camera_encode_frame(IOTC_Camera* cam, uint8_t * yuv_frame,
								size_t yuv_length)
{
    int encLength = 0;

    encLength = h264_compress_frame(&cam->encoder, -1, yuv_frame, cam->h264_buf);
    cam->encodedLength=encLength;

	return;
}

int read_and_encode_frame(IOTC_Camera* cam)
{
	struct v4l2_buffer buf;

	memset(&buf, 0, sizeof(struct v4l2_buffer));

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	if (camera_ioctl(cam->fd, VIDIOC_DQBUF, &buf) < 0) {
		switch (errno) {
		case EAGAIN:
			return 0;
		case EIO:
			/* Could ignore EIO, see spec. */
			/* fall through */
		default:

			return -1;
		}
	}

	camera_encode_frame(cam, cam->buffers[buf.index].start, buf.length);

	if (camera_ioctl(cam->fd, VIDIOC_QBUF, &buf) < 0) {
        printf("VIDIOC_DQBUF ERROR\n");
		return -1;
	}

	return 0;
}
