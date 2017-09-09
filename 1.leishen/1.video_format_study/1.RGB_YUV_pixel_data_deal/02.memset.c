#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, const char *argv[])
{
	char *str=malloc(100);
	strcpy(str, "abcdef");
	printf("%s\n", str);

	str++;
	memset(str,'z',3);
	printf("%s\n", str);

	return 0;
}
