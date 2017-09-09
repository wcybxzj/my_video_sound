#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//将PCM16LE双声道音频采样数据转换为WAVE格式音频数据
//WAVE格式音频（扩展名为“.wav”）是Windows系统中最常见的一种音频。
//该格式的实质就是在PCM文件的前面加了一个文件头。
//本程序的函数就可以通过在PCM文件前面加一个WAVE文件头从而封装为WAVE格式音频

//WAVE文件是一种RIFF格式的文件。
//其基本块名称是“WAVE”，其中包含了两个子块“fmt”和“data”。
//从编程的角度简单说来就是由WAVE_HEADER、WAVE_FMT、WAVE_DATA、采样数据共4个部分组成

//其中前3部分的结构如下所示。在写入WAVE文件头的时候给其中的每个字段赋上合适的值就可以了。
//但是有一点需要注意：WAVE_HEADER和WAVE_DATA中包含了一个文件长度信息的dwSize字段，该字段的值必须在写入完音频采样数据之后才能获得。
//因此这两个结构体最后才写入WAVE文件中。

//channels:声道
//sample_rate:采样频率 44100khz
int simplest_pcm16le_to_wave(const char *pcmpath,int channels,int sample_rate,const char *wavepath)
{
	typedef struct WAVE_HEADER{
		char         fccID[4];
		unsigned   long    dwSize;
		char         fccType[4];
	}WAVE_HEADER;

	typedef struct WAVE_FMT{
		char         fccID[4];
		unsigned   long       dwSize;
		unsigned   short     wFormatTag;
		unsigned   short     wChannels;
		unsigned   long       dwSamplesPerSec;
		unsigned   long       dwAvgBytesPerSec;
		unsigned   short     wBlockAlign;
		unsigned   short     uiBitsPerSample;
	}WAVE_FMT;

	typedef struct WAVE_DATA{
		char       fccID[4];
		unsigned long dwSize;
	}WAVE_DATA;

	if(channels==0||sample_rate==0){
		channels = 2;
		sample_rate = 44100;
	}
	int bits = 16;

	WAVE_HEADER   pcmHEADER;
	WAVE_FMT   pcmFMT;
	WAVE_DATA   pcmDATA;

	unsigned   short   m_pcmData;
	FILE   *fp,*fpout;

	fp=fopen(pcmpath, "rb");
	if(fp == NULL) {
		printf("open pcm file error\n");
		return -1;
	}
	fpout=fopen(wavepath,   "wb+");
	if(fpout == NULL) {
		printf("create wav file error\n");
		return -1;
	}
	//WAVE_HEADER
	memcpy(pcmHEADER.fccID,"RIFF",strlen("RIFF"));
	memcpy(pcmHEADER.fccType,"WAVE",strlen("WAVE"));
	fseek(fpout,sizeof(WAVE_HEADER),1);
	//WAVE_FMT
	pcmFMT.dwSamplesPerSec=sample_rate;// 44100khz/每秒
	pcmFMT.dwAvgBytesPerSec=pcmFMT.dwSamplesPerSec*sizeof(m_pcmData);// 1秒的数据量
	pcmFMT.uiBitsPerSample=bits;//采样位数
	memcpy(pcmFMT.fccID,"fmt ",strlen("fmt "));
	pcmFMT.dwSize=16;
	pcmFMT.wBlockAlign=2;
	pcmFMT.wChannels=channels;
	pcmFMT.wFormatTag=1;
	fwrite(&pcmFMT,sizeof(WAVE_FMT),1,fpout);

	//WAVE_DATA;
	memcpy(pcmDATA.fccID,"data",strlen("data"));
	pcmDATA.dwSize=0;
	fseek(fpout,sizeof(WAVE_DATA),SEEK_CUR);

	fread(&m_pcmData,sizeof(unsigned short),1,fp);
	while(!feof(fp)){
		pcmDATA.dwSize+=2;
		fwrite(&m_pcmData,sizeof(unsigned short),1,fpout);
		fread(&m_pcmData,sizeof(unsigned short),1,fp);
	}

	pcmHEADER.dwSize=44+pcmDATA.dwSize;

	rewind(fpout);
	fwrite(&pcmHEADER,sizeof(WAVE_HEADER),1,fpout);
	fseek(fpout,sizeof(WAVE_FMT),SEEK_CUR);
	fwrite(&pcmDATA,sizeof(WAVE_DATA),1,fpout);

	fclose(fp);
	fclose(fpout);

	return 0;
}

int main(int argc, const char *argv[])
{
	simplest_pcm16le_to_wave("/root/www/video_sound_data/2.pcm_data/1.pcm", 2, 44100, "/root/www/video_sound_data/2.pcm_data/1.wav");
	return 0;
}
