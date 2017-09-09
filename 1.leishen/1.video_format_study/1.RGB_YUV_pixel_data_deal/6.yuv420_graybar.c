#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, const char *argv[])
{
	simplest_yuv420_graybar(640, 360,0,255,10,"graybar_640x360.yuv");
}

//生成YUV420P格式的灰阶测试图
int simplest_yuv420_graybar(int width, int height,int ymin,int ymax,int barnum,char *url_out){
	int barwidth;
	float lum_inc;
	unsigned char lum_temp;
	int uv_width,uv_height;
	FILE *fp=NULL;
	unsigned char *data_y=NULL;
	unsigned char *data_u=NULL;
	unsigned char *data_v=NULL;
	int t=0,i=0,j=0;

	barwidth=width/barnum;//每一个块宽64
	lum_inc=((float)(ymax-ymin))/((float)(barnum-1));//增幅 28.33
	uv_width=width/2;
	uv_height=height/2;

	//Y:U:V 2:1:1
	data_y=(unsigned char *)malloc(width*height);
	data_u=(unsigned char *)malloc(uv_width*uv_height);
	data_v=(unsigned char *)malloc(uv_width*uv_height);

	if((fp=fopen(url_out,"wb+"))==NULL){
		printf("Error: Cannot create file!");
		return -1;
	}

	//Output Info
	printf("Y, U, V value from picture's left to right:\n");
	for(t=0;t<(width/barwidth);t++){//0-9
		//printf("%d\n",(unsigned char)(t*lum_inc));
		lum_temp=ymin+(char)(t*lum_inc);
		printf("%3d, 128, 128\n",lum_temp);
	}

	//Gen Data
	for(j=0;j<height;j++){//360
		for(i=0;i<width;i++){//640
			t=i/barwidth;
			lum_temp=ymin+(char)(t*lum_inc);
			data_y[j*width+i]=lum_temp;
			//printf("%d, lum_temp:%d\n",j*width+i, lum_temp);
		}
	}

	//我写的uv版本:效果一样
	//int size = uv_height*uv_width;
	//for (i = 0; i < size; i++) {
	//	data_u[i]=128;
	//	data_v[i]=128;
	//}

	//原始版本:
	for(j=0;j<uv_height;j++){
		for(i=0;i<uv_width;i++){
			//printf("%d\n",j*uv_width+i);
			data_u[j*uv_width+i]=128;
		}
	}

	for(j=0;j<uv_height;j++){
		for(i=0;i<uv_width;i++){
			data_v[j*uv_width+i]=128;
		}
	}

	fwrite(data_y,width*height,1,fp);
	fwrite(data_u,uv_width*uv_height,1,fp);
	fwrite(data_v,uv_width*uv_height,1,fp);
	fclose(fp);
	free(data_y);
	free(data_u);
	free(data_v);
	return 0;
}
