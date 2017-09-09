#include <stdio.h>
#include <stdlib.h>

//将PCM16LE双声道音频采样数据转换为PCM8音频采样数据
//本程序中的函数可以通过计算的方式将PCM16LE双声道数据16bit的采样位数转换为8bit
//
//PCM16LE格式的采样数据的取值范围是-32768到32767，而PCM8格式的采样数据的取值范围是0到255。
//
//PCM16LE转换到PCM8需要经过两个步骤：
//第一步是将-32768到32767的16bit有符号数值转换为-128到127的8bit有符号数值，
//第二步是将-128到127的8bit有符号数值转换为0到255的8bit无符号数值。
//
//在本程序中，16bit采样数据是通过short类型变量存储的，而8bit采样数据是通过unsigned char类型存储的
int simplest_pcm16le_to_pcm8(char *url){
	FILE *fp=fopen(url,"rb+");
	FILE *fp1=fopen("/root/www/video_sound_data/2.pcm_data/pcm8.pcm","wb+");

	int cnt=0;
	unsigned char *sample=(unsigned char *)malloc(4);
	while(!feof(fp)){
		short *samplenum16=NULL;
		char samplenum8=0;
		unsigned char samplenum8_u=0;
		fread(sample,1,4,fp);

		//L
		samplenum16=(short *)sample;//(signed short:-32768到32767)
		samplenum8=(*samplenum16)>>8;//16bit->有符号8bit     (-128到127)
		samplenum8_u=samplenum8+128;//有符号8bit->无符号8bit (0到255)
		fwrite(&samplenum8_u,1,1,fp1);

		//R
		samplenum16=(short *)(sample+2);
		samplenum8=(*samplenum16)>>8;
		samplenum8_u=samplenum8+128;
		fwrite(&samplenum8_u,1,1,fp1);
		cnt++;
	}
	printf("Sample Cnt:%d\n",cnt);

	free(sample);
	fclose(fp);
	fclose(fp1);
	return 0;
}

int main(int argc, const char *argv[])
{
	simplest_pcm16le_to_pcm8("/root/www/video_sound_data/2.pcm_data/1.pcm");
	return 0;
}
