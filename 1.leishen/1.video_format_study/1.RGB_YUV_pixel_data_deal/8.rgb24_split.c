#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char *argv[])
{
	//simplest_rgb24_split("colorbar_640x360.rgb", 640, 360,1);
	//simplest_rgb24_split("1024x1534_rgb24.rgb", 1024, 1534, 1);
	simplest_rgb24_split("495x526_rgb24.rgb", 1024, 1534, 1);
	return 0;
}

//分离RGB24像素数据中的R、G、B分量
/**
 * Split R, G, B planes in RGB24 file.
 * @param url  Location of Input RGB file.
 * @param w    Width of Input RGB file.
 * @param h    Height of Input RGB file.
 * @param num  Number of frames to process.
 *
 */
int simplest_rgb24_split(char *url, int w, int h,int num){
	FILE *fp=fopen(url,"rb+");
	FILE *fp1=fopen("output_r.y","wb+");
	FILE *fp2=fopen("output_g.y","wb+");
	FILE *fp3=fopen("output_b.y","wb+");

	int i, j;
	unsigned char *pic=(unsigned char *)malloc(w*h*3);

	for(i=0;i<num;i++){
		fread(pic,1,w*h*3,fp);
		for(j=0;j<w*h*3;j=j+3){
			//R
			fwrite(pic+j,1,1,fp1);
			//G
			fwrite(pic+j+1,1,1,fp2);
			//B
			fwrite(pic+j+2,1,1,fp3);
		}
	}

	free(pic);
	fclose(fp);
	fclose(fp1);
	fclose(fp2);
	fclose(fp3);

	return 0;
}
