#include "stringReader.h"

struct stringReader *sr_new(char *s){
	struct stringReader *sr;

	if (!s) return NULL;
	sr=(struct stringReader *)malloc(sizeof(struct stringReader));
	sr->pos=0;
	sr->length=strlen(s);
	sr->data=(char *)malloc(sr->length+1);
	strcpy(sr->data,s);
	sr->buf=(char *)malloc(sr->length+1);
	sr->buf[0]=0;
	return sr;
}

void sr_delete(struct stringReader *sr){
	if (!sr) return;
	free(sr->data);
	free(sr->buf);
	free(sr);
}

void sr_rewind(struct stringReader *sr){
	if (!sr) return;
	sr->pos=0;
}

int sr_tell(struct stringReader *sr){
	if (!sr) return -1;
	return sr->pos;
}

void sr_seek(struct stringReader *sr,int pos){
	if (!sr) return;
	if (pos<0){
		sr->pos=0;
		return;
	}
	sr->pos=pos;
}

int sr_getc(struct stringReader *sr){
	if (!sr) return -1;
	if (sr->pos>=sr->length) return -1;
	return sr->data[sr->pos++];
}

char *sr_gets(struct stringReader *sr){
	int i;
	char *s;

	if (!sr) return NULL;
	if (sr->pos>=sr->length) return NULL;
	for (i=sr->pos;i<sr->length;){
		sr->buf[i]=sr->data[i];
		if (sr->buf[i++]=='\n') break;
	}
	sr->buf[i]=0;
	s=&sr->buf[sr->pos];
	sr->pos=i;
	return s;
}

char *sr_getline(struct stringReader *sr){
	int i;
	char *s;

	if (!sr) return NULL;
	s=sr_gets(sr);
	for (i=strlen(s);i>=0&&s[i]<' ';--i){
		s[i]=0;
	}
	return s;
}

char *sr_peek_s(struct stringReader *sr){
	int r;

	if (!sr) return NULL;
	r=sscanf(&sr->data[sr->pos],"%s",sr->buf);
	if (r<=0) return NULL;
	return sr->buf;
}

char *sr_scan_s(struct stringReader *sr){
	int n,r;

	if (!sr) return NULL;
	r=sscanf(&sr->data[sr->pos],"%s%n",sr->buf,&n);
	if (r<=0) return NULL;
	sr->pos+=n;
	return sr->buf;
}

int sr_scan_c(struct stringReader *sr,char *c){
	int n,r;

	if (!sr) return -1;
	if (c){
		r=sscanf(&sr->data[sr->pos],"%c%n",c,&n);
	}else{
		r=sscanf(&sr->data[sr->pos],"%*c%n",&n);
	}
	if (r>0){
		sr->pos+=n;
	}
	return r;
}

int sr_scan_d(struct stringReader *sr,int *d){
	int n,r;

	if (!sr) return -1;
	if (d){
		r=sscanf(&sr->data[sr->pos],"%d%n",d,&n);
	}else{
		r=sscanf(&sr->data[sr->pos],"%*d%n",&n);
	}
	if (r>0){
		sr->pos+=n;
	}
	return r;
}

int sr_scan_lld(struct stringReader *sr,long long *lld){
	int n,r;

	if (!sr) return -1;
	if (lld){
		r=sscanf(&sr->data[sr->pos],"%I64d%n",lld,&n);
	}else{
		r=sscanf(&sr->data[sr->pos],"%*d%n",&n);
	}
	if (r>0){
		sr->pos+=n;
	}
	return r;
}

int sr_scan_x(struct stringReader *sr,int *x){
	int n,r;

	if (!sr) return -1;
	if (x){
		r=sscanf(&sr->data[sr->pos],"%x%n",x,&n);
	}else{
		r=sscanf(&sr->data[sr->pos],"%*x%n",&n);
	}
	if (r>0){
		sr->pos+=n;
	}
	return r;
}

int sr_scan_llx(struct stringReader *sr,long long *llx){
	int n,r;

	if (!sr) return -1;
	if (llx){
		r=sscanf(&sr->data[sr->pos],"%I64x%n",llx,&n);
	}else{
		r=sscanf(&sr->data[sr->pos],"%*x%n",&n);
	}
	if (r>0){
		sr->pos+=n;
	}
	return r;
}

int sr_scan_f(struct stringReader *sr,float *f){
	int n,r;

	if (!sr) return -1;
	if (f){
		r=sscanf(&sr->data[sr->pos],"%f%n",f,&n);
	}else{
		r=sscanf(&sr->data[sr->pos],"%*f%n",&n);
	}
	if (r>0){
		sr->pos+=n;
	}
	return r;
}

int sr_scan_lf(struct stringReader *sr,double *lf){
	int n,r;

	if (!sr) return -1;
	if (lf){
		r=sscanf(&sr->data[sr->pos],"%lf%n",lf,&n);
	}else{
		r=sscanf(&sr->data[sr->pos],"%*f%n",&n);
	}
	if (r>0){
		sr->pos+=n;
	}
	return r;
}
