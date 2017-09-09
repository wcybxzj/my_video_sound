#gcc -o 1.test 1.test.c -lavutil -lavformat -lavcodec -lz -lavutil -lm -lrt -liconv -ldl

CC=g++
CFLAGS1+=  -lavfilter  -lavformat -lavcodec -lavutil  -lavdevice -lpostproc  -lswresample -lswscale  -lSDL  -lm -lz -lpthread -ldl -lrt -liconv -lmp3lame -lx264 -lx265 -lfdk-aac -llzma -lbz2 -lnuma

CFLAGS2+= -lavfilter -lavformat -lavcodec -lavutil  -lavdevice -lpostproc  -lswresample -lswscale  -lSDL2  -lm -lz -lpthread -ldl -lrt -liconv -lmp3lame -lx264 -lx265 -lfdk-aac -llzma -lbz2 -lnuma

all: 1.simplest_ffmpeg_video_filter 2.simplest_ffmpeg_video_filter_pure

1.simplest_ffmpeg_video_filter:1.simplest_ffmpeg_video_filter.cpp
	$(CC) 1.simplest_ffmpeg_video_filter.cpp  -o $@ $(CFLAGS1)

2.simplest_ffmpeg_video_filter_pure:2.simplest_ffmpeg_video_filter_pure.cpp
	$(CC) 2.simplest_ffmpeg_video_filter_pure.cpp  -o $@ $(CFLAGS1)

clean:
	$(RM) -r 1.simplest_ffmpeg_video_filter
	$(RM) -r 2.simplest_ffmpeg_video_filter_pure
	$(RM) -r 1.yuv 2.yuv

