/*
;	foidl_allocators.c
;	Library support for structure allocations
;
; Copyright (c) Frank V. Castellucci
; All Rights Reserved
;
; Licensed under the Apache License, Version 2.0 (the "License");
; you may not use this file except in compliance with the License.
; You may obtain a copy of the License at
;
;     http://www.apache.org/licenses/LICENSE-2.0
;
; Unless required by applicable law or agreed to in writing, software
; distributed under the License is distributed on an "AS IS" BASIS,
; WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; See the License for the specific language governing permissions and
; limitations under the License.
*/

#define ALLOC_IMPL

#include	<foidlrt.h>
#include 	<stdio.h>

static PFRTAny gc_initialized = (PFRTAny) &_false.fclass;

const PFRTAny emtndarray[0];// = {end,end};

static ft unknown_allocs = 0;
static ft general_allocs = 0;
static ft global_allocs = 0;

static ft unknown_free = 0;
static ft general_free = 0;
static ft global_free = 0;

void foidl_memstats() {
	printf("--------------- Allc ----------------------\n");
	printf("Unknown allocations %llu\n", unknown_allocs);
	printf("General allocations %llu\n", general_allocs);
	printf("Global allocations %llu\n", global_allocs);
	printf("--------------- Free ----------------------\n");
	printf("Unknown frees %llu\n", unknown_free);
	printf("General frees %llu\n", general_free);
	printf("Global frees %llu\n", global_free);
	printf("-------------------------------------------\n");
}

void * foidl_xall(uint32_t sz) {
	PFRTTypeG res = calloc(sz+sizeof(ft),1);
	res->fsig = unkwn_signature;
	++unknown_allocs;
	return (void *)&res->fclass;
//	return calloc(sz,1);
}

void * foidl_alloc(ft sz) {
	void *res = foidl_xall(sz);
	PFRTTypeG g = res - sizeof(ft);
	g->fsig = alloc_signature;
	--unknown_allocs;
	++general_allocs;
	return res;
}

void * foidl_galloc(ft sz) {
	void *res = foidl_xall(sz);
	PFRTTypeG g = res - sizeof(ft);
	g->fsig = global_signature;
	--unknown_allocs;
	++global_allocs;
	return res;
}

void * foidl_xreall(void *p, uint32_t newsz) {
	PFRTTypeG g = p - sizeof(ft);
	if(g->fsig == alloc_signature || g->fsig == unkwn_signature) {
		PFRTTypeG newg = realloc((void *) g, newsz);
		return (void *)&newg->fclass;
	}
	else {
		unknown_handler();
		return p;
		//return realloc(p, newsz);
	}
}

void foidl_xdel(void *v) {
	PFRTTypeG g = v - sizeof(ft);
	if(g->fsig == alloc_signature) {
		++general_free;
		free(g);
	}
	else if(g->fsig == global_signature){
		++global_free;
	}
	else if(g->fsig == unkwn_signature) {
		++unknown_free;
		free(g);
	}
	else {
		unknown_handler();
	}
}

void foidl_delete(void *v) {
	foidl_xdel(v);
}

void foidl_gc_init() {
	if(gc_initialized == false) {
		gc_initialized = true;
	}

}

void foidl_release(void *v) {

}

//	Object Types

PFRTAny    allocAny(ft fclass, ft ftype, void *value) {
	PFRTAny 	s = (PFRTAny) foidl_alloc(sizeof (struct FRTType));
	s->fclass = fclass;
	s->ftype  = ftype;
	s->value  = value;
	s->count  = 1;
	return s;
}


PFRTAny allocStringWithBufferSize (uint32_t cnt) {
	PFRTAny 	s = (PFRTAny) foidl_alloc(sizeof (struct FRTType));
	char 		*newp = foidl_xall(cnt+1);
	s->fclass = scalar_class;
	s->ftype  = string_type;
	s->value  = (void *)newp;
	s->count  = cnt;
	return s;
}

PFRTAny allocGlobalStringCopy(char *v) {
	PFRTAny 	c = (PFRTAny) foidl_galloc(sizeof (struct FRTType));
	uint32_t plen = strlen(v);
	char 	*newp = foidl_xall(plen+1);
	strcpy(newp,v);
	c->fclass = scalar_class;
	c->ftype  = string_type;
	c->count  = plen;
	c->value  = (void *) newp;
	return c;
}

PFRTAny allocGlobalKeywordCopy(char *v) {
	PFRTAny 	c = (PFRTAny) foidl_galloc(sizeof (struct FRTType));
	uint32_t plen = strlen(v);
	char 	*newp = foidl_xall(plen+1);
	strcpy(newp,v);
	c->fclass = scalar_class;
	c->ftype  = keyword_type;
	c->count  = plen;
	c->value  = (void *) newp;
	return c;
}

