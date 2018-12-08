#define UNICODE
#define _UNICODE

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <string.h>
#include "txt2bin_ui.h"
#include "txt2bin_about.h"

#define tbuflen 1024
#define romlen 0x1000

struct datast{
	int datalen;
	int pagecount;
	unsigned char pagenumber[0x100];
	char data[romlen];
};

int argc;
WCHAR **argv;

int chcount,shortnoiseins,lastpage,lengthcount,temp,c,n,i,j;
int chstate[32],length[0x100];
char databuf[4],fbuf[tbuflen],cbuf[tbuflen],dummymain[romlen],dummysub[romlen],rommain[romlen],romsub[romlen];
WCHAR wbuf[tbuflen];
struct datast datamain[0x100]={},datasub[0x100]={};
FILE *fp;

int note(){
	int y;

	switch (databuf[0]){
	case 'C':
		y=0x0;
		break;
	case 'D':
		y=0x2;
		break;
	case 'E':
		y=0x4;
		break;
	case 'F':
		y=0x5;
		break;
	case 'G':
		y=0x7;
		break;
	case 'A':
		y=0x9;
		break;
	case 'B':
		y=0xB;
		break;
	case '^':
	case '=':
	case '~':
		return 0xFE;
	default:
		return -1;
	}
	if (databuf[1]=='#'){
		++y;
	}
	return ((databuf[2]-'0')<<4)|y;
}

int noisenote(){
	switch (note()){
	case 0x09:
		return 0x0F;
	case 0x19:
		return 0x0E;
	case 0x29:
		return 0x0D;
	case 0x32:
		return 0x0C;
	case 0x39:
		return 0x0B;
	case 0x42:
		return 0x0A;
	case 0x49:
		return 0x09;
	case 0x51:
		return 0x08;
	case 0x55:
		return 0x07;
	case 0x59:
		return 0x06;
	case 0x62:
		return 0x05;
	case 0x69:
		return 0x04;
	case 0x79:
		return 0x03;
	case 0x89:
		return 0x02;
	case 0x99:
		return 0x01;
	case 0xa9:
		return 0x00;
	case 0xFE:
		return 0xFE;
	default:
		return -1;
	}
}

int divnote(){
	int y;

	y=note();
	switch (y){
	case 0xFE:
		return 0xFE;
	case -1:
		return 0xFF;
	}
	y=(y>>4)*12+(y&0x0F);
	y=77-y;
	if (y<0) return 0xFF;
	return y;
}

int ins(){
	int y;

	y=-1;
	if (databuf[0]=='.') return -1;
	databuf[2]=0;
	sscanf(databuf,"%d",&y);
	return y;
}

int vol(){
	int y;

	y=-1;
	if (databuf[0]!='v') return -1;
	databuf[3]=0;
	sscanf(databuf+1,"%d",&y);
	if (y==64) return -1;
	return y>>2;
}

int vrc7vol(){
	int y;

	y=-1;
	if (databuf[0]!='v') return -1;
	databuf[3]=0;
	sscanf(databuf+1,"%d",&y);
	switch (y){
	case 64:
		return 0x00;
	case 48:
		return 0x01;
	case 32:
		return 0x02;
	case 24:
		return 0x03;
	case 16:
		return 0x04;
	case 12:
		return 0x05;
	case 8:
		return 0x06;
	case 6:
		return 0x07;
	case 4:
		return 0x08;
	case 3:
		return 0x09;
	case 2:
		return 0x0A;
	case 1:
		return 0x0C;
	default:
		return -1;
	}
}

int fdsvol(){
	int y;

	y=-1;
	if (databuf[0]!='v') return -1;
	databuf[3]=0;
	sscanf(databuf+1,"%d",&y);
	return y>>1;
}

int fme7vol(){
	int y;

	y=-1;
	if (databuf[0]!='v') return -1;
	databuf[3]=0;
	sscanf(databuf+1,"%d",&y);
	switch (y){
	case 64:
		return 0x0F;
	case 48:
		return 0x0E;
	case 32:
		return 0x0D;
	case 24:
		return 0x0C;
	case 16:
		return 0x0B;
	case 12:
		return 0x0A;
	case 8:
		return 0x09;
	case 6:
		return 0x08;
	case 4:
		return 0x07;
	case 3:
		return 0x06;
	case 2:
		return 0x05;
	case 1:
		return 0x03;
	default:
		return -1;
	}
}

