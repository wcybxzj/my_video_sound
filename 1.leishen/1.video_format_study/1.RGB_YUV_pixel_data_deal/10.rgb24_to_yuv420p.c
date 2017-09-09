#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>


//雷神的程序有问题,去stackoverflow找的代码
//http://stackoverflow.com/questions/9465815/rgb-to-yuv420-algorithm-efficiency

char *get_date_str(){
	time_t mytime;
	struct tm *mytm;
	char* str=malloc(50);
	mytime = time(NULL);
	mytm = localtime(&mytime);
	strftime(str, 50, "%Y-%m-%d %H-%M-%S", mytm);
	return str;
}

void RGB24_TO_YUV420(uint8_t *rgb, size_t width, size_t height, uint8_t *destination)
{
	size_t image_size = width * height;
	size_t upos = image_size;
	size_t vpos = upos + upos / 4;
	size_t i = 0;
	int y, x;
	for(y= 0; y< height; ++y)
	{
		if( !(y% 2) )
		{
			for( x = 0; x < width; x += 2 )
			{
				uint8_t r = rgb[3 * i];
				uint8_t g = rgb[3 * i + 1];
				uint8_t b = rgb[3 * i + 2];
				destination[i++] =    ((66*r + 129*g + 25*b+128) >> 8) + 16;
				destination[upos++] = ((-38*r + -74*g + 112*b+ 128) >> 8) + 128;
				destination[vpos++] = ((112*r + -94*g + -18*b+ 128) >> 8) + 128;

				r = rgb[3 * i];
				g = rgb[3 * i + 1];
				b = rgb[3 * i + 2];
				destination[i++] =    ((66*r + 129*g + 25*b + 128) >> 8) + 16;

			}
		}
		else
		{
			for( x = 0; x < width; x += 1 )
			{
				uint8_t r = rgb[3 * i];
				uint8_t g = rgb[3 * i + 1];
				uint8_t b = rgb[3 * i + 2];
				destination[i++] =    ((66*r + 129*g + 25*b+128) >> 8) + 16;
			}
		}
	}
}

/**
 * Convert RGB24 file to YUV420P file
 * @param url_in  Location of Input RGB file.
 * @param w       Width of Input RGB file.
 * @param h       Height of Input RGB file.
 * @param num     Number of frames to process.
 * @param url_out Location of Output YUV file.
 */
int simplest_rgb24_to_yuv420(char *url_in, int w, int h,int num,char *url_out){
	FILE *fp=fopen(url_in,"rb+");
	FILE *fp1=fopen(url_out,"wb+");

	unsigned char *pic_rgb24=(unsigned char *)malloc(w*h*3);
	unsigned char *pic_yuv420=(unsigned char *)malloc(w*h*3/2);//yuv420p
	int i;
	for(i=0;i<num;i++){
		fread(pic_rgb24,1,w*h*3,fp);
		RGB24_TO_YUV420(pic_rgb24,w,h,pic_yuv420);
		fwrite(pic_yuv420,1,w*h*3/2,fp1);
	}

	free(pic_rgb24);
	free(pic_yuv420);
	fclose(fp);
	fclose(fp1);

	return 0;
}

//将RGB24格式像素数据转换为YUV420P格式像素数据
int main(int argc, const char *argv[])
{
	char *filename = get_date_str();
	strcat(filename,".yuv");
	simplest_rgb24_to_yuv420("1024x1534_rgb24.rgb",1024,1534,1,filename);
	return 0;
}
