//������ʹ�õİ����͹��ߺ�����
#include <assert.h>
#include "avcodec.h"
#include "dsputil.h"

#define EDGE_WIDTH   16
#define STRIDE_ALIGN 16

#define INT_MAX 2147483647

#define FFMAX(a,b) ((a) > (b) ? (a) : (b))

//�ڴ涯̬���亯������һ�¼򵥲���У������ϵͳ����
void *av_malloc(unsigned int size)
{
	void *ptr;

	if (size > INT_MAX)
		return NULL;
	ptr = malloc(size);

	return ptr;
}

//�ڴ涯̬�ط��亯������һ�¼򵥲���У������ϵͳ����
void *av_realloc(void *ptr, unsigned int size)
{
	if (size > INT_MAX)
		return NULL;

	return realloc(ptr, size);
}

//�ڴ涯̬�ͷź�������һ�¼򵥲���У������ϵͳ����
void av_free(void *ptr)
{
	if (ptr)
		free(ptr);
}

//�ڴ涯̬���亯�������� av_malloc()�������ٰѷ�����ڴ���0.
void *av_mallocz(unsigned int size)
{
	void *ptr;

	ptr = av_malloc(size);
	if (!ptr)
		return NULL;

	memset(ptr, 0, size);
	return ptr;
}

//�����ڴ涯̬���亯����Ԥ����һЩ�ڴ��������ε���ϵͳ�����ﵽ���ٵ�Ŀ�ġ�
void *av_fast_realloc(void *ptr, unsigned int *size, unsigned int min_size)
{
	if (min_size <  *size)
		return ptr;

	*size = FFMAX(17 *min_size / 16+32, min_size);

	return av_realloc(ptr,  *size);
}

//��̬�ڴ��ͷź�����ע�⴫��ı��������͡�
void av_freep(void *arg)
{
	void **ptr = (void **)arg;
	av_free(*ptr);
	*ptr = NULL;
}

AVCodec *first_avcodec = NULL;

//�ѱ������������һ�����������ڲ��ҡ�
void register_avcodec(AVCodec *format)
{
	AVCodec **p;
	p = &first_avcodec;
	while (*p != NULL)
		p = &(*p)->next;
	*p = format;
	format->next = NULL;
}

//�������ڲ�ʹ�õĻ���������Ϊ��Ƶͼ���� RGB ��YUV ������ʽ������ÿ���������ĸ�������
typedef struct InternalBuffer
{
	uint8_t *base[4];
	uint8_t *data[4];
	int  linesize[4];
} InternalBuffer;

#define INTERNAL_BUFFER_SIZE 32

#define ALIGN(x, a) (((x)+(a)-1)&~((a)-1))

//�������ͼ���ʽҪ���ͼ�񳤿����ֽڶ��������� 1������ 2���� 4 ���� 8���� 16 ���ֽڶ��롣
void avcodec_align_dimensions(AVCodecContext *s, int *width, int *height)
{
	//Ĭ�ϳ����� 1���ֽڶ��롣
	int w_align = 1;
	int h_align = 1;

	switch (s->pix_fmt)
	{
		case PIX_FMT_YUV420P:
		case PIX_FMT_YUV422:
		case PIX_FMT_UYVY422:
		case PIX_FMT_YUV422P:
		case PIX_FMT_YUV444P:
		case PIX_FMT_GRAY8:
		case PIX_FMT_YUVJ420P:
		case PIX_FMT_YUVJ422P:
		case PIX_FMT_YUVJ444P: //FIXME check for non mpeg style codecs and use less alignment
			w_align = 16;
			h_align = 16;
			break;
		case PIX_FMT_YUV411P:
		case PIX_FMT_UYVY411:
			w_align = 32;
			h_align = 8;
			break;
		case PIX_FMT_YUV410P:
		case PIX_FMT_RGB555:
		case PIX_FMT_PAL8:
			break;
		case PIX_FMT_BGR24:
			break;
		default:
			w_align = 1;
			h_align = 1;
			break;
	}

	*width = ALIGN(*width, w_align);
	*height = ALIGN(*height, h_align);
}

