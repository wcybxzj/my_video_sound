#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define NUM 1

int main(int argc, const char *argv[])
{
	simplest_yuv420_halfy("1024x1534_yuv420p.yuv",1024, 1534, 1);
	//simplest_yuv420_halfy("292x240_yuv420p.yuv",292,240, 1);
	return 0;
}

//yuv420中y是亮度，u和v是颜色
//y占用w*h, u和v各自占用(w*h)/4
//如果打算将图像的亮度减半，只要将图像的每个像素的Y值取出来分别进行除以2的工作就可以了。
//图像的每个Y值占用1 Byte，取值范围是0至255，对应C语言中的unsigned char数据类型
int simplest_yuv420_halfy(char *url, int w, int h,int num){
	FILE *fp=fopen(url,"rb+");
	FILE *fp1=fopen("output_half.yuv","wb+");
	unsigned char *pic=(unsigned char *)malloc(w*h*3/2);
	int i,j;
	for(i=0;i<num;i++){
		fread(pic,1,w*h*3/2,fp);//读出一帧内
		//将和y有关的数据全部减半,亮度就减半了
		for(j=0;j<w*h;j++){
			unsigned char temp=pic[j]/2;
			//printf("%d,\n",temp);
			pic[j]=temp;
		}
		//颜色数据不变，把修改过的每一帧进行保存
		fwrite(pic,1,w*h*3/2,fp1);
	}

	free(pic);
	fclose(fp);
	fclose(fp1);

	return 0;
}
