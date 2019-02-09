/*
AVI文件解析的相关函数，注意有些地方有些技巧性代码。

注意1：AVI 文件容器媒体数据有两种存放方式，非交织存放和交织存放。
交织存放:
就是音视频数据以帧为最小连续单位，
相互间隔存放，这样音视频帧互相交织在一起，
并且存放的间隔没有特别规定；

非交织存放:
就是把单一媒体的所有数据帧连续存放在一起，非交织存放的avi 文件很少。

注意2： AVI 文件索引结构AVIINDEXENTRY 中的dwChunkOffset 字段指示的偏移有的是相对文件开始字节的偏
移，有的是相对文件数据块chunk 的偏移。

注意3：附带的 avi测试文件是交织存放的
*/

#include "avformat.h"

#include <assert.h>

#define AVIIF_INDEX			0x10

#define AVIF_HASINDEX		0x00000010	// Index at end of file?
#define AVIF_MUSTUSEINDEX	0x00000020

#define INT_MAX	2147483647

#define MKTAG(a,b,c,d) (a | (b << 8) | (c << 16) | (d << 24))

#define FFMIN(a,b) ((a) > (b) ? (b) : (a))
#define FFMAX(a,b) ((a) > (b) ? (a) : (b))

static int avi_load_index(AVFormatContext *s);
static int guess_ni_flag(AVFormatContext *s);

//AVI 文件中的流参数定义，和AVStream 数据结构协作。
typedef struct
{
	// 帧偏移，视频用帧计数，音频用字节计数，用于计算pts 表示时间
	int64_t frame_offset; // current frame(video) or byte(audio) counter(used to compute the pts)
	// 表示需要读的数据大小，初值是帧裸数组大小，全部读完后为0。
	int remaining;
	// 包大小，非交织和帧裸数据大小相同，交织比帧裸数据大小大8 字节
	int packet_size;

	int scale;
	int rate;
	int sample_size; // size of one sample (or packet) (in the rate/scale sense) in bytes

	int64_t cum_len; // temporary storage (used during seek)

	int prefix;      // normally 'd'<<8 + 'c' or 'w'<<8 + 'b'
	int prefix_count;
} AVIStream;

//AVI文件中的文件格式参数相关定义，和AVFormatContext 协作。
typedef struct
{
	int64_t riff_end;    // RIFF块大小
	int64_t movi_list;   // 媒体数据块开始字节相对文件开始字节的偏移
	int64_t movi_end;    // 媒体数据块开始字节相对文件开始字节的偏移
	int non_interleaved; // 指示是否是非交织AVI
	// 为了和AVPacket 中的stream_index 相区别加一个后缀。
	// 指示当前应该读取的流的索引。初值为-1，表示没有确定应该读的流。
	// 实际表示AVFormatContext 结构中AVStream*streams[]数组中的索引。}
	int stream_index_2;  // 为了和AVPacket中的stream_index相区别
} AVIContext;

//CodecTag数据结构，用于关联具体媒体格式的ID和Tag 标签。
typedef struct
{
	int id;
	unsigned int tag;
} CodecTag;

//瘦身后的ffplay支持的一些视频媒体ID和Tag 标签数组。
const CodecTag codec_bmp_tags[] =
{
	{CODEC_ID_MSRLE, MKTAG('m', 'r', 'l', 'e')},
	{CODEC_ID_MSRLE, MKTAG(0x1, 0x0, 0x0, 0x0)},
	{CODEC_ID_NONE,  0},
};

//瘦身后的ffplay支持的一些音频媒体ID和Tag 标签数组。
const CodecTag codec_wav_tags[] =
{
	{CODEC_ID_TRUESPEECH, 0x22},
	{0, 0},
};

//以媒体tag标签为关键字，查找 codec_bmp_tags或 codec_wav_tags数组，返回媒体ID。
enum CodecID codec_get_id(const CodecTag *tags, unsigned int tag)
{
	while (tags->id != CODEC_ID_NONE)
	{
		//比较Tag关键字，相等时返回对应媒体ID。
		if (toupper((tag >> 0) &0xFF) == toupper((tags->tag >> 0) &0xFF)
				&& toupper((tag >> 8) &0xFF) == toupper((tags->tag >> 8) &0xFF)
				&& toupper((tag >> 16)&0xFF) == toupper((tags->tag >> 16)&0xFF)
				&& toupper((tag >> 24)&0xFF) == toupper((tags->tag >> 24)&0xFF))
			return (enum CodecID)tags->id;

		tags++;
	}
	//所有关键字都不匹配，返回 CODEC_ID_NONE。
	return CODEC_ID_NONE;
}