PFRTAny allocGlobalString(char *v) {
	PFRTAny 	c = (PFRTAny) foidl_galloc(sizeof (struct FRTType));
	c->fclass = scalar_class;
	c->ftype  = string_type;
	c->count  = strlen(v);
	c->value  = (void *) v;
	return c;
}

PFRTAny allocStringWithCopyCnt(uint32_t cnt, char *p) {
	char 	*newp = foidl_xall(cnt+1);
	strncpy(newp,p,cnt);
	PFRTAny 	s = (PFRTAny) foidl_alloc(sizeof (struct FRTType));
	s->fclass = scalar_class;
	s->ftype  = string_type;
	s->value  = (void *)newp;
	s->count  = cnt;
	return s;
}

PFRTAny allocStringWithCptr(char *p, long int cnt) {
	PFRTAny 	s = (PFRTAny) foidl_alloc(sizeof (struct FRTType));
	s->fclass = scalar_class;
	s->ftype  = string_type;
	s->value  = (void *)p;
	s->count  = (ft) cnt;
	return s;
}

PFRTAny allocStringWithCopy(char *p) {
	uint32_t plen = strlen(p);
	char 	*newp = foidl_xall(plen+1);
	strcpy(newp,p);
	PFRTAny 	s = (PFRTAny) foidl_alloc(sizeof (struct FRTType));
	s->fclass = scalar_class;
	s->ftype  = string_type;
	s->value  = (void *)newp;
	s->count  = plen;
	return s;
}

PFRTAny allocAndConcatString(uint32_t tlen,
		char *base, uint32_t bcnt, char *p1, uint32_t p1cnt) {
	char 	*newp = foidl_xall(tlen+1);
	strcpy(newp,base);
	strncpy(&newp[bcnt],p1,p1cnt);
	PFRTAny 	s = (PFRTAny) foidl_alloc(sizeof (struct FRTType));
	s->fclass = scalar_class;
	s->ftype  = string_type;
	s->value  = (void *)newp;
	s->count  = tlen;
	return s;
}

PFRTAny allocRegex(PFRTAny sbase, void* regex) {
	PFRTRegEx 	s = (PFRTRegEx) foidl_alloc(sizeof (struct FRTRegEx));
	s->fclass = scalar_class;
	s->ftype  = regex_type;
	s->value  = sbase;
	s->count  = sbase->count;
	s->hash   = sbase->hash;
	s->regex  = regex;
	return (PFRTAny) s;

}

PFRTAny allocGlobalCharType(int v) {
	PFRTAny 	c = (PFRTAny) foidl_galloc(sizeof (struct FRTType));
	c->fclass = scalar_class;
	c->ftype  = character_type;
	c->count  = 1;
	c->value  = (void *) (ft) v;
	return c;
}

// New IO Channel

PFRTIOChannel allocFileChannel(PFRTAny name, PFRTAny mode, PFRTAny args) {
	PFRTIOFileChannel fc = foidl_alloc(sizeof(struct FRTIOFileChannel));
	fc->fclass = io_class;
	fc->ftype  = file_type;
	fc->ctype  = chan_file;
	fc->name   = name;
	fc->mode   = mode;
	fc->settings = args;
	return (PFRTIOChannel) fc;
}

PFRTResponse allocResponse(ft ftype, PFRTAny resp) {
	PFRTResponse fc = foidl_alloc(sizeof(struct FRTResponse));
	fc->fclass = response_class;
	fc->ftype  = ftype;
	fc->count  = 1;
	fc->value  = (void *)resp;
	return fc;
}

//	Function and thread/worker reference instance

PFRTFuncRef2 allocFuncRef2(void *fn, ft maxarg, invoke_funcptr ifn) {
	PFRTFuncRef2 fr = foidl_alloc(sizeof (struct FRTFuncRef2));
	fr->fclass = function_class;
	fr->ftype  = funcinst_type;
	fr->mcount = maxarg;
	fr->fnptr  = fn;
	fr->args   = foidl_vector_inst_bang();
	fr->invokefnptr = ifn;
	return fr;
}

PFRTWorker  allocWorker(PFRTFuncRef2 ref) {
	PFRTWorker wrk = foidl_alloc(sizeof(struct FRTWorker));
	wrk->fclass = worker_class;
	wrk->ftype = worker_type;
	wrk->count = 0;
	wrk->fnptr = ref;
	wrk->work_state = wrk_alloc;
	return wrk;
}

EXTERNC PFRTThread      allocThread(PFRTThreadPool poolref, int id) {
	PFRTThread pthrd = foidl_alloc(sizeof(struct FRTThread));
	pthrd->fclass = worker_class;
	pthrd->ftype = thread_type;
	pthrd->count = 0;
	pthrd->pool_parent = (void *)poolref;
	pthrd->thid = id;
	return pthrd;
}