//У����Ƶͼ��ĳ����Ƿ�Ϸ���
int avcodec_check_dimensions(void *av_log_ctx, unsigned int w, unsigned int h)
{
	if ((int)w > 0 && (int)h > 0 && (w + 128)*(uint64_t)(h + 128) < INT_MAX / 4)
		return 0;

	return  - 1;
}

//ÿ��ȡ internal_buffer_count ������� base[0]���ж��Ƿ��ѷ����ڴ棬
//�� data[0]���ж��Ƿ��ѱ�ռ�á� base[]�� data[]�ж������塣
//�� avcodec_alloc_context���Ѱ� internal_buffer������ 0�����Կ�����base[0]���ж�
int avcodec_default_get_buffer(AVCodecContext *s, AVFrame *pic)
{
	int i;
	int w = s->width;
	int h = s->height;
	int align_off;
	InternalBuffer *buf;

	assert(pic->data[0] == NULL);
	assert(INTERNAL_BUFFER_SIZE > s->internal_buffer_count);

	//У����Ƶͼ��ĳ����Ƿ�Ϸ���
	if (avcodec_check_dimensions(s, w, h))
		return  - 1;

	//���û�з����ڴ棬�ͷ��䶯̬�ڴ沢�� 0��
	if (s->internal_buffer == NULL)
		s->internal_buffer = av_mallocz(INTERNAL_BUFFER_SIZE *sizeof(InternalBuffer));

	//ȡ�����еĵ�һ��û��ռ���ڴ档
	buf = &((InternalBuffer*)s->internal_buffer)[s->internal_buffer_count];

	if (buf->base[0])
	{
		/* ����ڴ��ѷ�������� */
	}
	else
	{
		//���û�з����ڴ�Ͱ���ͼ���ʽҪ������ڴ棬������һЩ��Ǻͼ���һЩ����ֵ��
		int h_chroma_shift, v_chroma_shift;
		int pixel_size, size[3];
		AVPicture picture;
		//���� CbCrɫ�ȷ����������� Y���ȷ��������ıȣ��������λʵ�֡�
		avcodec_get_chroma_sub_sample(s->pix_fmt, &h_chroma_shift, &v_chroma_shift);
		//�������������ض�ͼ�����ظ�ʽ��Ҫ��
		avcodec_align_dimensions(s, &w, &h);
		//�ѳ����Ŵ�һЩ�� ������mpeg4 ��Ƶ�б����㷨�е��˶�����Ҫ��ԭʼͼ������չ�����㲻��������
		//��ʸ����Ҫ��(�˶�ʸ�����Գ���ԭʼͼ��߽�)
		w+= EDGE_WIDTH*2;
		h+= EDGE_WIDTH*2;
		//�����ض���ʽ��ͼ������������������Ĵ�С�����г���(linesize/stride)�ȵȡ�
		avpicture_fill(&picture, NULL, s->pix_fmt, w, h);
		pixel_size = picture.linesize[0] * 8 / w;
		assert(pixel_size >= 1);

		if (pixel_size == 3 *8)
			w = ALIGN(w, STRIDE_ALIGN << h_chroma_shift);
		else
			w = ALIGN(pixel_size *w, STRIDE_ALIGN << (h_chroma_shift + 3)) / pixel_size;

		size[1] = avpicture_fill(&picture, NULL, s->pix_fmt, w, h);
		size[0] = picture.linesize[0] *h;
		size[1] -= size[0];
		if (picture.data[2])
			size[1] = size[2] = size[1] / 2;
		else
			size[2] = 0;
		//ע�� base[]��data[]���黹����Ϊ��ǵ���;�� free()ʱ�ķ�NULL�жϣ�����Ҫ�� 0��
		memset(buf->base, 0, sizeof(buf->base));
		memset(buf->data, 0, sizeof(buf->data));

		for (i = 0; i < 3 && size[i]; i++)
		{
			const int h_shift = i == 0 ? 0 : h_chroma_shift;
			const int v_shift = i == 0 ? 0 : v_chroma_shift;

			buf->linesize[i] = picture.linesize[i];

			//ʵ���Է����ڴ棬���ڴ��� 0��
			buf->base[i] = (uint8_t*) av_malloc(size[i] + 16); //FIXME 16
			if (buf->base[i] == NULL)
				return  - 1;
			memset(buf->base[i], 128, size[i]);//128��ʹ0,http://blog.csdn.net/leixiaohua1020/article/details/50534150
			//�ڴ������㡣
			align_off = ALIGN((buf->linesize[i] * EDGE_WIDTH >> v_shift) + ( EDGE_WIDTH >> h_shift), STRIDE_ALIGN);

			if ((s->pix_fmt == PIX_FMT_PAL8) || !size[2])
				buf->data[i] = buf->base[i];
			else
				buf->data[i] = buf->base[i] + align_off;
		}
	}

	for (i = 0; i < 4; i++)
	{
		//�ѷ�����ڴ������ֵ�� pic ָ��Ľṹ�У����ݳ�ȥ��
		pic->base[i] = buf->base[i];
		pic->data[i] = buf->data[i];
		pic->linesize[i] = buf->linesize[i];
	}
	//�ڴ��������+1��ע���ͷ�ʱ�Ĳ�������֤������Ӧ���ڴ������ǿ��еġ�
	s->internal_buffer_count++;

	return 0;
}

