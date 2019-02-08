//文件实现了 URLProtocol抽象层广义文件操作函数，
//由于URLProtocol 是底层其他具体文件(file,pipe 等) 的简单封装，
//这一层只是一个中转站，大部分函数都是简单中转到底层的具体实现函数
#include "../berrno.h"
#include "avformat.h"

URLProtocol *first_protocol = NULL;

//ffplay 抽象底层的file， pipe 等为URLProtocol， 然后把这些URLProtocol串联起来做成链表， 便于查找。
//register_protocol实际就是串联的各个URLProtocol，全局表头为 first_protocol
int register_protocol(URLProtocol *protocol)
{
	URLProtocol **p;
	p = &first_protocol;
	//移动指针到URLProtocol链表末尾
	while (*p != NULL)
		p = &(*p)->next;
	//在URLProtocol链表末尾直接挂接当前的URLProtocol 指针
	*p = protocol;
	protocol->next = NULL;
	return 0;
}

//打开广义输入文件。
//此函数主要有三部分逻辑，
//首先从文件路径名中分离出协议字符串到proto_str字符数组中，
//接着遍历URLProtocol链表查找匹配proto_str 字符数组中的字符串来确定使用的协议，
//最后调用相应的文件协议的打开函数打开输入文件。
//传入参数:
//char *filename=file:123.avi
int url_open(URLContext **puc, const char *filename, int flags)
{
	printf("url_open()\n");
	URLContext *uc;
	URLProtocol *up;
	const char *p;
	char proto_str[128],  *q;
	int err;

	//以冒号和结束符作为边界从文件名中分离出的协议字符串到proto_str字符数组。
	//由于协议只能是字符， 所以在边界前识别到非字符就断定是 file
	p = filename;
	q = proto_str;
	while (*p != '\0' &&  *p != ':')
	{
		if (!isalpha(*p))  // protocols can only contain alphabetic chars
			goto file_proto;
		if ((q - proto_str) < sizeof(proto_str) - 1)
			*q++ =  *p;
		p++;
	}

	//如果协议字符串只有一个字符，我们就认为是 windows下的逻辑盘符，断定是file
	// if the protocol has length 1, we consider it is a dos drive
	if (*p == '\0' || (q - proto_str) <= 1)
	{
file_proto:
		strcpy(proto_str, "file");
	}
	else
	{
		*q = '\0';
	}

	//遍历URLProtocol链表匹配使用的协议，如果没有找到就返回错误码
	up = first_protocol;
	if (up==NULL) {
		printf("up is NULL\n");
	}
	while (up != NULL)
	{
		printf("proto_str:%s\n", proto_str);//file
		printf("up->name:%s\n", up->name);
		if (!strcmp(proto_str, up->name))
			goto found;
		up = up->next;
	}
	err =  - ENOENT;
	goto fail;

found:
	//如果找到就分配 URLContext结构内存，特别注意内存大小要加上文件名长度，
	//文件名字符串结束标记0也要预先分配 1个字节内存，
	//这 1个字节就是 URLContext结构中的 char filename[1]
	uc = (URLContext *)av_malloc(sizeof(URLContext) + strlen(filename));
	if (!uc)
	{
		err =  - ENOMEM;
		goto fail;
	}
	//strcpy 函数会自动在filename 字符数组后面补0 作为字符串结束标记，所以不用特别赋值为0
	strcpy(uc->filename, filename);
	uc->prot = up;
	uc->flags = flags;
	uc->max_packet_size = 0; // default: stream file
	err = up->url_open(uc, filename, flags);
	if (err < 0)
	{
		av_free(uc);
		*puc = NULL;
		return err;
	}
	*puc = uc;
	return 0;

fail:
	*puc = NULL;
	return err;
}

//简单的中转读操作到底层协议的读函数，完成读操作
int url_read(URLContext *h, unsigned char *buf, int size)
{
	int ret;
	if (h->flags &URL_WRONLY)
		return AVERROR_IO;
	ret = h->prot->url_read(h, buf, size);
	return ret;
}

offset_t url_seek(URLContext *h, offset_t pos, int whence)
{
	offset_t ret;

	if (!h->prot->url_seek)
		return  - EPIPE;
	ret = h->prot->url_seek(h, pos, whence);
	return ret;
}

int url_close(URLContext *h)
{
	int ret;

	ret = h->prot->url_close(h);
	av_free(h);
	return ret;
}

//取得最大数据包大小
int url_get_max_packet_size(URLContext *h)
{
	return h->max_packet_size;
}
