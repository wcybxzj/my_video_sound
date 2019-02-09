CC=g++

CFLAGS1+= -lSDL  -lm -lz -lpthread -ldl -lrt

ffplay:
	$(CC) ffplay.c  -o $@ $(CFLAGS1) -L/usr/local/lib -lmyavcodec -lmyavformat

clean:
	rm -f ffplay
