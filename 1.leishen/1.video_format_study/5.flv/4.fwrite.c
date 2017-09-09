#include <stdio.h>
#include <stdlib.h>

void write_data_to_file()
{
	FILE *fp;
	int num = 0x12345678;
	fp = fopen("/tmp/123.txt","w+");
	if (fp==NULL) {
		printf("fopen error\n");
		exit(1);
	}
	fwrite(&num, sizeof(int), 1, fp);
	printf("%d\n",num);
	fclose(fp);
}

void read_data_from_file()
{
	FILE *fp;
	int num;
	fp = fopen("/tmp/123.txt","w+");
	fread(&num, sizeof(int),1, fp);
	printf("read from file num:%d\n", num);
}

int main(int argc, const char *argv[])
{
	write_data_to_file();
	read_data_from_file();
	return 0;
}
