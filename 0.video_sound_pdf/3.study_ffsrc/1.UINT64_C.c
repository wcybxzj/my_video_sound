#include <stdio.h>
#include <stdint.h>

#define AV_NOPTS_VALUE          ((int64_t)UINT64_C(0x8000000000000000))

int main(int argc, const char *argv[])
{
	//printf("%ld\n", AV_NOPTS_VALUE);//有符号长整型
	printf("%lu\n", AV_NOPTS_VALUE);//无符号长整型
	return 0;
}
