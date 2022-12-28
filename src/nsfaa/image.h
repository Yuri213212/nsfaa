#define maxSideCount	0xFF

struct nsfHeader{
	uint32_t id;		//0x4D53454E
	uint8_t id2;		//0x1A
	uint8_t headver;	//0x01
	uint8_t songs;		//song count
	uint8_t entry;		//default song is nth
	uint16_t pstart;	//pointer of _START_ (load address of data)
	uint16_t vinit;		//vector of _INIT_
	uint16_t vplay;		//vector of _PLAY_
	uint8_t name[32];	//string of Name
	uint8_t artist[32];	//string of Artist
	uint8_t copyright[32];	//string of (C)
	uint16_t div2A03;	//16666 (frequency divider from 1000000Hz to call _PLAY_ for 2A03)
	uint8_t bankswitch[8];	//initial bank switch data, all 0 for unused
	uint16_t div2A07;	//20000 (frequency divider from 1000000Hz to call _PLAY_ for 2A07)
	uint8_t apu;		//0=2A03,1=2A07,2=both
	uint8_t expand;		//-,-,S5B,N163,MMC5,FDS,VRC7,VRC6
	uint32_t dummy;		//0
};

struct nesHeader{
	uint32_t id;		//0x1A53454E
	uint8_t PRGbanks;	//count of 16K banks of PRG ROM
	uint8_t CHRbanks;	//count of 8K banks of CHR ROM
	uint8_t mapperLo;	//mapperlo(4),mirrorhi,trainer,battery,mirrorlo
	uint8_t mapperHi;	//mapperhi(4),-,-,PC10,VS
	uint32_t dummy2;	//0
	uint32_t dummy;		//0
};

struct fdsHeader{
	uint32_t id;		//0x1A534446
	uint32_t sides;		//side count, 0~255, total file size is 16+65500*sides
	uint32_t dummy[2];	//0
};

struct _image{
	struct _rom *roms[maxSideCount];
	struct _bank *bank;
	int sideCount;
	int imageType;
	int nsf_vinit;
	int nsf_vplay;
	struct nsfHeader nsfh;
	struct nesHeader nesh;
	struct fdsHeader fdsh;
	uint8_t data[];
};

int image_getMaxSize(int imageType){
	switch (imageType){
	case IT_bin:
		return rom_getMaxSize(imageType,0);
	case IT_nsf:
		return sizeof(struct nsfHeader)+rom_getMaxSize(imageType,0);
	case IT_nes:
		return sizeof(struct nesHeader)+rom_getMaxSize(imageType,0)+rom_getMaxSize(imageType,1);
	case IT_fds:
		return sizeof(struct fdsHeader)+fdsDiskSize*maxSideCount;
	default:
		return 0;
	}
}

struct _image *image_new(int imageType){
	struct _image *image;
	int maxSize;

	maxSize=image_getMaxSize(imageType);
	image=(struct _image *)malloc(sizeof(struct _image)+maxSize);
	memset(image,0x00,sizeof(struct _image)+maxSize);
	image->imageType=imageType;
	image->nsf_vinit=-1;
	image->nsf_vplay=-1;
	image->nsfh.id=0x4D53454E;
	image->nsfh.id2=0x1A;
	image->nsfh.headver=0x01;
	image->nsfh.songs=1;
	image->nsfh.entry=1;
	image->nsfh.div2A03=16666;
	image->nsfh.div2A07=20000;
	image->nesh.id=0x1A53454E;
	image->fdsh.id=0x1A534446;
	switch (imageType){
	case IT_bin:
	case IT_nsf:
		image->roms[0]=rom_new(imageType,0);
		image->sideCount=1;
		break;
	case IT_nes:
		image->roms[0]=rom_new(imageType,0);
		image->roms[1]=rom_new(imageType,1);
		image->sideCount=2;
		break;
	case IT_fds:
		break;
	default:
		free(image);
		return NULL;
	}
	return image;
}

void image_delete(struct _image *image){
	int i;

	if (!image) return;
	for (i=0;i<image->sideCount;++i){
		rom_delete(image->roms[i]);
	}
	free(image);
}

struct _bank *image_getBank(struct _image *image){
	if (!image) return NULL;
	return image->bank;
}

