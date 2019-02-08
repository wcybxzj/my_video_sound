CC=g++

CFLAGS1+= -lSDL  -lm -lz -lpthread -ldl -lrt

all: allcodecs dsputil imgconvert msrle truespeech utils_codec

allcodecs:
	$(CC)  allcodecs.c    -c  -o $@.o $(CFLAGS1)

dsputil:
	$(CC)  dsputil.c      -c  -o $@.o $(CFLAGS1)

imgconvert:
	$(CC)  imgconvert.c   -c  -o $@.o $(CFLAGS1)

msrle:
	$(CC)  msrle.c        -c  -o $@.o $(CFLAGS1)

truespeech:
	$(CC)  truespeech.c   -c  -o $@.o $(CFLAGS1)

utils_codec:
	$(CC)  utils_codec.c  -c  -o $@.o $(CFLAGS1)

clean:
	rm -f allcodecs.o dsputil.o imgconvert.o msrle.o truespeech.o utils_codec.o
