#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char *argv[])
{
	FILE *fp =fopen("01.txt","rb+");
	FILE *fp1=fopen("02.txt","wb+");
	FILE *fp2=fopen("03.txt","wb+");
	unsigned char *pic=(unsigned char *)malloc(10);
	fread(pic,1,10,fp);
	fwrite(pic,1,2,fp1);
	fwrite(pic+2,1,2,fp2);
	free(pic);
	fclose(fp);
	fclose(fp1);
	fclose(fp2);
	return 0;
}

