#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

int reverse_bytes(char *p, char c) {
	int r = 0;
	int i;
	for (i=0; i<c; i++){
		r |= ( *(p+i) << (((c-1)*8)-8*i));
	}
	return r;
}

int getw(FILE*fp)
{
	char* s;
	int i;
	s = (char*) &i;
	s[1]=getc(fp);
	s[0]=getc(fp);
	return(i);
}

int main(int argc, const char *argv[])
{
	unsigned short short_data;
	char char_data[20];
	FILE *fp;
	fp = fopen("/root/www/video_sound_data/5.flv_data/408.flv","r");
	if (fp==NULL) {
		printf("fopen fp\n");
		exit(1);
	}

	int num;
	num = fread(char_data, 1, 15, fp);

	////方法1:
	//num = fread(&short_data, sizeof(unsigned short), 1,fp);
	//num = reverse_bytes((char *) &short_data , sizeof(short));
	//printf("%u\n", num);

	//方法2:
	num = getw(fp);
	printf("%u\n", num);

	return 0;
}