int image_getMin(struct _image *image){
	if (!image) return -1;
	if (!image->sideCount) return -1;
	return rom_getMin(image->roms[0]);
}

//return 0 for success
int image_setTrimEnd(struct _image *image,HVLSTR str){
	int intbuf;

	if (!image){
		vlstr_addf(errmsg,szErr_Assert,"image_setTrimEnd",0);
		return -1;
	}
	if (!str){
		vlstr_addf(errmsg,szErr_Assert,"image_setTrimEnd",1);
		return -1;
	}
	if (!image->sideCount) return 0;
	intbuf=1;
	sscanf(vlstr_getData(str),"%d",&intbuf);
	rom_setTrimEnd(image->roms[0],intbuf);
	return 0;
}

//return 0 for success
int image_setNsfSongs(struct _image *image,HVLSTR str){
	int intbuf;

	if (!image){
		vlstr_addf(errmsg,szErr_Assert,"image_setNsfSongs",0);
		return -1;
	}
	if (!str){
		vlstr_addf(errmsg,szErr_Assert,"image_setNsfSongs",1);
		return -1;
	}
	intbuf=1;
	sscanf(vlstr_getData(str),"%d",&intbuf);
	image->nsfh.songs=intbuf;
	return 0;
}

//return 0 for success
int image_setNsfEntry(struct _image *image,HVLSTR str){
	int intbuf;

	if (!image){
		vlstr_addf(errmsg,szErr_Assert,"image_setNsfEntry",0);
		return -1;
	}
	if (!str){
		vlstr_addf(errmsg,szErr_Assert,"image_setNsfEntry",1);
		return -1;
	}
	intbuf=1;
	sscanf(vlstr_getData(str),"%d",&intbuf);
	image->nsfh.entry=intbuf;
	return 0;
}

//return 0 for success
int image_setNsfName(struct _image *image,HVLSTR str){
	if (!image){
		vlstr_addf(errmsg,szErr_Assert,"image_setNsfName",0);
		return -1;
	}
	if (!str){
		vlstr_addf(errmsg,szErr_Assert,"image_setNsfName",1);
		return -1;
	}
	vlstr_drop(str);
	strncpy((char *)image->nsfh.name,vlstr_getData(str),31);
	image->nsfh.name[31]=0;
	return 0;
}

//return 0 for success
int image_setNsfArtist(struct _image *image,HVLSTR str){
	if (!image){
		vlstr_addf(errmsg,szErr_Assert,"image_setNsfArtist",0);
		return -1;
	}
	if (!str){
		vlstr_addf(errmsg,szErr_Assert,"image_setNsfArtist",1);
		return -1;
	}
	vlstr_drop(str);
	strncpy((char *)image->nsfh.artist,vlstr_getData(str),31);
	image->nsfh.artist[31]=0;
	return 0;
}

//return 0 for success
int image_setNsfCopyright(struct _image *image,HVLSTR str){
	if (!image){
		vlstr_addf(errmsg,szErr_Assert,"image_setNsfCopyright",0);
		return -1;
	}
	if (!str){
		vlstr_addf(errmsg,szErr_Assert,"image_setNsfCopyright",1);
		return -1;
	}
	vlstr_drop(str);
	strncpy((char *)image->nsfh.copyright,vlstr_getData(str),31);
	image->nsfh.copyright[31]=0;
	return 0;
}

//return 0 for success
int image_setNsfDiv2A03(struct _image *image,HVLSTR str){
	int intbuf;

	if (!image){
		vlstr_addf(errmsg,szErr_Assert,"image_setNsfDiv2A03",0);
		return -1;
	}
	if (!str){
		vlstr_addf(errmsg,szErr_Assert,"image_setNsfDiv2A03",1);
		return -1;
	}
	intbuf=16666;
	sscanf(vlstr_getData(str),"%d",&intbuf);
	image->nsfh.div2A03=intbuf;
	return 0;
}

//return 0 for success
int image_setNsfDiv2A07(struct _image *image,HVLSTR str){
	int intbuf;

	if (!image){
		vlstr_addf(errmsg,szErr_Assert,"image_setNsfDiv2A07",0);
		return -1;
	}
	if (!str){
		vlstr_addf(errmsg,szErr_Assert,"image_setNsfDiv2A07",1);
		return -1;
	}
	intbuf=20000;
	sscanf(vlstr_getData(str),"%d",&intbuf);
	image->nsfh.div2A07=intbuf;
	return 0;
}