PFRTList   allocList(ft , PFRTLinkNode);

PFRTThreadPool allocThreadPool() {
	PFRTThreadPool tp = foidl_alloc(sizeof(struct FRTThreadPool));
	tp->fclass = worker_class;
	tp->ftype = thrdpool_type;
	tp->count = 0;
	tp->run_value = 0;
	tp->fnptr = NULL;
	tp->thread_list = (PFRTAny) allocList(0,empty_link);
	tp->work_list = (PFRTAny) allocList(0,empty_link);
	tp->pause_work = false;
	tp->block_queue = false;
	tp->stop_work = false;
	return  tp;
}

//	Collection related

PFRTLinkNode   allocLinkNode() {
	PFRTLinkNode l = (PFRTLinkNode) foidl_alloc(sizeof(struct FRTLinkNode));
	l->fclass = collection_class;
	l->ftype  = linknode_type;
	l->data   = end;
	l->next   = empty_link;
	return l;
}

PFRTLinkNode   allocLinkNodeWith(PFRTAny data, PFRTLinkNode nextNode) {
	PFRTLinkNode l = allocLinkNode();
	l->fclass = collection_class;
	l->ftype  = linknode_type;
	l->data   = data;
	l->next   = nextNode;
	return l;
}

PFRTList   allocList(ft cnt, PFRTLinkNode root) {
	PFRTList 	l = (PFRTList) foidl_alloc(sizeof(struct FRTList));
	l->fclass = collection_class;
	l->ftype  = list2_type;
	l->count  = cnt;
	l->hash   = 0;
	l->root   = root;
	l->rest   = empty_link;
	return l;
}

PFRTVector allocVector(ft cnt, ft shift, PFRTHamtNode root,
	PFRTHamtNode tail) {
	PFRTVector a = (PFRTVector) foidl_alloc(sizeof(struct FRTVector));
	a->fclass = collection_class;
	a->ftype  = vector2_type;
	a->count  = cnt;
	a->hash   = 0;
	a->root   = root;
	a->tail   = tail;
	a->shift  = shift;
	a->hash   = (ft) 0;
	return a;
}

PFRTSet allocSet(ft cnt, ft shift, PFRTBitmapNode root) {
	PFRTSet a = (PFRTSet) foidl_alloc(sizeof(struct FRTSet));
	a->fclass = collection_class;
	a->ftype  = set2_type;
	a->count  = cnt;
	a->root   = root;
	a->shift  = shift;
	a->hash   = 0;
	return a;
}

PFRTMap allocMap(ft cnt, ft shift, PFRTBitmapNode root) {
	PFRTMap a = (PFRTMap) foidl_alloc(sizeof(struct FRTMap));
	a->fclass = collection_class;
	a->ftype  = map2_type;
	a->count  = cnt;
	a->root   = root;
	a->shift  = shift;
	a->hash   = 0;
	return a;
}

PFRTMapEntry allocMapEntryWith(PFRTAny key, PFRTAny value) {
	PFRTMapEntry me = (PFRTMapEntry) foidl_alloc(sizeof(struct FRTMapEntry));
	me->fclass = collection_class;
	me->ftype  = mapentry_type;
	me->key    = key;
	me->value  = value;
	return me;
}

PFRTSeries 	allocSeries() {
	PFRTSeries s = (PFRTSeries) foidl_alloc(sizeof(struct FRTSeries));
	s->fclass = collection_class;
	s->ftype  = series_type;
	return s;
}

//	Generic array allocator

PFRTAny 	*allocRawAnyArray(ft cnt) {
	PFRTAny *res = (PFRTAny *) foidl_alloc(cnt * sizeof(PFRTAny));
	for(ft i = 0; i < cnt;i++) res[i] = end;
	return res;
}

PFRTBitmapNode allocNode() {
	PFRTBitmapNode a = (PFRTBitmapNode)
		foidl_alloc(sizeof(struct FRTBitmapNode));
	a->fclass = bitmapnode_class;
	a->datamap = 0;
	a->nodemap = 0;
	a->slots = (PFRTAny *) emtndarray;
	return a;
}

PFRTHamtNode allocHamtNode() {
	PFRTHamtNode a = (PFRTHamtNode) foidl_alloc(sizeof(struct FRTHamtNode));
	a->fclass = hamptnode_class;
	for(ft i=0; i < WCNT; i++) a->slots[i] = end;
	return a;
}

PFRTBitmapNode allocNodeWith(uint32_t datamap,
	uint32_t nodemap, ft slen) {
	PFRTBitmapNode res = allocNode();
	res->datamap = datamap;
	res->nodemap = nodemap;
	res->slots   = (PFRTAny*) allocRawAnyArray(slen);
	return res;
}

