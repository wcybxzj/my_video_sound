#include <stdio.h>
#include <stdlib.h>

int simplest_pcm16le_halfvolumeleft(char *url){
	FILE *fp=fopen(url,"rb+");
	FILE *fp1=fopen("/root/www/video_sound_data/2.pcm_data/output_halfleft.pcm","wb+");
	if (fp==NULL||fp1==NULL) {
		printf("fopen error\n");
		exit(1);
	}
	int cnt=0;
	unsigned char *sample=(unsigned char *)malloc(4);

	while(!feof(fp)){
		short *samplenum=NULL;
		fread(sample,1,4,fp);
		samplenum=(short *)sample;
		*samplenum=*samplenum/2;
		//L
		fwrite(sample,1,2,fp1);
		//R
		fwrite(sample+2,1,2,fp1);
		cnt++;
	}
	printf("Sample Cnt:%d\n",cnt);

	free(sample);
	fclose(fp);
	fclose(fp1);
	return 0;
}

//将PCM16LE双声道音频采样数据中左声道的音量降一半
int main(int argc, const char *argv[])
{
	simplest_pcm16le_halfvolumeleft("/root/www/video_sound_data/2.pcm_data/1.pcm");
	return 0;
}
