#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// for Linux platform, plz make sure the size of data type is correct for BMP spec.
// if you use this on Windows or other platforms, plz pay attention to this.
typedef int LONG;
typedef unsigned char BYTE;
typedef unsigned int DWORD;
typedef unsigned short WORD;

// __attribute__((packed)) on non-Intel arch may cause some unexpected error, plz be informed.

typedef struct tagBITMAPFILEHEADER
{
	WORD    bfType; // 2  /* Magic identifier */
	DWORD   bfSize; // 4  /* File size in bytes */
	WORD    bfReserved1; // 2
	WORD    bfReserved2; // 2
	DWORD   bfOffBits; // 4 /* Offset to image data, bytes */
} __attribute__((packed)) BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
{
	DWORD    biSize; // 4 /* Header size in bytes */
	LONG     biWidth; // 4 /* Width of image */
	LONG     biHeight; // 4 /* Height of image */
	WORD     biPlanes; // 2 /* Number of colour planes */
	WORD     biBitCount; // 2 /* Bits per pixel */
	DWORD    biCompress; // 4 /* Compression type */
	DWORD    biSizeImage; // 4 /* Image size in bytes */
	LONG     biXPelsPerMeter; // 4
	LONG     biYPelsPerMeter; // 4 /* Pixels per meter */
	DWORD    biClrUsed; // 4 /* Number of colours */
	DWORD    biClrImportant; // 4 /* Important colours */
} __attribute__((packed)) BITMAPINFOHEADER;

/*
   typedef struct tagRGBQUAD
   {
   unsigned char    rgbBlue;
   unsigned char    rgbGreen;
   unsigned char    rgbRed;
   unsigned char    rgbReserved;
   } RGBQUAD;
 * for biBitCount is 16/24/32, it may be useless
 */

typedef struct
{
	BYTE    b;
	BYTE    g;
	BYTE    r;
} RGB_data; // RGB TYPE, plz also make sure the order

char *get_date_str(){
	time_t mytime;
	struct tm *mytm;
	char* str=malloc(50);
	mytime = time(NULL);
	mytm = localtime(&mytime);
	strftime(str, 50, "%Y-%m-%d %H-%M-%S", mytm);
	return str;
}

int bmp_generator(int width, int height, unsigned char *data)
{
	char *filename;
	BITMAPFILEHEADER bmp_head;
	BITMAPINFOHEADER bmp_info;
	int size = width * height * 3;

	bmp_head.bfType = 0x4D42; // 'BM'
	bmp_head.bfSize= size + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER); // 24 + head + info no quad
	bmp_head.bfReserved1 = bmp_head.bfReserved2 = 0;
	bmp_head.bfOffBits = bmp_head.bfSize - size;
	// finish the initial of head

	printf("bfSize:%d\n", bmp_head.bfSize);
	printf("bfOffBits:%d\n", bmp_head.bfOffBits);

	bmp_info.biSize = 40;
	bmp_info.biWidth = width;
	bmp_info.biHeight = -height;
	bmp_info.biPlanes = 1;
	bmp_info.biBitCount = 24; // bit(s) per pixel, 24 is true color
	bmp_info.biCompress = 0;
	bmp_info.biSizeImage = size;
	bmp_info.biXPelsPerMeter = 0;
	bmp_info.biYPelsPerMeter = 0;
	bmp_info.biClrUsed = 0 ;
	bmp_info.biClrImportant = 0;
	// finish the initial of infohead;

	// copy the data
	FILE *fp;
	filename = get_date_str();
	strcat(filename,".bmp");
	if (!(fp = fopen(filename,"wb"))) return 0;

	fwrite(&bmp_head, 1, sizeof(BITMAPFILEHEADER), fp);
	fwrite(&bmp_info, 1, sizeof(BITMAPINFOHEADER), fp);
	fwrite(data, 1, size, fp);
	fclose(fp);

	printf("%s success\n", filename);
	return 1;
}

void work(char *rgb24path, int w, int h)
{
	int i,j;
	FILE *fp_rgb24;
	unsigned char * rgb24_buffer=(unsigned char *)malloc(w*h*3);
	if((fp_rgb24=fopen(rgb24path,"rb"))==NULL){
		printf("Error: Cannot open input RGB24 file.\n");
		return;
	}

	fread(rgb24_buffer,1,w*h*3,fp_rgb24);

	//BMP save R1|G1|B1,R2|G2|B2 as B1|G1|R1,B2|G2|R2
	//It saves pixel data in Little Endian
	//So we change 'R' and 'B'
	//for(j=0;j<h;j++){
	//	for(i=0;i<w;i++){
	//		//buffer[i][j].r = rgb24_buffer[(j*w+i)*3+2];
	//		//buffer[i][j].g = rgb24_buffer[(j*w+i)*3+1];
	//		//buffer[i][j].b = rgb24_buffer[(j*w+i)*3+0];

	//		buffer[i][j].r = rgb24_buffer[(j*w+i)*3+0];
	//		buffer[i][j].g = rgb24_buffer[(j*w+i)*3+1];
	//		buffer[i][j].b = rgb24_buffer[(j*w+i)*3+2];
	//	}
	//}

	for (j = 0; j < w; j++)
	{
		for (i = 0; i < h; i++)
		{
			char temp=rgb24_buffer[(i*w+j)*3+2];
			rgb24_buffer[(i*w+j)*3+2]=rgb24_buffer[(i*w+j)*3+0];
			rgb24_buffer[(i*w+j)*3+0]=temp;
		}
	}

	bmp_generator(w, h, (BYTE*)rgb24_buffer);
}

//将RGB24格式像素数据封装为BMP图像
int main(int argc, char **argv)
{
	//work("1024x1534_rgb24.rgb",1024 ,1534);
	work("/root/www/video_sound/1.leishen/7.3.libswscale/2.create_test_pic/colorbar_1280x720_rgb24.rgb",1280,720);
	return EXIT_SUCCESS;
}
