#include "vlstr.h"

struct vlstr *vlstr_new(){
	struct vlstr *str;

	str=(struct vlstr *)malloc(sizeof(struct vlstr));
	str->length=0;
	str->buflen=initlen;
	str->data=(char *)malloc(str->buflen);
	str->data[0]=0;
	str->wbuf=NULL;
	return str;
}

void vlstr_delete(struct vlstr *str){
	if (!str) return;
	free(str->data);
	if (str->wbuf){
		free(str->wbuf);
	}
	free(str);
}

struct vlstr *vlstr_clone(struct vlstr *stri){
	struct vlstr *str;

	if (!stri) return NULL;
	str=(struct vlstr *)malloc(sizeof(struct vlstr));
	str->length=stri->length;
	str->buflen=stri->buflen;
	str->data=(char *)malloc(str->buflen);
	strcpy(str->data,stri->data);
	str->wbuf=NULL;
	return str;
}

struct vlstr *vlstr_subString(struct vlstr *stri,int start,int length){
	struct vlstr *str;

	if (!stri) return NULL;
	if (start<0||start>stri->length) return NULL;
	if (length<0){
		length=stri->length-start;
	}
	if (start+length>stri->length) return NULL;
	str=(struct vlstr *)malloc(sizeof(struct vlstr));
	str->length=length;
	for (str->buflen=initlen;length>=str->buflen;str->buflen<<=1);
	str->data=(char *)malloc(str->buflen);
	strncpy(str->data,stri->data+start,length);
	str->data[length]=0;
	str->wbuf=NULL;
	return str;
}

void vlstr_clear(struct vlstr *str){
	if (!str) return;
	str->length=0;
	str->data[0]=0;
}

void vlstr_addc(struct vlstr *str,char c){
	char *s;

	if (!str) return;
	if (!c) return;
	if (str->length+1>=str->buflen){
		str->buflen<<=1;
		s=(char *)malloc(str->buflen);
		strcpy(s,str->data);
		free(str->data);
		str->data=s;
	}
	str->data[str->length]=c;
	++str->length;
	str->data[str->length]=0;
}

void vlstr_adds(struct vlstr *str,const char *si){
	int length;
	char *s;

	if (!str) return;
	if (!si) return;
	length=strlen(si)+1;
	if (str->length+length>=str->buflen){
		for (str->buflen<<=1;str->length+length>=str->buflen;str->buflen<<=1);
		s=(char *)malloc(str->buflen);
		strcpy(s,str->data);
		free(str->data);
		str->data=s;
	}
	strcpy(&str->data[str->length],si);
	str->length+=length-1;
}

void vlstr_addsw(struct vlstr *str,const wchar_t *si){
	int length;
	char *s;

	if (!str) return;
	if (!si) return;
	length=WideCharToMultiByte(CP_UTF8,0,si,-1,NULL,0,NULL,NULL);
	s=(char *)malloc(length);
	WideCharToMultiByte(CP_UTF8,0,si,-1,s,length,NULL,NULL);
	vlstr_adds(str,s);
	free(s);
}

void vlstr_addf(struct vlstr *str,const char *format,...){
	va_list args;
	int length;
	char *s;

	if (!str) return;
	if (!format) return;
	va_start(args,format);
	length=vsnprintf(NULL,0,format,args)+1;
	va_end(args);
	s=(char *)malloc(length);
	va_start(args,format);
	vsprintf(s,format,args);
	va_end(args);
	vlstr_adds(str,s);
	free(s);
}

void vlstr_addfw(struct vlstr *str,const wchar_t *format,...){
	va_list args;
	int length;
	wchar_t *s;

	if (!str) return;
	if (!format) return;
	va_start(args,format);
	length=vsnwprintf(NULL,0,format,args)+1;
	va_end(args);
	s=(wchar_t *)malloc(length*sizeof(wchar_t));
	va_start(args,format);
	vswprintf(s,format,args);
	va_end(args);
	vlstr_addsw(str,s);
	free(s);
}

