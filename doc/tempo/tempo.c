#include <stdio.h>
#include <stdlib.h>

struct data{
	int a;
	int b;
	double q;
	struct data *prev;
	struct data *next;
};

struct data *data_new(){
	struct data *root;

	root=(struct data *)malloc(sizeof(struct data));
	root->prev=root;
	root->next=root;
	return root;
}

void data_delete(struct data *root){
	struct data *p,*q;

	for (p=root->next;p!=root;){
		q=p;
		p=p->next;
		free(q);
	}
	free(root);
}

void data_add(struct data *root,int a,int b){
	struct data *p;

	p=(struct data *)malloc(sizeof(struct data));
	p->a=a;
	p->b=b;
	p->q=450.0*a/b;
	p->prev=root->prev;
	p->next=root;
	root->prev->next=p;
	root->prev=p;
}

void data_xchg(struct data *p,struct data *q){
	struct data *pp,*pn,*qp,*qn;

	if (p==q) return;
	pp=p->prev;
	pn=p->next;
	qp=q->prev;
	qn=q->next;
	pp->next=q;
	pn->prev=q;
	qp->next=p;
	qn->prev=p;
	if (pp==q){
		p->next=q;
		q->prev=p;
	}else{
		p->next=qn;
		q->prev=pp;
	}
	if (pn==q){
		p->prev=q;
		q->next=p;
	}else{
		p->prev=qp;
		q->next=pn;
	}
}

void data_sort(struct data *root){
	struct data *p,*end,*max;

	for (end=root->prev;end!=root->next;end=end->prev){
		max=end;
		for (p=root->next;p!=end;p=p->next){
			if (p->q>max->q){
				max=p;
			}
		}
		data_xchg(max,end);
		end=max;
	}
}

int gcd(int big,int small){
	if (small==0) return big;
	return gcd(small,big%small);
}

int main(){
	struct data *root,*p;
	int a,b;
	FILE *fp;

	fp=fopen("tempo.txt","wb");
	if (!fp){
		printf("Open 'tempo.txt' failed\r\n");
		return 0;
	}
	root=data_new();
	data_add(root,1,1);
	for (b=2;b<256;++b){
		for (a=1;a<b;++a){
			if (gcd(b,a)!=1) continue;
			data_add(root,a,b);
		}
	}
	data_sort(root);
	for (p=root->next;p!=root;p=p->next){
		fprintf(fp,"%d\t%d\t%lf\r\n",p->a,p->b,p->q);
	}
	data_delete(root);
	return 0;
}