//校验AVI文件，读取AVI文件媒体数据块的偏移大小信息， 和avi_probe()函数部分相同。
static int get_riff(AVIContext *avi, ByteIOContext *pb)
{
	//printf("get_riff()\n");
	uint32_t tag;
	tag = get_le32(pb);

	//校验 AVI文件开始关键字串"RIFF"。
	if (tag != MKTAG('R', 'I', 'F', 'F'))
		return  - 1;

	avi->riff_end = get_le32(pb); // RIFF chunk size
	//printf("avi->riff_end:%d\n", avi->riff_end);//64360
	avi->riff_end += url_ftell(pb); // RIFF chunk end
	//printf("avi->riff_end:%d\n", avi->riff_end);//64368
	tag = get_le32(pb);

	//校验 AVI文件关键字串"AVI "或"AVIX"。
	if (tag != MKTAG('A', 'V', 'I', ' ') && tag != MKTAG('A', 'V', 'I', 'X'))
		return  - 1;

	//如果通过 AVI文件关键字串"RIFF"和"AVI "或"AVIX"校验，就认为是 AVI文件，这种方式非常可靠。
	return 0;
}

//排序建立AVI索引表，函数名为clean_index,不准确，功能以具体的实现代码为准。
static void clean_index(AVFormatContext *s)
{
	int i, j;

	for (i = 0; i < s->nb_streams; i++)
	{
		//对每个流都建一个独立的索引表。
		AVStream *st = s->streams[i];
		AVIStream *ast = (AVIStream *)st->priv_data;
		int n = st->nb_index_entries;
		int max = ast->sample_size;
		int64_t pos, size, ts;

		//如果索引表项大于1，则认为索引表已建好，不再排序重建。如果sample_size 为0,则没办法重建。
		if (n != 1 || ast->sample_size == 0)
			continue;

		//此种情况多半是用在非交织存储的avi音频流。不管交织还是非交织存储，视频流通常都有索引。
		//防止包太小需要太多的索引项占有大量内存，设定最小帧size 阈值为1024。比如有些音频流，最小解
		//码帧只十多个字节， 如果文件比较大则在索引上耗费太多内存
		while (max < 1024)
			max += max;

		//取位置，大小，时钟等基本参数。
		pos = st->index_entries[0].pos;
		size = st->index_entries[0].size;
		ts = st->index_entries[0].timestamp;

		//以max指定的字节打包成帧，添加到索引表。
		for (j = 0; j < size; j += max)
		{
			av_add_index_entry(st, pos + j, ts + j / ast->sample_size, FFMIN(max, size - j), 0, AVINDEX_KEYFRAME);
		}
	}
}//end clean_index()

