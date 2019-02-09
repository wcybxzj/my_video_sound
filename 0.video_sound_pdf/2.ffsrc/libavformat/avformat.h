#ifndef AVFORMAT_H
#define AVFORMAT_H

#ifdef __cplusplus
extern "C"
{
#endif

#define LIBAVFORMAT_VERSION_INT ((50<<16)+(4<<8)+0)
#define LIBAVFORMAT_VERSION     50.4.0
#define LIBAVFORMAT_BUILD       LIBAVFORMAT_VERSION_INT

#define LIBAVFORMAT_IDENT       "Lavf" AV_STRINGIFY(LIBAVFORMAT_VERSION)

#include "../libavcodec/avcodec.h"
#include "avio.h"

#define AVERROR_UNKNOWN     (-1)	// unknown error
#define AVERROR_IO          (-2)	// i/o error
#define AVERROR_NUMEXPECTED (-3)	// number syntax expected in filename
#define AVERROR_INVALIDDATA (-4)	// invalid data found
#define AVERROR_NOMEM       (-5)	// not enough memory
#define AVERROR_NOFMT       (-6)	// unknown format
#define AVERROR_NOTSUPP     (-7)	// operation not supported

#define AVSEEK_FLAG_BACKWARD 1		// seek backward
#define AVSEEK_FLAG_BYTE     2		// seeking based on position in bytes
#define AVSEEK_FLAG_ANY      4		// seek to any frame, even non keyframes

#define AVFMT_NOFILE        0x0001	// no file should be opened

#define PKT_FLAG_KEY		0x0001

#define AVINDEX_KEYFRAME	0x0001

#define AVPROBE_SCORE_MAX	100

#define MAX_STREAMS 20

	//音视频数据包定义，在瘦身后的ffplay 中， 每一个包是一个完整的数据帧。注意保存音视频数据包的内存
	//是malloc 出来的，用完后应及时用free归还给系统。
	typedef struct AVPacket
	{
		int64_t pts; // presentation time stamp in time_base units
		int64_t dts; // decompression time stamp in time_base units
		int64_t pos; // byte position in stream, -1 if unknown
		uint8_t *data;// 实际保存音视频数据缓存的首地址
		int size; // 实际保存音视频数据缓存的大小
		int stream_index;// 前音视频数据包对应的流索引，在本例中用于区别音频还是视频
		int flags;// 数据包的一些标记，比如是否是关键帧等
		void(*destruct)(struct AVPacket*);
	} AVPacket;

	//音视频数据包链表定义， 注意每一个AVPacketList 仅含有一个AVPacket，
	//和传统的很多很多节点的 list不同，不要被 list名字迷惑。
	typedef struct AVPacketList
	{
		AVPacket pkt;
		struct AVPacketList *next;
	} AVPacketList;

	//释放掉音视频数据包占用的内存，把首地址置空是一个很好的习惯
	static inline void av_destruct_packet(AVPacket *pkt)
	{
		av_free(pkt->data);
		pkt->data = NULL;
		pkt->size = 0;
	}

	//判断一些指针，中转一下，释放掉音视频数据包占用的内存
	static inline void av_free_packet(AVPacket *pkt)
	{
		if (pkt && pkt->destruct)
			pkt->destruct(pkt);
	}

	//读文件往数据包中填数据，注意程序跑到这里时，文件偏移量已确定，要读数据的大小也确定，
	//但是数据包的缓存没有分配。分配好内存后，要初始化包的一些变量
	static inline int av_get_packet(ByteIOContext *s, AVPacket *pkt, int size)
	{
		int ret;
		unsigned char *data;
		if ((unsigned)size > (unsigned)size + FF_INPUT_BUFFER_PADDING_SIZE)
			return AVERROR_NOMEM;

		data = (unsigned char*)av_malloc(size + FF_INPUT_BUFFER_PADDING_SIZE);
		if (!data)
			return AVERROR_NOMEM;

		//数据包中 pad部分清 0，这是一个很好的习惯。 缓存清0 不管在什么情况下都是好习惯。
		memset(data + size, 0, FF_INPUT_BUFFER_PADDING_SIZE);

		pkt->pts = AV_NOPTS_VALUE;
		pkt->dts = AV_NOPTS_VALUE;
		pkt->pos =  - 1;
		pkt->flags = 0;
		pkt->stream_index = 0;
		pkt->data = data;
		pkt->size = size;
		pkt->destruct = av_destruct_packet;

		pkt->pos = url_ftell(s);
		//实际读广义文件填充数据包，如果读文件错误时通常是到了末尾，要归还刚刚malloc 出来的内存
		ret = url_fread(s, pkt->data, size);
		if (ret <= 0)
			av_free_packet(pkt);
		else
			pkt->size = ret;

		return ret;
	}

	//为识别文件格式，要读一部分文件头数据来分析匹配 ffplay 支持的文件格式文件特征。
	//于是 AVProbeData 结构就定义了文件名，首地址和大小。此处的读独立于其他文件操作
	typedef struct AVProbeData
	{
		const char *filename;
		unsigned char *buf;
		int buf_size;
	} AVProbeData;

	//文件索引结构， flags 和size 位定义是为了节省内存
	typedef struct AVIndexEntry
	{
		int64_t pos;
		int64_t timestamp;
		int flags: 2;
		int size: 30; //yeah trying to keep the size of this small to reduce memory requirements (its 24 vs 32 byte due to possible 8byte align)
	} AVIndexEntry;

	//AVStream 抽象的表示一个媒体流，定义了所有媒体一些通用的属性
	typedef struct AVStream
	{
		AVCodecContext *actx;  // codec context, change from AVCodecContext *codec;
		void *priv_data;       // AVIStream
		AVRational time_base; // 由 av_set_pts_info()函数初始化
		AVIndexEntry *index_entries; // only used if the format does not support seeking natively
		int nb_index_entries;
		int index_entries_allocated_size;
		double frame_last_delay;//帧最后延迟
	} AVStream;

	//AVFormatParameters 结构在瘦身后的ffplay 中没有实际意义，为保证函数接口不变，没有删除
	typedef struct AVFormatParameters
	{
		int dbg; //only for debug
	} AVFormatParameters;

	//AVInputFormat定义输入文件容器格式，着重于功能函数，
	//一种文件容器格式对应一个AVInputFormat结构，在程序运行时有多个实例，但瘦身后ffplay仅一个实例
	typedef struct AVInputFormat
	{
		const char *name;//文件格式容器格式名
		int priv_data_size;//程序运行时,文件容器格式对应的上下文结构大小,便于内存分配
		int(*read_probe)(AVProbeData*);
		int(*read_header)(struct AVFormatContext *, AVFormatParameters *ap);
		int(*read_packet)(struct AVFormatContext *, AVPacket *pkt);
		int(*read_close)(struct AVFormatContext*);
		const char *extensions;     // 文件扩展名
		struct AVInputFormat *next;//将所有格式的文件容器格式连成一个链表
	} AVInputFormat;

	//AVFormatContext 结构表示程序运行的当前文件容器格式使用的上下文，
	//着重于所有文件容器共有的属性，程序运行后仅一个实例
	typedef struct AVFormatContext  // format I/O context
	{
		struct AVInputFormat *iformat;// 关联程序运行时，实际的文件容器格式指针
		void *priv_data;// 关联具体文件容器格式上下文的指针，在本例中是AVIContext
		ByteIOContext pb;// 关联广义输入文件
		int nb_streams;// 广义输入文件中媒体流计数
		AVStream *streams[MAX_STREAMS];// 关联广义输入文件中的媒体流
	} AVFormatContext;

	int avidec_init(void);

	void av_register_input_format(AVInputFormat *format);

	void av_register_all(void);

	AVInputFormat *av_probe_input_format(AVProbeData *pd, int is_opened);
	int match_ext(const char *filename, const char *extensions);

	int av_open_input_stream(AVFormatContext **ic_ptr, ByteIOContext *pb, const char *filename, 
			AVInputFormat *fmt, AVFormatParameters *ap);

	int av_open_input_file(AVFormatContext **ic_ptr, const char *filename, AVInputFormat *fmt, 
			int buf_size, AVFormatParameters *ap);

	int av_read_frame(AVFormatContext *s, AVPacket *pkt);
	int av_read_packet(AVFormatContext *s, AVPacket *pkt);
	void av_close_input_file(AVFormatContext *s);
	AVStream *av_new_stream(AVFormatContext *s, int id);
	void av_set_pts_info(AVStream *s, int pts_wrap_bits, int pts_num, int pts_den);

	int av_index_search_timestamp(AVStream *st, int64_t timestamp, int flags);
	int av_add_index_entry(AVStream *st, int64_t pos, int64_t timestamp, int size, int distance, int flags);

	int strstart(const char *str, const char *val, const char **ptr);
	void pstrcpy(char *buf, int buf_size, const char *str);

#ifdef __cplusplus
}

#endif

#endif
