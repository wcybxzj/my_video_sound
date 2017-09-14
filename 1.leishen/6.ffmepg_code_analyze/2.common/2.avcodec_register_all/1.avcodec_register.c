#include <stdio.h>
#include <stdlib.h>

typedef struct AVCodec {
	int num;
	struct AVCodec **next;
} AVCodec;

static AVCodec *first_avcodec = NULL;
static AVCodec **last_avcodec = &first_avcodec;

void *avpriv_atomic_ptr_cas(void * volatile *ptr, void *oldval, void *newval)
{
	if (*ptr == oldval) {
		*ptr = newval;
		return oldval;
	}
	return *ptr;
}

void avcodec_register(AVCodec *codec)
{
	AVCodec **p;
	p = last_avcodec;
	codec->next = NULL;

	while(*p || avpriv_atomic_ptr_cas((void * volatile *)p, NULL, codec))
		p = &(*p)->next;

	last_avcodec = &codec->next;
}

//证明了avcodec_register是尾部插入
//输出:
//1
//2
int main(int argc, const char *argv[])
{
	AVCodec *p1 = malloc(sizeof(AVCodec));
	p1->num = 1;
	p1->next=NULL;
	avcodec_register(p1);

	AVCodec *p2 = malloc(sizeof(AVCodec));
	p2->num = 2;
	p2->next=NULL;
	avcodec_register(p2);

	struct AVCodec *tmp = first_avcodec;
	while (tmp) {
		printf("num:%d\n",tmp->num);
		tmp=tmp->next;
	}

	return 0;
}
