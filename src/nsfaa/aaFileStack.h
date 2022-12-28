struct aaFileData{
	FILE *fp;
	int linenum;
	int cmdnum;
	int haslen;
	int codelen;
	int cmdc;
	HVLSTR cmds;
	HVLSTR extras;
	HVLSTR cmddata;
	HVLSTR rowdata;
	HSR cmdReader;
	HSR rowReader;
};

struct aaFileData *aafd_new(FILE *fp){
	struct aaFileData *aafd;

	if (!fp) return NULL;
	aafd=(struct aaFileData *)malloc(sizeof(struct aaFileData));
	aafd->fp=fp;
	aafd->linenum=0;
	aafd->cmdnum=0;
	aafd->haslen=0;
	aafd->codelen=0;
	aafd->cmdc=-1;
	aafd->cmds=NULL;
	aafd->extras=NULL;
	aafd->cmddata=NULL;
	aafd->rowdata=NULL;
	aafd->cmdReader=NULL;
	aafd->rowReader=NULL;
	return aafd;
}

void aafd_delete(struct aaFileData *aafd){
	if (!aafd) return;
	vlstr_delete(aafd->cmds);
	vlstr_delete(aafd->extras);
	vlstr_delete(aafd->cmddata);
	vlstr_delete(aafd->rowdata);
	sr_delete(aafd->cmdReader);
	sr_delete(aafd->rowReader);
	free(aafd);
}

void aafd_readLine(struct aaFileData *aafd){
	if (!aafd) return;
	++aafd->linenum;
	vlstr_delete(aafd->rowdata);
	aafd->rowdata=vlstr_readLine(aafd->fp);
	if (!aafd->rowdata) return;
	vlstr_addc(aafd->rowdata,' ');
	sr_delete(aafd->rowReader);
	aafd->rowReader=sr_new(vlstr_getData(aafd->rowdata));
	aafd->cmdnum=0;
}

void aafd_getCmd(struct aaFileData *aafd){
	if (!aafd) return;
	while (!aafd->rowdata||!sr_peek_s(aafd->rowReader)){
		aafd_readLine(aafd);
		if (!aafd->rowdata){
			aafd->cmdc=-1;
			return;
		}
	}
	++aafd->cmdnum;
	vlstr_delete(aafd->cmddata);
	aafd->cmddata=vlstr_newEx(sr_scan_s(aafd->rowReader));
	sr_delete(aafd->cmdReader);
	aafd->cmdReader=sr_new(vlstr_getData(aafd->cmddata));
	if (sr_scan_d(aafd->cmdReader,&aafd->codelen)>0){
		aafd->haslen=1;
	}else{
		aafd->haslen=0;
		aafd->codelen=0;
	}
	aafd->cmdc=sr_getc(aafd->cmdReader);
	if (aafd->cmdc<0){
		aafd->cmdc=' ';
		return;
	}
	vlstr_delete(aafd->cmds);
	aafd->cmds=vlstr_newEx(sr_scan_s(aafd->cmdReader));
	vlstr_delete(aafd->extras);
	aafd->extras=vlstr_new();
	vlstr_addc(aafd->extras,aafd->cmdc);
	vlstr_addstr(aafd->extras,aafd->cmds);
}

void aafd_getChar(struct aaFileData *aafd){
	if (!aafd) return;
	if (aafd->cmds) return;
	aafd->cmds=vlstr_new();
	vlstr_addc(aafd->cmds,sr_getc(aafd->rowReader));
}

void aafd_getString(struct aaFileData *aafd){
	if (!aafd) return;
	vlstr_delete(aafd->extras);
	while (!aafd->rowdata||!sr_peek_s(aafd->rowReader)){
		aafd_readLine(aafd);
		if (!aafd->rowdata){
			aafd->extras=NULL;
			return;
		}
	}
	aafd->extras=vlstr_newEx(sr_scan_s(aafd->rowReader));
}

void aafd_nextLine(struct aaFileData *aafd){
	if (!aafd) return;
	vlstr_delete(aafd->rowdata);
	aafd->rowdata=NULL;
}

void aafd_getLine(struct aaFileData *aafd){
	if (!aafd) return;
	aafd_readLine(aafd);
	vlstr_delete(aafd->extras);
	if (!aafd->rowdata){
		aafd->extras=NULL;
		return;
	}
	aafd->extras=vlstr_clone(aafd->rowdata);
	vlstr_drop(aafd->rowdata);
	aafd_nextLine(aafd);
}

//return 0 for success
int ps_aaFile_push(HPS hps,HVLSTR filePath){
	struct aaFileData *aafd;
	FILE *fp;

	if (!hps){
		vlstr_addf(errmsg,szErr_Assert,"ps_aaFile_push",0);
		return -1;
	}
	if (!filePath){
		vlstr_addf(errmsg,szErr_Assert,"ps_aaFile_push",1);
		return -1;
	}
	if (ps_push(hps,vlstr_clone(filePath),L"rb")>0){
		vlstr_addf(errmsg,szErr_aaFileStack_PushDupicateFile,vlstr_getData(ps_getFullPath(hps)));
		ps_pop(hps);
		return -1;
	}
	fp=ps_getFp(hps);
	if (!fp){
		vlstr_addf(errmsg,szErr_OpenFile,vlstr_getData(filePath));
		ps_pop(hps);
		return -1;
	}
	aafd=aafd_new(fp);
	ps_attatchData(hps,aafd,(_DELETE)aafd_delete);
	utf8_checkBOM(fp);
	return 0;
}

void ps_aaFile_clear(HPS hps){
	struct aaFileData *aafd;

	while (ps_checkDataDestructor(hps,(_DELETE)aafd_delete)){
		aafd=ps_getData(hps);
		vlstr_addf(errmsg,szErr_aaFileStack_ClearAt,vlstr_getData(ps_getFullPath(hps)),aafd->linenum,aafd->cmdnum,vlstr_getData(aafd->cmddata));
		ps_pop(hps);
	}
}
