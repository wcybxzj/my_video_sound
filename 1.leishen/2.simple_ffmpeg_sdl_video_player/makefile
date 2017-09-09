#gcc -o 1.test 1.test.c -lavutil -lavformat -lavcodec -lz -lavutil -lm -lrt -liconv -ldl

CC=g++
CFLAGS1+=  -lavfilter  -lavformat -lavcodec -lavutil  -lavdevice -lpostproc  -lswresample -lswscale  -lSDL  -lm -lz -lpthread -ldl -lrt -liconv -lmp3lame -lx264 -lx265 -lfdk-aac -llzma -lbz2 -lnuma

CFLAGS2+= -lavfilter -lavformat -lavcodec -lavutil  -lavdevice -lpostproc  -lswresample -lswscale  -lSDL2  -lm -lz -lpthread -ldl -lrt -liconv -lmp3lame -lx264 -lx265 -lfdk-aac -llzma -lbz2 -lnuma

all:1.test 2.ffmpeg_sdl1_player 3.ffmpeg_sdl2_player 4.ffmpeg_without_libavformat 5.simplest_ffmpeg_decoder 6.hello

1.test:
	$(CC) 1.test.c  -o $@ $(CFLAGS1)

2.ffmpeg_sdl1_player:
	$(CC) 2.ffmpeg_sdl1_player.c  -o $@ $(CFLAGS1)

3.ffmpeg_sdl2_player:
	$(CC) 3.ffmpeg_sdl2_player.c  -o $@ $(CFLAGS2)

4.ffmpeg_without_libavformat:
	$(CC) 4.ffmpeg_without_libavformat.c  -o $@ $(CFLAGS2)

5.simplest_ffmpeg_decoder:
	$(CC) 5.simplest_ffmpeg_decoder.c  -o $@ $(CFLAGS2)

6.hello:
	$(CC) 6.hello.c  -o $@ $(CFLAGS2)

clean:
	$(RM) -r 1.test 2.ffmpeg_sdl1_player 3.ffmpeg_sdl2_player 4.ffmpeg_without_libavformat 5.simplest_ffmpeg_decoder 6.hello
