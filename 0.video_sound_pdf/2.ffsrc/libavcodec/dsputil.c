//定义dsp 优化限幅运算使用的查找表，实现其初始化函数。
#include "avcodec.h"
#include "dsputil.h"

//256+2048=2304
uint8_t cropTbl[256+2 * MAX_NEG_CROP] = {0, };

void dsputil_static_init(void)
{
	int i;
	//初始化限幅运算查找表:
	//前MAX_NEG_CROP 个数组项为0，
	//接着的 256个数组项分别为 0 到255，
	//后面 MAX_NEG_CROP个数组项为 255。
	//用查表代替比较实现限幅运算。
	//1024->1279
	for (i = 0; i < 256; i++)
		cropTbl[i + MAX_NEG_CROP] = i;

	for (i = 0; i < MAX_NEG_CROP; i++)
	{
		cropTbl[i] = 0; //0->1023
		cropTbl[i + MAX_NEG_CROP + 256] = 255; //1280->2303
	}
}
