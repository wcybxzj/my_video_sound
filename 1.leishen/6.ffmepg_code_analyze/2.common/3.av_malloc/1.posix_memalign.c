#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//http://blog.csdn.net/leixiaohua1020/article/details/41176777
//雷神介绍IBM那段内存对齐的很不错
int main(int argc, const char *argv[])
{
	void* ptr = NULL;
	int error = posix_memalign(&ptr, 16, 1024);
	if (error != 0) {
		printf("error\n");
	}else{
		printf("success%d\n",error);
	}
	strcpy(ptr,"abc");
	printf("%s\n",ptr);
	return 0;
}

