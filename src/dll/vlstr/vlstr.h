#define WINVER		0x0400

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <wchar.h>
#include <ctype.h>

#define initlen		sizeof(void *)
#define defpathlen	MAX_PATH

struct vlstr{
	int length;
	int buflen;
	char *data;
	wchar_t *wbuf;
};
