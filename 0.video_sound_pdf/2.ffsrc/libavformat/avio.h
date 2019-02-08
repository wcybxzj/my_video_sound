//文件读写模块定义的数据结构和函数声明， ffplay 把这些全部放到这个.h文件中
#ifndef AVIO_H
#define AVIO_H

#define URL_EOF (-1)

typedef int64_t offset_t;

#define URL_RDONLY 0
#define URL_WRONLY 1
#define URL_RDWR   2

//URLContext 结构表示程序运行的当前广义文件协议使用的上下文，
//着重于所有广义文件协议共有的属性(并且是在程序运行时才能确定其值)和关联其他结构的字段
typedef struct URLContext
{
	// 关联相应的广义输入文件协议
	struct URLProtocol *prot;
	int flags;
	// 如果非0，表示最大包大小，用于分配足够的缓存
	int max_packet_size; // if non zero, the stream is packetized with this max packet size
	// 在本例中，关联一个文件句柄
	void *priv_data;
	// 在本例中，存取本地文件名， filename仅指示本地文件名首地址
	char filename[1]; // specified filename
} URLContext;

//URLProtocol定义广义的文件协议，着重于功能函数，
//一种广义的文件协议对应一个 URLProtocol结构，
//本例删掉了pipe， udp， tcp等输入协议，仅保留一个file 协议
typedef struct URLProtocol
{
	const char *name;// 协议文件名，便于人性化阅读理解。
	int(*url_open)(URLContext *h, const char *filename, int flags);
	int(*url_read)(URLContext *h, unsigned char *buf, int size);
	int(*url_write)(URLContext *h, unsigned char *buf, int size);
	offset_t(*url_seek)(URLContext *h, offset_t pos, int whence);
	int(*url_close)(URLContext *h);
	struct URLProtocol *next;// 把所有支持的输入协议串链起来，便于遍历查找
} URLProtocol;

//ByteIOContext结构扩展 URLProtocol结构成内部有缓冲机制的广泛意义上的文件，
//改善广义输入文件的 IO性能
typedef struct ByteIOContext
{
	unsigned char *buffer;//缓存首地址
	int buffer_size;//缓存大小
	unsigned char *buf_ptr,  *buf_end;//缓存读指针, 缓存末指针
	void *opaque;//指向URLContext 结构的指针，便于跳转
	int (*read_buf)(void *opaque, uint8_t *buf, int buf_size);
	int (*write_buf)(void *opaque, uint8_t *buf, int buf_size);
	offset_t(*seek)(void *opaque, offset_t offset, int whence);
	offset_t pos;    // position in the file of the current buffer
	int must_flush;  // true if the next seek should flush
	int eof_reached; // true if eof reached
	int write_flag;  // true if open for writing
	int max_packet_size;//如果非0，表示最大数据帧大小，用于分配足够的缓存。
	int error;       // contains the error code or 0 if no error happened
} ByteIOContext;

int url_open(URLContext **h, const char *filename, int flags);
int url_read(URLContext *h, unsigned char *buf, int size);
int url_write(URLContext *h, unsigned char *buf, int size);
offset_t url_seek(URLContext *h, offset_t pos, int whence);
int url_close(URLContext *h);
int url_get_max_packet_size(URLContext *h);

int register_protocol(URLProtocol *protocol);

int init_put_byte(ByteIOContext *s,
		unsigned char *buffer,
		int buffer_size,
		int write_flag,
		void *opaque,
		int(*read_buf)(void *opaque, uint8_t *buf, int buf_size),
		int(*write_buf)(void *opaque, uint8_t *buf, int buf_size),
		offset_t(*seek)(void *opaque, offset_t offset, int whence));

offset_t url_fseek(ByteIOContext *s, offset_t offset, int whence);
void url_fskip(ByteIOContext *s, offset_t offset);
offset_t url_ftell(ByteIOContext *s);
offset_t url_fsize(ByteIOContext *s);
int url_feof(ByteIOContext *s);
int url_ferror(ByteIOContext *s);

int url_fread(ByteIOContext *s, unsigned char *buf, int size); // get_buffer
int get_byte(ByteIOContext *s);
unsigned int get_le32(ByteIOContext *s);
unsigned int get_le16(ByteIOContext *s);

int url_setbufsize(ByteIOContext *s, int buf_size);
int url_fopen(ByteIOContext *s, const char *filename, int flags);
int url_fclose(ByteIOContext *s);

int url_open_buf(ByteIOContext *s, uint8_t *buf, int buf_size, int flags);
int url_close_buf(ByteIOContext *s);

#endif
