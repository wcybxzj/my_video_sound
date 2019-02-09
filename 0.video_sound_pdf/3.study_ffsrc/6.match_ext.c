#include <stdio.h>
#include <string.h>

//比较文件的扩展名来识别文件类型。
int match_ext(const char *filename, const char *extensions)
{
	const char *ext,  *p;
	char ext1[32],  *q;

	if (!filename)
		return 0;

	//用'.'号作为扩展名分割符，在文件名中找扩展名分割符。
	ext = strrchr(filename, '.');
	//printf("ext:%s\n", ext);//.avi

	if (ext)
	{
		ext++;//avi
		p = extensions;//avi
		for (;;)
		{
			q = ext1;
			while (*p != '\0' &&  *p != ',' && q - ext1 < sizeof(ext1) - 1){
				//printf("p:%c\n",*p);
				//printf("q:%c\n",*q);
				*q++ = *p++;
			}
			*q = '\0';
			//printf("ext1:%s\n", ext1);//avi
			//printf("ext:%s\n", ext);//avi
			if (!strcasecmp(ext1, ext))
				return 1;
			if (*p == '\0')
				break;
			p++;
		}
	}
	return 0;
}

int main(int argc, const char *argv[])
{
	int re;
	re = match_ext("123.avi", "avi");
	printf("%d\n", re);

	re = match_ext("123.456.mp3", "mp3");
	printf("%d\n", re);
	return 0;
}