//�ͷ�ռ�õ��ڴ��������֤�� 0�� internal_buffer_count-1������Ϊ��Ч���ݣ������ǿ���������
void avcodec_default_release_buffer(AVCodecContext *s, AVFrame *pic)
{
	int i;
	InternalBuffer *buf,  *last, temp;
	//�򵥵Ĳ���У�飬�ڴ�������Ѿ��������
	assert(s->internal_buffer_count);

	//�����ڴ����飬���Ҷ�Ӧ pic ���ڴ�������� data[0]�ڴ��ַΪ�Ƚ��б��ǡ�
	buf = NULL;
	for (i = 0; i < s->internal_buffer_count; i++)
	{
		buf = &((InternalBuffer*)s->internal_buffer)[i]; //just 3-5 checks so is not worth to optimize
		if (buf->data[0] == pic->data[0])
			break;
	}
	assert(i < s->internal_buffer_count);
	//�ڴ��������-1, ɾ�����һ��.
	s->internal_buffer_count--;
	last = &((InternalBuffer*)s->internal_buffer)[s->internal_buffer_count];
	//�ѽ�Ҫ���е���������������һ�������֤ internal_buffer_count������ȷ����ע�����ﲢ
	//û���ڴ��ͷŵĶ����������´θ����ѷ�����ڴ�
	temp =  *buf;
	*buf =  *last;
	*last = temp;
	// ��data[i]�ÿգ�ָʾ�����ڴ�û�б�ռ�ã� ʵ�ʷ�����׵�ַ������base[]�С�
	// �������������� INTERNAL_BUFFER_SIZE�� avframe��������ѭ��ʹ��
	for (i = 0; i < 3; i++)
	{
		pic->data[i] = NULL;
	}
}

//���»�û��档
int avcodec_default_reget_buffer(AVCodecContext *s, AVFrame *pic)
{
	if (pic->data[0] == NULL)  // If no picture return a new buffer
	{
		return s->get_buffer(s, pic);
	}

	return 0;
}

//�ͷ��ڴ�������ռ�õ��ڴ档
void avcodec_default_free_buffers(AVCodecContext *s)
{
	int i, j;

	if (s->internal_buffer == NULL)
		return ;

	for (i = 0; i < INTERNAL_BUFFER_SIZE; i++)
	{
		InternalBuffer *buf = &((InternalBuffer*)s->internal_buffer)[i];
		for (j = 0; j < 4; j++)
		{
			//av_freep()�������õ� av_free()�������˷�NULL �жϣ� ���ҷ���ʱ����NULL�� ������ѭ�����Ե�4��
			//��ѭ�����Ե� INTERNAL_BUFFER_SIZE
			av_freep(&buf->base[j]);
			buf->data[j] = NULL;
		}
	}
	av_freep(&s->internal_buffer);

	s->internal_buffer_count = 0;
}

