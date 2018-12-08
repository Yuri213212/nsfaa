#define UNICODE
#define _UNICODE

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <string.h>
#include "dmc2bin_ui.h"
#include "dmc2bin_about.h"

#define tbuflen 1024

int argc;
WCHAR **argv;

char data[0x1000],cbuf[tbuflen],fbuf[tbuflen];
WCHAR wbuf[tbuflen];
int c,i,temp;
FILE *fp;

int main(){
	argv=CommandLineToArgvW(GetCommandLineW(),&argc);
	if (argc<2){
		MessageBoxW(NULL,szAbout,szAppName,MB_ICONINFORMATION);
		LocalFree(argv);
		return -1;
	}
	if (!(fp=_wfopen(argv[1],L"rb"))){
		swprintf(wbuf,szErr_OpenFile,argv[1]);
		MessageBoxW(NULL,wbuf,szAppName,MB_ICONERROR);
		LocalFree(argv);
		return -1;
	}
	memset(data,0xFF,0x1000);
	data[0xFFE]=0x0F;
	for (i=0;i<0xFF2;++i){
		c=fgetc(fp);
		if (c<0) break;
		data[i]=c;
	}
	if (!i||i>=0xFF2){
		MessageBoxW(NULL,szErr_InputFile,szAppName,MB_ICONERROR);
		LocalFree(argv);
		return -1;
	}
	data[0xFFF]=i>>4;
	GetFullPathNameW(argv[1],tbuflen,wbuf,NULL);
	WideCharToMultiByte(CP_UTF8,0,wbuf,-1,fbuf,tbuflen,0,0);
	for (temp=i=strlen(fbuf);fbuf[i]!='\\'&&fbuf[i]!='/'&&i>=0;--i);
	for (;fbuf[i]!='.'&&i<temp;++i);
	sprintf(fbuf+i,".bin");
	MultiByteToWideChar(CP_UTF8,0,fbuf,-1,wbuf,tbuflen);
	if (!(fp=_wfopen(wbuf,L"wb"))){
		sprintf(cbuf,szErr_WriteFile,fbuf);
		MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
		MessageBoxW(NULL,wbuf,szAppName,MB_ICONERROR);
		LocalFree(argv);
		return -1;
	}
	fwrite(data,0x1000,1,fp);
	fclose(fp);
	LocalFree(argv);
	return 0;
}