/*
 ./ffplay
	av_open_input_file()
	url_open()
	proto_str:file
	up->name
	avo_read_header
	size:1360
	LIST
	size:56
	avih
	size:1140
	LIST
	size:56
	strh
	size:1064
	strf
	size:126
	LIST
	size:56
	strh
	size:50
	strf
	size:2
	url_fskip size:2
	size:62468
	LIST
	movi
	avi_load_index()
*/
//读取AVI文件头，读取AVI文件索引，并识别具体的媒体格式，关联一些数据结构。
static int avi_read_header(AVFormatContext *s, AVFormatParameters *ap)
{
	printf("avi_read_header()\n");
	AVIContext *avi = (AVIContext *)s->priv_data;
	ByteIOContext *pb = &s->pb;
	uint32_t tag, tag1, handler;
	int codec_type, stream_index, frame_period, bit_rate;
	unsigned int size, nb_frames;
	int i, n;
	AVStream *st;
	AVIStream *ast;

	//当前应该读取的流的索引赋初值为-1，表示没有确定应该读的流。
	avi->stream_index_2 =  - 1;

	//校验AVI文件，读取AVI文件媒体数据块的偏移大小信息。
	if (get_riff(avi, pb) < 0)
		return  - 1;

	stream_index =  - 1; // first list tag
	codec_type =  - 1;
	frame_period = 0;

	//AVI 文件的基本结构是块，一个文件有多个块，并且块还可以内嵌，在这里循环读文件头中的块。
	for (;;)
	{
		if (url_feof(pb))
			goto fail;

		//读取每个块的标签和大小。
		tag = get_le32(pb);
		size = get_le32(pb);
		printf("size:%d\n", size);
		switch (tag)
		{
			case MKTAG('L', 'I', 'S', 'T'):  // ignored, except when start of video packets
				printf("LIST\n");
				tag1 = get_le32(pb);
				if (tag1 == MKTAG('m', 'o', 'v', 'i'))
				{
					printf("movi\n");
					//读取movi媒体数据块的偏移和大小。
					avi->movi_list = url_ftell(pb) - 4;
					if (size)
						avi->movi_end = avi->movi_list + size;
					else
						avi->movi_end = url_fsize(pb);
					//AVI 文件头后面是movi 媒体数据块，所以到了movi块，文件头肯定读完，需要跳出循环。
					goto end_of_header; // 读到数据段就认为文件头结束了，就goto
				}
				break;
			case MKTAG('a', 'v', 'i', 'h'):  // avi header, using frame_period is bad idea
				printf("avih\n");
				frame_period = get_le32(pb);
				bit_rate = get_le32(pb) *8;
				get_le32(pb);
				//读取 non_interleaved的初值。
				avi->non_interleaved |= get_le32(pb) &AVIF_MUSTUSEINDEX;

				url_fskip(pb, 2 *4);
				n = get_le32(pb);
				for (i = 0; i < n; i++)
				{
					//读取流数目n后，分配 AVStream 和 AVIStream数据结构，在 st->priv_data = ast;行把它们关联起来。
					//特别注意 av_new_stream()函数关联AVFormatContext 和AVStream 结构， 分配关联AVCodecContext 结构
					AVIStream *ast;
					st = av_new_stream(s, i);
					if (!st)
						goto fail;

					ast = (AVIStream *)av_mallocz(sizeof(AVIStream));
					if (!ast)
						goto fail;
					st->priv_data = ast;

					st->actx->bit_rate = bit_rate;
				}
				url_fskip(pb, size - 7 * 4);
				break;
			case MKTAG('s', 't', 'r', 'h'):  // stream header
				printf("strh\n");
				//指示当前流在 AVFormatContext结构中 AVStream*streams[MAX_STREAMS]数组中的索引。
				stream_index++;
				//从 strh块读取所有流共有的一些信息，跳过有些不用的字段，填写需要的字段。
				tag1 = get_le32(pb);
				handler = get_le32(pb);

				//出现这种情况通常代表媒体文件数据有错， ffplay 简单的跳过。
				if (stream_index >= s->nb_streams)
				{
					url_fskip(pb, size - 8);
					break;
				}
				st = s->streams[stream_index];
				ast = (AVIStream *)st->priv_data;

				get_le32(pb); // flags
				get_le16(pb); // priority
				get_le16(pb); // language
				get_le32(pb); // initial frame
				ast->scale = get_le32(pb);
				ast->rate = get_le32(pb);
				if (ast->scale && ast->rate)
				{}
				else if (frame_period)
				{
					ast->rate = 1000000;
					ast->scale = frame_period;
				}
				else
				{
					ast->rate = 25;
					ast->scale = 1;
				}
				//设置当前流的时间信息，用于计算 pts表示时间，进而同步。
				av_set_pts_info(st, 64, ast->scale, ast->rate);

				ast->cum_len = get_le32(pb); // start
				nb_frames = get_le32(pb);

				get_le32(pb); // buffer size
				get_le32(pb); // quality
				ast->sample_size = get_le32(pb); // sample ssize

				switch (tag1)
				{
					case MKTAG('v', 'i', 'd', 's'):
						//特别注意视频流的每一帧大小不同，所以 sample_size 设置为 0；
						//对比音频流每一帧大小固定的情况。
						codec_type = CODEC_TYPE_VIDEO;
						ast->sample_size = 0;
						break;
					case MKTAG('a', 'u', 'd', 's'):
						codec_type = CODEC_TYPE_AUDIO;
						break;
					case MKTAG('t', 'x', 't', 's'):
						//FIXME
						codec_type = CODEC_TYPE_DATA; //CODEC_TYPE_SUB ?  FIXME
						break;
					case MKTAG('p', 'a', 'd', 's'):
						//如果是填充流， stream_index 减1 就实现了简单的丢弃，
						//不计入流数目总数。
						codec_type = CODEC_TYPE_UNKNOWN;
						stream_index--;
						break;
					default:
						goto fail;
				}
				ast->frame_offset = ast->cum_len *FFMAX(ast->sample_size, 1);
				url_fskip(pb, size - 12 * 4);
				break;
			case MKTAG('s', 't', 'r', 'f'):  // stream header
				printf("strf\n");
				//从strf块读取流中编解码器的一些信息，跳过有些不用的字段，填写需要的字段。
				//注意有些编解码器需要的附加信息从此块中读出，保持至 extradata并最终传给相应的编解码器
				if (stream_index >= s->nb_streams)
				{
					url_fskip(pb, size);
				}
				else
				{
					st = s->streams[stream_index];
					switch (codec_type)
					{
						case CODEC_TYPE_VIDEO:    // BITMAPINFOHEADER
							get_le32(pb); // size
							st->actx->width = get_le32(pb);
							st->actx->height = get_le32(pb);
							get_le16(pb); // panes
							st->actx->bits_per_sample = get_le16(pb); // depth
							tag1 = get_le32(pb);
							get_le32(pb); // ImageSize
							get_le32(pb); // XPelsPerMeter
							get_le32(pb); // YPelsPerMeter
							get_le32(pb); // ClrUsed
							get_le32(pb); // ClrImportant

							if (size > 10 *4 && size < (1 << 30))
							{
								//对视频， extradata 通常是保存的是BITMAPINFO
								st->actx->extradata_size = size - 10 * 4;
								st->actx->extradata = (unsigned char*) av_malloc(st->actx->extradata_size +
										FF_INPUT_BUFFER_PADDING_SIZE);
								url_fread(pb, st->actx->extradata, st->actx->extradata_size);
							}

							if (st->actx->extradata_size &1)
								get_byte(pb);
							/* Extract palette from extradata if bpp <= 8 */
							/* This code assumes that extradata contains only palette */
							/* This is true for all paletted codecs implemented in ffmpeg */
							if (st->actx->extradata_size && (st->actx->bits_per_sample <= 8))
							{
								int min = FFMIN(st->actx->extradata_size, AVPALETTE_SIZE);
								st->actx->palctrl = (AVPaletteControl*) av_mallocz(sizeof(AVPaletteControl));
								memcpy(st->actx->palctrl->palette, st->actx->extradata, min);
								st->actx->palctrl->palette_changed = 1;
							}
							st->actx->codec_type = CODEC_TYPE_VIDEO;
							st->actx->codec_id = codec_get_id(codec_bmp_tags, tag1);
							st->frame_last_delay = 1.0 * ast->scale / ast->rate;
							break;
						case CODEC_TYPE_AUDIO:
							{
								AVCodecContext *actx = st->actx;
								int id = get_le16(pb);
								actx->codec_type = CODEC_TYPE_AUDIO;
								actx->channels = get_le16(pb);
								actx->sample_rate = get_le32(pb);
								actx->bit_rate = get_le32(pb) *8;
								actx->block_align = get_le16(pb);
								if (size == 14)  // We're dealing with plain vanilla WAVEFORMAT
									actx->bits_per_sample = 8;
								else
									actx->bits_per_sample = get_le16(pb);

								// wav_codec_get_id(id, codec->bits_per_sample);
								actx->codec_id = codec_get_id(codec_wav_tags, id);

								if (size > 16)
								{
									actx->extradata_size = get_le16(pb); // We're obviously dealing with WAVEFORMATEX
									if (actx->extradata_size > 0)
									{
										//对音频， extradata 通常是保存的是WAVEFORMATEX
										if (actx->extradata_size > size - 18)
											actx->extradata_size = size - 18;
										actx->extradata = (unsigned char*)av_mallocz(actx->extradata_size +
												FF_INPUT_BUFFER_PADDING_SIZE);
										url_fread(pb, actx->extradata, actx->extradata_size);
									}
									else
									{
										actx->extradata_size = 0;
									}
									// It is possible for the chunk to contain garbage at the end
									if (size - actx->extradata_size - 18 > 0)
										url_fskip(pb, size - actx->extradata_size - 18);
								}
							}

							if (size % 2) // 2-aligned (fix for Stargate SG-1 - 3x18 - Shades of Grey.avi)
								url_fskip(pb, 1);

							break;
						default:
							//对其他流类型， ffplay 简单的设置为data 流。常规的是音频流和视频流，其他的少见。
							st->actx->codec_type = CODEC_TYPE_DATA;
							st->actx->codec_id = CODEC_ID_NONE;
							url_fskip(pb, size);
							break;
					}
				}
				break;
			default:  // skip tag
				//对其他不识别的块 chunk，跳过。
				size += (size &1);
				url_fskip(pb, size);
				printf("url_fskip size:%d\n", size);
				break;
		}
	}

end_of_header:
	if (stream_index != s->nb_streams - 1) // check stream number
	{
fail:
		//校验流的数目，如果有误，释放相关资源，返回-1错误。
		for (i = 0; i < s->nb_streams; i++)
		{
			av_freep(&s->streams[i]->actx->extradata);
			av_freep(&s->streams[i]);
		}
		return  - 1;
	}

	//加载 AVI文件索引。
	printf("avi_load_index()\n");
	avi_load_index(s);

	//判别是否是非交织 avi。
	avi->non_interleaved |= guess_ni_flag(s);
	if (avi->non_interleaved){
		//对那些非交织存储的媒体流，人工的补上索引，便于读取操作。
		//printf("clean_index()\n");
		clean_index(s);
	}
	return 0;
}

