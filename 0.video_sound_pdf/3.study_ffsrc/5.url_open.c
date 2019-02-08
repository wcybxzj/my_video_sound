#include <stdio.h>

int url_open(URLContext **puc, const char *filename, int flags)
{
URLContext *uc;
URLProtocol *up;
const char *p;
char proto_str[128],  *q;
int err;

p = filename;
q = proto_str;
while (*p != '\0' &&  *p != ':')
{
	// protocols can only contain alphabetic chars
	if (!isalpha(*p))
		goto file_proto;
	if ((q - proto_str) < sizeof(proto_str) - 1)
		*q++ =  *p;
	p++;
}

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

	up = first_protocol;
	while (up != NULL)
	{
		if (!strcmp(proto_str, up->name))
			goto found;
		up = up->next;
	}
	err =  - ENOENT;
	goto fail;

found:
	uc = av_malloc(sizeof(URLContext) + strlen(filename));
	if (!uc)
	{
		err =  - ENOMEM;
		goto fail;
	}
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
>---*puc = NULL;
>---return err;
}

int main(int argc, const char *argv[])
{
	
	return 0;
}
