#define imageTypeCount	4
enum imageTypeEnum{
	IT_dummy=0x00,
	IT_bin=0x01,
	IT_nsf=0x02,
	IT_nes=0x04,
	IT_fds=0x08,
};

const char imageTypes[imageTypeCount][4]={
	"bin",
	"nsf",
	"nes",
	"fds",
};

int getImageTypeEnum(char *s){
	int i;

	if (!s) return IT_dummy;
	for (i=0;i<imageTypeCount;++i){
		if (!strcmp(s,imageTypes[i])) return 1<<i;
	}
	return IT_dummy;
}

const char *getImageTypeString(int imageType){
	switch (imageType){
	case IT_bin:
		return imageTypes[0];
	case IT_nsf:
		return imageTypes[1];
	case IT_nes:
		return imageTypes[2];
	case IT_fds:
		return imageTypes[3];
	default:
		return NULL;
	}
}