//avi 文件可以简单认为音视频媒体数据时间基相同，因此音视频数据需要同步读取，同步解码，播放才 能同步。
//交织存储的 avi 文件:
//临近存储的音视频帧解码时间表示时间相近，
//微小的解码时间表示时间差别可以用帧缓存队列抵消，所以可以简单的按照文件顺序读取媒体数据。
//非交织存储的 avi 文件:
//视频和音频这两种媒体数据相隔甚远，小缓存简单的顺序读文件时，
//不能同时读到音频和视频数据，最后导致不同步， ffplay 采取按最近时间点来决定读音频还是视频数据
int avi_read_packet(AVFormatContext *s, AVPacket *pkt)
{
	AVIContext *avi = (AVIContext*)s->priv_data;
	ByteIOContext *pb = &s->pb;
	int n, d[8], size;
	offset_t i, sync;

	if (avi->non_interleaved)
	{
		//如果是非交织 AVI，用最近时间点来决定读取视频还是音频数据。
		int best_stream_index = 0;
		AVStream *best_st = NULL;
		AVIStream *best_ast;
		int64_t best_ts = INT64_MAX;
		int i;
		//遍历所有媒体流，按照已经播放的流数据，计算下一个最近的时间点。
		for (i = 0; i < s->nb_streams; i++)
		{
			AVStream *st = s->streams[i];
			AVIStream *ast = (AVIStream*)st->priv_data;
			int64_t ts = ast->frame_offset;
			//把帧偏移换算成帧数。
			if (ast->sample_size)
				ts /= ast->sample_size;

			//把帧数换算成pts表示时间。
			ts = av_rescale(ts, AV_TIME_BASE *(int64_t)st->time_base.num, st->time_base.den);
			//取最小的时间点对应的时间，流指针，流索引作为要读取的最佳 (读取)流参数。
			//每次读取时间点(ast->frame_offset)最近的包
			if (ts < best_ts)
			{
				best_ts = ts;
				best_st = st;
				best_stream_index = i;
			}
		}
		//保存最佳流对应的 AVIStream，便于432 行赋值并传递参数packet_size 和remaining。
		best_ast = (AVIStream*)best_st->priv_data;
		//换算最小的时间点，查找索引表取出对应的索引。
		//在缓存足够大，一次性完整读取帧数据时，此时 best_ast->remaining 参数为0。
		best_ts = av_rescale(best_ts, best_st->time_base.den, AV_TIME_BASE *(int64_t)best_st->time_base.num);
		if (best_ast->remaining)
			i = av_index_search_timestamp(best_st, best_ts, AVSEEK_FLAG_ANY | AVSEEK_FLAG_BACKWARD);
		else
			i = av_index_search_timestamp(best_st, best_ts, AVSEEK_FLAG_ANY);

		if (i >= 0)
		{
			//找到最佳索引，取出其他参数， 在 426行 seek到相应位置，在 430行保存最佳流索引，在432 行保存
			//并传递要读取的数据大小(通过最佳流索引找到最佳流，再找到对应AVIStream 结构，再找到数据大小)。
			int64_t pos = best_st->index_entries[i].pos;
			pos += best_ast->packet_size - best_ast->remaining;
			url_fseek(&s->pb, pos + 8, SEEK_SET);//原书426行
			assert(best_ast->remaining <= best_ast->packet_size);
			avi->stream_index_2 = best_stream_index;//原书430行
			if (!best_ast->remaining)
				best_ast->packet_size = best_ast->remaining = best_st->index_entries[i].size;//原书432行
		}
	}

resync:

	if (avi->stream_index_2 >= 0)
	{
		//如果找到最佳流索引，以此为根参数，取出其他参数和读取媒体数据。
		AVStream *st = s->streams[avi->stream_index_2];
		AVIStream *ast = (AVIStream*) st->priv_data;
		int size;

		if (ast->sample_size <= 1) // minorityreport.AVI block_align=1024 sample_size=1 IMA-ADPCM
			size = INT_MAX;
		else if (ast->sample_size < 32)
			size = 64 * ast->sample_size;
		else
			size = ast->sample_size;

		//在缓存足够大，一次全部读取一帧媒体数据的情况下， 451 行判断不成立， size 等于 ast->sample_size
		if (size > ast->remaining){//原书451行
			size = ast->remaining;
		}

		//调用 av_get_packet()函数实际读取媒体数据到pkt 包中。
		av_get_packet(pb, pkt, size);

		//修改媒体流的一些其他参数。
		pkt->dts = ast->frame_offset;

		if (ast->sample_size)
			pkt->dts /= ast->sample_size;

		pkt->stream_index = avi->stream_index_2;
		//在简单情况顺序播放时，下面if没有什么实际意义。
		if (st->actx->codec_type == CODEC_TYPE_VIDEO)
		{
			if (st->index_entries)
			{
				AVIndexEntry *e;
				int index;
				index = av_index_search_timestamp(st, pkt->dts, 0);
				e = &st->index_entries[index];
				if (index >= 0 && e->timestamp == ast->frame_offset)
				{
					if (e->flags &AVINDEX_KEYFRAME)
						pkt->flags |= PKT_FLAG_KEY;
				}
			}
			else
			{
				pkt->flags |= PKT_FLAG_KEY; // if no index, better to say that all frames are key frames
			}
		}
		else
		{
			//如果没有索引，较好的办法是把所有帧都设为关键帧。
			pkt->flags |= PKT_FLAG_KEY;
		}

		//修改帧偏移。
		if (ast->sample_size)
			ast->frame_offset += pkt->size;
		else
			ast->frame_offset++;

		ast->remaining -= size;
		if (!ast->remaining)
		{
			//缓存足够大时，程序一定跑到这里，复位标志性参数。
			avi->stream_index_2 =  - 1;
			ast->packet_size = 0;
			if (size &1)
			{
				get_byte(pb);
				size++;
			}
		}
		//返回实际读到的数据大小。
		return size;
	}

	//把数组 d[8]清为-1，为了在下面的流标记查找时不会出错。
	memset(d,  - 1, sizeof(int) *8);
	for (i = sync = url_ftell(pb); !url_feof(pb); i++)
	{
		//交织 avi时顺序读取文件，媒体数据。
		int j;

		if (i >= avi->movi_end)
			break;
		//首先要找到流标记，比如 00db,00dc,01wb等。在32bit CPU 上为存取数据方便，
		//把avi文件中的帧标记和帧大小共 8个字节对应赋值到 int型数组d[8]中，这样每次是整数操作。
		for (j = 0; j < 7; j++)
			d[j] = d[j + 1];

		//上面把整型缓存前移一个单位。
		//下面从文件中读一个字节补充到整型缓存，计算包大小和流索引。
		d[7] = get_byte(pb);

		size = d[4] + (d[5] << 8) + (d[6] << 16) + (d[7] << 24);

		if (d[2] >= '0' && d[2] <= '9' && d[3] >= '0' && d[3] <= '9')
		{
			n = (d[2] - '0') *10+(d[3] - '0');
		}
		else
		{
			n = 100; //invalid stream id
		}
		//校验size 大小，如果偏移位置加size 超过数据块大小就不是有效的流标记。
		//校验流索引，如果<0就不是有效的流标记。流索引从0开始计数，媒体文件通常不超过10个流
		if (i + size > avi->movi_end || d[0] < 0)
			continue;

		//处理诸如junk等需要跳过的块。
		if ((d[0] == 'i' && d[1] == 'x' && n < s->nb_streams)
				|| (d[0] == 'J' && d[1] == 'U' && d[2] == 'N' && d[3] == 'K'))
		{
			url_fskip(pb, size);
			goto resync;
		}

		//计算流索引号n。
		if (d[0] >= '0' && d[0] <= '9' && d[1] >= '0' && d[1] <= '9')
		{
			n = (d[0] - '0') *10+(d[1] - '0');
		}
		else
		{
			n = 100; //invalid stream id
		}

		//parse ##dc/##wb
		if (n < s->nb_streams)
		{
			//如果流索引号n比流总数小，认为有效。 (我个人认为这个校验不太严格。 )
			AVStream *st;
			AVIStream *ast;
			st = s->streams[n];
			ast = (AVIStream*) st->priv_data;

			if(sync + 9 <= i)
			{
				int dbg=0;
			}
			else
			{
				int dbg1=0;
			}

			if (((ast->prefix_count < 5 || sync + 9 > i) && d[2] < 128 && d[3] < 128)
					|| d[2] * 256 + d[3] == ast->prefix)
			{
				//if(d[2]*256+d[3]==ast->prefix)为真表示 "db","dc","wb"等字串匹配，找到正确帧标记。
				//判断 d[2]<128 && d[3]<128 是因为 'd','b','c','w'等字符的 ascii码小于 128。
				//判断 ast->prefix_count<5 || sync + 9 > i，是判断单一媒体的 5帧内或找帧标记超过 9个字节。
				//563 行到569 行是单一媒体帧边界初次识别成功和以后识别成功的简单处理，计数自增或保存标记。
				if (d[2] * 256 + d[3] == ast->prefix){//原书563行
					ast->prefix_count++;
				}
				else
				{
					ast->prefix = d[2] *256+d[3];
					ast->prefix_count = 0;
				}//原书569行

				//找到相应的流索引后，保存相关参数，跳转到实质性读媒体程序。
				avi->stream_index_2 = n;
				ast->packet_size = size + 8;
				ast->remaining = size;
				goto resync;
			}
		}

		// palette changed chunk
		if (d[0] >= '0' && d[0] <= '9' && d[1] >= '0' && d[1] <= '9'
				&& (d[2] == 'p' && d[3] == 'c') && n < s->nb_streams && i + size <= avi->movi_end)
		{
			//处理调色板改变块数据，读取调色板数据到编解码器上下文的调色板数组中。
			AVStream *st;
			int first, clr, flags, k, p;

			st = s->streams[n];

			first = get_byte(pb);
			clr = get_byte(pb);
			if (!clr) // all 256 colors used
				clr = 256;
			flags = get_le16(pb);
			p = 4;
			for (k = first; k < clr + first; k++)
			{
				int r, g, b;
				r = get_byte(pb);
				g = get_byte(pb);
				b = get_byte(pb);
				get_byte(pb);
				st->actx->palctrl->palette[k] = b + (g << 8) + (r << 16);
			}
			st->actx->palctrl->palette_changed = 1;
			goto resync;
		}
	}

	return  - 1;
}

