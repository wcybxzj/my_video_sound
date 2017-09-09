#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WHITESPACES " \n\t\r"

char *av_get_token(const char **buf, const char *term)
{
    char *out     = malloc(strlen(*buf) + 1);
    char *ret     = out, *end = out;
    const char *p = *buf;
    if (!out)
        return NULL;
    p += strspn(p, WHITESPACES);

    while (*p && !strspn(p, term)) {
        char c = *p++;
        if (c == '\\' && *p) {
            *out++ = *p++;
            end    = out;
        } else if (c == '\'') {
            while (*p && *p != '\'')
                *out++ = *p++;
            if (*p) {
                p++;
                end = out;
            }
        } else {
            *out++ = c;
        }
    }

    do
        *out-- = 0;
    while (out >= end && strspn(out, WHITESPACES));

    *buf = p;

    return ret;
}

int main(int argc, const char *argv[])
{
	const char *buf= "movie=my_logo.png[wm];[in][wm]overlay=5:5[out]";
	char *name = av_get_token(&buf, "=,;[");
	printf("%s\n", name);//movie
	return 0;
}
