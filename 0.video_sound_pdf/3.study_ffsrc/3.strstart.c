#include <stdio.h>

int strstart(const char *str, const char *val, const char **ptr)
{
	const char *p,  *q;
	p = str;
	q = val;
	while (*q != '\0')
	{
		if (*p !=  *q)
			return 0;
		p++;
		q++;
	}
	if (ptr)
		*ptr = p;
	return 1;
}

int main(int argc, const char *argv[])
{
	const char *filename ="file:123.avi";
	strstart(filename, "file:", &filename);
	printf("%s\n", filename);//123.avi
	return 0;
}