//实质读取AVI文件的索引。
static int avi_read_idx1(AVFormatContext *s, int size)
{
	AVIContext *avi = (AVIContext*) s->priv_data;
	ByteIOContext *pb = &s->pb;
	int nb_index_entries, i;
	AVStream *st;
	AVIStream *ast;
	unsigned int index, tag, flags, pos, len;
	unsigned last_pos =  - 1;

	//如果没有索引块chunk，直接返回。
	nb_index_entries = size / 16;
	if (nb_index_entries <= 0)
		return  - 1;

	//遍历整个索引项。
	for (i = 0; i < nb_index_entries; i++)// read the entries and sort them in each stream component
	{
		tag = get_le32(pb);
		flags = get_le32(pb);
		pos = get_le32(pb);
		len = get_le32(pb);
		//如果第一个索引指示的偏移量大于数据块的偏移量，则索引指示的偏移量是相对文件开始字节的偏移量。
		//索引加载到内存后，
		//如果是相对数据块的偏移量就要换算成相对于文件开始字节的偏移量， 便于seek
		//操作。在 631行和 633行统一处理这两个情况。
		if (i == 0 && pos > avi->movi_list){//原文631行
			avi->movi_list = 0;
		}

		pos += avi->movi_list;//原文633行

		//计算流ID，如索引块中的00dc， 01wb等关键字表示的流ID分别为数字0和1。
		index = ((tag &0xff) - '0') *10;
		index += ((tag >> 8) &0xff) - '0';
		if (index >= s->nb_streams)
			continue;

		st = s->streams[index];
		ast = (AVIStream*) st->priv_data;

		if (last_pos == pos)
			avi->non_interleaved = 1;
		else
			av_add_index_entry(st, pos, ast->cum_len, len, 0, (flags &AVIIF_INDEX) ? AVINDEX_KEYFRAME : 0);

		if (ast->sample_size)
			ast->cum_len += len / ast->sample_size;
		else
			ast->cum_len++;
		last_pos = pos;
	}
	return 0;
}

