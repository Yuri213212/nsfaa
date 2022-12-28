#include <stdio.h>
#include <math.h>

#define basenote	(4*12+9)	//A-4
#define basefreq	440.0

#define fcpu		1789772.5
#define sr		44100.0
#define fxcount		8

int divregs[12],fx16regs[96];
int divrefnote,fx16refnote;

char *getnote(int note){
	note=(note%12+12)%12;
	switch (note){
	case 0:
		return "C-";
	case 1:
		return "C#";
	case 2:
		return "D-";
	case 3:
		return "D#";
	case 4:
		return "E-";
	case 5:
		return "F-";
	case 6:
		return "F#";
	case 7:
		return "G-";
	case 8:
		return "G#";
	case 9:
		return "A-";
	case 10:
		return "A#";
	case 11:
		return "B-";
	default:
		return NULL;
	}
}

double divfreq(int reg){
	return fcpu/16.0/(reg+1);
}

double fxfreq(int reg,int len){
	return floor(fcpu*12.0*pow(2.0,20.0)/(45.0*sr))*sr*reg/(pow(2.0,38.0)*fxcount*len);
}

double freq2note(double freq){
	return log2(freq/basefreq)*12.0+basenote;
}

double note2freq(int note){
	return pow(2.0,(note-basenote)/12.0)*basefreq;
}

double note2divfreq(int note){
	int reg,oct;

	note-=divrefnote;
	reg=divregs[(note%12+12)%12];
	oct=(note+120)/12-10;
	return divfreq(reg)*pow(2.0,oct);
}

double note2fx16freq(int note){
	if ((note-fx16refnote)>=0&&(note-fx16refnote)<96){
		return fxfreq(fx16regs[note-fx16refnote],16);
	}
	return note2divfreq(note);
}

