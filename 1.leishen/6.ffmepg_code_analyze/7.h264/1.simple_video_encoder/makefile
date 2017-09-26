#gcc -o 1.test 1.test.c -lavutil -lavformat -lavcodec -lz -lavutil -lm -lrt -liconv -ldl

CC=g++

#CFLAGS1+= -lavformat  -lavdevice -lpostproc  -lswresample -lswscale -lavcodec  -lavutil -lSDL  -lm -lz -lpthread -ldl -lrt -liconv -lmp3lame -lx264 -lx265 -lfdk-aac -llzma -lbz2 -lnuma -lasound   -lavfilter -lxcb-shm -lxcb-xfixes -lxcb-shape

CFLAGS1+= -lx264

all: 1.simple_video_encoder_yuv_to_h264

1.simple_video_encoder_yuv_to_h264:1.simple_video_encoder_yuv_to_h264.cpp
	$(CC) 1.simple_video_encoder_yuv_to_h264.cpp  -o $@ $(CFLAGS1)

clean:
	$(RM) -r 1.simple_video_encoder_yuv_to_h264
	$(RM) -r 480x272.h264


