#ifndef UTF8FORMAT_H
#define UTF8FORMAT_H

#include <stdio.h>

//read to ignore potential BOM from start of file, returns whether BOM was read
int utf8_checkBOM(FILE *fp);

//write BOM to file
void utf8_writeBOM(FILE *fp);

#endif
