#define __STDC_CONSTANT_MACROS
#define __STDC_FORMAT_MACROS

#ifdef __cplusplus
extern "C"
{
	#include <stdio.h>
	#include <stdlib.h>
	#include <inttypes.h>
}
#endif

/*
g++ 1.PRIx64.cpp
./a.out
a is 0x12345678 & b is 0x87654321 & c is 0x11111111
*/
int main(void)
{
	uint64_t a = 0x12345678;
	uint64_t b = 0x87654321;
	uint64_t c = 0x11111111;

	printf("a is %#" PRIx64
			" & b is %#" PRIx64
			" & c is %#" PRIx64 "\n",
			a, b, c);
	return EXIT_SUCCESS;
}
