struct labelList{
	struct labelList *prev;
	struct labelList *next;
	struct labelList *ancestor;
	struct labelList *parent;
	struct refStack *rs;
	HVLSTR name;
	int level;
	uint16_t addr;
};

struct labelList *ll_new(){
	struct labelList *root;

	root=(struct labelList *)malloc(sizeof(struct labelList));
	root->prev=root;
	root->next=root;
	root->ancestor=NULL;
	root->parent=NULL;
	root->rs=NULL;
	root->name=NULL;
	root->level=-1;
	root->addr=0;
	return root;
}

void ll_delete(struct labelList *root){
	struct labelList *p,*q;

	if (!root) return;
	for (p=root->next;p!=root;){
		q=p->next;
		rs_delete(p->rs);
		vlstr_delete(p->name);
		free(p);
		p=q;
	}
	free(root);
}

struct labelList *ll_query(struct labelList *root,HVLSTR name){
	struct labelList *ll;

	for (ll=root->next;ll!=root;ll=ll->next){
		if (!vlstr_compare(ll->name,name)){
			return ll;
		}
	}
	return NULL;
}

int ll_isDefined(struct labelList *ll){
	if (!ll) return 0;
	return rs_isEmpty(ll->rs);
}

uint16_t ll_addr(struct labelList *ll){
	if (!ll) return 0;
	return ll->addr;
}

//return 0 for success
int ll_addRef(struct labelList *ll,struct _bank *bank,int refType,uint16_t addr){
	if (!ll){
		vlstr_addf(errmsg,szErr_Assert,"ll_addRef",0);
		return -1;
	}
	if (!bank){
		vlstr_addf(errmsg,szErr_labelList_AddRefNoBank);
		return -1;
	}
	return rs_push(ll->rs,bank,refType,addr);
}

//return 0 for success
int ll_push(struct labelList *root,struct labelList *ancestor,struct _bank *bank,HVLSTR name,uint16_t addr,int level,int refType){
	struct labelList *ll;

	if (!root){
		vlstr_addf(errmsg,szErr_Assert,"ll_push",0);
		return -1;
	}
	if (!name){
		vlstr_addf(errmsg,szErr_Assert,"ll_push",1);
		return -1;
	}
	ll=(struct labelList *)malloc(sizeof(struct labelList));
	ll->prev=root;
	ll->next=root->next;
	ll->ancestor=ancestor;
	if (ancestor&&ancestor->level==level-1){
		ll->parent=ancestor;
	}else{
		ll->parent=NULL;
	}
	ll->rs=rs_new(bank);
	ll->name=vlstr_clone(name);
	ll->level=level;
	ll->addr=addr;
	root->next->prev=ll;
	root->next=ll;
	if (refType!=RT_dummy) return ll_addRef(ll,bank,refType,addr);
	return 0;
}

//return 0 for success
int ll_defineAddr(struct labelList *ll,uint16_t addr){
	if (!ll){
		vlstr_addf(errmsg,szErr_Assert,"ll_defineAddr",0);
		return -1;
	}
	ll->addr=addr;
	return rs_resolve(ll->rs,addr);
}

void ll_pop(struct labelList *root){
	struct labelList *ll;

	if (!root) return;
	ll=root->next;
	if (ll==root) return;
	root->next=ll->next;
	ll->next->prev=root;
	rs_delete(ll->rs);
	vlstr_delete(ll->name);
	free(ll);
}

//return 0 for success
int ll_resolve(struct labelList *root,int level){
	struct labelList *ll;

	if (!root){
		vlstr_addf(errmsg,szErr_Assert,"ll_resolve",0);
		return -1;
	}
	for (ll=root->next;ll!=root&&ll->level>level;){
		if (!rs_isEmpty(ll->rs)){//is reference
			if (!ll->parent){//no parent
				ll->level=level;
				if (ll->ancestor&&ll->ancestor->level==level-1){
					ll->parent=ll->ancestor;
				}
				ll=ll->next;
				continue;
			}else{//has parent
				if (!rs_isEmpty(ll->parent->rs)){//parent is reference
					rs_moveAll(ll->parent->rs,ll->rs);
				}else{//parent is defination
					if (rs_resolve(ll->rs,ll->parent->addr)) return -1;
				}
			}
		}
		ll=ll->next;
		ll_pop(ll->prev->prev);
	}
	return 0;
}

//return 0 for success
int ll_finalize(struct labelList *root){
	struct labelList *ll;

	if (!root){
		vlstr_addf(errmsg,szErr_Assert,"ll_finalize",0);
		return -1;
	}
	for (ll=root->next;ll!=root;ll=ll->next){
		if (!rs_isEmpty(ll->rs)){//is reference
			vlstr_addf(errmsg,szErr_labelList_FinalizeLabelUndefined,vlstr_getData(ll->name));
			return -1;
		}
	}
	return 0;
}
