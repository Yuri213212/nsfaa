#define maxBankCount	0x100
#define nesBankCount	0xFF

#define nsfBankSize	0x1000	//4K
#define prgBankSize	0x4000	//16K
#define chrBankSize	0x2000	//8K
#define fdsDiskSize	65500

struct tag12{
	uint8_t tag1;
	uint8_t id[14];
	uint8_t dummy[6];
	uint8_t sideAB;
	uint8_t disk;
	uint8_t dummy2[2];
	uint8_t bootToId;
	uint8_t dummy3[30];
	uint8_t tag2;
	uint8_t bankCount;
};

struct _rom{
	struct _bank *banks[maxBankCount];
	int bankCount;
	int bankLimit;
	int imageType;
	int side;
	int trimEnd;
	int maxSize;
	int bootToId;
	struct tag12 t12;
	uint8_t data[];
};

int rom_getBanklimit(int imageType){
	switch (imageType){
	case IT_bin:
	case IT_nsf:
		return maxBankCount;
	case IT_nes:
	case IT_fds:
		return nesBankCount;
	default:
		return 0;
	}
}

int rom_getMaxSize(int imageType,int side){
	switch (imageType){
	case IT_bin:
		if (!side) return maxBankSize*rom_getBanklimit(imageType);
		return 0;
	case IT_nsf:
		if (!side) return nsfBankSize*rom_getBanklimit(imageType);
		return 0;
	case IT_nes:
		if (!side) return prgBankSize*rom_getBanklimit(imageType);
		if (side==1) return chrBankSize*rom_getBanklimit(imageType);
		return 0;
	case IT_fds:
		return fdsDiskSize-sizeof(struct tag12);
	default:
		return 0;
	}
}

struct _rom *rom_new(int imageType,int side){
	struct _rom *rom;
	int maxSize;

	maxSize=rom_getMaxSize(imageType,side);
	if (!maxSize) return NULL;
	rom=(struct _rom *)malloc(sizeof(struct _rom)+maxSize);
	memset(rom,0x00,sizeof(struct _rom)+maxSize);
	rom->bankLimit=rom_getBanklimit(imageType);
	rom->imageType=imageType;
	rom->side=side;
	rom->trimEnd=1;
	rom->maxSize=maxSize;
	rom->bootToId=-1;
	rom->t12.tag1=0x01;
	memcpy(&(rom->t12.id),"*NINTENDO-HVC*",14);
	rom->t12.sideAB=side&1;
	rom->t12.disk=side>>1;
	rom->t12.tag2=0x02;
	return rom;
}

void rom_delete(struct _rom *rom){
	int i;

	if (!rom) return;
	for (i=0;i<rom->bankCount;++i){
		bank_delete(rom->banks[i]);
	}
	free(rom);
}

int rom_bankCount(struct _rom *rom){
	if (!rom) return -1;
	return rom->bankCount;
}

struct _bank *rom_getBank(struct _rom *rom){
	if (!rom) return NULL;
	if (!rom->bankCount) return NULL;
	return rom->banks[rom->bankCount-1];
}

int rom_getMin(struct _rom *rom){
	if (!rom) return -1;
	if (!rom->bankCount) return -1;
	return bank_min(rom->banks[0]);
}

void rom_setTrimEnd(struct _rom *rom,int trimEnd){
	if (!rom) return;
	rom->trimEnd=trimEnd;
}

void rom_setBootToId(struct _rom *rom,int bootToId){
	if (!rom) return;
	rom->bootToId=bootToId;
}

//return 0 for success
int rom_newBank(struct _rom *rom,struct bankType *bt){
	if (!rom){
		vlstr_addf(errmsg,szErr_Assert,"rom_newBank",0);
		return -1;
	}
	if (!bt){
		vlstr_addf(errmsg,szErr_Assert,"rom_newBank",1);
		return -1;
	}
	if (rom->bankCount>=rom->bankLimit){
		vlstr_addf(errmsg,szErr_rom_NewBankMaxBankCount,rom->side,rom->bankLimit);
		return -1;
	}
	rom->banks[rom->bankCount]=bank_new(bt,rom->imageType,rom->side,rom->bankCount);
	if (!rom->banks[rom->bankCount]){
		vlstr_addf(errmsg,szErr_Assert,"rom_newBank",2);
		return -1;
	}
	++rom->bankCount;
	return 0;
}

//return 0 for success
int rom_setBankId(struct _rom *rom,int id){
	if (!rom){
		vlstr_addf(errmsg,szErr_Assert,"rom_setBankId",0);
		return -1;
	}
	if (!rom->bankCount){
		vlstr_addf(errmsg,szErr_rom_SetBankIdNoBank,rom->side);
		return -1;
	}
	bank_setId(rom->banks[rom->bankCount-1],id);
	return 0;
}

//return -1 for fail
int rom_getLength(struct _rom *rom){
	int length,i,j;

	if (!rom){
		vlstr_addf(errmsg,szErr_Assert,"rom_getLength",0);
		return -1;
	}
	if (rom->bankCount){
		bank_setNoHead(rom->banks[0],1);
		bank_setNoTail(rom->banks[rom->bankCount-1],rom->trimEnd);
	}
	j=0;
	for (i=0;i<rom->bankCount;++i){
		length=bank_getLength(rom->banks[i]);
		if (length<0) return -1;
		j+=length;
	}
	if (j>rom->maxSize){
		vlstr_addf(errmsg,szErr_rom_GetLengthTooLong,rom->side);
		return -1;
	}
	switch (rom->imageType){
	case IT_bin:
	case IT_nsf:
	case IT_nes:
		return j;
	case IT_fds:
		return fdsDiskSize;
	default:
		vlstr_addf(errmsg,szErr_Assert,"rom_getLength",1);
		return -1;
	}
}

uint8_t *rom_getData(struct _rom *rom){
	int length,i,j;

	if (!rom) return NULL;
	if (rom->bankCount){
		bank_setNoHead(rom->banks[0],1);
		bank_setNoTail(rom->banks[rom->bankCount-1],rom->trimEnd);
	}
	j=0;
	for (i=0;i<rom->bankCount;++i){
		length=bank_getLength(rom->banks[i]);
		if (length<0) return NULL;
		if (!length) continue;
		memcpy(rom->data+j,bank_getData(rom->banks[i]),length);
		j+=length;
	}
	switch (rom->imageType){
	case IT_bin:
	case IT_nsf:
	case IT_nes:
		return rom->data;
	case IT_fds:
		rom->t12.bankCount=rom->bankCount;
		if (rom->bootToId>=0){
			rom->t12.bootToId=rom->bootToId;
		}else{
			rom->t12.bootToId=rom->bankCount;
		}
		return (uint8_t *)&rom->t12;
	default:
		return NULL;
	}
}
