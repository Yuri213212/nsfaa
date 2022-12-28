#define maxBankSize	0x10000	//64K

struct tag34{
	uint8_t tag3;
	uint8_t index;
	uint8_t id;
	uint8_t name[8];
	uint8_t pstartLo;	//separate into uint8_t to avoid aligning problem
	uint8_t pstartHi;
	uint8_t sizeLo;
	uint8_t sizeHi;
	uint8_t t34Type;
	uint8_t tag4;
};

struct _bank{
	struct bankType *bt;
	int imageType;
	int side;
	int offset;
	int min;
	int max;
	int noHead;
	int noTail;
	struct tag34 t34;
	uint8_t dummy[sizeof(struct tag34)];
	uint8_t data[maxBankSize];
};

struct _bank *bank_new(struct bankType *bt,int imageType,int side,int index){
	struct _bank *bank;

	if (!bt) return NULL;
	bank=(struct _bank *)malloc(sizeof(struct _bank));
	bank->bt=bt;
	bank->imageType=imageType;
	bank->side=side;
	bank->offset=0;
	bank->min=maxBankSize;
	bank->max=-1;
	memset(&(bank->t34),0x00,sizeof(struct tag34));
	bank->t34.tag3=0x03;
	bank->t34.index=index;
	bank->t34.id=index;
	memcpy(&(bank->t34.name),bt->name,bankTypeNameLen);
	bank->t34.t34Type=bt->t34Type;
	bank->t34.tag4=0x04;
	memset(bank->data,0xFF,maxBankSize);
	return bank;
}

void bank_delete(struct _bank *bank){
	if (!bank) return;
	free(bank);
}

uint16_t bank_offset(struct _bank *bank){
	if (!bank) return 0;
	return bank->offset;
}

int bank_min(struct _bank *bank){
	if (!bank) return -1;
	if (bank->max<0) return -1;
	return bank->min;
}

void bank_setId(struct _bank *bank,int id){
	if (!bank) return;
	bank->t34.id=id;
}

void bank_setNoHead(struct _bank *bank,int noHead){
	if (!bank) return;
	bank->noHead=noHead;
}

void bank_setNoTail(struct _bank *bank,int noTail){
	if (!bank) return;
	bank->noTail=noTail;
}

//return 0 for success
int bank_setOffset(struct _bank *bank,uint16_t offset){
	if (!bank){
		vlstr_addf(errmsg,szErr_Assert,"bank_setOffset",0);
		return -1;
	}
	if (!bank->bt->isFragmented){
		vlstr_addf(errmsg,szErr_bank_SetOffsetForbidden,bank->side,bank->t34.index,bank->bt->name);
		return -1;
	}
	bank->offset=offset;
	return 0;
}

//return 0 for success
int bank_write(struct _bank *bank,uint16_t addr,uint8_t data){
	int mask;

	if (!bank){
		vlstr_addf(errmsg,szErr_Assert,"bank_write",0);
		return -1;
	}
	if (addr<bank->bt->minLimit){
		vlstr_addf(errmsg,szErr_bank_Underflow,bank->side,bank->t34.index,addr,bank->bt->minLimit);
		return -1;
	}
	if (addr>bank->bt->maxLimit){
		vlstr_addf(errmsg,szErr_bank_Overflow,bank->side,bank->t34.index,addr,bank->bt->maxLimit);
		return -1;
	}
	mask=bank->bt->bankSize-1;
	if (bank->bt->isFragmented){
		addr=(addr+bank->offset)&mask;
	}
	bank->data[addr]=data;
	if (addr<bank->min){
		bank->min=addr;
	}
	if (addr>bank->max){
		bank->max=addr;
	}
	if ((bank->min&~mask)!=(bank->max&~mask)){
		vlstr_addf(errmsg,szErr_bank_Wrap,bank->side,bank->t34.index,addr);
		return -1;
	}
	return 0;
}

//return 0 for success
int bank_writeLabel(struct _bank *bank,uint16_t offset,uint16_t addr,uint8_t data){
	int mask;

	if (!bank){
		vlstr_addf(errmsg,szErr_Assert,"bank_writeLabel",0);
		return -1;
	}
	if (addr<bank->bt->minLimit){
		vlstr_addf(errmsg,szErr_bank_Underflow,bank->side,bank->t34.index,addr,bank->bt->minLimit);
		return -1;
	}
	if (addr>bank->bt->maxLimit){
		vlstr_addf(errmsg,szErr_bank_Overflow,bank->side,bank->t34.index,addr,bank->bt->maxLimit);
		return -1;
	}
	mask=bank->bt->bankSize-1;
	if (bank->bt->isFragmented){
		addr=(addr+offset)&mask;
	}
	bank->data[addr]=data;
	if (addr<bank->min){
		bank->min=addr;
	}
	if (addr>bank->max){
		bank->max=addr;
	}
	if ((bank->min&~mask)!=(bank->max&~mask)){
		vlstr_addf(errmsg,szErr_bank_Wrap,bank->side,bank->t34.index,addr);
		return -1;
	}
	return 0;
}

//return -1 for fail
int bank_getLength(struct _bank *bank){
	int length,mask;

	if (!bank){
		vlstr_addf(errmsg,szErr_Assert,"bank_getLength",0);
		return -1;
	}
	mask=bank->bt->bankSize-1;
	switch (bank->imageType){
	case IT_bin:
	case IT_nes:
		return bank->bt->bankSize;
	case IT_nsf:
		if (bank->noTail){
			length=(bank->max&mask)+1;
		}else{
			length=bank->bt->bankSize;
		}
		if (bank->noHead){
			length-=(bank->min&mask);
		}
		return length;
	case IT_fds:
		if (bank->max<0) return sizeof(struct tag34);
		return (bank->max&mask)+1-(bank->min&mask)+sizeof(struct tag34);
	default:
		vlstr_addf(errmsg,szErr_Assert,"bank_getLength",1);
		return -1;
	}
}

uint8_t *bank_getData(struct _bank *bank){
	uint8_t *result;
	int mask;
	uint16_t temp;

	if (!bank) return NULL;
	if (bank->max<0) return bank->data;
	if (bank->bt->isFragmented) return bank->data;
	mask=bank->bt->bankSize-1;
	switch (bank->imageType){
	case IT_bin:
	case IT_nes:
		return bank->data+(bank->min&~mask);
	case IT_nsf:
		if (bank->noHead){
			return bank->data+bank->min;
		}else{
			return bank->data+(bank->min&~mask);
		}
	case IT_fds:
		temp=bank->min;
		bank->t34.pstartLo=temp&0xFF;
		bank->t34.pstartHi=temp>>8;
		temp=bank->max-bank->min+1;
		bank->t34.sizeLo=temp&0xFF;
		bank->t34.sizeHi=temp>>8;
		result=bank->data+bank->min-sizeof(struct tag34);
		memcpy(result,&(bank->t34),sizeof(struct tag34));
		return result;
	default:
		return NULL;
	}
}
