struct aaStatus{
	HPS hps;
	HVLSTR filePath;
	struct _ins *ins;
	int defnumlen;
	int labellevel;
	struct labelList *ll;
	struct aaFileData *aafd;
	struct _image *image;
	uint16_t pc;
};

struct aaStatus *aas_new(HPS hps){
	struct aaStatus *aas;

	if (!hps) return NULL;
	aas=(struct aaStatus *)malloc(sizeof(struct aaStatus));
	aas->hps=hps;
	aas->filePath=NULL;
	aas->ins=NULL;
	aas->defnumlen=1;
	aas->labellevel=0;
	aas->ll=ll_new();
	aas->aafd=NULL;
	aas->image=NULL;
	aas->pc=0;
	return aas;
}

void aas_delete(struct aaStatus *aas){
	if (!aas) return;
	ll_delete(aas->ll);
	ps_aaFile_clear(aas->hps);
	image_delete(aas->image);
	free(aas);
}

//return 0 for success
int aas_writeIns(struct aaStatus *aas){
	if (!aas){
		vlstr_addf(errmsg,szErr_Assert,"aas_writeIns",0);
		return -1;
	}
	if (!aas->ins) return 0;
	if (aas->aafd->codelen<1||aas->aafd->codelen>2||!(aas->aafd->codelen&aas->ins->oplen)){
		vlstr_addf(errmsg,szErr_aaStatus_WriteInsOprandLen,aas->aafd->codelen);
		return -1;
	}
	if (aas->aafd->codelen+aas->ins->oplen==5){
		if (image_writeByte(aas->image,aas->pc,aas->ins->codealt)) return -1;
	}else{
		if (image_writeByte(aas->image,aas->pc,aas->ins->code)) return -1;
	}
	++aas->pc;
	aas->ins=NULL;
	return 0;
}

//return 0 for success
int aas_addLabel(struct aaStatus *aas,int refType){
	struct labelList *lp;
	int rel;

	if (!aas){
		vlstr_addf(errmsg,szErr_Assert,"aas_addLabel",0);
		return -1;
	}
	lp=ll_query(aas->ll,aas->aafd->cmds);
	if (!lp||lp->level<aas->labellevel){//undefined and unused, or defined/used in low level
		if (ll_push(aas->ll,lp,image_getBank(aas->image),aas->aafd->cmds,aas->pc,aas->labellevel,refType)) return -1;
	}else switch (refType){
	case RT_dummy://define label
		if (ll_isDefined(lp)){//defined in current level
			vlstr_addf(errmsg,szErr_aaStatus_AddLabelRedefine,aas->pc,vlstr_getData(aas->aafd->cmds));
			return -1;
		}else{//undefined but used in current level
			if (ll_defineAddr(lp,aas->pc)) return -1;
		}
		break;
	case RT_rel://relative label
		if (ll_isDefined(lp)){//defined in current level
			rel=lp->addr-aas->pc-1;
			if (rel<-0x80||rel>0x7F){
				vlstr_addf(errmsg,szErr_RelOutOfRange,aas->pc,rel);
				return -1;
			}
			if (image_writeByte(aas->image,aas->pc,rel)) return -1;
		}else{//undefined but used in current level
			if (ll_addRef(lp,image_getBank(aas->image),refType,aas->pc)) return -1;
		}
		break;
	case RT_low://low label
		if (ll_isDefined(lp)){//defined in current level
			if (image_writeByte(aas->image,aas->pc,lp->addr&0xFF)) return -1;
		}else{//undefined but used in current level
			if (ll_addRef(lp,image_getBank(aas->image),refType,aas->pc)) return -1;
		}
		break;
	case RT_high://high label
		if (ll_isDefined(lp)){//defined in current level
			if (image_writeByte(aas->image,aas->pc,lp->addr>>8)) return -1;
		}else{//undefined but used in current level
			if (ll_addRef(lp,image_getBank(aas->image),refType,aas->pc)) return -1;
		}
		break;
	case RT_both://full label
		if (ll_isDefined(lp)){//defined in current level
			if (image_writeByte(aas->image,aas->pc,lp->addr&0xFF)) return -1;
			if (image_writeByte(aas->image,aas->pc+1,lp->addr>>8)) return -1;
		}else{//undefined but used in current level
			if (ll_addRef(lp,image_getBank(aas->image),refType,aas->pc)) return -1;
		}
		break;
	default:
		vlstr_addf(errmsg,szErr_Assert,"aas_addLabel",1);
		return -1;
	}
	aas->pc+=aas->aafd->codelen;
	return 0;
}

