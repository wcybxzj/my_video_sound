#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int main(int argc, const char *argv[])
{
	simplest_yuv420_psnr("1024x1534_yuv420p.yuv","output_half.yuv",256,256,1);
	return 0;
}

//比较的2张图的Y数据部分
//图1:YUV原图
//图2:YUV亮度减半,也就是Y数据减半的图,利用4.yuv420_halfy_brightness.c生成
int simplest_yuv420_psnr(char *url1,char *url2,int w,int h,int num){
	FILE *fp1=fopen(url1,"rb+");
	FILE *fp2=fopen(url2,"rb+");
	unsigned char *pic1=(unsigned char *)malloc(w*h);
	unsigned char *pic2=(unsigned char *)malloc(w*h);
	int i,j;
	for(i=0;i<num;i++){
		fread(pic1,1,w*h,fp1);
		fread(pic2,1,w*h,fp2);

		double mse_sum=0,mse=0,psnr=0;
		for(j=0;j<w*h;j++){
			mse_sum+=pow((double)(pic1[j]-pic2[j]),2);
		}
		mse=mse_sum/(w*h);
		psnr=10*log10(255.0*255.0/mse);
		printf("%5.3f\n",psnr);

		fseek(fp1,w*h/2,SEEK_CUR);
		fseek(fp2,w*h/2,SEEK_CUR);
	}

	free(pic1);
	free(pic2);
	fclose(fp1);
	fclose(fp2);
	return 0;
}
