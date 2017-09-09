#include <stdio.h>
#include <stdlib.h>

//生成RGB24格式的彩条测试图
int main(int argc, const char *argv[])
{
	simplest_rgb24_colorbar(640, 360,"colorbar_640x360.rgb");
	return 0;
}

//颜色
//(R, G, B)
//白
//(255, 255, 255)
//黄
//(255, 255,   0)
//青
//(  0, 255, 255)
//绿
//(  0, 255,   0)
//品
//(255,   0, 255)
//红
//(255,   0,   0)
//蓝
//(  0,   0, 255)
//黑
//(  0,   0,   0)
int simplest_rgb24_colorbar(int width, int height,char *url_out){

	unsigned char *data=NULL;
	int barwidth;
	char filename[100]={0};
	FILE *fp=NULL;
	int i=0,j=0;

	data=(unsigned char *)malloc(width*height*3);
	barwidth=width/8;

	if((fp=fopen(url_out,"wb+"))==NULL){
		printf("Error: Cannot create file!");
		return -1;
	}

	for(j=0;j<height;j++){
		for(i=0;i<width;i++){
			int barnum=i/barwidth;
			switch(barnum){
				case 0:{
						   data[(j*width+i)*3+0]=255;
						   data[(j*width+i)*3+1]=255;
						   data[(j*width+i)*3+2]=255;
						   break;
					   }
				case 1:{
						   data[(j*width+i)*3+0]=255;
						   data[(j*width+i)*3+1]=255;
						   data[(j*width+i)*3+2]=0;
						   break;
					   }
				case 2:{
						   data[(j*width+i)*3+0]=0;
						   data[(j*width+i)*3+1]=255;
						   data[(j*width+i)*3+2]=255;
						   break;
					   }
				case 3:{
						   data[(j*width+i)*3+0]=0;
						   data[(j*width+i)*3+1]=255;
						   data[(j*width+i)*3+2]=0;
						   break;
					   }
				case 4:{
						   data[(j*width+i)*3+0]=255;
						   data[(j*width+i)*3+1]=0;
						   data[(j*width+i)*3+2]=255;
						   break;
					   }
				case 5:{
						   data[(j*width+i)*3+0]=255;
						   data[(j*width+i)*3+1]=0;
						   data[(j*width+i)*3+2]=0;
						   break;
					   }
				case 6:{
						   data[(j*width+i)*3+0]=0;
						   data[(j*width+i)*3+1]=0;
						   data[(j*width+i)*3+2]=255;

						   break;
					   }
				case 7:{
						   data[(j*width+i)*3+0]=0;
						   data[(j*width+i)*3+1]=0;
						   data[(j*width+i)*3+2]=0;
						   break;
					   }
			}

		}
	}
	fwrite(data,width*height*3,1,fp);
	fclose(fp);
	free(data);

	return 0;
}


