#gcc -o 1.test 1.test.c -lavutil -lavformat -lavcodec -lz -lavutil -lm -lrt -liconv -ldl

CC=g++
CFLAGS1+=  -lavfilter  -lavformat -lavcodec -lavutil  -lavdevice -lpostproc  -lswresample -lswscale  -lSDL  -lm -lz -lpthread -ldl -lrt -liconv -lmp3lame -lx264 -lx265 -lfdk-aac -llzma -lbz2 -lnuma

CFLAGS2+= -lavfilter -lavformat -lavcodec -lavutil  -lavdevice -lpostproc  -lswresample -lswscale  -lSDL2  -lm -lz -lpthread -ldl -lrt -liconv -lmp3lame -lx264 -lx265 -lfdk-aac -llzma -lbz2 -lnuma

all:1.simplest_ffmpeg_sdl1_audio_player 2.simplest_ffmpeg_sdl2_audio_player 3.ffmpeg_decoder 4.sdl2_audio_player

1.simplest_ffmpeg_sdl1_audio_player:
	$(CC) 1.simplest_ffmpeg_sdl1_audio_player.cpp  -o $@ $(CFLAGS1)

2.simplest_ffmpeg_sdl2_audio_player:
	$(CC) 2.simplest_ffmpeg_sdl2_audio_player.cpp  -o $@ $(CFLAGS2)

3.ffmpeg_decoder:
	$(CC) 3.ffmpeg_decoder.cpp  -o $@ $(CFLAGS2)

4.sdl2_audio_player:
	$(CC) 4.sdl2_audio_player.cpp  -o $@ $(CFLAGS2)

clean:
	$(RM) -r 1.simplest_ffmpeg_sdl1_audio_player 2.simplest_ffmpeg_sdl2_audio_player 3.ffmpeg_decoder 4.sdl2_audio_player