int divvol(){
	int y;

	y=-1;
	if (databuf[0]!='v') return -1;
	databuf[3]=0;
	sscanf(databuf+1,"%d",&y);
	switch (y){
	case 48:
		return 0x07;
	case 32:
		return 0x06;
	case 24:
		return 0x05;
	case 16:
		return 0x04;
	case 12:
		return 0x03;
	case 8:
		return 0x02;
	case 4:
		return 0x01;
	default:
		return -1;
	}
}

int annotation(){
	int y;

	y=-1;
	if (databuf[0]!='Q'&&databuf[0]!='O'&&databuf[0]!='G') return -1;
	if (databuf[1]=='.'){
		databuf[1]='0';
	}
	if (databuf[2]=='.'){
		databuf[2]='0';
	}
	databuf[3]=0;
	sscanf(databuf+1,"%x",&y);
	return y;
}

int romcmp(char * const rom1,char * const rom2,int len){
	static int i;
	static char data1[romlen],data2[romlen];

	memcpy(data1,rom1,romlen);
	memcpy(data2,rom2,romlen);
	if (len<0x100){
		for (i=0;i<16;++i){
			memset(data1+i*0x100+len,0xFF,0x100-len);
			memset(data2+i*0x100+len,0xFF,0x100-len);
		}
	}
	return memcmp(data1,data2,romlen);
}

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
	GetFullPathNameW(argv[1],tbuflen,wbuf,NULL);
	fscanf(fp,"%d",&chcount);
	fscanf(fp,"%d",&lastpage);
	fclose(fp);
	switch (chcount){
	case 16://dpcm,noise,fm1..6,fx1..8
		memset(dummymain     ,0xFF,romlen);
		memset(dummysub      ,0x00,0x200);
		memset(dummysub+0x200,0x0F,0x600);
		memset(dummysub+0x800,0x00,0x800);
		shortnoiseins=18;
		break;
	case 11://dpcm,noise,w64,fx1..8
		memset(dummymain     ,0xFF,romlen);
		memset(dummysub      ,0x00,0x300);
		memset(dummysub+0x300,0xFF,0x500);
		memset(dummysub+0x800,0x00,0x800);
		shortnoiseins=18;
		break;
	case 17://dpcm,noise,rec5..6,tri,rec1..2,rec3..4,fx1..8
		memset(dummysub      ,0xFF,0x800);
		memset(dummysub+0x800,0x00,0x800);
	case 9://dpcm,noise,rec5..6,tri,rec1..2,rec3..4
		memset(dummymain      ,0xFF,0x900);
		memset(dummymain+0x900,0x00,0x700);
		shortnoiseins=3;
		break;
	case 20://dpcm,noise,sqr1..3,rec5..6,tri,rec1..2,rec3..4,fx1..8
		memset(dummysub      ,0xFF,0x800);
		memset(dummysub+0x800,0x00,0x800);
	case 12://dpcm,noise,sqr1..3,rec5..6,tri,rec1..2,rec3..4
		memset(dummymain      ,0xFF,0x100);
		memset(dummymain+0x100,0x0F,0x100);
		memset(dummymain+0x200,0xFF,0x300);
		memset(dummymain+0x500,0x00,0x100);
		memset(dummymain+0x600,0x3F,0x100);
		memset(dummymain+0x700,0x00,0x100);
		memset(dummymain+0x800,0x3F,0x400);
		memset(dummymain+0xC00,0x00,0x100);
		memset(dummymain+0xD00,0x7F,0x200);
		memset(dummymain+0xF00,0x00,0x100);
		shortnoiseins=3;
		break;
	default:
		swprintf(wbuf,szErr_Format,chcount);
		MessageBoxW(NULL,wbuf,szAppName,MB_ICONERROR);
		LocalFree(argv);
		return -1;
	}
	WideCharToMultiByte(CP_UTF8,0,wbuf,-1,fbuf,tbuflen,0,0);
	for (i=strlen(fbuf);fbuf[i]!='\\'&&fbuf[i]!='/'&&i>=0;--i);
	fbuf[i+1]=0;
	for (n=0;n<=lastpage;++n){
		sprintf(cbuf,"%s%02d.txt",fbuf,n);
		MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
		if (!(fp=_wfopen(wbuf,L"rb"))) continue;
		for (i=31;i;--i){
			chstate[i]=0xF0;
		}
		memcpy(rommain,dummymain,romlen);
		memcpy(romsub,dummysub,romlen);
		for (lengthcount=0;lengthcount<0x100;++lengthcount){
			for (c=fgetc(fp);c>=0&&c!='|';c=fgetc(fp));
			if (c<0) break;
			switch (chcount){
			case 16://dpcm,noise,fm1..6,fx1..8
					//[dpcm] main=dpcmindex(8);00:key-off,others hit:key-on,miss:do nothing
					//[script] main=pointerlo(4),pointerhi(4);call $8HL0, null for do nothing
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				temp=note();
				if (temp==0xFE){
					rommain[0x000+lengthcount]=0x00;
				}else if(temp>=0){
					rommain[0x000+lengthcount]=temp;
				}
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				temp=annotation();
				if (temp>=0){
					romsub[0x000+lengthcount]=temp|0x80;
				}
				fgetc(fp);
					//[noise] main=short,control(3),period(4);control=000:key-on,control=111&period=E:key-off
					//[noise] sub=-,-,-,-,vol(4);vol!=0:change vol
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				temp=noisenote();
				if (temp>=0){
					rommain[0x100+lengthcount]=temp;
				}
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				temp=ins();
				if (temp==shortnoiseins){
					rommain[0x100+lengthcount]|=0x80;
				}
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				temp=vol();
				if (temp>=0){
					romsub[0x100+lengthcount]=temp;
				}
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
					//[fm] main=note(4),oct(3),~reset(1);note<C:key-on,note=E:key-off
					//[fm] sub=ins(4),vol(4);vol!=0xf:change all
				for (i=2;i<8;++i){
					databuf[0]=fgetc(fp);
					databuf[1]=fgetc(fp);
					databuf[2]=fgetc(fp);
					temp=note();
					if (temp==0xFE){
						chstate[i]=0xF0;
						rommain[i*0x100+lengthcount]=0xEF;
					}else if (temp>=0){
						if ((temp&0xF0)==chstate[i]){
							rommain[i*0x100+lengthcount]=((temp<<4)&0xF0)|((temp>>3)&0x0E)|0x01;
						}else{
							chstate[i]=temp&0xF0;
							rommain[i*0x100+lengthcount]=((temp<<4)&0xF0)|((temp>>3)&0x0E);
						}
					}
					fgetc(fp);
					fgetc(fp);
					databuf[0]=fgetc(fp);
					databuf[1]=fgetc(fp);
					databuf[2]=fgetc(fp);
					temp=vrc7vol();
					if (temp>=0){
						romsub[i*0x100+lengthcount]=temp;
					}
					databuf[0]=fgetc(fp);
					databuf[1]=fgetc(fp);
					databuf[2]=fgetc(fp);
					temp=annotation();
					if (temp==0x00){
						rommain[i*0x100+lengthcount]&=~0x01;
					}
					fgetc(fp);
				}
					//[fx] main=oct(3),~reset(1),note(4);note<C:key-on,note=E:key-off
					//[fx] sub=ins(4),vol(4);vol!=0:change vol,ins!=0:change ins
				for (;i<16;++i){
					databuf[0]=fgetc(fp);
					databuf[1]=fgetc(fp);
					databuf[2]=fgetc(fp);
					temp=note();
					if (temp==0xFE){
						chstate[i]=0xF0;
						rommain[i*0x100+lengthcount]=temp;
					}else if (temp>=0){
						if (chstate[i]!=0xF0){
							rommain[i*0x100+lengthcount]=((temp<<1)&0xE0)|(temp&0x0F)|0x10;
						}else{
							chstate[i]=temp&0xF0;
							rommain[i*0x100+lengthcount]=((temp<<1)&0xE0)|(temp&0x0F);
						}
					}
					databuf[0]=fgetc(fp);
					databuf[1]=fgetc(fp);
					temp=ins();
					if (temp>=0){
						romsub[i*0x100+lengthcount]|=temp<<4;
					}
					databuf[0]=fgetc(fp);
					databuf[1]=fgetc(fp);
					databuf[2]=fgetc(fp);
					temp=vol();
					if (temp>=0){
						romsub[i*0x100+lengthcount]|=temp;
					}
					databuf[0]=fgetc(fp);
					databuf[1]=fgetc(fp);
					databuf[2]=fgetc(fp);
					temp=annotation();
					if (temp==0xFF){
						rommain[i*0x100+lengthcount]|=0x10;
					}else if (temp==0x00){
						rommain[i*0x100+lengthcount]&=~0x10;
					}
					fgetc(fp);
				}
				break;
			case 11://dpcm,noise,w64,fx1..8
					//[dpcm] main=dpcmindex(8);00:key-off,others hit:key-on,miss:do nothing
					//[script] main=pointerlo(4),pointerhi(4);call $8HL0, null for do nothing
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				temp=note();
				if (temp==0xFE){
					rommain[0x000+lengthcount]=0x00;
				}else if(temp>=0){
					rommain[0x000+lengthcount]=temp;
				}
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				temp=annotation();
				if (temp>=0){
					romsub[0x000+lengthcount]=temp|0x80;
				}
				fgetc(fp);
					//[noise] main=short,control(3),period(4);control=000:key-on,control=111&period=E:key-off
					//[noise] sub=-,-,-,-,vol(4);vol!=0:change vol
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				temp=noisenote();
				if (temp>=0){
					rommain[0x100+lengthcount]=temp;
				}
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				temp=ins();
				if (temp==shortnoiseins){
					rommain[0x100+lengthcount]|=0x80;
				}
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				temp=vol();
				if (temp>=0){
					romsub[0x100+lengthcount]=temp;
				}
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
					//[w64] main=-,oct(3),note(4);note<C:key-on,note=E:key-off
					//[w64] sub=-,-,vol(6);vol!=0:change vol
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				temp=note();
				if (temp>=0){
					rommain[0x200+lengthcount]=temp;
				}
				fgetc(fp);
				fgetc(fp);
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				temp=fdsvol();
				if (temp>=0){
					romsub[0x200+lengthcount]=temp;
				}
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
					//[fx] main=oct(3),~reset(1),note(4);note<C:key-on,note=E:key-off
					//[fx] sub=ins(4),vol(4);vol!=0:change vol,ins!=0:change ins
				for (i=8;i<16;++i){
					databuf[0]=fgetc(fp);
					databuf[1]=fgetc(fp);
					databuf[2]=fgetc(fp);
					temp=note();
					if (temp==0xFE){
						chstate[i]=0xF0;
						rommain[i*0x100+lengthcount]=temp;
					}else if (temp>=0){
						if (chstate[i]!=0xF0){
							rommain[i*0x100+lengthcount]=((temp<<1)&0xE0)|(temp&0x0F)|0x10;
						}else{
							chstate[i]=temp&0xF0;
							rommain[i*0x100+lengthcount]=((temp<<1)&0xE0)|(temp&0x0F);
						}
					}
					databuf[0]=fgetc(fp);
					databuf[1]=fgetc(fp);
					temp=ins();
					if (temp>=0){
						romsub[i*0x100+lengthcount]|=temp<<4;
					}
					databuf[0]=fgetc(fp);
					databuf[1]=fgetc(fp);
					databuf[2]=fgetc(fp);
					temp=vol();
					if (temp>=0){
						romsub[i*0x100+lengthcount]|=temp;
					}
					databuf[0]=fgetc(fp);
					databuf[1]=fgetc(fp);
					databuf[2]=fgetc(fp);
					temp=annotation();
					if (temp==0xFF){
						rommain[i*0x100+lengthcount]|=0x10;
					}else if (temp==0x00){
						rommain[i*0x100+lengthcount]&=~0x10;
					}
					fgetc(fp);
				}
				break;
			case 17://dpcm,noise,rec5..6,tri,rec1..2,rec3..4,fx1..8
			case 9://dpcm,noise,rec5..6,tri,rec1..2,rec3..4
					//[dpcm] main=dpcmindex(8);00:key-off,others hit:key-on,miss:do nothing
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				temp=note();
				if (temp==0xFE){
					rommain[0x000+lengthcount]=0x00;
				}else if(temp>=0){
					rommain[0x000+lengthcount]=temp;
				}
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
					//[noise] main=short,control(3),period(4);control=000:key-on,control=111&period=E:key-off
					//[noise] sub=-,-,-,-,vol(4);vol!=0:change vol
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				temp=noisenote();
				if (temp>=0){
					rommain[0x100+lengthcount]=temp;
				}
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				temp=ins();
				if (temp==shortnoiseins){
					rommain[0x100+lengthcount]|=0x80;
				}
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				temp=vol();
				if (temp>=0){
					rommain[0x900+lengthcount]=temp;
				}
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
					//[tri] main=-,oct(3),note(4);note<C:key-on,note=E:key-off
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				rommain[0x200+lengthcount]=note();
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
					//[rec5,6,1,2] main=-,oct(3),note(4);note<C:key-on,note=E:key-off
					//[rec5,6,1,2] sub=ins(2),control(2),vol(4);ins,control!=0:change ins,vol!=0:change vol
				for (i=3;i<7;++i){
					databuf[0]=fgetc(fp);
					databuf[1]=fgetc(fp);
					databuf[2]=fgetc(fp);
					rommain[i*0x100+lengthcount]=note();
					databuf[0]=fgetc(fp);
					databuf[1]=fgetc(fp);
					temp=ins();
					if (temp>=0){
						rommain[0x700+i*0x100+lengthcount]|=((temp<<6)&0xC0)|0x30;
					}
					databuf[0]=fgetc(fp);
					databuf[1]=fgetc(fp);
					databuf[2]=fgetc(fp);
					temp=vol();
					if (temp>=0){
						rommain[0x700+i*0x100+lengthcount]|=temp;
					}
					fgetc(fp);
					fgetc(fp);
					fgetc(fp);
					fgetc(fp);
				}
					//[rec3,4] main=-,oct(3),note(4);note<C:key-on,note=E:key-off
					//[rec3,4] sub=control,ins(3),vol(4);ins,control!=0:change ins,vol!=0:change vol
				for (;i<9;++i){
					databuf[0]=fgetc(fp);
					databuf[1]=fgetc(fp);
					databuf[2]=fgetc(fp);
					rommain[i*0x100+lengthcount]=note();
					databuf[0]=fgetc(fp);
					databuf[1]=fgetc(fp);
					temp=ins();
					if (temp>=0){
						rommain[0x700+i*0x100+lengthcount]|=((temp<<4)&0x70)|0x80;
					}
					databuf[0]=fgetc(fp);
					databuf[1]=fgetc(fp);
					databuf[2]=fgetc(fp);
					temp=vol();
					if (temp>=0){
						rommain[0x700+i*0x100+lengthcount]|=temp;
					}
					fgetc(fp);
					fgetc(fp);
					fgetc(fp);
					fgetc(fp);
				}
				if (chcount==17){
						//[fx] main=oct(3),~reset(1),note(4);note<C:key-on,note=E:key-off
						//[fx] sub=ins(4),vol(4);vol!=0:change vol,ins!=0:change ins
					for (i=0;i<8;++i){
						databuf[0]=fgetc(fp);
						databuf[1]=fgetc(fp);
						databuf[2]=fgetc(fp);
						temp=note();
						if (temp==0xFE){
							chstate[i]=0xF0;
							romsub[i*0x100+lengthcount]=temp;
						}else if (temp>=0){
							if (chstate[i]!=0xF0){
								romsub[i*0x100+lengthcount]=((temp<<1)&0xE0)|(temp&0x0F)|0x10;
							}else{
								chstate[i]=temp&0xF0;
								romsub[i*0x100+lengthcount]=((temp<<1)&0xE0)|(temp&0x0F);
							}
						}
						databuf[0]=fgetc(fp);
						databuf[1]=fgetc(fp);
						temp=ins();
						if (temp>=0){
							romsub[0x800+i*0x100+lengthcount]|=temp<<4;
						}
						databuf[0]=fgetc(fp);
						databuf[1]=fgetc(fp);
						databuf[2]=fgetc(fp);
						temp=vol();
						if (temp>=0){
							romsub[0x800+i*0x100+lengthcount]|=temp;
						}
						databuf[0]=fgetc(fp);
						databuf[1]=fgetc(fp);
						databuf[2]=fgetc(fp);
						temp=annotation();
						if (temp==0xFF){
							romsub[i*0x100+lengthcount]|=0x10;
						}else if (temp==0x00){
							romsub[i*0x100+lengthcount]&=~0x10;
						}
						fgetc(fp);
					}
				}
				break;
			case 20://dpcm,noise,sqr1..3,rec5..6,tri,rec1..2,rec3..4,fx1..8
			case 12://dpcm,noise,sqr1..3,rec5..6,tri,rec1..2,rec3..4
					//[dpcm] main=dpcmindex(8);00:key-off,others hit:key-on,miss:do nothing
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				temp=note();
				if (temp==0xFE){
					rommain[0x000+lengthcount]=0x00;
				}else if(temp>=0){
					rommain[0x000+lengthcount]=temp;
				}
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
					//[noise] main=noisevol(4),noiseperiod(4);control=0:key-on,control=1&period=E:key-off;vol!=0:change vol
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				temp=noisenote();
				if (temp<0){
					fgetc(fp);
					fgetc(fp);
				}else if (temp==0xFE){
					rommain[0x100+lengthcount]&=0x0E;
				}else{
					rommain[0x100+lengthcount]&=temp;
					rommain[0x200+lengthcount]&=~0x80;
					databuf[0]=fgetc(fp);
					databuf[1]=fgetc(fp);
					temp=ins();
					if (temp!=shortnoiseins){
						rommain[0x300+lengthcount]&=~0x80;
					}
				}
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				temp=vol();
				if (temp>=0){
					rommain[0x100+lengthcount]|=temp<<4;
				}
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
					//[sqr1] main=noisecontrol,sqr1note(7);note<126:key-on,note=126:key-off
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				rommain[0x200+lengthcount]&=divnote()|0x80;
				fgetc(fp);
				fgetc(fp);
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				temp=fme7vol();
				if (temp>=0){
					rommain[0x500+lengthcount]|=temp;
				}
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
					//[sqr2] main=noiseins,sqr2note(7)
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				rommain[0x300+lengthcount]&=divnote()|0x80;
				fgetc(fp);
				fgetc(fp);
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				temp=fme7vol();
				if (temp>=0){
					rommain[0x500+lengthcount]|=temp<<4;
				}
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
					//[sqr3] main=1,sqr3note(7)
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				rommain[0x400+lengthcount]&=divnote()|0x80;
				fgetc(fp);
				fgetc(fp);
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				temp=fme7vol();
				if (temp>=0){
					rommain[0x600+lengthcount]|=(temp<<4)&0xC0;
					rommain[0x700+lengthcount]|=(temp<<6)&0xC0;
				}
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
					//[tri] main=sqr3volh(2),trinote(6);note<62:key-on,note=62:key-off
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				rommain[0x600+lengthcount]&=divnote()|0xC0;
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
					//[rec5] main=rec5ins(2),rec5note(6);note<62:key-on,note=62:key-off
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				rommain[0x800+lengthcount]&=divnote();
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				temp=ins();
				if (temp>=0){
					rommain[0x800+lengthcount]|=(temp<<6)&0xC0;
				}
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				temp=divvol();
				if (temp>=0){
					rommain[0x700+lengthcount]|=temp;
				}
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
					//[rec6] main=rec6ins(2),rec6note(6)
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				rommain[0x900+lengthcount]&=divnote();
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				temp=ins();
				if (temp>=0){
					rommain[0x900+lengthcount]|=(temp<<6)&0xC0;
				}
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				temp=divvol();
				if (temp>=0){
					rommain[0x700+lengthcount]|=temp<<3;
				}
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
					//[rec1] main=rec1ins(2),rec1note(6)
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				rommain[0xA00+lengthcount]&=divnote();
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				temp=ins();
				if (temp>=0){
					rommain[0xA00+lengthcount]|=(temp<<6)&0xC0;
				}
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				temp=divvol();
				if (temp>=0){
					rommain[0xC00+lengthcount]|=temp;
				}
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
					//[rec2] main=rec2ins(2),rec2note(6)
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				rommain[0xB00+lengthcount]&=divnote();
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				temp=ins();
				if (temp>=0){
					rommain[0xB00+lengthcount]|=(temp<<6)&0xC0;
				}
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				temp=divvol();
				if (temp>=0){
					rommain[0xC00+lengthcount]|=temp<<3;
				}
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
					//[rec3] main=rec3insh(1),rec3note(7);note<126:key-on,note=126:key-off
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				rommain[0xD00+lengthcount]&=divnote();
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				temp=ins();
				if (temp>=0){
					rommain[0xD00+lengthcount]|=(temp<<5)&0x80;
					rommain[0xC00+lengthcount]|=(temp<<6)&0xC0;
				}
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				temp=divvol();
				if (temp>=0){
					rommain[0xF00+lengthcount]|=temp;
				}
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
					//[rec4] main=rec4insh(1),rec4note(7)
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				rommain[0xE00+lengthcount]&=divnote();
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				temp=ins();
				if (temp>=0){
					rommain[0xE00+lengthcount]|=(temp<<5)&0x80;
					rommain[0xF00+lengthcount]|=(temp<<6)&0xC0;
				}
				databuf[0]=fgetc(fp);
				databuf[1]=fgetc(fp);
				databuf[2]=fgetc(fp);
				temp=divvol();
				if (temp>=0){
					rommain[0xF00+lengthcount]|=temp<<3;
				}
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				fgetc(fp);
				if (chcount==20){
						//[fx] main=oct(3),~reset(1),note(4);note<C:key-on,note=E:key-off
						//[fx] sub=ins(4),vol(4);vol!=0:change vol,ins!=0:change ins
					for (i=0;i<8;++i){
						databuf[0]=fgetc(fp);
						databuf[1]=fgetc(fp);
						databuf[2]=fgetc(fp);
						temp=note();
						if (temp==0xFE){
							chstate[i]=0xF0;
							romsub[i*0x100+lengthcount]=temp;
						}else if (temp>=0){
							if (chstate[i]!=0xF0){
								romsub[i*0x100+lengthcount]=((temp<<1)&0xE0)|(temp&0x0F)|0x10;
							}else{
								chstate[i]=temp&0xF0;
								romsub[i*0x100+lengthcount]=((temp<<1)&0xE0)|(temp&0x0F);
							}
						}
						databuf[0]=fgetc(fp);
						databuf[1]=fgetc(fp);
						temp=ins();
						if (temp>=0){
							romsub[0x800+i*0x100+lengthcount]|=temp<<4;
						}
						databuf[0]=fgetc(fp);
						databuf[1]=fgetc(fp);
						databuf[2]=fgetc(fp);
						temp=vol();
						if (temp>=0){
							romsub[0x800+i*0x100+lengthcount]|=temp;
						}
						databuf[0]=fgetc(fp);
						databuf[1]=fgetc(fp);
						databuf[2]=fgetc(fp);
						temp=annotation();
						if (temp==0xFF){
							romsub[i*0x100+lengthcount]|=0x10;
						}else if (temp==0x00){
							romsub[i*0x100+lengthcount]&=~0x10;
						}
						fgetc(fp);
					}
				}
				break;
			}
			fgets(cbuf,tbuflen,fp);
		}
		length[n]=lengthcount;
		for (j=0;datamain[j].datalen;++j){
			if (lengthcount<=datamain[j].datalen){
				if (!romcmp(rommain,datamain[j].data,lengthcount)){
					datamain[j].pagenumber[datamain[j].pagecount++]=n;
					goto sub;
				}
			}else{
				if (!romcmp(rommain,datamain[j].data,datamain[j].datalen)){
					datamain[j].pagenumber[datamain[j].pagecount++]=n;
					datamain[j].datalen=lengthcount;
					memcpy(datamain[j].data,rommain,romlen);
					goto sub;
				}
			}
		}
		datamain[j].datalen=lengthcount;
		datamain[j].pagecount=1;
		datamain[j].pagenumber[0]=n;
		memcpy(datamain[j].data,rommain,romlen);
sub:
		if (chcount!=9&&chcount!=12){
			if (chcount!=17&&chcount!=20){
				if (!romcmp(romsub,dummysub,lengthcount)) goto end;
			}
			for (j=0;datasub[j].datalen;++j){
				if (lengthcount<=datasub[j].datalen){
					if (!romcmp(romsub,datasub[j].data,lengthcount)){
						datasub[j].pagenumber[datasub[j].pagecount++]=n;
						goto end;
					}
				}else{
					if (!romcmp(romsub,datasub[j].data,datasub[j].datalen)){
						datasub[j].pagenumber[datasub[j].pagecount++]=n;
						datasub[j].datalen=lengthcount;
						memcpy(datasub[j].data,romsub,romlen);
						goto end;
					}
				}
			}
			datasub[j].datalen=lengthcount;
			datasub[j].pagecount=1;
			datasub[j].pagenumber[0]=n;
			memcpy(datasub[j].data,romsub,romlen);
		}
end:
		fclose(fp);
	}
	for (j=0;datamain[j].datalen;++j){
		strcpy(cbuf,fbuf);
		i=strlen(cbuf);
		for (n=0;n<datamain[j].pagecount;++n){
			temp=datamain[j].pagenumber[n];
			cbuf[i++]=temp/10+'0';
			cbuf[i++]=temp%10+'0';
		}
		cbuf[i++]=0;
		strcat(cbuf,"main");
		i+=3;
		temp=(datamain[j].datalen-1)>>4;
		cbuf[i++]=temp>9?temp+'7':temp+'0';
		temp=(datamain[j].datalen-1)&0x0F;
		cbuf[i++]=temp>9?temp+'7':temp+'0';
		cbuf[i++]=0;
		strcat(cbuf,".bin");
		MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
		if (!(fp=_wfopen(wbuf,L"wb"))){
			sprintf(fbuf,szErr_WriteFile,cbuf);
			MultiByteToWideChar(CP_UTF8,0,fbuf,-1,wbuf,tbuflen);
			MessageBoxW(NULL,wbuf,szAppName,MB_ICONERROR);
			LocalFree(argv);
			return -1;
		}
		fwrite(datamain[j].data,romlen,1,fp);
		fclose(fp);
	}
	if (chcount==17||chcount==20){
		for (j=0;datasub[j].datalen;++j){
			strcpy(cbuf,fbuf);
			i=strlen(cbuf);
			for (n=0;n<datasub[j].pagecount;++n){
				temp=datasub[j].pagenumber[n];
				cbuf[i++]=temp/10+'0';
				cbuf[i++]=temp%10+'0';
			}
			cbuf[i++]=0;
			strcat(cbuf,"ex");
			i+=1;
			temp=(datasub[j].datalen-1)>>4;
			cbuf[i++]=temp>9?temp+'7':temp+'0';
			temp=(datasub[j].datalen-1)&0x0F;
			cbuf[i++]=temp>9?temp+'7':temp+'0';
			cbuf[i++]=0;
			strcat(cbuf,".bin");
			MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
			if (!(fp=_wfopen(wbuf,L"wb"))){
				sprintf(fbuf,szErr_WriteFile,cbuf);
				MultiByteToWideChar(CP_UTF8,0,fbuf,-1,wbuf,tbuflen);
				MessageBoxW(NULL,wbuf,szAppName,MB_ICONERROR);
				LocalFree(argv);
				return -1;
			}
			fwrite(datasub[j].data,romlen,1,fp);
			fclose(fp);
		}
	}else if (chcount!=9&&chcount!=12){
		for (j=0;datasub[j].datalen;++j){
			strcpy(cbuf,fbuf);
			i=strlen(cbuf);
			for (n=0;n<datasub[j].pagecount;++n){
				temp=datasub[j].pagenumber[n];
				cbuf[i++]=temp/10+'0';
				cbuf[i++]=temp%10+'0';
			}
			cbuf[i++]=0;
			strcat(cbuf,"sub.bin");
			MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
			if (!(fp=_wfopen(wbuf,L"wb"))){
				sprintf(fbuf,szErr_WriteFile,cbuf);
				MultiByteToWideChar(CP_UTF8,0,fbuf,-1,wbuf,tbuflen);
				MessageBoxW(NULL,wbuf,szAppName,MB_ICONERROR);
				LocalFree(argv);
				return -1;
			}
			fwrite(datasub[j].data,romlen,1,fp);
			fclose(fp);
		}
	}
	LocalFree(argv);
	return 0;
}
