EXEC = IOTCamera

CFLAGS = -g 
LDLIBS += -lpthread -lx264 -lasound -lm

SRCS = main.c \
	h264capture.c \
	h264encoder.c \
	msgdefine.h	\
	socketserver.h	\
	socketserver.c	\

	

$(EXEC): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(EXEC) *.o *.h264
