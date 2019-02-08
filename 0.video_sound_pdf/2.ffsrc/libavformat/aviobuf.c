//有缓存的广义文件ByteIOContext 相关的文件操作，比如open， read， close， seek 等等
#include "avformat.h"
#include "avio.h"
#include <stdarg.h>

#define IO_BUFFER_SIZE 32768

//初始化广义文件 ByteIOContext 结构，一些简单的赋值操作
int init_put_byte(ByteIOContext *s,
		unsigned char *buffer,
		int buffer_size,
		int write_flag,
		void *opaque,
		int(*read_buf)(void *opaque, uint8_t *buf, int buf_size),
		int(*write_buf)(void *opaque, uint8_t *buf, int buf_size),
		offset_t(*seek)(void *opaque, offset_t offset, int whence))
{
	s->buffer = buffer;
	s->buffer_size = buffer_size;
	s->buf_ptr = buffer;
	s->write_flag = write_flag;
	if (!s->write_flag)
		s->buf_end = buffer;// 初始情况下，缓存中没有效数据，所以buf_end 指向缓存首地址。
	else
		s->buf_end = buffer + buffer_size;
	s->opaque = opaque;
	s->write_buf = write_buf;
	s->read_buf = read_buf;
	s->seek = seek;
	s->pos = 0;
	s->must_flush = 0;
	s->eof_reached = 0;
	s->error = 0;
	s->max_packet_size = 0;

	return 0;
}

//广义文件 ByteIOContext 的seek 操作。
//输入变量： s为广义文件句柄， offset为偏移量， whence 为定位方式。
//输出变量：相对广义文件开始的偏移量
offset_t url_fseek(ByteIOContext *s, offset_t offset, int whence)
{
	printf("url_fseek ByteIOContext * addresss:%x, offset_t:%d, whence:%d\n",
			s, offset, whence);
	offset_t offset1;

	//只支持SEEK_CUR 和SEEK_SET 定位方式， 不支持SEEK_END 方式。
	//SEEK_CUR:从文件当前读写位置为基准偏移 offset字节。
	//SEEK_SET:从文件开始位置偏移 offset字节。
	if (whence != SEEK_CUR && whence != SEEK_SET)
		return  - EINVAL;

	//ffplay 把SEEK_CUR 和SEEK_SET 统一成SEEK_SET 方式处理，
	//所以如果是SEEK_CUR 方式就要转换成SEEK_SET 的偏移量。
	//offset1 = s->pos - (s->buf_end - s->buffer) + (s->buf_ptr - s->buffer) 算式关系请参照3.6节的示意图，
	//表示广义文件的当前实际偏移
	if (whence == SEEK_CUR)
	{
		printf("SEEK_CUR\n");
		printf("s->pos:%d, s->buf_end:%d ,s->buffer:%d ,s->buf_ptr:%d , s->buffer:%d\n",
				s->pos, s->buf_end  ,s->buffer ,s->buf_ptr , s->buffer);

		offset1 = s->pos - (s->buf_end - s->buffer) + (s->buf_ptr - s->buffer);
		printf("url_fseek, offset1:%d\n", offset1);
		if (offset == 0)
			return offset1;// 如果偏移量为0，返回实际偏移位置
		//加上实际偏移量，统一成相对广义文件开始的绝对偏移量
		//计算绝对偏移量，赋值给offset
		offset += offset1;
	}
	//计算绝对偏移量相对当前缓存的偏移量，赋值给offset1。
	offset1 = offset - (s->pos - (s->buf_end - s->buffer));
	printf("offset1:%d\n", offset1);//0
	//判断绝对偏移量是否在当前缓存中，如果在当前缓存中，就简单的修改buf_ptr指针
	if (offset1 >= 0 && offset1 <= (s->buf_end - s->buffer))
	{
		printf("can do the seek inside the buffer\n");
		printf("s->buf_ptr:%x, s->buffer:%x\n", s->buf_ptr, s->buffer);//s->buf_ptr:10003b40, s->buffer:10003b10
		s->buf_ptr = s->buffer + offset1; // can do the seek inside the buffer
	}
	else
	{
		if (!s->seek)
			return  - EPIPE;
		//调用底层具体的文件系统的seek函数完成实际的seek 操作，此时缓存需重新初始化，
		//buf_end重新指向缓存首地址，并修改pos变量为广义文件当前实际偏移量。
		s->buf_ptr = s->buffer;
		s->buf_end = s->buffer;
		if (s->seek(s->opaque, offset, SEEK_SET) == (offset_t) - EPIPE)
			return  - EPIPE;
		s->pos = offset;
	}
	s->eof_reached = 0;
	//返回广义文件当前的实际偏移量
	return offset;
}

//广义文件 ByteIOContext的当前实际偏移量再偏移 offset字节，调用 url_fseek实现。
void url_fskip(ByteIOContext *s, offset_t offset)
{
	url_fseek(s, offset, SEEK_CUR);
}

//返回广义文件 ByteIOContext的当前实际偏移量。
offset_t url_ftell(ByteIOContext *s)
{
	return url_fseek(s, 0, SEEK_CUR);
}

