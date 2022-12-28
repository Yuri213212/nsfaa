#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct stringReader{
	int pos;
	int length;
	char *data;
	char *buf;
};