//return 0 for success
int image_setNsfApu(struct _image *image,HVLSTR str){
	int intbuf;

	if (!image){
		vlstr_addf(errmsg,szErr_Assert,"image_setNsfApu",0);
		return -1;
	}
	if (!str){
		vlstr_addf(errmsg,szErr_Assert,"image_setNsfApu",1);
		return -1;
	}
	intbuf=0;
	sscanf(vlstr_getData(str),"%d",&intbuf);
	image->nsfh.apu=intbuf;
	return 0;
}

//return 0 for success
int image_setNsfExpand(struct _image *image,HVLSTR str){
	int intbuf;

	if (!image){
		vlstr_addf(errmsg,szErr_Assert,"image_setNsfExpand",0);
		return -1;
	}
	if (!str){
		vlstr_addf(errmsg,szErr_Assert,"image_setNsfExpand",1);
		return -1;
	}
	intbuf=0;
	sscanf(vlstr_getData(str),"%x",&intbuf);
	image->nsfh.expand=intbuf;
	return 0;
}

//return 0 for success
int image_setNsfBankSwitch(struct _image *image,HVLSTR str){
	HSR hsr;
	int intbuf,i;

	if (!image){
		vlstr_addf(errmsg,szErr_Assert,"image_setNsfBankSwitch",0);
		return -1;
	}
	if (!str){
		vlstr_addf(errmsg,szErr_Assert,"image_setNsfBankSwitch",1);
		return -1;
	}
	hsr=sr_new(vlstr_getData(str));
	for (i=0;i<8;++i){
		intbuf=0;
		sr_scan_x(hsr,&intbuf);
		image->nsfh.bankswitch[i]=intbuf;
	}
	sr_delete(hsr);
	return 0;
}

//return 0 for success
int image_setNsfVinit(struct _image *image,int addr){
	if (!image){
		vlstr_addf(errmsg,szErr_Assert,"image_setNsfVinit",0);
		return -1;
	}
	if (image->nsf_vinit>=0){
		vlstr_addf(errmsg,szErr_MacroRedefine,macros[M_nsf_init]);
		return -1;
	}
	image->nsf_vinit=addr;
	return 0;
}

//return 0 for success
int image_setNsfVplay(struct _image *image,int addr){
	if (!image){
		vlstr_addf(errmsg,szErr_Assert,"image_setNsfVplay",0);
		return -1;
	}
	if (image->nsf_vplay>=0){
		vlstr_addf(errmsg,szErr_MacroRedefine,macros[M_nsf_play]);
		return -1;
	}
	image->nsf_vplay=addr;
	return 0;
}

//return 0 for success
int image_setNesMapper(struct _image *image,HVLSTR str){
	int intbuf;

	if (!image){
		vlstr_addf(errmsg,szErr_Assert,"image_setNesMapper",0);
		return -1;
	}
	if (!str){
		vlstr_addf(errmsg,szErr_Assert,"image_setNesMapper",1);
		return -1;
	}
	intbuf=0;
	sscanf(vlstr_getData(str),"%x",&intbuf);
	image->nesh.mapperLo=((intbuf<<4)&0xF0)|(image->nesh.mapperLo&0x0F);
	image->nesh.mapperHi=(intbuf&0xF0)|(image->nesh.mapperHi&0x0F);
	return 0;
}

//return 0 for success
int image_setNesMirror(struct _image *image,HVLSTR str){
	int intbuf;

	if (!image){
		vlstr_addf(errmsg,szErr_Assert,"image_setNesMirror",0);
		return -1;
	}
	if (!str){
		vlstr_addf(errmsg,szErr_Assert,"image_setNesMirror",1);
		return -1;
	}
	intbuf=0;
	sscanf(vlstr_getData(str),"%d",&intbuf);
	image->nesh.mapperLo=(image->nesh.mapperLo&0xF6)|(intbuf&0x01)|(intbuf&~0x01?0x08:0x00);
	return 0;
}