//返回广义文件 ByteIOContext的大小
offset_t url_fsize(ByteIOContext *s)
{
	offset_t size;
	//判断当前广义文件 ByteIOContext是否能 seek，如果不能就返回错误
	if (!s->seek)
		return  - EPIPE;

	//调用底层的seek函数取得文件大小
	size = s->seek(s->opaque,  - 1, SEEK_END) + 1;
	//注意seek操作改变了读指针，所以要重新seek 到当前读指针位置
	s->seek(s->opaque, s->pos, SEEK_SET);
	return size;
}

//判断当前广义文件 ByteIOContext是否到末尾
int url_feof(ByteIOContext *s)
{
	return s->eof_reached;
}

int url_ferror(ByteIOContext *s)
{
	return s->error;
}

// Input stream
//填充广义文件 ByteIOContext 内部的数据缓存区
static void fill_buffer(ByteIOContext *s)
{
	printf("fill_buffer()\n");
	int len;

	//如果到了广义文件 ByteIOContext 末尾就直接返回
	if (s->eof_reached)
		return ;
	//调用底层文件系统的读函数实际读数据填到缓存，注意这里经过了好几次跳转才到底层读函数。
	//首先跳转的 url_read_buf()函数，再跳转到url_read()，再跳转到实际文件协议的读函数完成读操作
	len = s->read_buf(s->opaque, s->buffer, s->buffer_size);
	printf("url_read_buf return len:%d\n", len);//32768
	if (len <= 0)
	{   // do not modify buffer if EOF reached so that a seek back can be done without rereading data
		// 如果是到达文件末尾就不要改 buffer参数，这样不用重新读数据就可以做seek back操作
		s->eof_reached = 1;

		//设置错误码，便于分析定位。
		if (len < 0)
			s->error = len;
	}
	else
	{
		printf("before:\n");
		printf("s->pos:%d\n",s->pos);
		printf("s->buf_ptr:%x\n", s->buf_ptr);
		printf("s->buf_end:%x\n", s->buf_end);

		//如果正确读取，修改一下基本参数。参加 3.6节中的示意图
		s->pos += len;
		s->buf_ptr = s->buffer;
		s->buf_end = s->buffer + len;

		printf("after\n");
		printf("s->pos:%d\n",s->pos);
		printf("s->buf_ptr:%x\n", s->buf_ptr);
		printf("s->buf_end:%x\n", s->buf_end);
	}
}//end fill_buffer()

//从广义文件 ByteIOContext 中读取一个字节。
int get_byte(ByteIOContext *s) // NOTE: return 0 if EOF, so you cannot use it if EOF handling is necessary
{
	//printf("get_byte(), s->buf_ptr:%x , s->buf_end:%x\n", s->buf_ptr , s->buf_end);
	if (s->buf_ptr < s->buf_end)
	{
		//如果广义文件 ByteIOContext内部缓存有数据，就修改读指针，返回读取的数据
		return  *s->buf_ptr++;
	}
	else
	{
		//如果广义文件 ByteIOContext内部缓存没有数据，就先填充内部缓存
		fill_buffer(s);
		//如果广义文件 ByteIOContext 内部缓存有数据，就修改读指针，返回读取的数据。
		//如果没有数据就是 到了文件末尾，返回 0。
		//NOTE: return 0 if EOF, so you cannot use it if EOF handling is necessary
		if (s->buf_ptr < s->buf_end)
			return  *s->buf_ptr++;
		else
			return 0;
	}
}

//从广义文件 ByteIOContext 中以小端方式读取两个字节,实现代码充分复用get_byte()函数
unsigned int get_le16(ByteIOContext *s)
{
	unsigned int val;
	val = get_byte(s);
	val |= get_byte(s) << 8;
	return val;
}

//从广义文件 ByteIOContext 中以小端方式读取四个字节,实现代码充分复用get_le16()函数
unsigned int get_le32(ByteIOContext *s)
{
	unsigned int val;
	val = get_le16(s);
	val |= get_le16(s) << 16;
	return val;
}

#define url_write_buf NULL

//简单中转读操作函数
static int url_read_buf(void *opaque, uint8_t *buf, int buf_size)
{
	URLContext *h = (URLContext *)opaque;
	return url_read(h, buf, buf_size);
}

static offset_t url_seek_buf(void *opaque, offset_t offset, int whence)
{
	URLContext *h = (URLContext *)opaque;
	return url_seek(h, offset, whence);
}

//设置并分配广义文件 ByteIOContext内部缓存的大小。更多的应用在修改内部缓存大小场合。
int url_setbufsize(ByteIOContext *s, int buf_size) // must be called before any I/O
{
	uint8_t *buffer;
	//分配广义文件 ByteIOContext内部缓存。
	buffer =(uint8_t *) av_malloc(buf_size);
	if (!buffer)
		return  - ENOMEM;

	//释放掉原来广义文件 ByteIOContext的内部缓存，这是一个保险的操作
	av_free(s->buffer);
	//设置广义文件 ByteIOContext 内部缓存相关参数
	s->buffer = buffer;
	s->buffer_size = buf_size;
	s->buf_ptr = buffer;
	if (!s->write_flag)
		s->buf_end = buffer;// 因为此时只是分配了内存,并没有读入数据,所以buf_end 指向首地址
	else
		s->buf_end = buffer + buf_size;
	return 0;
}

