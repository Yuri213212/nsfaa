#include "pathStack.h"

struct pathStack *ps_new(){
	struct pathStack *ps;
	HVLSTR str;
	int i;

	ps=(struct pathStack *)malloc(sizeof(struct pathStack));
	ps->prev=ps;
	ps->next=ps;
	str=vlstr_GetModuleFileName(NULL);
	i=vlstr_lastIndexOf(str,'\\');
	if (i<0){
		vlstr_delete(str);
		free(ps);
		return NULL;
	}
	ps->filePath=vlstr_subString(str,0,i);
	vlstr_delete(str);
	ps->dir=vlstr_GetCurrentDirectory();
	ps->relPath=NULL;
	ps->fp=NULL;
	ps->data=NULL;
	ps->_deleteData=NULL;
	return ps;
}

void ps_clear(struct pathStack *root){
	struct pathStack *p,*q;

	if (!root) return;
	for (p=root->next;p!=root;){
		q=p->next;
		vlstr_delete(p->filePath);
		vlstr_delete(p->dir);
		vlstr_delete(p->relPath);
		if (p->fp){
			fclose(p->fp);
		}
		if (p->_deleteData){
			p->_deleteData(p->data);
		}
		free(p);
		p=q;
	}
	root->prev=root;
	root->next=root;
}

void ps_delete(struct pathStack *root){
	if (!root) return;
	ps_clear(root);
	vlstr_delete(root->filePath);
	vlstr_delete(root->dir);
	free(root);
}

void ps_convertPath(HVLSTR filePath,struct pathStack *root){
	HVLSTR str;
	char *s;
	int i;

	if (!root) return;
	if (!filePath) return;
	s=vlstr_getData(filePath);
	if (s[0]==PATH_PDMARK){
		SetCurrentDirectoryW(vlstr_getDataw(root->filePath));
		s[0]='.';
	}else{
		SetCurrentDirectoryW(vlstr_getDataw(root->next->dir));
	}
	vlstr_GetFullPathName(filePath);
	i=vlstr_length(root->dir);
	str=vlstr_subString(filePath,0,i);
	if (!vlstr_compare(str,root->dir)){
		vlstr_delete(str);
		str=vlstr_subString(filePath,i+1,-1);
		vlstr_clear(filePath);
		vlstr_addc(filePath,PATH_PDMARK);
		vlstr_addc(filePath,'/');
		vlstr_addstr(filePath,str);
		vlstr_delete(str);
		vlstr_replaceChar(filePath,'\\','/');
		return;
	}
	vlstr_delete(str);
	i=vlstr_length(root->next->dir);
	str=vlstr_subString(filePath,0,i);
	if (!vlstr_compare(str,root->next->dir)){
		vlstr_delete(str);
		str=vlstr_subString(filePath,i+1,-1);
		vlstr_clear(filePath);
		vlstr_adds(filePath,"./");
		vlstr_addstr(filePath,str);
		vlstr_delete(str);
		vlstr_replaceChar(filePath,'\\','/');
		return;
	}
	vlstr_delete(str);
}

int ps_push(struct pathStack *root,HVLSTR filePath,wchar_t *mode){
	struct pathStack *ps;
	HVLSTR str;
	char *s;
	int i;

	if (!root) return -1;
	if (!filePath) return -1;
	if (!mode) return -1;
	str=vlstr_clone(filePath);
	ps_convertPath(str,root);
	s=vlstr_getData(filePath);
	if (s[0]==PATH_PDMARK){
		SetCurrentDirectoryW(vlstr_getDataw(root->filePath));
		s[0]='.';
	}else{
		SetCurrentDirectoryW(vlstr_getDataw(root->next->dir));
	}
	vlstr_GetFullPathName(filePath);
	i=vlstr_lastIndexOf(filePath,'\\');
	if (i<0){
		vlstr_delete(filePath);
		vlstr_delete(str);
		return -1;
	}
	ps=(struct pathStack *)malloc(sizeof(struct pathStack));
	ps->prev=root;
	ps->next=root->next;
	root->next->prev=ps;
	root->next=ps;
	ps->filePath=filePath;
	ps->dir=vlstr_subString(filePath,0,i);
	ps->relPath=str;
	ps->fp=_wfopen(vlstr_getDataw(filePath),mode);
	ps->data=NULL;
	ps->_deleteData=NULL;
	for (ps=ps->next;ps!=root;ps=ps->next){
		if (!vlstr_compare(filePath,ps->filePath)) return 1;
	}
	return 0;
}

void ps_pop(struct pathStack *root){
	struct pathStack *ps;

	if (!root) return;
	ps=root->next;
	if (ps==root) return;
	root->next=ps->next;
	ps->next->prev=root;
	vlstr_delete(ps->filePath);
	vlstr_delete(ps->dir);
	vlstr_delete(ps->relPath);
	if (ps->fp){
		fclose(ps->fp);
	}
	if (ps->_deleteData){
		ps->_deleteData(ps->data);
	}
	free(ps);
}

FILE *ps_getFp(struct pathStack *root){
	if (!root) return NULL;
	return root->next->fp;
}

void ps_attatchData(struct pathStack *root,void *data,_DELETE _deleteData){
	if (!root){
		if (_deleteData){
			_deleteData(data);
		}
		return;
	}
	if (root->next->_deleteData){
		root->next->_deleteData(root->next->data);
	}
	root->next->data=data;
	root->next->_deleteData=_deleteData;
}

void *ps_getData(struct pathStack *root){
	if (!root) return 0;
	return root->next->data;
}

int ps_checkDataDestructor(struct pathStack *root,_DELETE _deleteData){
	if (!root) return 0;
	return root->next->_deleteData==_deleteData;
}

void ps_cd(struct pathStack *root){
	if (!root) return;
	SetCurrentDirectoryW(vlstr_getDataw(root->next->dir));
}

HVLSTR ps_getFullPath(struct pathStack *root){
	if (!root) return NULL;
	if (root->next==root) return NULL;
	return root->next->filePath;
}

HVLSTR ps_getRelPath(struct pathStack *root){
	if (!root) return NULL;
	if (root->next==root) return NULL;
	return root->next->relPath;
}

HVLSTR ps_copyFullPath(struct pathStack *root){
	if (!root) return NULL;
	if (root->next==root) return NULL;
	return vlstr_clone(root->next->filePath);
}

HVLSTR ps_copyRelPath(struct pathStack *root){
	if (!root) return NULL;
	if (root->next==root) return NULL;
	return vlstr_clone(root->next->relPath);
}
