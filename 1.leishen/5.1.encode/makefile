#gcc -o 1.test 1.test.c -lavutil -lavformat -lavcodec -lz -lavutil -lm -lrt -liconv -ldl

CC=g++
CFLAGS1+=  -lavfilter  -lavformat -lavcodec -lavutil  -lavdevice -lpostproc  -lswresample -lswscale  -lSDL  -lm -lz -lpthread -ldl -lrt -liconv -lmp3lame -lx264 -lx265 -lfdk-aac -llzma -lbz2 -lnuma

CFLAGS2+= -lavfilter -lavformat -lavcodec -lavutil  -lavdevice -lpostproc  -lswresample -lswscale  -lSDL2  -lm -lz -lpthread -ldl -lrt -liconv -lmp3lame -lx264 -lx265 -lfdk-aac -llzma -lbz2 -lnuma

all:2.ffmpeg_yuv420p_to_jpeg_pic 3.ffmpeg_pcm_to_aac 4.ffmpeg_yuv_to_h264 5.ffmpeg_yuv_to_hevc 6.ffmpeg_yuv_to_h26x_pure

2.ffmpeg_yuv420p_to_jpeg_pic:2.ffmpeg_yuv420p_to_jpeg_pic.cpp
	$(CC) 2.ffmpeg_yuv420p_to_jpeg_pic.cpp -o $@ $(CFLAGS1)

3.ffmpeg_pcm_to_aac:3.ffmpeg_pcm_to_aac.cpp
	$(CC) 3.ffmpeg_pcm_to_aac.cpp -o $@ $(CFLAGS1)

4.ffmpeg_yuv_to_h264: 4.ffmpeg_yuv_to_h264.cpp
	$(CC) 4.ffmpeg_yuv_to_h264.cpp -o $@ $(CFLAGS1)

5.ffmpeg_yuv_to_hevc: 5.ffmpeg_yuv_to_hevc.cpp
	$(CC) 5.ffmpeg_yuv_to_hevc.cpp -o $@ $(CFLAGS1)

6.ffmpeg_yuv_to_h26x_pure:6.ffmpeg_yuv_to_h26x_pure.cpp
	$(CC) 6.ffmpeg_yuv_to_h26x_pure.cpp -o $@ $(CFLAGS1)

clean:
	$(RM) -r 2.jpg 3.aac 4.h264 5.h264 5.h265 6.h264 6.hevc
	$(RM) -r 2.ffmpeg_yuv420p_to_jpeg_pic 3.ffmpeg_pcm_to_aac 4.ffmpeg_yuv_to_h264 4.ffmpeg_yuv_to_hevc 5.ffmpeg_yuv_to_hevc
	$(RM) -r 6.ffmpeg_yuv_to_h26x_pure
