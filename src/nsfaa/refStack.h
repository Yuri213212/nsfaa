enum refTypeEnum{
	RT_dummy=-1,	//use this for definition instead of reference
	RT_rel,
	RT_low,
	RT_high,
	RT_both,
};

struct refStack{
	struct refStack *prev;
	struct refStack *next;
	struct _bank *bank;
	int type;
	uint16_t offset;
	uint16_t addr;
};

struct refStack *rs_new(struct _bank *bank){
	struct refStack *root;

	if (!bank) return NULL;
	root=(struct refStack *)malloc(sizeof(struct refStack));
	root->prev=root;
	root->next=root;
	root->bank=bank;
	root->type=RT_dummy;
	root->offset=bank_offset(bank);
	root->addr=0;
	return root;
}

void rs_delete(struct refStack *root){
	struct refStack *p,*q;

	if (!root) return;
	for (p=root->next;p!=root;){
		q=p->next;
		free(p);
		p=q;
	}
	free(root);
}

//return 0 for success
int rs_push(struct refStack *root,struct _bank *bank,int type,uint16_t addr){
	struct refStack *rs;

	if (!root){
		vlstr_addf(errmsg,szErr_Assert,"rs_push",0);
		return -1;
	}
	if (!bank){
		vlstr_addf(errmsg,szErr_refStack_PushNoBank,addr);
		return -1;
	}
	rs=(struct refStack *)malloc(sizeof(struct refStack));
	rs->prev=root;
	rs->next=root->next;
	rs->bank=bank;
	rs->type=type;
	rs->offset=bank_offset(bank);
	rs->addr=addr;
	root->next->prev=rs;
	root->next=rs;
	return 0;
}

void rs_moveAll(struct refStack *root,struct refStack *root2){
	if (!root) return;
	if (!root2) return;
	if (root2->prev==root2) return;
	root2->prev->next=root->next;
	root->next=root2->next;
	root2->prev=root2;
	root2->next=root2;
}

struct refStack *rs_pop(struct refStack *root){
	struct refStack *rs;

	if (!root) return NULL;
	rs=root->next;
	if (rs==root) return NULL;
	root->next=rs->next;
	rs->next->prev=root;
	return rs;
}

int rs_isEmpty(struct refStack *root){
	if (!root) return 1;
	if (root->next==root) return 1;
	return 0;
}

//return 0 for success
int rs_resolve(struct refStack *root,uint16_t addr){
	struct refStack *rs;
	int data;

	if (!root){
		vlstr_addf(errmsg,szErr_Assert,"rs_resolve",0);
		return -1;
	}
	for (rs=rs_pop(root);rs;rs=rs_pop(root)){
		switch (rs->type){
		case RT_rel:
			data=addr-rs->addr-1;
			if (data<-0x80||data>0x7F){
				vlstr_addf(errmsg,szErr_RelOutOfRange,rs->addr,data);
				return -1;
			}
			if (bank_writeLabel(rs->bank,rs->offset,rs->addr,data)) return -1;
			break;
		case RT_low:
			data=addr&0xFF;
			if (bank_writeLabel(rs->bank,rs->offset,rs->addr,data)) return -1;
			break;
		case RT_high:
			data=addr>>8;
			if (bank_writeLabel(rs->bank,rs->offset,rs->addr,data)) return -1;
			break;
		case RT_both:
			data=addr&0xFF;
			if (bank_writeLabel(rs->bank,rs->offset,rs->addr,data)) return -1;
			data=addr>>8;
			if (bank_writeLabel(rs->bank,rs->offset,rs->addr+1,data)) return -1;
			break;
		default:
			vlstr_addf(errmsg,szErr_Assert,"rs_resolve",1);
			return -1;
		}
		free(rs);
	}
	return 0;
}
