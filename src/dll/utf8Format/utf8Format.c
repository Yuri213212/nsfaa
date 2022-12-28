#include <stdio.h>

#define BOM_SIZE	3

const char BOM[BOM_SIZE]={0xEF,0xBB,0xBF};

int utf8_checkBOM(FILE *fp){
	int i;
	char c;

	fseek(fp,0,SEEK_END);
	i=ftell(fp);
	rewind(fp);
	if (i<BOM_SIZE) return 0;
	for (i=0;i<BOM_SIZE;++i){
		c=fgetc(fp);
		if (c!=BOM[i]){
			rewind(fp);
			return 0;
		}
	}
	return 1;
}

void utf8_writeBOM(FILE *fp){
	fwrite(BOM,BOM_SIZE,1,fp);
}
