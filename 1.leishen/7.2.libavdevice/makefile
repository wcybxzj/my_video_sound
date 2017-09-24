#gcc -o 1.test 1.test.c -lavutil -lavformat -lavcodec -lz -lavutil -lm -lrt -liconv -ldl

CC=g++
CFLAGS1+= -lavformat  -lavdevice -lpostproc  -lswresample -lswscale -lavcodec  -lavutil -lSDL  -lm -lz -lpthread -ldl -lrt -liconv -lmp3lame -lx264 -lx265 -lfdk-aac -llzma -lbz2 -lnuma -lasound   -lavfilter -lxcb-shm -lxcb-xfixes -lxcb-shape

CFLAGS2+= -lavformat -lavdevice -lpostproc  -lswresample -lswscale  -lavcodec  -lavutil  -lSDL2  -lm -lz -lpthread -ldl -lrt -liconv -lmp3lame -lx264 -lx265 -lfdk-aac -llzma -lbz2 -lnuma -lasound   -lavfilter -lxcb-shm -lxcb-xfixes -lxcb-shape

CFLAGS3+= -pthread  -lavdevice -lavfilter -lpostproc -lavformat -lavcodec -ldl -liconv -lXfixes -lXext -lX11 -lasound -lSDL  -lSDL2 -lx264 -lx265 -lmp3lame -lfdk-aac -llzma -lz -lrt -lswresample -lswscale -lavutil -lm -lxcb-shm -lxcb-xfixes -lxcb-shape -lXv -lbz2 -lnuma


all: 2.ffmpeg_read_camera 3.ffmpeg_screen_capture

2.ffmpeg_read_camera:2.ffmpeg_read_camera.cpp
	$(CC) 2.ffmpeg_read_camera.cpp  -o $@ $(CFLAGS3)

3.ffmpeg_screen_capture: 3.ffmpeg_screen_capture.cpp
	$(CC) 3.ffmpeg_screen_capture.cpp  -o $@ $(CFLAGS3)

clean:
	$(RM) -r 2.ffmpeg_read_camera 3.ffmpeg_screen_capture

