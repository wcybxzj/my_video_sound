
CC=g++
CFLAGS1+=  -lavfilter  -lavformat -lavcodec -lavutil  -lavdevice -lpostproc  -lswresample -lswscale  -lSDL  -lm -lz -lpthread -ldl -lrt -liconv -lmp3lame -lx264 -lx265 -lfdk-aac -llzma -lbz2 -lnuma

CFLAGS2+= -lavfilter -lavformat -lavcodec -lavutil  -lavdevice -lpostproc  -lswresample -lswscale  -lSDL2  -lm -lz -lpthread -ldl -lrt -liconv -lmp3lame -lx264 -lx265 -lfdk-aac -llzma -lbz2 -lnuma

all:1.log

1.log:1.log.c
	$(CC) 1.log.c  -o $@ $(CFLAGS1)


clean:
	$(RM) -r 1.log my_log.txt