int main(){
	double reffreq,cfreq0,cfreq1;
	int refnote,i,j;
	int reg[96];

	printf(":_ADDR_TUNING_AH ;push address\n");
	printf("~MSGTUNING\t\"A-4=440.00Hz\n");

//div
	refnote=ceil(freq2note(divfreq(0x100)))+11;
	divrefnote=refnote-59;
	printf(";DIV\t0=%s%d\n",getnote(divrefnote),(divrefnote+120)/12-10);
	printf("@%02X\t:TRIKEYOFFSET\n",24+divrefnote-12);
	printf("@%02X\t:RECKEYOFFSET\n",24+divrefnote);
	j=0;
	reffreq=note2freq(refnote);
	cfreq0=divfreq(0x80);
	for (i=0x80;i<=0xFF;++i){
		cfreq1=divfreq(i+1);
		if (cfreq0>=reffreq&&reffreq>cfreq1){
			if (cfreq0-reffreq>reffreq-cfreq1){
				++i;
				cfreq1=divfreq(i+1);
			}
			reg[j]=i;
			++j;
			reffreq=note2freq(refnote-j);
		}
		cfreq0=cfreq1;
	}
	if (j!=12){
		printf("error\n");
		return -1;
	}
	printf("~DIVNOTELO\n");
	for (j=4;j>=0;--j){
		for (i=11;i>=0;--i){
			printf("$%02X ",(((reg[i]+1)<<j)-1)&0xFF);
		}
		printf("\n");
	}
	printf("~DIVNOTEHI\n");
	for (j=4;j>=0;--j){
		for (i=11;i>=0;--i){
			printf("$%02X ",(((reg[i]+1)<<j)-1)>>8);
		}
		printf("\n");
	}
	for (i=11;i>=0;--i){
		divregs[11-i]=(((reg[i]+1)<<4)-1);
	}
/*
for (j=4;j>=0;--j){
	for (i=11;i>=0;--i){
		printf("%02X\t%02X\n",(((reg[i]+1)<<j)-1)>>8,(((reg[i]+1)<<j)-1)&0xFF);
	}
}
*/
//fx16
	refnote=floor(freq2note(fxfreq(0x40000,16)))-95;
	fx16refnote=refnote;
	printf(";FX16\t0=%s%d\n",getnote(refnote),(refnote+120)/12-10);
	printf("@%02X\t:FX16KEYOFFSET\n",24+refnote);
	j=0;
	reffreq=note2divfreq(refnote);
	cfreq0=fxfreq(0x80,16);
	for (i=0x80;i<=0x3FFFF;++i){
		cfreq1=fxfreq(i+1,16);
		if (cfreq0<=reffreq&&reffreq<cfreq1){
			if (cfreq0-reffreq<reffreq-cfreq1){
				++i;
				cfreq1=fxfreq(i+1,16);
			}
			reg[j]=i;
			fx16regs[j]=i;
			++j;
			reffreq=note2divfreq(refnote+j);
		}
		cfreq0=cfreq1;
	}
	if (j!=96){
		printf("error\n");
		return -1;
	}
	printf("~FX16NOTELO\n");
	for (j=0;j<8;++j){
		for (i=0;i<12;++i){
			printf("$%02X ",reg[j*12+i]&0xFF);
		}
		printf("\n");
	}
	printf("~FX16NOTEMID\n");
	for (j=0;j<8;++j){
		for (i=0;i<12;++i){
			printf("$%02X ",(reg[j*12+i]>>8)&0xFF);
		}
		printf("\n");
	}
	printf("~FX16NOTEHI\n");
	for (j=0;j<8;++j){
		for (i=0;i<12;++i){
			printf("$%02X ",(reg[j*12+i]>>16)|0xF0);
		}
		printf("\n");
	}
/*
for (j=0;j<8;++j){
	for (i=0;i<12;++i){
		printf("%02X\t%02X\t%02X\n",(reg[j*12+i]>>16)|0xF0,(reg[j*12+i]>>8)&0xFF,reg[j*12+i]&0xFF);
	}
}
*/
//fx32
	refnote=floor(freq2note(fxfreq(0x40000,32)))-95;
	printf(";FX32\t0=%s%d\n",getnote(refnote),(refnote+120)/12-10);
	printf("@%02X\t:FX32KEYOFFSET\n",24+refnote);
	j=0;
	reffreq=note2fx16freq(refnote);
	cfreq0=fxfreq(0x80,32);
	for (i=0x80;i<=0x3FFFF;++i){
		cfreq1=fxfreq(i+1,32);
		if (cfreq0<=reffreq&&reffreq<cfreq1){
			if (cfreq0-reffreq<reffreq-cfreq1){
				++i;
				cfreq1=fxfreq(i+1,32);
			}
			reg[j]=i;
			++j;
			reffreq=note2fx16freq(refnote+j);
		}
		cfreq0=cfreq1;
	}
	if (j!=96){
		printf("error\n");
		return -1;
	}
	printf("~FX32NOTELO\n");
	for (j=0;j<8;++j){
		for (i=0;i<12;++i){
			printf("$%02X ",reg[j*12+i]&0xFF);
		}
		printf("\n");
	}
	printf("~FX32NOTEMID\n");
	for (j=0;j<8;++j){
		for (i=0;i<12;++i){
			printf("$%02X ",(reg[j*12+i]>>8)&0xFF);
		}
		printf("\n");
	}
	printf("~FX32NOTEHI\n");
	for (j=0;j<8;++j){
		for (i=0;i<12;++i){
			printf("$%02X ",(reg[j*12+i]>>16)|0xE0);
		}
		printf("\n");
	}
/*
for (j=0;j<8;++j){
	for (i=0;i<12;++i){
		printf("%02X\t%02X\t%02X\n",(reg[j*12+i]>>16)|0xE0,(reg[j*12+i]>>8)&0xFF,reg[j*12+i]&0xFF);
	}
}
*/

	printf("~_ADDR_TUNING_AH ;pop address\n");
	return 0;
}
