#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM 1

//windows YUV Player Deluxe
//1024x1534_yuv444p.yuv输出的3个文件
//y文件 1024x1534  YUV Pixel Format:Y
//u文件 1024x1534  YUV Pixel Format:Y
//u文件 1024x1534  YUV Pixel Format:Y
int main(int argc, const char *argv[])
{
	//simplest_yuv444_split("1024x1534_yuv444p.yuv",1024, 1534, 1);
	simplest_yuv444_split("292x240_yuv444p.yuv",292,240, 1);
	return 0;
}

//分离YUV444P像素数据中的Y、U、V分量
//本程序中的函数可以将YUV444P数据中的Y、U、V三个分量分离开来并保存成三个文件。
int simplest_yuv444_split(char *url, int w, int h,int num){
	char name1[30];
	char name2[30];
	char name3[30];
	time_t t;
	t = time(NULL);
	snprintf(name1, 30, "%ld_444p_y.yuv", t);
	snprintf(name2, 30, "%ld_444p_u.yuv", t);
	snprintf(name3, 30, "%ld_444p_v.yuv", t);
	FILE *fp=fopen(url,"rb+");
	FILE *fp1=fopen(name1,"wb+");
	FILE *fp2=fopen(name2,"wb+");
	FILE *fp3=fopen(name3,"wb+");
	if (fp==NULL) {
		printf("fp fopen error\n");
		exit(1);
	}
	if (fp==NULL||fp1==NULL||fp2==NULL||fp3==NULL) {
		printf("fp1 fp2 fp3 fopen error\n");
		exit(1);
	}
	unsigned char *pic=(unsigned char *)malloc(w*h*3);
	int i;
	for(i=0;i<num;i++){
		fread(pic,1,w*h*3,fp);
		//Y
		fwrite(pic,1,w*h,fp1);
		//U
		fwrite(pic+w*h,1,w*h,fp2);
		//V
		fwrite(pic+w*h*2,1,w*h,fp3);
	}
	free(pic);
	fclose(fp);
	fclose(fp1);
	fclose(fp2);
	fclose(fp3);
	return 0;
}