//return 0 for success
int aas_save(struct aaStatus *aas){
	int length,i;
	const char *ext;
	HVLSTR filePath;
	FILE *fp;

	if (!aas){
		vlstr_addf(errmsg,szErr_Assert,"aas_save",0);
		return -1;
	}
	if (!aas->image){
		vlstr_addf(errmsg,szErr_aaStatus_NoImage);
		return -1;
	}
	length=image_getLength(aas->image);
	if (length<0){
		vlstr_addf(errmsg,szErr_Assert,"aas_save",1);
		return -1;
	}
	ext=getImageTypeString(aas->image->imageType);
	if (!ext){
		vlstr_addf(errmsg,szErr_Assert,"aas_save",2);
		return -1;
	}
	i=vlstr_indexOf(aas->filePath,'.');
	if (i<0){
		i=vlstr_length(aas->filePath);
	}
	filePath=vlstr_subString(aas->filePath,0,i);
	vlstr_addc(filePath,'.');
	vlstr_adds(filePath,ext);
	ps_push(aas->hps,filePath,L"wb");
	fp=ps_getFp(aas->hps);
	if (!fp){
		vlstr_addf(errmsg,szErr_OpenFile,vlstr_getData(filePath));
		ps_pop(aas->hps);
		return -1;
	}
	fwrite(image_getData(aas->image),1,length,fp);
	ps_pop(aas->hps);
	return 0;
}

