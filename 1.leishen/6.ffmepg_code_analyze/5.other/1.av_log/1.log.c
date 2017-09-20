#define __STDC_CONSTANT_MACROS
#define __STDC_FORMAT_MACROS

#ifdef __cplusplus
extern "C"
{
#include <stdio.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
}
#endif

void my_logoutput(void* ptr, int level, const char* fmt,va_list vl){
	FILE *fp = fopen("my_log.txt","a+");
	if(fp){
		vfprintf(fp,fmt,vl);
		fflush(fp);
		fclose(fp);
	}
}

int main(int argc, const char *argv[])
{
	av_log_set_callback(my_logoutput);
	av_log(NULL, AV_LOG_ERROR, "Usage: %s <input file> <output file>\n", argv[0]);
	av_log(NULL, AV_LOG_ERROR, "Usage: %s <input file> <output file>\n", argv[0]);
	return 0;
}
