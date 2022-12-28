#ifndef STRINGREADER_H
#define STRINGREADER_H

typedef void* HSR;

//create string reader object of string, data will be copied
//call sr_delete to free memory
HSR sr_new(char *s);

//destroy string reader object
void sr_delete(HSR hsr);

//set head to start
void sr_rewind(HSR hsr);

//query head position
int sr_tell(HSR hsr);

//set head position
void sr_seek(HSR hsr,int pos);

//get character from string
int sr_getc(HSR hsr);

//get a line with EOL
//returned pointer will be void if other method was called
char *sr_gets(HSR hsr);

//get a line with ending control characters trimmed
//returned pointer will be void if other method was called
char *sr_getline(HSR hsr);

//scan %s without changing head
//returned pointer will be void if other method was called
char *sr_peek_s(HSR hsr);

//scan %s
//returned pointer will be void if other method was called
char *sr_scan_s(HSR hsr);

//scan %c
int sr_scan_c(HSR hsr,char *c);

//scan %d
int sr_scan_d(HSR hsr,int *d);

//scan %lld
int sr_scan_lld(HSR hsr,long long *lld);

//scan %x
int sr_scan_x(HSR hsr,int *x);

//scan %llx
int sr_scan_llx(HSR hsr,long long *llx);

//scan %f
int sr_scan_f(HSR hsr,float *f);

//scan %lf
int sr_scan_lf(HSR hsr,double *lf);

#endif
