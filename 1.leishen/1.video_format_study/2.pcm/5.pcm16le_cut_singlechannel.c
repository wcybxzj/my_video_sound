#include <stdio.h>
#include <stdlib.h>

//将从PCM16LE单声道音频采样数据中截取一部分数据
//本程序中的函数可以从PCM16LE单声道数据中截取一段数据，并输出截取数据的样值
int simplest_pcm16le_cut_singlechannel(char *url,int start_num,int dur_num){
	FILE *fp=fopen(url,"rb+");
	FILE *fp1=fopen("/root/www/video_sound_data/2.pcm_data/output_cut.pcm","wb+");
	FILE *fp_stat=fopen("/root/www/video_sound_data/2.pcm_data/output_cut.txt", "wb+");
	if (fp==NULL||fp1==NULL||fp_stat==NULL) {
		printf("fopen error\n");
		exit(1);
	}
	unsigned char *sample=(unsigned char *)malloc(2);
	int cnt=0;
	while(!feof(fp)){
		fread(sample,1,2,fp);//读取一个采样位数这里是16bits
		if(cnt>start_num&&cnt<=(start_num+dur_num)){
			fwrite(sample,1,2,fp1);

			//方法1:
			//计算出2个char组成的short的值
			short samplenum=sample[1];
			samplenum=samplenum*256;
			samplenum=samplenum+sample[0];
			fprintf(fp_stat,"%6d,",samplenum);

			//方法2:
			//short * my_short = (short *)sample;
			//fprintf(fp_stat,"%6d,", *my_short);

			//写10个一行
			if(cnt%10==0){
				fprintf(fp_stat,"\n",samplenum);
			}
		}
		cnt++;
	}
	free(sample);
	fclose(fp);
	fclose(fp1);
	fclose(fp_stat);
	return 0;
}

int main(int argc, const char *argv[])
{
	simplest_pcm16le_cut_singlechannel("/root/www/video_sound_data/2.pcm_data/output_l.pcm", 60000, 120);
	return 0;
}
