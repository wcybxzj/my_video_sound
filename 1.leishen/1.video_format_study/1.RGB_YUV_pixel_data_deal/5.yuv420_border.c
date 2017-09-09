#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define NUM 1

int main(int argc, const char *argv[])
{
	//simplest_yuv420_border("1024x1534_yuv420p.yuv", 1024, 1534, 20, 1);
	simplest_yuv420_border("292x240_yuv420p.yuv", 292, 240, 20 ,1);
	return 0;
}

//(5)将YUV420P像素数据的周围加上边框
//本程序中的函数可以通过修改YUV数据中特定位置的亮度分量Y的数值，给图像添加一个“边框”的效果
//https://en.wikipedia.org/wiki/YUV
//https://zh.wikipedia.org/wiki/YUV
//根据图片的像素找像素对应的YUV的公式
//方法1和方法2的理论依据:
//size.total = size.width * size.height;
//y = yuv[position.y * size.width + position.x];
//u = yuv[(position.y / 2) * (size.width / 2) + (position.x / 2) + size.total];
//v = yuv[(position.y / 2) * (size.width / 2) + (position.x / 2) + size.total + (size.total / 4)];

int simplest_yuv420_border(char *url, int w, int h,int border,int num){
	FILE *fp=fopen(url,"rb+");
	FILE *fp1=fopen("output_border.yuv","wb+");

	unsigned char *pic=(unsigned char *)malloc(w*h*3/2);
	int i,j,k,u,v;
	int total = w*h;
	for(i=0;i<num;i++){
		fread(pic,1,w*h*3/2,fp);
		//Y
		for(j=0;j<h;j++){
			for(k=0;k<w;k++){
				if(k<border||k>(w-border)||j<border||j>(h-border)){
					//方法1:通过把Y设置成最亮达到四边都是边框的效果
					pic[j*w+k]=255;//255是最亮

					//方法2:通过将U色度和V浓度成无来达到类似方法1的效果，效果不好我只是测试下
					//pic[(j/2) * (w / 2) + (k/ 2) +total]=128;//U
					//pic[(j/2) * (w / 2) + (k/ 2) +total + (total / 4)]=128;//V
				}
			}
		}
		fwrite(pic,1,w*h*3/2,fp1);
	}

	free(pic);
	fclose(fp);
	fclose(fp1);

	return 0;
}