void vlstr_addstr(struct vlstr *str,struct vlstr *str2){
	if (!str) return;
	if (!str2) return;
	vlstr_adds(str,str2->data);
}

struct vlstr *vlstr_concat(struct vlstr *str1,struct vlstr *str2){
	struct vlstr *str;

	if (!str1) return NULL;
	if (!str2) return NULL;
	str=vlstr_new();
	vlstr_adds(str,str1->data);
	vlstr_adds(str,str2->data);
	return str;
}

struct vlstr *vlstr_newEx(const char *s){
	struct vlstr *str;

	if (!s) return NULL;
	str=vlstr_new();
	vlstr_adds(str,s);
	return str;
}

struct vlstr *vlstr_newExw(const wchar_t *s){
	struct vlstr *str;

	if (!s) return NULL;
	str=vlstr_new();
	vlstr_addsw(str,s);
	return str;
}

char vlstr_last(struct vlstr *str){
	if (!str) return 0;
	if (!str->length) return 0;
	return str->data[str->length-1];
}

char vlstr_drop(struct vlstr *str){
	char c;

	if (!str) return 0;
	if (!str->length) return 0;
	--str->length;
	c=str->data[str->length];
	str->data[str->length]=0;
	return c;
}

struct vlstr *vlstr_read(FILE *fp){
	struct vlstr *str;
	int c;

	if (!fp) return NULL;
	for (c=getc(fp);c>=0&&c<' ';c=getc(fp));
	if (c<0) return NULL;
	str=vlstr_new();
	for (;c>' ';c=getc(fp)){
		vlstr_addc(str,c);
	}
	return str;
}

struct vlstr *vlstr_readLine(FILE *fp){
	struct vlstr *str;
	int c;

	if (!fp) return NULL;
	c=getc(fp);
	if (c<0) return NULL;
	str=vlstr_new();
	for (;c>=0&&c!='\r'&&c!='\n';c=getc(fp)){
		vlstr_addc(str,c);
	}
	if (c=='\r'){
		c=getc(fp);
		if (c>=0&&c!='\n'){
			ungetc(c,fp);
		}
	}
	return str;
}

struct vlstr *vlstr_readBlock(FILE *fp,int length){
	struct vlstr *str;
	char *s;

	if (!fp) return NULL;
	if (!length<=0) return NULL;
	s=(char *)malloc(length+1);
	if (!fread(s,length,1,fp)) return NULL;
	s[length]=0;
	str=vlstr_new();
	vlstr_adds(str,s);
	free(s);
	return str;
}

void vlstr_write(FILE *fp,struct vlstr *str){
	if (!fp||!str) return;
	fprintf(fp,"%s",str->data);
}

void vlstr_writeLine(FILE *fp,struct vlstr *str){
	if (!fp||!str) return;
	fprintf(fp,"%s\r\n",str->data);
}

void vlstr_writeBlock(FILE *fp,struct vlstr *str,int length){
	char *s;

	if (!fp) return;
	if (!str) return;
	s=(char *)malloc(length);
	strncpy(s,str->data,length-1);
	s[length-1]=0;
	fwrite(s,length,1,fp);
	free(s);
}

int vlstr_length(struct vlstr *str){
	if (!str) return -1;
	return str->length;
}

char *vlstr_getData(struct vlstr *str){
	if (!str) return NULL;
	return str->data;
}

char *vlstr_copyData(struct vlstr *str){
	char *s;

	if (!str) return NULL;
	s=(char *)malloc(str->length+1);
	strcpy(s,str->data);
	return s;
}

wchar_t *vlstr_copyDataw(struct vlstr *str){
	int length;
	wchar_t *s;

	if (!str) return NULL;
	length=MultiByteToWideChar(CP_UTF8,0,str->data,-1,NULL,0);
	s=(wchar_t *)malloc(length*sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8,0,str->data,-1,s,length);
	return s;
}

