//�ļ���дģ�鶨������ݽṹ�ͺ��������� ffplay ����Щȫ���ŵ����.h�ļ���
#ifndef AVIO_H
#define AVIO_H

#define URL_EOF (-1)

typedef int64_t offset_t;

#define URL_RDONLY 0
#define URL_WRONLY 1
#define URL_RDWR   2

//URLContext �ṹ��ʾ�������еĵ�ǰ�����ļ�Э��ʹ�õ������ģ�
//���������й����ļ�Э�鹲�е�����(�������ڳ�������ʱ����ȷ����ֵ)�͹��������ṹ���ֶ�
typedef struct URLContext
{
	// ������Ӧ�Ĺ��������ļ�Э��
	struct URLProtocol *prot;
	int flags;
	// �����0����ʾ������С�����ڷ����㹻�Ļ���
	int max_packet_size; // if non zero, the stream is packetized with this max packet size
	// �ڱ����У�����һ���ļ����
	void *priv_data;
	// �ڱ����У���ȡ�����ļ����� filename��ָʾ�����ļ����׵�ַ
	char filename[1]; // specified filename
} URLContext;

//URLProtocol���������ļ�Э�飬�����ڹ��ܺ�����
//һ�ֹ�����ļ�Э���Ӧһ�� URLProtocol�ṹ��
//����ɾ����pipe�� udp�� tcp������Э�飬������һ��file Э��
typedef struct URLProtocol
{
	const char *name;// Э���ļ������������Ի��Ķ����⡣
	int(*url_open)(URLContext *h, const char *filename, int flags);
	int(*url_read)(URLContext *h, unsigned char *buf, int size);
	int(*url_write)(URLContext *h, unsigned char *buf, int size);
	offset_t(*url_seek)(URLContext *h, offset_t pos, int whence);
	int(*url_close)(URLContext *h);
	struct URLProtocol *next;// ������֧�ֵ�����Э�鴮�����������ڱ�������
} URLProtocol;

//ByteIOContext�ṹ��չ URLProtocol�ṹ���ڲ��л�����ƵĹ㷺�����ϵ��ļ���
//���ƹ��������ļ��� IO����
typedef struct ByteIOContext
{
	unsigned char *buffer;//�����׵�ַ
	int buffer_size;//�����С
	unsigned char *buf_ptr,  *buf_end;//�����ָ��, ����ĩָ��
	void *opaque;//ָ��URLContext �ṹ��ָ�룬������ת
	int (*read_buf)(void *opaque, uint8_t *buf, int buf_size);
	int (*write_buf)(void *opaque, uint8_t *buf, int buf_size);
	offset_t(*seek)(void *opaque, offset_t offset, int whence);
	offset_t pos;    // position in the file of the current buffer
	int must_flush;  // true if the next seek should flush
	int eof_reached; // true if eof reached
	int write_flag;  // true if open for writing
	int max_packet_size;//�����0����ʾ�������֡��С�����ڷ����㹻�Ļ��档
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