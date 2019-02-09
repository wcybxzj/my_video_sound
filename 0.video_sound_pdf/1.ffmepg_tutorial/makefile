#gcc -o 1.test 1.test.c -lavutil -lavformat -lavcodec -lz -lavutil -lm -lrt -liconv -ldl

CC=g++
CFLAGS1+=  -lavfilter  -lavformat -lavcodec -lavutil  -lavdevice -lpostproc  -lswresample -lswscale  -lSDL  -lm -lz -lpthread -ldl -lrt -liconv -lmp3lame -lx264 -lx265 -lfdk-aac -llzma -lbz2 -lnuma

CFLAGS2+= -lavfilter -lavformat -lavcodec -lavutil  -lavdevice -lpostproc  -lswresample -lswscale  -lSDL2  -lm -lz -lpthread -ldl -lrt -liconv -lmp3lame -lx264 -lx265 -lfdk-aac -llzma -lbz2 -lnuma

all:tutorial01 tutorial02  tutorial03 tutorial04  tutorial05 tutorial06  tutorial07

tutorial01:
	$(CC) tutorial01.c  -o $@ $(CFLAGS1)

tutorial02:
	$(CC) tutorial02.c  -o $@ $(CFLAGS1)

tutorial03:
	$(CC) tutorial03.c  -o $@ $(CFLAGS1)

tutorial04:
	$(CC) tutorial04.c  -o $@ $(CFLAGS1)

tutorial05:
	$(CC) tutorial05.c  -o $@ $(CFLAGS1)

tutorial06:
	$(CC) tutorial06.c  -o $@ $(CFLAGS1)

tutorial07:
	$(CC) tutorial07.c  -o $@ $(CFLAGS1)

clean:
	$(RM) -r tutorial01 tutorial02 tutorial03 tutorial04 tutorial05 tutorial06  tutorial07