//return 0 for success
int aas_compile(struct aaStatus *aas,HVLSTR filePath){
	struct labelList *lp;
	struct _ins *ins;
	char *data;
	FILE *fp;
	int intbuf,temp;

	if (!aas){
		vlstr_addf(errmsg,szErr_Assert,"aas_compile",0);
		return -1;
	}
	if (!filePath){
		vlstr_addf(errmsg,szErr_Assert,"aas_compile",1);
		return -1;
	}
	aas->filePath=filePath;
	if (ps_aaFile_push(aas->hps,filePath)) return -1;
	aas->aafd=ps_getData(aas->hps);
	if (!aas->aafd){
		vlstr_addf(errmsg,szErr_Assert,"aas_compile",2);
		return -1;
	}
	for (;;){
		aafd_getCmd(aas->aafd);
		if (aas->aafd->haslen&&aas->aafd->codelen<0){
			vlstr_addf(errmsg,szErr_aaStatus_CompileMinusLength,aas->aafd->codelen);
			return -1;
		}
		if (aas->aafd->cmdc<0){
			ps_pop(aas->hps);
			aas->aafd=ps_getData(aas->hps);
			if (!aas->aafd) break;
			continue;
		}
/*
printf("L%d",aas->aafd->linenum);
printf("C%d ",aas->aafd->cmdnum);
printf("%s\t",vlstr_getData(aas->aafd->cmddata));
printf("%d\t",aas->aafd->codelen);
printf("%c\t",aas->aafd->cmdc);
printf("%s\t",vlstr_getData(aas->aafd->cmds));
printf("%s\r\n",vlstr_getData(aas->aafd->extras));
*/
		switch (aas->aafd->cmdc){
		case '.':	//get address through error
			if (aas->ins){
				++aas->pc;
			}
			vlstr_addf(errmsg,szInfo_aaStatus_CompileGetAddress,aas->pc);
			return 1;
		case '@':	//specify address
			if (aas->ins){
				vlstr_addf(errmsg,szErr_aaStatus_CompileIllegalOprand);
				return -1;
			}
			if (aas->aafd->codelen){
				vlstr_addf(errmsg,szErr_aaStatus_CompileCodeLen,aas->aafd->codelen);
				return -1;
			}
			data=vlstr_getData(aas->aafd->cmds);
			if (!data){
				vlstr_addf(errmsg,szErr_Assert,"aas_compile",3);
				return -1;
			}
			intbuf=-1;
			sscanf(data,"%x",&intbuf);
			if (intbuf<0||intbuf>0xFFFF){
				vlstr_addf(errmsg,szErr_aaStatus_CompileSetAddr,intbuf);
				return -1;
			}
			aas->pc=intbuf;
			break;
		case '|':	//data boundary address
			if (aas->ins){
				vlstr_addf(errmsg,szErr_aaStatus_CompileIllegalOprand);
				return -1;
			}
			if (aas->aafd->codelen){
				vlstr_addf(errmsg,szErr_aaStatus_CompileCodeLen,aas->aafd->codelen);
				return -1;
			}
			data=vlstr_getData(aas->aafd->cmds);
			if (!data){
				vlstr_addf(errmsg,szErr_Assert,"aas_compile",4);
				return -1;
			}
			intbuf=-1;
			sscanf(data,"%x",&intbuf);
			if (intbuf<0||intbuf>0xFFFF){
				vlstr_addf(errmsg,szErr_aaStatus_CompileBoundary,intbuf);
				return -1;
			}
			if (aas->pc>intbuf){
				vlstr_addf(errmsg,szErr_aaStatus_CompileExceedBoundary,aas->pc);
				return -1;
			}
			break;
		case ':':	//define storage with label
			if (aas->ins){
				vlstr_addf(errmsg,szErr_aaStatus_CompileIllegalOprand);
				return -1;
			}
			if (vlstr_length(aas->aafd->cmds)<=0){
				vlstr_addf(errmsg,szErr_aaStatus_CompileVoidLabel);
				return -1;
			}
			if (aas_addLabel(aas,RT_dummy)) return -1;
			break;
		case '~':	//set address by defined label
			if (aas->ins){
				vlstr_addf(errmsg,szErr_aaStatus_CompileIllegalOprand);
				return -1;
			}
			if (aas->aafd->codelen){
				vlstr_addf(errmsg,szErr_aaStatus_CompileCodeLen,aas->aafd->codelen);
				return -1;
			}
			if (vlstr_length(aas->aafd->cmds)<=0){
				vlstr_addf(errmsg,szErr_aaStatus_CompileVoidLabel);
				return -1;
			}
			lp=ll_query(aas->ll,aas->aafd->cmds);
			if (!ll_isDefined(lp)){//undefined
				vlstr_addf(errmsg,szErr_aaStatus_CompileLabelUndefined,vlstr_getData(aas->aafd->cmds));
				return -1;
			}
			aas->pc=ll_addr(lp);
			break;
		case '!':	//const label relative
			if (!aas->aafd->haslen){
				aas->aafd->codelen=1;
			}
			if (aas->aafd->codelen!=1){
				vlstr_addf(errmsg,szErr_aaStatus_CompileCodeLen,aas->aafd->codelen);
				return -1;
			}
			if (vlstr_length(aas->aafd->cmds)<=0){
				vlstr_addf(errmsg,szErr_aaStatus_CompileVoidLabel);
				return -1;
			}
			if (aas_writeIns(aas)) return -1;
			if (aas_addLabel(aas,RT_rel)) return -1;
			break;
		case '%':	//const lable low
			if (!aas->aafd->haslen){
				aas->aafd->codelen=1;
			}
			if (aas->aafd->codelen!=1){
				vlstr_addf(errmsg,szErr_aaStatus_CompileCodeLen,aas->aafd->codelen);
				return -1;
			}
			if (vlstr_length(aas->aafd->cmds)<=0){
				vlstr_addf(errmsg,szErr_aaStatus_CompileVoidLabel);
				return -1;
			}
			if (aas_writeIns(aas)) return -1;
			if (aas_addLabel(aas,RT_low)) return -1;
			break;
		case '^':	//const label high
			if (!aas->aafd->haslen){
				aas->aafd->codelen=1;
			}
			if (aas->aafd->codelen!=1){
				vlstr_addf(errmsg,szErr_aaStatus_CompileCodeLen,aas->aafd->codelen);
				return -1;
			}
			if (vlstr_length(aas->aafd->cmds)<=0){
				vlstr_addf(errmsg,szErr_aaStatus_CompileVoidLabel);
				return -1;
			}
			if (aas_writeIns(aas)) return -1;
			if (aas_addLabel(aas,RT_high)) return -1;
			break;
		case '&':	//const label
			if (!aas->aafd->haslen){
				aas->aafd->codelen=2;
			}
			if (aas->aafd->codelen!=2){
				vlstr_addf(errmsg,szErr_aaStatus_CompileCodeLen,aas->aafd->codelen);
				return -1;
			}
			if (vlstr_length(aas->aafd->cmds)<=0){
				vlstr_addf(errmsg,szErr_aaStatus_CompileVoidLabel);
				return -1;
			}
			if (aas_writeIns(aas)) return -1;
			if (aas_addLabel(aas,RT_both)) return -1;
			break;
		case '#':	//const in dec
			if (!aas->aafd->haslen){
				aas->aafd->codelen=aas->defnumlen;
			}
			if (aas->aafd->codelen>4||aas->aafd->codelen<0){
				vlstr_addf(errmsg,szErr_aaStatus_CompileCodeLen,aas->aafd->codelen);
				return -1;
			}
			if (aas_writeIns(aas)) return -1;
			data=vlstr_getData(aas->aafd->cmds);
			if (!data){
				vlstr_addf(errmsg,szErr_Assert,"aas_compile",5);
				return -1;
			}
			intbuf=0;
			sscanf(data,"%d",&intbuf);
			for (temp=0;temp<aas->aafd->codelen;++temp){
				if (image_writeByte(aas->image,aas->pc+temp,intbuf&0xFF)) return -1;
				intbuf>>=8;
			}
			aas->pc+=aas->aafd->codelen;
			break;
		case '$':	//const in hex
			if (!aas->aafd->haslen){
				aas->aafd->codelen=aas->defnumlen;
			}
			if (aas->aafd->codelen>4||aas->aafd->codelen<0){
				vlstr_addf(errmsg,szErr_aaStatus_CompileCodeLen,aas->aafd->codelen);
				return -1;
			}
			if (aas_writeIns(aas)) return -1;
			data=vlstr_getData(aas->aafd->cmds);
			if (!data){
				vlstr_addf(errmsg,szErr_Assert,"aas_compile",6);
				return -1;
			}
			intbuf=0;
			sscanf(data,"%x",&intbuf);
			for (temp=0;temp<aas->aafd->codelen;++temp){
				if (image_writeByte(aas->image,aas->pc+temp,intbuf&0xFF)) return -1;
				intbuf>>=8;
			}
			aas->pc+=aas->aafd->codelen;
			break;
		case '\'':	//const char
			if (!aas->aafd->haslen){
				aas->aafd->codelen=1;
			}
			if (aas->aafd->codelen!=1){
				vlstr_addf(errmsg,szErr_aaStatus_CompileCodeLen,aas->aafd->codelen);
				return -1;
			}
			if (aas_writeIns(aas)) return -1;
			aafd_getChar(aas->aafd);
			data=vlstr_getData(aas->aafd->cmds);
			if (!data){
				vlstr_addf(errmsg,szErr_Assert,"aas_compile",7);
				return -1;
			}
			if (image_writeByte(aas->image,aas->pc,data[0])) return -1;
			aas->pc+=aas->aafd->codelen;
			break;
		case '\"':	//const str
			if (!aas->aafd->haslen){
				aas->aafd->codelen=vlstr_length(aas->aafd->cmds);
			}
			if (aas->aafd->codelen<0){
				vlstr_addf(errmsg,szErr_aaStatus_CompileCodeLen,aas->aafd->codelen);
				return -1;
			}
			data=vlstr_getData(aas->aafd->cmds);
			if (!data){
				vlstr_addf(errmsg,szErr_Assert,"aas_compile",8);
				return -1;
			}
			intbuf=vlstr_length(aas->aafd->cmds);
			for (temp=0;temp<aas->aafd->codelen&&temp<intbuf;++temp){
				if (image_writeByte(aas->image,aas->pc+temp,data[temp])) return -1;
			}
			for (;temp<aas->aafd->codelen;++temp){
				if (image_writeByte(aas->image,aas->pc+temp,0x00)) return -1;
			}
			aas->pc+=aas->aafd->codelen;
			break;
		case ';':	//comment
			if (aas->ins){
				vlstr_addf(errmsg,szErr_aaStatus_CompileIllegalOprand);
				return -1;
			}
			if (aas->aafd->codelen){
				vlstr_addf(errmsg,szErr_aaStatus_CompileCodeLen,aas->aafd->codelen);
				return -1;
			}
			aafd_nextLine(aas->aafd);
			break;
		case '{':	//block begin
			if (aas->ins){
				vlstr_addf(errmsg,szErr_aaStatus_CompileIllegalOprand);
				return -1;
			}
			if (aas->aafd->codelen){
				vlstr_addf(errmsg,szErr_aaStatus_CompileCodeLen,aas->aafd->codelen);
				return -1;
			}
			++aas->labellevel;
			break;
		case '}':	//block end
			if (aas->ins){
				vlstr_addf(errmsg,szErr_aaStatus_CompileIllegalOprand);
				return -1;
			}
			if (aas->aafd->codelen){
				vlstr_addf(errmsg,szErr_aaStatus_CompileCodeLen,aas->aafd->codelen);
				return -1;
			}
			if (!aas->labellevel){
				vlstr_addf(errmsg,szErr_aaStatus_CompileTooMuchEnd);
				return -1;
			}
			--aas->labellevel;
			if (ll_resolve(aas->ll,aas->labellevel)) return -1;
			break;
		case '\\':	//macro
			if (aas->ins){
				vlstr_addf(errmsg,szErr_aaStatus_CompileIllegalOprand);
				return -1;
			}
			if (aas->aafd->codelen){
				vlstr_addf(errmsg,szErr_aaStatus_CompileCodeLen,aas->aafd->codelen);
				return -1;
			}
			data=vlstr_getData(aas->aafd->cmds);
			if (!data){
				vlstr_addf(errmsg,szErr_Assert,"aas_compile",9);
				return -1;
			}
			intbuf=getMacroEnum(data);
			if (intbuf<0){
				vlstr_addf(errmsg,szErr_aaStatus_CompileUnknownMacro,data);
				return -1;
			}
			switch (intbuf){
			case M_image:
				if (aas->image){
					vlstr_addf(errmsg,szErr_MacroRedefine,macros[intbuf]);
					return -1;
				}
				aafd_getString(aas->aafd);
				data=vlstr_getData(aas->aafd->extras);
				intbuf=getImageTypeEnum(data);
				if (intbuf<0){
					vlstr_addf(errmsg,szErr_aaStatus_CompileUnknownImage,data);
					return -1;
				}
				aas->image=image_new(intbuf);
				break;
			case M_defnumlen:
				aafd_getString(aas->aafd);
				data=vlstr_getData(aas->aafd->extras);
				intbuf=1;
				sscanf(data,"%d",&intbuf);
				aas->defnumlen=intbuf;
				break;
			case M_bank:
				if (!aas->image){
					vlstr_addf(errmsg,szErr_aaStatus_NoImage);
					return -1;
				}
				aafd_getString(aas->aafd);
				if (image_newBank(aas->image,aas->aafd->extras)) return -1;
				break;
			case M_include:
				aafd_getString(aas->aafd);
				if (ps_aaFile_push(aas->hps,aas->aafd->extras)) return -1;
				aas->aafd=ps_getData(aas->hps);
				if (!aas->aafd){
					vlstr_addf(errmsg,szErr_Assert,"aas_compile",10);
					return -1;
				}
				break;
			case M_import:
				if (!aas->image){
					vlstr_addf(errmsg,szErr_aaStatus_NoImage);
					return -1;
				}
				aafd_getString(aas->aafd);
				ps_push(aas->hps,vlstr_clone(aas->aafd->extras),L"rb");
				fp=ps_getFp(aas->hps);
				if (!fp){
					vlstr_addf(errmsg,szErr_OpenFile,vlstr_getData(filePath));
					ps_pop(aas->hps);
					return -1;
				}
				if (image_importFile(aas->image,aas->pc,fp)){
					ps_pop(aas->hps);
					return -1;
				}
				ps_pop(aas->hps);
				break;
			case M_nsf_trimend:
				if (!aas->image){
					vlstr_addf(errmsg,szErr_aaStatus_NoImage);
					return -1;
				}
				aafd_getString(aas->aafd);
				if (image_setTrimEnd(aas->image,aas->aafd->extras)) return -1;
				break;
			case M_nsf_songs:
				if (!aas->image){
					vlstr_addf(errmsg,szErr_aaStatus_NoImage);
					return -1;
				}
				aafd_getString(aas->aafd);
				if (image_setNsfSongs(aas->image,aas->aafd->extras)) return -1;
				break;
			case M_nsf_entry:
				if (!aas->image){
					vlstr_addf(errmsg,szErr_aaStatus_NoImage);
					return -1;
				}
				aafd_getString(aas->aafd);
				if (image_setNsfEntry(aas->image,aas->aafd->extras)) return -1;
				break;
			case M_nsf_name:
				if (!aas->image){
					vlstr_addf(errmsg,szErr_aaStatus_NoImage);
					return -1;
				}
				aafd_getLine(aas->aafd);
				if (image_setNsfName(aas->image,aas->aafd->extras)) return -1;
				break;
			case M_nsf_artist:
				if (!aas->image){
					vlstr_addf(errmsg,szErr_aaStatus_NoImage);
					return -1;
				}
				aafd_getLine(aas->aafd);
				if (image_setNsfArtist(aas->image,aas->aafd->extras)) return -1;
				break;
			case M_nsf_copyright:
				if (!aas->image){
					vlstr_addf(errmsg,szErr_aaStatus_NoImage);
					return -1;
				}
				aafd_getLine(aas->aafd);
				if (image_setNsfCopyright(aas->image,aas->aafd->extras)) return -1;
				break;
			case M_nsf_div2A03:
				if (!aas->image){
					vlstr_addf(errmsg,szErr_aaStatus_NoImage);
					return -1;
				}
				aafd_getString(aas->aafd);
				if (image_setNsfDiv2A03(aas->image,aas->aafd->extras)) return -1;
				break;
			case M_nsf_div2A07:
				if (!aas->image){
					vlstr_addf(errmsg,szErr_aaStatus_NoImage);
					return -1;
				}
				aafd_getString(aas->aafd);
				if (image_setNsfDiv2A07(aas->image,aas->aafd->extras)) return -1;
				break;
			case M_nsf_apu:
				if (!aas->image){
					vlstr_addf(errmsg,szErr_aaStatus_NoImage);
					return -1;
				}
				aafd_getString(aas->aafd);
				if (image_setNsfApu(aas->image,aas->aafd->extras)) return -1;
				break;
			case M_nsf_expand:
				if (!aas->image){
					vlstr_addf(errmsg,szErr_aaStatus_NoImage);
					return -1;
				}
				aafd_getString(aas->aafd);
				if (image_setNsfExpand(aas->image,aas->aafd->extras)) return -1;
				break;
			case M_nsf_bankswitch:
				if (!aas->image){
					vlstr_addf(errmsg,szErr_aaStatus_NoImage);
					return -1;
				}
				aafd_getLine(aas->aafd);
				if (image_setNsfBankSwitch(aas->image,aas->aafd->extras)) return -1;
				break;
			case M_nsf_init:
				if (!aas->image){
					vlstr_addf(errmsg,szErr_aaStatus_NoImage);
					return -1;
				}
				if (image_setNsfVinit(aas->image,aas->pc)) return -1;
				break;
			case M_nsf_play:
				if (!aas->image){
					vlstr_addf(errmsg,szErr_aaStatus_NoImage);
					return -1;
				}
				if (image_setNsfVplay(aas->image,aas->pc)) return -1;
				break;
			case M_nes_mapper:
				if (!aas->image){
					vlstr_addf(errmsg,szErr_aaStatus_NoImage);
					return -1;
				}
				aafd_getString(aas->aafd);
				if (image_setNesMapper(aas->image,aas->aafd->extras)) return -1;
				break;
			case M_nes_mirror:
				if (!aas->image){
					vlstr_addf(errmsg,szErr_aaStatus_NoImage);
					return -1;
				}
				aafd_getString(aas->aafd);
				if (image_setNesMirror(aas->image,aas->aafd->extras)) return -1;
				break;
			case M_nes_battery:
				if (!aas->image){
					vlstr_addf(errmsg,szErr_aaStatus_NoImage);
					return -1;
				}
				aafd_getString(aas->aafd);
				if (image_setNesBattery(aas->image,aas->aafd->extras)) return -1;
				break;
			case M_nes_bankoffset:
				if (!aas->image){
					vlstr_addf(errmsg,szErr_aaStatus_NoImage);
					return -1;
				}
				aafd_getString(aas->aafd);
				if (image_setNesBankOffset(aas->image,aas->aafd->extras)) return -1;
				break;
			case M_fds_newside:
				if (!aas->image){
					vlstr_addf(errmsg,szErr_aaStatus_NoImage);
					return -1;
				}
				if (image_newFdsSide(aas->image)) return -1;
				break;
			case M_fds_boottoid:
				if (!aas->image){
					vlstr_addf(errmsg,szErr_aaStatus_NoImage);
					return -1;
				}
				aafd_getString(aas->aafd);
				if (image_setFdsBootToId(aas->image,aas->aafd->extras)) return -1;
				break;
			case M_fds_bankid:
				if (!aas->image){
					vlstr_addf(errmsg,szErr_aaStatus_NoImage);
					return -1;
				}
				aafd_getString(aas->aafd);
				if (image_setFdsBankId(aas->image,aas->aafd->extras)) return -1;
				break;
			default:
				vlstr_addf(errmsg,szErr_Assert,"aas_compile",11);
				return -1;
			}
			break;
		default:	//instruction
			if (aas->ins){
				vlstr_addf(errmsg,szErr_aaStatus_CompileIllegalOprand);
				return -1;
			}
			if (!aas->aafd->haslen){
				aas->aafd->codelen=1;
			}
			if (aas->aafd->codelen!=1){
				vlstr_addf(errmsg,szErr_aaStatus_CompileCodeLen,aas->aafd->codelen);
				return -1;
			}
			data=vlstr_getData(aas->aafd->extras);
			ins=ins_getIns(data);
			if (!ins){
				vlstr_addf(errmsg,szErr_aaStatus_CompileUnknownIns,data);
				return -1;
			}
			if (ins->oplen){
				aas->ins=ins;
				continue;
			}
			if (image_writeByte(aas->image,aas->pc,ins->code)) return -1;
			aas->pc+=aas->aafd->codelen;
			break;
		}
	}
	if (aas->labellevel){
		vlstr_addf(errmsg,szErr_aaStatus_CompileTooMuchBegin);
		return -1;
	}
	if (ll_finalize(aas->ll)) return -1;
	return aas_save(aas);
}
