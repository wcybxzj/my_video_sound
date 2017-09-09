#include <stdio.h>
#include <stdlib.h>

int simplest_pcm16le_doublespeed(char *url){
	FILE *fp=fopen(url,"rb+");
	FILE *fp1=fopen("/root/www/video_sound_data/2.pcm_data/output_doublespeed.pcm","wb+");

	int cnt=0;
	unsigned char *sample=(unsigned char *)malloc(4);

	while(!feof(fp)){
		fread(sample,1,4,fp);
		if(cnt%2!=0){
			//L
			fwrite(sample,1,2,fp1);
			//R
			fwrite(sample+2,1,2,fp1);
		}
		cnt++;
	}
	printf("Sample Cnt:%d\n",cnt);

	free(sample);
	fclose(fp);
	fclose(fp1);
	return 0;
}

//将PCM16LE双声道音频采样数据的声音速度提高一倍
//本程序中的函数可以通过抽象的方式将PCM16LE双声道数据的速度提高一倍
//从源代码可以看出，本程序只采样了每个声道奇数点的样值。
//处理完成后，原本22秒左右的音频变成了11秒左右。
//音频的播放速度提高了2倍，音频的音调也变高了很多。
int main(int argc, const char *argv[])
{
	simplest_pcm16le_doublespeed("/root/www/video_sound_data/2.pcm_data/1.pcm");
	return 0;
}
