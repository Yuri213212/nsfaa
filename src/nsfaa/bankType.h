enum t34TypeEnum{
	T34T_dummy=-1,
	T34T_prg,
	T34T_chr,
	T34T_ci,
};

#define bankTypeListCount	20
#define bankTypeNameLen		8

struct bankType{
	char name[bankTypeNameLen];
	int imageTypeMask;
	int bankSize;
	int minLimit;
	int maxLimit;
	int isFragmented;
	int t34Type;
};

struct bankType bankTypeList[bankTypeListCount]={
	{"BIN_16K",	IT_bin,			0x4000,	0x0000,0xFFFF,0,T34T_dummy	},
	{"BIN_1K",	IT_bin,			0x400,	0x0000,0xFFFF,0,T34T_dummy	},
	{"BIN_256",	IT_bin,			0x100,	0x0000,0xFFFF,0,T34T_dummy	},
	{"BIN_2K",	IT_bin,			0x800,	0x0000,0xFFFF,0,T34T_dummy	},
	{"BIN_32K",	IT_bin,			0x8000,	0x0000,0xFFFF,0,T34T_dummy	},
	{"BIN_4K",	IT_bin,			0x1000,	0x0000,0xFFFF,0,T34T_dummy	},
	{"BIN_512",	IT_bin,			0x200,	0x0000,0xFFFF,0,T34T_dummy	},
	{"BIN_64K",	IT_bin,			0x10000,0x0000,0xFFFF,0,T34T_dummy	},
	{"BIN_8K",	IT_bin,			0x2000,	0x0000,0xFFFF,0,T34T_dummy	},
	{"CHR-8K",	IT_nes,			0x2000,	0x0000,0x1FFF,1,T34T_chr	},
	{"CHR_8K",	IT_bin|IT_nes|IT_fds,	0x2000,	0x0000,0x1FFF,0,T34T_chr	},
	{"CI_2K",	IT_fds,			0x800,	0x2000,0x3FFF,0,T34T_ci		},
	{"FDS_64K",	IT_fds,			0x10000,0x6000,0xDFFF,0,T34T_prg	},
	{"NSF_16K",	IT_bin|IT_nsf,		0x4000,	0x6000,0xFFFF,0,T34T_dummy	},
	{"NSF_32K",	IT_bin|IT_nsf,		0x8000,	0x6000,0xFFFF,0,T34T_dummy	},
	{"NSF_4K",	IT_bin|IT_nsf,		0x1000,	0x6000,0xFFFF,0,T34T_dummy	},
	{"NSF_8K",	IT_bin|IT_nsf,		0x2000,	0x6000,0xFFFF,0,T34T_dummy	},
	{"PRG-16K",	IT_nes,			0x4000,	0x8000,0xFFFF,1,T34T_dummy	},
	{"PRG_16K",	IT_bin|IT_nes,		0x4000,	0x8000,0xFFFF,0,T34T_dummy	},
	{"PRG_64K",	IT_fds,			0x10000,0x0000,0xFFFF,0,T34T_prg	},
};

int bt_bsch(char *s,int l,int r){
	int m,res;

	if (!s) return -1;
	m=(l+r)/2;
	res=strcmp(s,bankTypeList[m].name);
	if (!res) return m;
	if (res<0){
		if (m-1<l) return -1;
		return bt_bsch(s,l,m-1);
	}else{
		if (m+1>r) return -1;
		return bt_bsch(s,m+1,r);
	}
}

struct bankType *bt_getBankType(char *s){
	int i;

	if (!s) return NULL;
	i=bt_bsch(s,0,bankTypeListCount-1);
	if (i<0) return NULL;
	return &bankTypeList[i];
}
