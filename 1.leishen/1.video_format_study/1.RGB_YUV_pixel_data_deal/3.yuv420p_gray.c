#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define NUM 1

int main(int argc, const char *argv[])
{
	simplest_yuv420_gray("1024x1534_yuv420p.yuv",1024, 1534, 1);
	//simplest_yuv420_gray("292x240_yuv420p.yuv",292,240, 1);
	return 0;
}

//将YUV420P像素数据去掉颜色（变成灰度图）
//本程序中的函数可以将YUV420P格式像素数据的彩色去掉，变成纯粹的灰度图。
//YUV:Y是亮度 U和V是颜色
//如果想把YUV格式像素数据变成灰度图像，只需要将U、V分量设置成128即可。
//这是因为U、V是图像中的经过偏置处理的色度分量。色度分量在偏置处理前的取值范围是-128至127，这时候的无色对应的是“0”值。
//经过偏置后色度分量取值变成了0至255，因而此时的无色对应的就是128了。
int simplest_yuv420_gray(char *url, int w, int h,int num){
	int i;
	char name1[50];
	unsigned char *pic=(unsigned char *)malloc(w*h*3/2);
	time_t t;
	t = time(NULL);
	snprintf(name1, 50, "%ld_420p_gray.yuv", t);
	FILE *fp=fopen(url,"rb+");
	FILE *fp1=fopen(name1,"wb+");

	if (fp==NULL || fp1==NULL) {
		printf("fopen error\n");
		exit(1);
	}

	for(i=0;i<num;i++){
		fread(pic,1,w*h*3/2,fp);
		//Gray
		//跳过Y, U和V都设置成128
		memset(pic+w*h,128,w*h/2);
		fwrite(pic,1,w*h*3/2,fp1);
	}

	free(pic);
	fclose(fp);
	fclose(fp1);
	return 0;
}
