CC=g++
CFLAGS2+= -lavfilter -lavformat -lavcodec -lavutil  -lavdevice -lpostproc  -lswresample -lswscale  -lSDL2  -lm -lz -lpthread -ldl -lrt -liconv -lmp3lame -lx264 -lx265 -lfdk-aac -llzma -lbz2 -lnuma

all:1.simplest_ffmpeg_transcoder

1.simplest_ffmpeg_transcoder: 1.simplest_ffmpeg_transcoder.cpp
	$(CC) 1.simplest_ffmpeg_transcoder.cpp  -g -o $@ $(CFLAGS2)

clean:
	rm -rf 1.simplest_ffmpeg_transcoder
	rm -rf 1.flv 1.mpv 1.avi