//判断是否是非交织存放媒体数据，其中ni 是non_interleaved 的缩写，非交织的意思。
//如果是非交织存放返回 1，交织存放返回0。
//非交织存放的 avi 文件，如果有多个媒体流，肯定有某个流的开始字节文件偏移量大于其他某个流的
//末尾字节的文件偏移量。程序利用这个来判断是否是非交织存放，否则认定为交织存放。
static int guess_ni_flag(AVFormatContext *s)
{
	int i;
	int64_t last_start = 0;
	int64_t first_end = INT64_MAX;
	//遍历AVI文件中所有的索引，取流开始偏移量的最大值和末尾偏移量的最小值判断。
	for (i = 0; i < s->nb_streams; i++)
	{
		AVStream *st = s->streams[i];
		int n = st->nb_index_entries;
		//如果某个流没有index项，认为这个流没有数据，这个流忽略不计。
		if (n <= 0)
			continue;
		//遍历AVI文件中所有的索引，取流开始偏移量的最大值。
		if (st->index_entries[0].pos > last_start)
			last_start = st->index_entries[0].pos;
		//遍历AVI文件中所有的索引，取流末尾偏移量的最小值。
		if (st->index_entries[n - 1].pos < first_end)
			first_end = st->index_entries[n - 1].pos;
	}
	//如果某个流的开始最大值大于某个流的末尾最小值，认为是非交织存储，否则是交织存储。
	return last_start > first_end;
}

