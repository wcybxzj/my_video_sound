#define __STDC_CONSTANT_MACROS

#ifdef __cplusplus
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}
#endif

#include <stdio.h>

int main(int argc, char* argv[]){
	printf("%s\n", avcodec_configuration());
	av_register_all();
	printf("ffmpeg version:%d\n", avcodec_version());
	return 0;
}