//return 0 for success
int image_setNesBattery(struct _image *image,HVLSTR str){
	int intbuf;

	if (!image){
		vlstr_addf(errmsg,szErr_Assert,"image_setNesBattery",0);
		return -1;
	}
	if (!str){
		vlstr_addf(errmsg,szErr_Assert,"image_setNesBattery",1);
		return -1;
	}
	intbuf=0;
	sscanf(vlstr_getData(str),"%d",&intbuf);
	image->nesh.mapperLo=(image->nesh.mapperLo&0xFD)|(intbuf?0x02:0x00);
	return 0;
}

//return 0 for success
int image_setNesBankOffset(struct _image *image,HVLSTR str){
	int intbuf;

	if (!image){
		vlstr_addf(errmsg,szErr_Assert,"image_setNesBankOffset",0);
		return -1;
	}
	if (!str){
		vlstr_addf(errmsg,szErr_Assert,"image_setNesBankOffset",1);
		return -1;
	}
	if (!image->sideCount){
		vlstr_addf(errmsg,szErr_Assert,"image_setNesBankOffset",2);
		return -1;
	}
	intbuf=0;
	sscanf(vlstr_getData(str),"%x",&intbuf);
	return bank_setOffset(image->bank,intbuf);
}

//return 0 for success
int image_setFdsBootToId(struct _image *image,HVLSTR str){
	int intbuf;

	if (!image){
		vlstr_addf(errmsg,szErr_Assert,"image_setFdsBootToId",0);
		return -1;
	}
	if (!str){
		vlstr_addf(errmsg,szErr_Assert,"image_setFdsBootToId",1);
		return -1;
	}
	if (!image->sideCount){
		vlstr_addf(errmsg,szErr_image_SetFdsBootToIdNoSide);
		return -1;
	}
	intbuf=0;
	sscanf(vlstr_getData(str),"%d",&intbuf);
	rom_setBootToId(image->roms[image->sideCount-1],intbuf);
	return 0;
}

//return 0 for success
int image_setFdsBankId(struct _image *image,HVLSTR str){
	int intbuf;

	if (!image){
		vlstr_addf(errmsg,szErr_Assert,"image_setFdsBankId",0);
		return -1;
	}
	if (!str){
		vlstr_addf(errmsg,szErr_Assert,"image_setFdsBankId",1);
		return -1;
	}
	if (!image->sideCount){
		vlstr_addf(errmsg,szErr_image_SetFdsBankIdNoSide);
		return -1;
	}
	intbuf=0;
	sscanf(vlstr_getData(str),"%d",&intbuf);
	return rom_setBankId(image->roms[image->sideCount-1],intbuf);
}

//return 0 for success
int image_newFdsSide(struct _image *image){
	struct _rom *rom;

	if (!image){
		vlstr_addf(errmsg,szErr_Assert,"image_newFdsSide",0);
		return -1;
	}
	if (image->imageType!=IT_fds){
		vlstr_addf(errmsg,szErr_image_NewFdsSideImageType);
		return -1;
	}
	if (image->sideCount>=maxSideCount){
		vlstr_addf(errmsg,szErr_image_NewFdsSideMaxSideCount,maxSideCount);
		return -1;
	}
	rom=rom_new(image->imageType,image->sideCount);
	if (!rom){
		vlstr_addf(errmsg,szErr_Assert,"image_newFdsSide",1);
		return -1;
	}
	image->roms[image->sideCount]=rom;
	++image->sideCount;
	image->bank=NULL;
	return 0;
}

//return 0 for success
int image_newBank(struct _image *image,HVLSTR str){
	struct bankType *bt;
	char *bankType;
	int sidei;

	if (!image){
		vlstr_addf(errmsg,szErr_Assert,"image_newBank",0);
		return -1;
	}
	if (!str){
		vlstr_addf(errmsg,szErr_Assert,"image_newBank",1);
		return -1;
	}
	if (!image->sideCount){
		vlstr_addf(errmsg,szErr_image_NewBankNoSide);
		return -1;
	}
	bankType=vlstr_getData(str);
	bt=bt_getBankType(bankType);
	if (!bt){
		vlstr_addf(errmsg,szErr_image_NewBankUnknownType,bankType);
		return -1;
	}
	if (!(image->imageType&bt->imageTypeMask)){
		vlstr_addf(errmsg,szErr_image_NewBankForbiddenType,bankType);
		return -1;
	}
	if (image->imageType==IT_nes&&bt->t34Type!=T34T_chr){
		sidei=0;
	}else{
		sidei=image->sideCount-1;
	}
	if (rom_newBank(image->roms[sidei],bt)) return -1;
	image->bank=rom_getBank(image->roms[sidei]);
	if (!image->bank){
		vlstr_addf(errmsg,szErr_Assert,"image_newBank",2);
		return -1;
	}
	return 0;
}

