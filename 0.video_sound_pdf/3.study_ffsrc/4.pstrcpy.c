#include <stdio.h>
#include <string.h>

void pstrcpy(char *dest, int size, const char *from)
{
	int c;
	char *q = dest;
	if (size <= 0)
		return ;

	for (;;)
	{
		c =  *from++;
		if (c == 0 || q >= dest+ size - 1)
			break;
		*q++ = c;
	}
	q = '\0';
}

void my_strcpy(char *dest, int size, const char *from)
{
	if (size<=0) {
		return;
	}
	char *my_dest = dest;
	const char *q = from;
	while (1) {
		if (*q==0 || my_dest>=dest+size-1) {
			break;
		}
		*my_dest++ = *q++;
	}
	dest++;
}


int main(int argc, const char *argv[])
{
	char buf[10];
	char *str="abcdef";

	pstrcpy(buf, 3, str);
	printf("%s\n",buf);//ab

	memset(buf, '\0', 10);
	strncpy(buf, str, 3);
	printf("%s\n",buf);//abc

	memset(buf, '\0', 10);
	my_strcpy(buf, 3, str);
	printf("%s\n",buf);//abc
	return 0;
}
