#include <stdio.h>
#include <stdlib.h>

int simplest_pcm16le_split(char *url){
	FILE *fp=fopen(url,"rb+");
	if (fp == NULL) {
		printf("fopen fp error\n");
		exit(1);
	}
	FILE *fp1=fopen("/root/www/video_sound_data/2.pcm_data/output_l.pcm","wb+");
	FILE *fp2=fopen("/root/www/video_sound_data/2.pcm_data/output_r.pcm","wb+");

	if (fp1==NULL||fp2==NULL) {
		printf("fopen fp1 or fp2 error\n");
		exit(1);
	}

	unsigned char *sample=(unsigned char *)malloc(4);
	while(!feof(fp)){
		fread(sample,1,4,fp);
		//L
		fwrite(sample,1,2,fp1);
		//R
		fwrite(sample+2,1,2,fp2);
	}
	free(sample);
	fclose(fp);
	fclose(fp1);
	fclose(fp2);
	return 0;
}

//分离PCM16LE双声道音频采样数据的左声道和右声道
int main(int argc, const char *argv[])
{
	simplest_pcm16le_split("/root/www/video_sound_data/2.pcm_data/1.pcm");
	return 0;
}