wchar_t *vlstr_getDataw(struct vlstr *str){
	if (!str) return NULL;
	if (str->wbuf){
		free(str->wbuf);
	}
	str->wbuf=vlstr_copyDataw(str);
	return str->wbuf;
}

int vlstr_compare(struct vlstr *str,struct vlstr *str2){
	if (!str&&!str2) return 0;
	if (!str) return -1;
	if (!str2) return 1;
	return strcmp(str->data,str2->data);
}

void vlstr_trimEnd(struct vlstr *str){
	if (!str) return;
	for (--str->length;str->length>=0&&isspace((int)str->data[str->length]);--str->length){
		str->data[str->length]=0;
	}
	if (str->data[str->length]){
		++str->length;
	}
}

void vlstr_replaceChar(struct vlstr *str,char search,char to){
	int i;

	if (!str) return;
	if (!search) return;
	if (!to) return;
	for (i=0;i<str->length;++i){
		if (str->data[i]==search){
			str->data[i]=to;
		}
	}
}

int vlstr_indexOf(struct vlstr *str,char c){
	int i;

	if (!str) return -1;
	for (i=0;i<str->length;++i){
		if (str->data[i]==c) return i;
	}
	return -1;
}

int vlstr_lastIndexOf(struct vlstr *str,char c){
	int i;

	if (!str) return -1;
	for (i=str->length-1;i>=0;--i){
		if (str->data[i]==c) return i;
	}
	return -1;
}

struct vlstr *vlstr_GetCurrentDirectory(){
	struct vlstr *str;
	int bufferlen,length;
	wchar_t *s;

	bufferlen=defpathlen;
	s=(wchar_t *)malloc(bufferlen*sizeof(wchar_t));
	for (length=GetCurrentDirectoryW(bufferlen,s);length>=bufferlen;length=GetCurrentDirectoryW(bufferlen,s)){
		free(s);
		bufferlen<<=1;
		s=(wchar_t *)malloc(bufferlen*sizeof(wchar_t));
	}
	if (!length){
		free(s);
		return NULL;
	}
	str=vlstr_newExw(s);
	free(s);
	return str;
}

struct vlstr *vlstr_GetModuleFileName(HMODULE hModule){
	struct vlstr *str;
	int bufferlen,length;
	wchar_t *s;

	bufferlen=defpathlen;
	s=(wchar_t *)malloc(bufferlen*sizeof(wchar_t));
	for (length=GetModuleFileNameW(hModule,s,bufferlen);length>=bufferlen;length=GetModuleFileNameW(hModule,s,bufferlen)){
		free(s);
		bufferlen<<=1;
		s=(wchar_t *)malloc(bufferlen*sizeof(wchar_t));
	}
	if (!length){
		free(s);
		return NULL;
	}
	str=vlstr_newExw(s);
	free(s);
	return str;
}

struct vlstr *vlstr_DragQueryFile(HDROP hDrop,int iFile){
	struct vlstr *str;
	int length;
	wchar_t *s;

	if (!hDrop||iFile<0) return NULL;
	length=DragQueryFileW(hDrop,iFile,NULL,0);
	if (!length) return NULL;
	s=(wchar_t *)malloc(length*sizeof(wchar_t));
	DragQueryFileW(hDrop,iFile,s,length);
	str=vlstr_newExw(s);
	free(s);
	return str;
}

void vlstr_GetFullPathName(struct vlstr *fileName){
	int length;
	wchar_t *s,*s2;

	if (!fileName) return;
	s=vlstr_getDataw(fileName);
	length=GetFullPathNameW(s,0,NULL,NULL);
	if (!length) return;
	s2=(wchar_t *)malloc(length*sizeof(wchar_t));
	GetFullPathNameW(s,length,s2,NULL);
	vlstr_clear(fileName);
	vlstr_addsw(fileName,s2);
	free(s2);
}