//����������������ռ�õ��ڴ棬�� 0�󲿷ֲ�������ֵ��
AVCodecContext *avcodec_alloc_context(void)
{
	AVCodecContext *s = (AVCodecContext*)av_malloc(sizeof(AVCodecContext));

	if (s == NULL)
		return NULL;
	//ע��������� 0��
	memset(s, 0, sizeof(AVCodecContext));

	s->get_buffer = avcodec_default_get_buffer;
	s->release_buffer = avcodec_default_release_buffer;

	s->pix_fmt = PIX_FMT_NONE;

	s->palctrl = NULL;
	s->reget_buffer = avcodec_default_reget_buffer;

	return s;
}

//�򿪱���������������������ʹ�õ������ģ��򵥱�������ֵ�����ó�ʼ��������ʼ���������
int avcodec_open(AVCodecContext *avctx, AVCodec *codec)
{
	int ret =  - 1;

	if (avctx->codec)
		goto end;

	if (codec->priv_data_size > 0)
	{
		//���������� priv_data_siz e �������ش����ã����û�����������
		//��Ҫ�� codec �ṹ�����ֱȽ�ȷ������������ʹ�õ������Ľṹ��С���������� if-else���
		avctx->priv_data = av_mallocz(codec->priv_data_size);
		if (!avctx->priv_data)
			goto end;
	}
	else
	{
		avctx->priv_data = NULL;
	}

	avctx->codec = codec;
	avctx->codec_id = codec->id;
	avctx->frame_number = 0;
	ret = avctx->codec->init(avctx);
	if (ret < 0)
	{
		av_freep(&avctx->priv_data);
		avctx->codec = NULL;
		goto end;
	}
	ret = 0;
end:
	return ret;

}

//��Ƶ���룬�򵥵���ת
int avcodec_decode_video(AVCodecContext *avctx, AVFrame *picture, int *got_picture_ptr,
		uint8_t *buf, int buf_size)
{
	int ret;

	*got_picture_ptr = 0;

	if (buf_size)
	{
		ret = avctx->codec->decode(avctx, picture, got_picture_ptr, buf, buf_size);

		if (*got_picture_ptr)
			avctx->frame_number++;
	}
	else
		ret = 0;

	return ret;
}

//��Ƶ���룬�򵥵���ת
int avcodec_decode_audio(AVCodecContext *avctx, int16_t *samples, int *frame_size_ptr,
		uint8_t *buf, int buf_size)
{
	int ret;

	*frame_size_ptr = 0;
	if (buf_size)
	{
		ret = avctx->codec->decode(avctx, samples, frame_size_ptr, buf, buf_size);
		avctx->frame_number++;
	}
	else
		ret = 0;
	return ret;
}

//�رս��������ͷŶ�̬������ڴ�
int avcodec_close(AVCodecContext *avctx)
{
	if (avctx->codec->close)
		avctx->codec->close(avctx);
	avcodec_default_free_buffers(avctx);
	av_freep(&avctx->priv_data);
	avctx->codec = NULL;
	return 0;
}

//���ұ���������ڱ����У��� avi �ļ�ͷ�õ� codec FOURCC������ FOURCC ���� codec_bmp_tags
//�� codec_wav_tags�õ� CodecID �����˺���
AVCodec *avcodec_find_decoder(enum CodecID id)
{
	AVCodec *p;
	p = first_avcodec;
	while (p)
	{
		if (p->decode != NULL && p->id == id)
			return p;
		p = p->next;
	}
	return NULL;
}

//��ʼ�������⣬�ڱ����н���ʼ���޷�����/���ұ���
void avcodec_init(void)
{
	static int inited = 0;

	if (inited != 0)
		return ;
	inited = 1;

	dsputil_static_init();
}