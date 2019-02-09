CC=g++

CFLAGS1+= -lSDL  -lm -lz -lpthread -ldl -lrt

all:allformats avidec aviobuf avio cutils file utils_format

allformats:
	$(CC) allformats.c -c  -o $@.o $(CFLAGS1)

avidec:
	$(CC) avidec.c     -c    -o $@.o $(CFLAGS1)

aviobuf:
	$(CC) aviobuf.c    -c   -o $@.o $(CFLAGS1)

avio:
	$(CC) avio.c         -c    -o $@.o $(CFLAGS1)

cutils:
	$(CC) cutils.c       -c   -o $@.o $(CFLAGS1)

file:
	$(CC) file.c          -c  -o $@.o $(CFLAGS1)

utils_format:
	$(CC) utils_format.c  -c  -o $@.o $(CFLAGS1)

clean:
	rm -f allformats.o avidec.o aviobuf.o avio.o cutils.o file.o utils_format.o