//打开广义文件 ByteIOContext
int url_fopen(ByteIOContext *s, const char *filename, int flags)
{
	URLContext *h;
	uint8_t *buffer;
	int buffer_size, max_packet_size;
	int err;

	err = url_open(&h, filename, flags);
	if (err < 0){
		printf("url_open fail:%d\n",err);
		return err;
	}
	//读取底层文件系统支持的最大包大小。如果非 0，则设置为内部缓存的大小；
	//否则内部缓存设置为默认大小 IO_BUFFER_SIZE(32768 字节)
	max_packet_size = url_get_max_packet_size(h);
	if (max_packet_size)
	{
		buffer_size = max_packet_size; // no need to bufferize more than one packet
	}
	else
	{
		buffer_size = IO_BUFFER_SIZE;
	}

	//分配广义文件 ByteIOContext内部缓存，如果错误就关闭文件返回错误码
	buffer = (uint8_t *)av_malloc(buffer_size);
	if (!buffer)
	{
		url_close(h);
		return  - ENOMEM;
	}

	//初始始化广义文件 ByteIOContext数据结构，如果错误就关闭文件，释放内部缓存，返回错误码
	if (init_put_byte(s,
				buffer,
				buffer_size,
				(h->flags & URL_WRONLY || h->flags & URL_RDWR),
				h,
				url_read_buf,
				url_write_buf,
				url_seek_buf) < 0)
	{
		url_close(h);
		av_free(buffer);
		return AVERROR_IO;
	}

	//保存最大包大小
	s->max_packet_size = max_packet_size;

	return 0;
}

//关闭广义文件 ByteIOContext，首先释放掉内部使用的缓存，
//再把自己的字段置0，最后转入底层文件系统的关闭函数实质性关闭文件
int url_fclose(ByteIOContext *s)
{
	URLContext *h = (URLContext *)s->opaque;
	av_free(s->buffer);
	memset(s, 0, sizeof(ByteIOContext));
	return url_close(h);
}

//广义文件 ByteIOContext读操作，注意此函数从get_buffer 改名而来，
//更贴切函数功能，也为了完备广义文件操作函数集。
int url_fread(ByteIOContext *s, unsigned char *buf, int size) // get_buffer
{
	printf("url_fread()\n");
	int len, size1;
	//考虑到size 可能比缓存中的数据大得多， 此时就多次读缓存，
	//所以用size1 保存要读取的总字节数，size意义变更为还需要读取的字节数
	size1 = size;
	printf("size:%d\n", size);
	printf("size1:%d\n", size1);

	//如果还需要读的字节数大于0，就进入循环继续读
	while (size > 0)
	{
		//计算当次循环应该读取的字节数 len，首先设置len为内部缓存数据长度，
		//再和需要读的字节数 size 比， 有条件修正 len的值。
		len = s->buf_end - s->buf_ptr;
		printf("before len:%d\n", len);
		if (len > size)
			len = size;
		printf("after len:%d\n", len);
		//如果内部缓存没有数据
		if (len == 0)
		{
			printf("buffer_size:%d\n", s->buffer_size);//32768
			//如果要读取的数据量比内部缓存数据量大，
			//就调用底层函数读取数据绕过内部缓存直接到目标缓存
			//这种情况就很难出现
			if (size > s->buffer_size)
			{
				len = s->read_buf(s->opaque, buf, size);
				//如果底层文件系统读错误，设置文件末尾标记和错误码，
				//跳出循环，返回实际读到的字节数
				if (len <= 0)
				{
					s->eof_reached = 1;
					if (len < 0)
						s->error = len;
					break;
				}
				else
				{
					//如果底层文件系统正确读，修改相关参数，进入下一轮循环。特别注意此处读文件绕过了内部缓存。
					s->pos += len;
					size -= len;
					buf += len;// 因为绕过了内部缓存，特别注意此处的修改
					s->buf_ptr = s->buffer;
					s->buf_end = s->buffer /* + len*/;// 因为绕过了内部缓存，特别注意此处的修改
				}
			}
			else
			{
				//如果要读取的数据量比内部缓存数据量小，就调用底层函数读取数据到内部缓存，判断读成功否
				fill_buffer(s);
				len = s->buf_end - s->buf_ptr;
				if (len == 0)
					break;
			}
		}
		else
		{
			printf("len !=0 ,size:%d, len:%d\n", size, len);
			//如果内部缓存有数据， 就拷贝len 长度的数据到缓存区， 并修改相关参数， 进入下一个循环的条件判断。
			memcpy(buf, s->buf_ptr, len);
			buf += len;
			s->buf_ptr += len;
			size -= len;
		}
	}
	//返回实际读取的字节数。
	return size1 - size;
}