//return 0 for success
int image_writeByte(struct _image *image,uint16_t addr,uint8_t data){
	if (!image){
		vlstr_addf(errmsg,szErr_Assert,"image_writeByte",0);
		return -1;
	}
	if (!image->bank){
		vlstr_addf(errmsg,szErr_image_WriteByteNoBank);
		return -1;
	}
	return bank_write(image->bank,addr,data);
}

//return 0 for success
int image_importFile(struct _image *image,uint16_t addr,FILE *fp){
	int c,i;

	if (!image){
		vlstr_addf(errmsg,szErr_Assert,"image_importFile",0);
		return -1;
	}
	if (!fp){
		vlstr_addf(errmsg,szErr_Assert,"image_importFile",1);
		return -1;
	}
	if (!image->bank){
		vlstr_addf(errmsg,szErr_image_ImportFileNoBank);
		return -1;
	}
	i=0;
	for (c=fgetc(fp);c>=0;c=fgetc(fp)){
		if (bank_write(image->bank,addr+i,c)) return -1;
		++i;
	}
	return 0;
}

//return -1 for fail
int image_getLength(struct _image *image){
	int length,i,j;

	if (!image){
		vlstr_addf(errmsg,szErr_Assert,"image_getLength",0);
		return -1;
	}
	switch (image->imageType){
	case IT_bin:
		j=0;
		break;
	case IT_nsf:
		if (image->sideCount!=1){
			vlstr_addf(errmsg,szErr_Assert,"image_getLength",1);
			return -1;
		}
		if (image->nsf_vinit<0){
			vlstr_addf(errmsg,szErr_image_GetLengthNoParam,macros[M_nsf_init]);
			return -1;
		}
		if (image->nsf_vplay<0){
			vlstr_addf(errmsg,szErr_image_GetLengthNoParam,macros[M_nsf_play]);
			return -1;
		}
		j=sizeof(struct nsfHeader);
		break;
	case IT_nes:
		if (image->sideCount!=2){
			vlstr_addf(errmsg,szErr_Assert,"image_getLength",2);
			return -1;
		}
		j=sizeof(struct nesHeader);
		break;
	case IT_fds:
		j=sizeof(struct fdsHeader);
		break;
	default:
		vlstr_addf(errmsg,szErr_Assert,"image_getLength",1);
		return -1;
	}
	for (i=0;i<image->sideCount;++i){
		length=rom_getLength(image->roms[i]);
		if (length<0) return -1;
		j+=length;
	}
	return j;
}

uint8_t *image_getData(struct _image *image){
	int length,i,j;

	if (!image) return NULL;
	switch (image->imageType){
	case IT_bin:
		j=0;
		break;
	case IT_nsf:
		j=sizeof(struct nsfHeader);
		image->nsfh.pstart=rom_getMin(image->roms[0]);
		image->nsfh.vinit=image->nsf_vinit;
		image->nsfh.vplay=image->nsf_vplay;
		memcpy(image->data,&image->nsfh,j);
		break;
	case IT_nes:
		j=sizeof(struct nesHeader);
		image->nesh.PRGbanks=rom_bankCount(image->roms[0]);
		image->nesh.CHRbanks=rom_bankCount(image->roms[1]);
		memcpy(image->data,&image->nesh,j);
		break;
	case IT_fds:
		j=sizeof(struct fdsHeader);
		image->fdsh.sides=image->sideCount;
		memcpy(image->data,&image->fdsh,j);
		break;
	default:
		return NULL;
	}
	for (i=0;i<image->sideCount;++i){
		length=rom_getLength(image->roms[i]);
		if (length<0) return NULL;
		if (!length) continue;
		memcpy(image->data+j,rom_getData(image->roms[i]),length);
		j+=length;
	}
	return image->data;
}
