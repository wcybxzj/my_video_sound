#include <stdio.h>
typedef struct AVCodec {
	int val;
	struct AVCodec *next;
} AVCodec;

AVCodec *first_avcodec = NULL;

void register_avcodec(AVCodec *format)
{
	AVCodec **p;
	p = &first_avcodec;
	while (*p != NULL)
		p = &(*p)->next;
	*p = format;
	format->next = NULL;
}

void foreach()
{
	AVCodec **p;
	p = &first_avcodec;

	while (*p) {
		printf("%d\n", (*p)->val);
		*p = (*p)->next;
	}

}

int main(int argc, const char *argv[])
{
	AVCodec a1, a2;
	a1.val =1;
	a2.val =2;
	a1.next = NULL;
	a2.next = NULL;

	register_avcodec(&a1);
	register_avcodec(&a2);

	foreach();


	return 0;
}