//加载AVI文件索引块chunk， 特别注意在avi_read_idx1()函数调用的 av_add_index_entry()函数是分媒
//体类型按照时间顺序重新排序的
static int avi_load_index(AVFormatContext *s)
{
	AVIContext *avi = (AVIContext*)s->priv_data;
	ByteIOContext *pb = &s->pb;
	uint32_t tag, size;
	offset_t pos = url_ftell(pb);

	url_fseek(pb, avi->movi_end, SEEK_SET);

	for (;;)
	{
		if (url_feof(pb))
			break;
		tag = get_le32(pb);
		size = get_le32(pb);

		switch (tag)
		{
			case MKTAG('i', 'd', 'x', '1'):
				if (avi_read_idx1(s, size) < 0)
				goto skip;
				else
					goto the_end;
				break;
			default:
skip:
				size += (size &1);
				url_fskip(pb, size);
				break;
		}
	}
the_end:
	url_fseek(pb, pos, SEEK_SET);
	return 0;
}

//关闭AVI文件，释放内存和其他相关资源。
static int avi_read_close(AVFormatContext *s)
{
	int i;
	AVIContext *avi = (AVIContext*)s->priv_data;

	for (i = 0; i < s->nb_streams; i++)
	{
		AVStream *st = s->streams[i];
		AVIStream *ast = (AVIStream*)st->priv_data;
		av_free(ast);
		av_free(st->actx->extradata);
		av_free(st->actx->palctrl);
	}

	return 0;
}

