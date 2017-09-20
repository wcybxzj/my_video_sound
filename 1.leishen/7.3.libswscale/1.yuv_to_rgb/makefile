#gcc -o 1.test 1.test.c -lavutil -lavformat -lavcodec -lz -lavutil -lm -lrt -liconv -ldl

CC=g++
CFLAGS1+=  -lavfilter  -lavformat -lavcodec -lavutil  -lavdevice -lpostproc  -lswresample -lswscale  -lSDL  -lm -lz -lpthread -ldl -lrt -liconv -lmp3lame -lx264 -lx265 -lfdk-aac -llzma -lbz2 -lnuma

CFLAGS2+= -lavfilter -lavformat -lavcodec -lavutil  -lavdevice -lpostproc  -lswresample -lswscale  -lSDL2  -lm -lz -lpthread -ldl -lrt -liconv -lmp3lame -lx264 -lx265 -lfdk-aac -llzma -lbz2 -lnuma

all:1.yuv420p_to_rgb24

1.yuv420p_to_rgb24:1.yuv420p_to_rgb24.cpp
	$(CC) 1.yuv420p_to_rgb24.cpp  -o $@ $(CFLAGS1)

clean:
	$(RM) -r 1.yuv420p_to_rgb24 sintel_1280x720_rgb24.rgb
