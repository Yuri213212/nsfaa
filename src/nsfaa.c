#define UNICODE
#define _UNICODE

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <string.h>
#include "6502ins.h"
#include "nsfaa_ui.h"
#include "nsfaa_about.h"

#define tbuflen 1024
#define romlen 0x10000

struct reference{
	int addr;
	int mode;
	struct reference *next;
};

struct label{
	char *name;
	int addr;
	int level;
	struct reference *rlist;
	struct reference *rlistend;
	struct label *ancestor;
	struct label *parent;
	struct label *next;
};

struct header{
	char id[6];
	char songs;
	char entry;
	unsigned short start;
	unsigned short vinit;
	unsigned short vplay;
	char name[32];
	char artist[32];
	char copyright[32];
	unsigned short ntsc;
	char bankswitch[8];
	unsigned short pal;
	char ntscpal;
	char expand;
	int dummy;
};

struct header head={
	{0x4E,0x45,0x53,0x4D,0x1A,0x01},
	1,
	1,
	0,
	0,
	0,
	"",
	"",
	"",
	16666,
	{0,0,0,0,0,0,0,0},
	20000,
	0,
	0x12,
	0
};

int argc;
WCHAR **argv;

unsigned short pc=0x8000;
int min=romlen,max=-1,labellevel=0,defnumlen=1,length,intbuf,c,i,temp,ophead=1,optail=0;
char rom[romlen]={0},combuf[tbuflen],labelbuf[tbuflen],cbuf[tbuflen];
WCHAR wbuf[tbuflen];
struct ins *ip=NULL;
struct reference *rp;
struct label *lroot=0,*lp,*lp2;
FILE *fp;

int insbsch(int l,int r){
	int m,res;

	m=(l+r)/2;
	res=strcmp(combuf,inslist[m].name);
	if (!res) return m;
	if (res<0){
		if (m-1<l) return -1;
		return insbsch(l,m-1);
	}else{
		if (m+1>r) return -1;
		return insbsch(m+1,r);
	}
}

int getLineNumber(){
	temp=1;
	intbuf=ftell(fp);
	rewind(fp);
	for (i=0;i<intbuf;++i){
		c=fgetc(fp);
		if (c=='\n') ++temp;
	}
	return temp;
}

void setPc(){
	if (min>pc){
		min=pc;
	}
	pc+=length;
	if (max<pc){
		max=pc;
	}
}

int clean(int x){
	for (lp=lroot;lp;lp=lroot){
		if (!x&&lp->rlist){
			sprintf(cbuf,szErr_LabelUndefined,lp->name);
			MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
			x=-1;
		}
		for (rp=lp->rlist;rp;rp=lp->rlist){
			lp->rlist=rp->next;
			free(rp);
		}
		free(lp->name);
		lroot=lp->next;
		free(lp);
	}
	if (fp){
		fclose(fp);
	}
	if (x){
		MessageBoxW(NULL,wbuf,szAppName,x==-1?MB_ICONERROR:MB_ICONINFORMATION);
		LocalFree(argv);
	}
	return x;
}

int delayedWriteOpcode(){
	if (!ip) return 0;
	if (length<1||length>2||!(length&ip->oplen)){
		sprintf(cbuf,szErr_OprandLen,getLineNumber(),ip->name,length);
		MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
		return clean(-1);
	}
	if (length+ip->oplen==5){
		rom[pc]=ip->codealt;
	}else{
		rom[pc]=ip->code;
	}
	++pc;
	ip=NULL;
	return 0;
}

