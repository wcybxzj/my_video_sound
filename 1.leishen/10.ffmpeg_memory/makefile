#gcc -o 1.test 1.test.c -lavutil -lavformat -lavcodec -lz -lavutil -lm -lrt -liconv -ldl

CC=g++
CFLAGS1+= -lavformat  -lavdevice -lpostproc  -lswresample -lswscale -lavcodec  -lavutil -lSDL  -lm -lz -lpthread -ldl -lrt -liconv -lmp3lame -lx264 -lx265 -lfdk-aac -llzma -lbz2 -lnuma -lasound   -lavfilter -lxcb-shm -lxcb-xfixes -lxcb-shape

CFLAGS2+= -lavformat -lavdevice -lpostproc  -lswresample -lswscale  -lavcodec  -lavutil  -lSDL2  -lm -lz -lpthread -ldl -lrt -liconv -lmp3lame -lx264 -lx265 -lfdk-aac -llzma -lbz2 -lnuma -lasound   -lavfilter -lxcb-shm -lxcb-xfixes -lxcb-shape

CFLAGS3+= -pthread  -lavdevice -lavfilter -lpostproc -lavformat -lavcodec -ldl -liconv -lXfixes -lXext -lX11 -lasound -lSDL  -lSDL2 -lvpx -lx264 -lx265 -lmp3lame -lfdk-aac -llzma -lz -lrt -lswresample -lswscale -lavutil -lm -lxcb-shm -lxcb-xfixes -lxcb-shape -lXv -lbz2 -lnuma

all: 1.ffmpeg_memory_player 2.ffmpeg_memory_transcoder

1.ffmpeg_memory_player:1.ffmpeg_memory_player.cpp
	$(CC) 1.ffmpeg_memory_player.cpp  -o $@ $(CFLAGS3)

2.ffmpeg_memory_transcoder:2.ffmpeg_memory_transcoder.cpp
	$(CC) 2.ffmpeg_memory_transcoder.cpp  -o $@ $(CFLAGS3)

clean:
	$(RM) -r 1.ffmpeg_memory_player  2.ffmpeg_memory_transcoder output.yuv output.h264

