#include "avformat.h"

extern URLProtocol file_protocol;

//简单的注册/初始化函数，把相应的协议，文件格式，解码器等用相应的链表串起来便于查找
void av_register_all(void)
{
	static int inited = 0;
	if (inited != 0)
		return ;
	inited = 1;

	//ffplay 把CPU 当做一个广义的 DSP。
	//有些计算可以用 CPU 自带的加速指令来优化，
	//ffplay 把这类函数独立出来放到 dsputil.h 和dsputil.c 文件中，
	//用函数指针的方法映射到各个 CPU 具体的加速优化实现函数，
	//此处初始化这些函数指针
	avcodec_init();

	////所有的解码器用链表的方式都串连起来，链表头指针是 first_avcodec
	avcodec_register_all();

	////所有的输入文件格式用链表的方式都串连起来，链表头指针是 first_iformat
	avidec_init();

	////把所有的输入协议用链表的方式都串连起来，比如 tcp/udp/file 等，链表头指针是first_protocol
	register_protocol(&file_protocol);
}