int main(){
	if (sizeof(struct header)!=0x80){
		swprintf(wbuf,szErr_Compile,sizeof(struct header));
		MessageBoxW(NULL,wbuf,szAppName,MB_ICONERROR);
		return -1;
	}
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
	memset(rom,0xFF,romlen);
	for (;;){
		length=0;
		intbuf=-1;
		if (fscanf(fp,"%d",&intbuf)>0){
			if (intbuf<0){
				swprintf(wbuf,szErr_MinusLength,getLineNumber(),intbuf);
				return clean(-1);
			}
			length=intbuf;
		}
		for (c=fgetc(fp);c>=0&&c<=' ';c=fgetc(fp));
		if (c<0) break;
		switch (c){ 
		case '.':	//get address through error
			if (ip){
				++pc;
			}
			swprintf(wbuf,szInfo_Period,getLineNumber(),pc);
			return clean(1);
		case '@':	//specify address
			if (ip){
				sprintf(cbuf,szErr_IllegalOprand,getLineNumber(),(char *)&c);
				MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
				return clean(-1);
			}
			if (length){
				sprintf(cbuf,szErr_CodeLen,getLineNumber(),(char *)&c,length);
				MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
				return clean(-1);
			}
			intbuf=-1;
			fscanf(fp,"%x",&intbuf);
			if (intbuf<0||intbuf>=romlen){
				swprintf(wbuf,szErr_At_Addr,getLineNumber(),intbuf);
				return clean(-1);
			}
			pc=intbuf;
			break;
		case ':':	//define storage
			if (ip){
				sprintf(cbuf,szErr_IllegalOprand,getLineNumber(),(char *)&c);
				MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
				return clean(-1);
			}
			if (fscanf(fp,"%s",labelbuf)<=0){
				swprintf(wbuf,szErr_VoidLabel,getLineNumber(),c);
				return clean(-1);
			}
			for (lp=lroot;lp;lp=lp->next){
				if (!strcmp(lp->name,labelbuf)){
					break;
				}
			}
			if (!lp||lp->level<labellevel){//undefined and unused, or defined/used in low level
				lp2=(struct label *)malloc(sizeof(struct label));
				lp2->name=(char *)malloc(strlen(labelbuf)+1);
				strcpy(lp2->name,labelbuf);
				lp2->addr=pc;
				lp2->level=labellevel;
				lp2->rlist=NULL;
				lp2->rlistend=NULL;
				lp2->ancestor=lp;
				lp2->parent=lp;
				if (lp&&lp->level<labellevel-1){
					lp2->parent=NULL;
				}
				lp2->next=lroot;
				lroot=lp2;
			}else{
				if (!lp->rlist){//defined in current level
					sprintf(cbuf,szErr_Colon_Redefine,getLineNumber(),labelbuf,pc);
					MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
					return clean(-1);
				}
				lp->addr=pc;
				for (rp=lp->rlist;rp;rp=lp->rlist){
					switch (rp->mode){
					case 1:	//low
						rom[rp->addr]=lp->addr&0xFF;
						break;
					case 2:	//high
						rom[rp->addr]=lp->addr>>8;
						break;
					case 3:	//both, little endian
						rom[rp->addr]=lp->addr&0xFF;
						rom[rp->addr+1]=lp->addr>>8;
						break;
					default:
						swprintf(wbuf,szErr_ReferenceMode,getLineNumber(),c,rp->mode);
						return clean(-1);
					}
					lp->rlist=rp->next;
					free(rp);
				}
				lp->rlistend=NULL;
			}
			pc+=length;
			break;
		case '%':	//const lable low (mode=1)
			if (!length){
				length=1;
			}
			if (length!=1){
				sprintf(cbuf,szErr_CodeLen,getLineNumber(),(char *)&c,length);
				MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
				return clean(-1);
			}
			if (fscanf(fp,"%s",labelbuf)<=0){
				swprintf(wbuf,szErr_VoidLabel,getLineNumber(),c);
				return clean(-1);
			}
			temp=delayedWriteOpcode();
			if (temp) return temp;
			for (lp=lroot;lp;lp=lp->next){
				if (!strcmp(lp->name,labelbuf)){
					break;
				}
			}
			if (!lp||lp->level<labellevel){//undefined and unused, or defined/used in low level
				lp2=(struct label *)malloc(sizeof(struct label));
				lp2->name=(char *)malloc(strlen(labelbuf)+1);
				strcpy(lp2->name,labelbuf);
				lp2->level=labellevel;
				rp=(struct reference *)malloc(sizeof(struct reference));
				rp->addr=pc;
				rp->mode=1;
				rp->next=NULL;
				lp2->rlist=rp;
				lp2->rlistend=rp;
				lp2->ancestor=lp;
				lp2->parent=lp;
				if (lp&&lp->level<labellevel-1){
					lp2->parent=NULL;
				}
				lp2->next=lroot;
				lroot=lp2;
			}else{
				if (!lp->rlist){//defined in current level
					rom[pc]=lp->addr&0xFF;
				}else{//undefined but used in current level
					rp=(struct reference *)malloc(sizeof(struct reference));
					rp->addr=pc;
					rp->mode=1;
					rp->next=lp->rlist;
					lp->rlist=rp;
				}
			}
			setPc();
			break;
		case '^':	//const label high (mode=2)
			if (!length){
				length=1;
			}
			if (length!=1){
				sprintf(cbuf,szErr_CodeLen,getLineNumber(),(char *)&c,length);
				MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
				return clean(-1);
			}
			if (fscanf(fp,"%s",labelbuf)<=0){
				swprintf(wbuf,szErr_VoidLabel,getLineNumber(),c);
				return clean(-1);
			}
			temp=delayedWriteOpcode();
			if (temp) return temp;
			for (lp=lroot;lp;lp=lp->next){
				if (!strcmp(lp->name,labelbuf)){
					break;
				}
			}
			if (!lp||lp->level<labellevel){//undefined and unused, or defined/used in low level
				lp2=(struct label *)malloc(sizeof(struct label));
				lp2->name=(char *)malloc(strlen(labelbuf)+1);
				strcpy(lp2->name,labelbuf);
				lp2->level=labellevel;
				rp=(struct reference *)malloc(sizeof(struct reference));
				rp->addr=pc;
				rp->mode=2;
				rp->next=NULL;
				lp2->rlist=rp;
				lp2->rlistend=rp;
				lp2->ancestor=lp;
				lp2->parent=lp;
				if (lp&&lp->level<labellevel-1){
					lp2->parent=NULL;
				}
				lp2->next=lroot;
				lroot=lp2;
			}else{
				if (!lp->rlist){//defined in current level
					rom[pc]=lp->addr>>8;
				}else{//undefined but used in current level
					rp=(struct reference *)malloc(sizeof(struct reference));
					rp->addr=pc;
					rp->mode=2;
					rp->next=lp->rlist;
					lp->rlist=rp;
				}
			}
			setPc();
			break;
		case '&':	//const label (mode=3)
			if (!length){
				length=2;
			}
			if (length!=2){
				sprintf(cbuf,szErr_CodeLen,getLineNumber(),(char *)&c,length);
				MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
				return clean(-1);
			}
			if (fscanf(fp,"%s",labelbuf)<=0){
				swprintf(wbuf,szErr_VoidLabel,getLineNumber(),c);
				return clean(-1);
			}
			temp=delayedWriteOpcode();
			if (temp) return temp;
			for (lp=lroot;lp;lp=lp->next){
				if (!strcmp(lp->name,labelbuf)){
					break;
				}
			}
			if (!lp||lp->level<labellevel){//undefined and unused, or defined/used in low level
				lp2=(struct label *)malloc(sizeof(struct label));
				lp2->name=(char *)malloc(strlen(labelbuf)+1);
				strcpy(lp2->name,labelbuf);
				lp2->level=labellevel;
				rp=(struct reference *)malloc(sizeof(struct reference));
				rp->addr=pc;
				rp->mode=3;
				rp->next=NULL;
				lp2->rlist=rp;
				lp2->rlistend=rp;
				lp2->ancestor=lp;
				lp2->parent=lp;
				if (lp&&lp->level<labellevel-1){
					lp2->parent=NULL;
				}
				lp2->next=lroot;
				lroot=lp2;
			}else{
				if (!lp->rlist){//defined in current level
					rom[pc]=lp->addr&0xFF;
					rom[pc+1]=lp->addr>>8;
				}else{//undefined but used in current level
					rp=(struct reference *)malloc(sizeof(struct reference));
					rp->addr=pc;
					rp->mode=3;
					rp->next=lp->rlist;
					lp->rlist=rp;
				}
			}
			setPc();
			break;
		case '#':	//const in dec
			if (!length){
				length=defnumlen;
			}
			if (length>4||length<0){
				sprintf(cbuf,szErr_CodeLen,getLineNumber(),(char *)&c,length);
				MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
				return clean(-1);
			}
			temp=delayedWriteOpcode();
			if (temp) return temp;
			fscanf(fp,"%d",&intbuf);
			for (temp=0;temp<length;++temp){
				rom[pc+temp]=intbuf&0xFF;
				intbuf>>=8;
			}
			setPc();
			break;
		case '$':	//const in hex
			if (!length){
				length=defnumlen;
			}
			if (length>4||length<0){
				sprintf(cbuf,szErr_CodeLen,getLineNumber(),(char *)&c,length);
				MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
				return clean(-1);
			}
			temp=delayedWriteOpcode();
			if (temp) return temp;
			fscanf(fp,"%x",&intbuf);
			for (temp=0;temp<length;++temp){
				rom[pc+temp]=intbuf&0xFF;
				intbuf>>=8;
			}
			setPc();
			break;
		case '\'':	//const char
			if (!length){
				length=1;
			}
			if (length!=1){
				sprintf(cbuf,szErr_CodeLen,getLineNumber(),(char *)&c,length);
				MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
				return clean(-1);
			}
			temp=delayedWriteOpcode();
			if (temp) return temp;
			fscanf(fp,"%c",&rom[pc]);
			setPc();
			break;
		case '\"':	//const str
			if (length<0||length>=tbuflen){
				sprintf(cbuf,szErr_CodeLen,getLineNumber(),(char *)&c,length);
				MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
				return clean(-1);
			}
			memset(labelbuf,0,tbuflen);
			fscanf(fp,"%s",labelbuf);
			if (!length){
				length=strlen(labelbuf);
			}else if (strlen(labelbuf)>length){
				sprintf(cbuf,szErr_Quotation_LenEx,getLineNumber(),labelbuf,length);
				MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
				return clean(-1);
			}
			temp=delayedWriteOpcode();
			if (temp) return temp;
			memcpy(rom+pc,labelbuf,length);
			setPc();
			break;
		case ';':	//comment
			if (ip){
				sprintf(cbuf,szErr_IllegalOprand,getLineNumber(),(char *)&c);
				MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
				return clean(-1);
			}
			if (length){
				sprintf(cbuf,szErr_CodeLen,getLineNumber(),(char *)&c,length);
				MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
				return clean(-1);
			}
			fgets(labelbuf,tbuflen,fp);
			break;
		case '{':	//block begin
			if (ip){
				sprintf(cbuf,szErr_IllegalOprand,getLineNumber(),(char *)&c);
				MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
				return clean(-1);
			}
			if (length){
				sprintf(cbuf,szErr_CodeLen,getLineNumber(),(char *)&c,length);
				MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
				return clean(-1);
			}
			++labellevel;
			break;
		case '}':	//block end
			if (ip){
				sprintf(cbuf,szErr_IllegalOprand,getLineNumber(),(char *)&c);
				MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
				return clean(-1);
			}
			if (length){
				sprintf(cbuf,szErr_CodeLen,getLineNumber(),(char *)&c,length);
				MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
				return clean(-1);
			}
			if (!labellevel){
				swprintf(wbuf,szErr_End_TooMuch,getLineNumber());
				return clean(-1);
			}
			--labellevel;
			lp2=(struct label *)malloc(sizeof(struct label));
			lp2->next=lroot;
			lroot=lp2;
			for (lp=lp2->next;lp&&lp->level>labellevel;lp=lp2->next){
				if (lp->rlist){//is reference
					if (!lp->parent){//no parent
						lp->level=labellevel;
						if (lp->ancestor&&lp->ancestor->level==labellevel-1){
							lp->parent=lp->ancestor;
						}
						lp2=lp;
						continue;
					}else{//has parent
						if (lp->parent->rlist){//parent is reference
							lp->rlistend->next=lp->parent->rlist;
							lp->parent->rlist=lp->rlist;
						}else{//parent is defination
							for (rp=lp->rlist;rp;rp=lp->rlist){
								switch (rp->mode){
								case 1:	//low
									rom[rp->addr]=lp->parent->addr&0xFF;
									break;
								case 2:	//high
									rom[rp->addr]=lp->parent->addr>>8;
									break;
								case 3:	//both, little endian
									rom[rp->addr]=lp->parent->addr&0xFF;
									rom[rp->addr+1]=lp->parent->addr>>8;
									break;
								default:
									swprintf(wbuf,szErr_ReferenceMode,getLineNumber(),c,rp->mode);
									return clean(-1);
								}
								lp->rlist=rp->next;
								free(rp);
							}
						}
					}
				}
				free(lp->name);
				lp2->next=lp->next;
				free(lp);
			}
			lp=lroot;
			lroot=lp->next;
			free(lp);
			break;
		case '\\':	//macro
			if (ip){
				sprintf(cbuf,szErr_IllegalOprand,getLineNumber(),(char *)&c);
				MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
				return clean(-1);
			}
			if (length){
				sprintf(cbuf,szErr_CodeLen,getLineNumber(),(char *)&c,length);
				MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
				return clean(-1);
			}
			fscanf(fp,"%s",combuf);
			if (!strcmp(combuf,"setophead")){
				fscanf(fp,"%d",&intbuf);
				ophead=intbuf;
			}else if (!strcmp(combuf,"setoptail")){
				fscanf(fp,"%d",&intbuf);
				optail=intbuf;
			}else if (!strcmp(combuf,"setdefnumlen")){
				fscanf(fp,"%d",&intbuf);
				defnumlen=intbuf;
			}else if (!strcmp(combuf,"songs")){
				fscanf(fp,"%d",&intbuf);
				head.songs=intbuf;
			}else if (!strcmp(combuf,"entry")){
				fscanf(fp,"%d",&intbuf);
				head.entry=intbuf;
			}else if (!strcmp(combuf,"name")){
				fgets(labelbuf,tbuflen,fp);
				labelbuf[tbuflen-1]=0;
				fgets(labelbuf,tbuflen,fp);
				if (labelbuf[tbuflen-1]){
					swprintf(wbuf,szErr_StrLen,getLineNumber(),L"name");
					return clean(-1);
				}
				for (i=strlen(labelbuf)-1;i>=0&&labelbuf[i]<' ';--i){
					labelbuf[i]=0;
				}
				strncpy(head.name,labelbuf,31);
				head.name[31]=0;
			}else if (!strcmp(combuf,"artist")){
				fgets(labelbuf,tbuflen,fp);
				labelbuf[tbuflen-1]=0;
				fgets(labelbuf,tbuflen,fp);
				if (labelbuf[tbuflen-1]){
					swprintf(wbuf,szErr_StrLen,getLineNumber(),L"artist");
					return clean(-1);
				}
				for (i=strlen(labelbuf)-1;i>=0&&labelbuf[i]<' ';--i){
					labelbuf[i]=0;
				}
				strncpy(head.artist,labelbuf,31);
				head.name[31]=0;
			}else if (!strcmp(combuf,"copyright")){
				fgets(labelbuf,tbuflen,fp);
				labelbuf[tbuflen-1]=0;
				fgets(labelbuf,tbuflen,fp);
				if (labelbuf[tbuflen-1]){
					swprintf(wbuf,szErr_StrLen,getLineNumber(),L"copyright");
					return clean(-1);
				}
				for (i=strlen(labelbuf)-1;i>=0&&labelbuf[i]<' ';--i){
					labelbuf[i]=0;
				}
				strncpy(head.copyright,labelbuf,31);
				head.name[31]=0;
			}else if (!strcmp(combuf,"ntsc")){
				fscanf(fp,"%d",&intbuf);
				head.ntsc=intbuf;
			}else if (!strcmp(combuf,"pal")){
				fscanf(fp,"%d",&intbuf);
				head.pal=intbuf;
			}else if (!strcmp(combuf,"ntscpal")){
				fscanf(fp,"%d",&intbuf);
				head.ntscpal=intbuf;
			}else if (!strcmp(combuf,"expand")){
				fscanf(fp,"%x",&intbuf);
				head.expand=intbuf;
			}else if (!strcmp(combuf,"bankswitch")){
				for (i=0;i<8;++i){
					fscanf(fp,"%x",&intbuf);
					head.bankswitch[i]=intbuf;
				}
			}else{
				sprintf(cbuf,szErr_Backslash_Unknown,getLineNumber(),combuf);
				MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
				return clean(-1);
			}
			break;
		default:	//instruction
			if (!length){
				length=1;
			}
			fseek(fp,-1,SEEK_CUR);
			fscanf(fp,"%s",combuf);
			if (ip){
				sprintf(cbuf,szErr_IllegalOprand,getLineNumber(),combuf);
				MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
				return clean(-1);
			}
			if (length!=1){
				sprintf(cbuf,szErr_CodeLen,getLineNumber(),combuf,length);
				MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
				return clean(-1);
			}
			i=insbsch(0,inslistlen-1);
			if (i<0){
				sprintf(cbuf,szErr_Opcode_Unknown,getLineNumber(),combuf);
				MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
				return clean(-1);
			}
			if (inslist[i].oplen){
				ip=&inslist[i];
				continue;
			}
			length=1;
			rom[pc]=inslist[i].code;
			setPc();
		}
	}
	if (labellevel){
		swprintf(wbuf,szErr_TooMuchBegin);
		return clean(-1);
	}
	for (lp=lroot;lp;lp=lp->next){
		if (!strcmp(lp->name,"_START_")){
			break;
		}
	}
	if (lp&&!lp->rlist){
		pc=head.start=lp->addr;
		length=0;
		setPc();
	}else{
		sprintf(cbuf,szErr_LabelUndefined,"_START_");
		MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
		return clean(-1);
	}
	if (min<0x6000){
		swprintf(wbuf,szErr_Underflow);
		return clean(-1);
	}
	if (head.start>min){
		swprintf(wbuf,szWarn_Underflow,head.start);
		MessageBoxW(NULL,wbuf,szAppName,MB_ICONWARNING);
	}
	if (ophead){
		for (lp=lroot;lp;lp=lp->next){
			if (!strcmp(lp->name,"_INIT_")){
				break;
			}
		}
		if (lp&&!lp->rlist){
			head.vinit=lp->addr;
		}else{
			sprintf(cbuf,szErr_LabelUndefined,"_INIT_");
			MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
			return clean(-1);
		}
		for (lp=lroot;lp;lp=lp->next){
			if (!strcmp(lp->name,"_PLAY_")){
				break;
			}
		}
		if (lp&&!lp->rlist){
			head.vplay=lp->addr;
		}else{
			sprintf(cbuf,szErr_LabelUndefined,"_PLAY_");
			MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
			return clean(-1);
		}
	}else{
		if (head.start&0xFFF){
			swprintf(wbuf,szErr_BinStart,head.start);
			return clean(-1);
		}
		if (head.start+4096<max){
			swprintf(wbuf,szWarn_Overflow,head.start+4096);
			MessageBoxW(NULL,wbuf,szAppName,MB_ICONWARNING);
		}
	}
	if (optail){
		max=(max+0xFFF)&~0xFFF;
	}
	temp=clean(0);
	if (temp) return temp;
	GetFullPathNameW(argv[1],tbuflen,wbuf,NULL);
	WideCharToMultiByte(CP_UTF8,0,wbuf,-1,labelbuf,tbuflen,0,0);
	for (temp=i=strlen(labelbuf);labelbuf[i]!='\\'&&labelbuf[i]!='/'&&i>=0;--i);
	for (;labelbuf[i]!='.'&&i<temp;++i);
	if (ophead){
		sprintf(labelbuf+i,".nsf");
	}else{
		sprintf(labelbuf+i,".bin");
	}
	MultiByteToWideChar(CP_UTF8,0,labelbuf,-1,wbuf,tbuflen);
	if (!(fp=_wfopen(wbuf,L"wb"))){
		sprintf(cbuf,szErr_WriteFile,labelbuf);
		MultiByteToWideChar(CP_UTF8,0,cbuf,-1,wbuf,tbuflen);
		MessageBoxW(NULL,wbuf,szAppName,MB_ICONERROR);
		LocalFree(argv);
		return -1;
	}
	if (ophead){
		fwrite(&head,sizeof(head),1,fp);
		fwrite(rom+head.start,max-head.start,1,fp);
	}else{
		fwrite(rom+head.start,4096,1,fp);
	}
	fclose(fp);
	LocalFree(argv);
	return 0;
}