//AVI 文件判断，取AVI 文件的关键字串"RIFF"和"AVI"判断，和get_riff()函数部分相同。
static int avi_probe(AVProbeData *p)
{
	if (p->buf_size <= 32) // check file header
		return 0;
	if (p->buf[0] == 'R' && p->buf[1] == 'I' && p->buf[2] == 'F' && p->buf[3] == 'F'
			&& p->buf[8] == 'A' && p->buf[9] == 'V' && p->buf[10] == 'I'&& p->buf[11] == ' ')
		return AVPROBE_SCORE_MAX;
	else
		return 0;
}

//初始化AVI文件格式 AVInputFormat结构，直接的赋值操作。
AVInputFormat avi_iformat =
{
	"avi",
	sizeof(AVIContext),
	avi_probe,
	avi_read_header,
	avi_read_packet,
	avi_read_close,
};

//注册avi文件格式， ffplay 把所有支持的文件格式用链表串联起来，表头是first_iformat， 便于查找。
int avidec_init(void)
{
	av_register_input_format(&avi_iformat);
	return 0;
}

/*
   AVIF_HASINDEX：标明该AVI文件有"idx1"块
   AVIF_MUSTUSEINDEX：标明必须根据索引表来指定数据顺序
   AVIF_ISINTERLEAVED：标明该AVI文件是interleaved格式的
   AVIF_WASCAPTUREFILE：标明该AVI文件是用捕捉实时视频专门分配的文件
   AVIF_COPYRIGHTED：标明该AVI文件包含有版权信息

AVIF_MUSTUSEINDEX : 表明应用程序需要使用index，而不是物理上的顺序，来定义数据的展现顺序。
例如，该标志可以用于创建一个编辑用的帧列表。
// */