PFRTBitmapNode allocNodeClone(PFRTBitmapNode src, ft slen) {
	PFRTBitmapNode res = allocNode();
	res->datamap = src->datamap;
	res->nodemap = src->nodemap;
	res->slots   = allocRawAnyArray(slen);
	for(ft i = 0; i < slen;i++) res->slots[i] = src->slots[i];
	return res;
}

PFRTBitmapNode allocNodeWithAll(uint32_t datamap,
	uint32_t nodemap,PFRTAny *slots) {
	PFRTBitmapNode res = allocNode();
	res->datamap = datamap;
	res->nodemap = nodemap;
	res->slots   = slots;
	return res;
}

//	Iterators

PFRTIterator allocStringIterator(PFRTAny str, itrNext next) {
	PFRTString_Iterator i = foidl_alloc(sizeof(struct FRTString_Iterator));
	i->fclass = iterator_class;
	i->ftype  = string_iterator_type;
    i->next =  next;
    i->str = (char *) str->value;   //  Base string
    i->slen = str->count;
    i->index = -1;      			//  Last fetched element
	return (PFRTIterator) i;
}

PFRTIterator allocTrieIterator(PFRTAssocType base,itrNext next) {
	uint32_t node_arity = nodeArity(base->root);
	uint32_t payload_arity = payloadArity(base->root);
	PFRTTrie_Iterator i = foidl_alloc(sizeof(struct FRTTrie_Iterator));
	i->fclass = iterator_class;
	i->ftype  = base->ftype == map2_type ?
		map_iterator_type : set_iterator_type;
	i->next   = next;
	i->get 	  = base->ftype == map2_type ? mapGetDefault : setGetDefault;
	i->currentStackLevel = -1;
	if(node_arity != 0) {
		i->currentStackLevel = 0;
		i->nodes[0] = base->root;
		i->nodeCursorAndLength[0] = 0;
		i->nodeCursorAndLength[1] = node_arity;
	}
	if(payload_arity != 0) {
		i->currentValueNode = base->root;
		i->currentValueCursor = 0;
		i->currentValueLength = payload_arity;
	}
	return (PFRTIterator) i;
}

PFRTIterator allocVectorIterator(PFRTVector v,itrNext next) {
	PFRTVector_Iterator	vi = (PFRTVector_Iterator)
		foidl_alloc(sizeof(struct FRTVector_Iterator));
	PFRTHamtNode vn = (PFRTHamtNode)vectorGetDefault((PFRTAny)  v,0);
	vi->fclass = iterator_class;
	vi->ftype  = vector_iterator_type;
	vi->next   = next;
	vi->get    = vectorGetDefault;
	vi->vector = v;
	vi->index  = 0;
	vi->base   = vi->index - (vi->index % 32);
	vi->node   = (vi->index < v->count) ? vn : (PFRTHamtNode) end;
	return (PFRTIterator) vi;
}

PFRTIterator allocListIterator(PFRTList l, itrNext next) {
	PFRTList_Iterator li = (PFRTList_Iterator)
		foidl_alloc(sizeof(struct FRTList_Iterator));
	li->fclass = iterator_class;
	li->ftype  = vector_iterator_type;
	li->next   = next;
	li->list   = l;
	li->node   = l->root;
	return (PFRTIterator) li;
}

PFRTIterator allocSeriesIterator(PFRTSeries s, itrNext next) {
	PFRTSeries_Iterator li = (PFRTSeries_Iterator)
		foidl_alloc(sizeof(struct FRTSeries_Iterator));
	li->fclass = iterator_class;
	li->ftype  = series_iterator_type;
	li->next   = next;
	li->counter = -1;
	li->initialValue = nil;
	li->lastValue = nil;
	li->series = s;
	return (PFRTIterator) li;
}


PFRTIterator allocChannelIterator(PFRTIOChannel cb, itrNext next) {
	PFRTChannel_Iterator ci = (PFRTChannel_Iterator)
		foidl_alloc(sizeof(struct FRTChannel_Iterator));
	ci->fclass = iterator_class;
	ci->ftype  = channel_iterator_type;
	ci->next   = next;
	ci->channel = cb;
	ci->currRef = 0;
	return (PFRTIterator) ci;
}


//
//	Deallocators
//

/*
	FuncRef2 args (vector) should only have to release
	level 0 root node and the tail node
*/

void deallocFuncRef2(PFRTFuncRef2 fref) {
	PFRTVector pv = (PFRTVector) fref->args;
	foidl_delete(fref);
	if(pv != empty_vector) {
		if(pv->tail != empty_node)
			foidl_delete(pv->tail);
		if(pv->root != empty_node)
			foidl_delete(pv->root);
		foidl_delete(pv);
	}
}
