#ifndef __TOOL_SERVER_H__
#define __TOOL_SERVER_H__

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <err.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <errno.h>
#include <netdb.h>
#include <stdbool.h>
#include <alsa/asoundlib.h>
#include <signal.h>

#define LISTEN_POT               30000

#define TEMP_FILE_LIST_SIZE     (64*1024)
#define CMD_PART_LENTH          500

#define SOCK_SNDRCV_LEN         (1024*32)
#define TCP_NODELAY             0x0001


void start_tcp_server();

#endif//end  TOOL_SERVER

