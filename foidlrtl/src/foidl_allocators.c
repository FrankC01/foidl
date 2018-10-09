/*
	foidl_xallators.c
	Library support for structure allocations
	
	Copyright Frank V. Castellucci
	All Rights Reserved
*/

#define ALLOC_IMPL

#include	<foidlrt.h>
#include 	<stdio.h>

static PFRTAny gc_initialized = (PFRTAny) &_false.fclass;

const PFRTAny emtndarray[0];// = {end,end}; 

void * foidl_xall(uint32_t sz) {
	return calloc(sz,1);
}

void foidl_xdel(void *v) {
	return free(v);
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
	PFRTAny 	s = (PFRTAny) foidl_xall(sizeof (struct FRTType));
	s->fclass = fclass;
	s->ftype  = ftype;
	s->value  = value;
	s->count  = 1;
	return s;
}

PFRTAny allocStringWithBufferSize (uint32_t cnt) {	
	PFRTAny 	s = (PFRTAny) foidl_xall(sizeof (struct FRTType));
	char 		*newp = foidl_xall(cnt+1);
	s->fclass = scalar_class;
	s->ftype  = string_type;
	s->value  = (void *)newp;
	s->count  = cnt;	
	return s;	
}

PFRTAny allocGlobalStringCopy(char *v) {
	PFRTTypeG 	c0 = (PFRTTypeG) malloc(sizeof (struct FRTTypeG));
	c0->fsig = global_signature;
	PFRTAny 	c = (PFRTAny) &c0->fclass;
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
	PFRTTypeG 	c0 = (PFRTTypeG) malloc(sizeof (struct FRTTypeG));
	c0->fsig = global_signature;
	PFRTAny 	c = (PFRTAny) &c0->fclass;
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
	PFRTTypeG 	c0 = (PFRTTypeG) malloc(sizeof (struct FRTTypeG));
	c0->fsig = global_signature;
	PFRTAny 	c = (PFRTAny) &c0->fclass;
	c->fclass = scalar_class;
	c->ftype  = string_type;
	c->count  = strlen(v);
	c->value  = (void *) v;
	return c;
}

PFRTAny allocStringWithCopyCnt(uint32_t cnt, char *p) {
	char 	*newp = foidl_xall(cnt+1);
	strncpy(newp,p,cnt);
	PFRTAny 	s = (PFRTAny) foidl_xall(sizeof (struct FRTType));
	s->fclass = scalar_class;
	s->ftype  = string_type;
	s->value  = (void *)newp;
	s->count  = cnt;	
	return s;	
}

PFRTAny allocStringWithCopy(char *p) {
	uint32_t plen = strlen(p);
	char 	*newp = foidl_xall(plen+1);
	strcpy(newp,p);
	PFRTAny 	s = (PFRTAny) foidl_xall(sizeof (struct FRTType));
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
	PFRTAny 	s = (PFRTAny) foidl_xall(sizeof (struct FRTType));
	s->fclass = scalar_class;
	s->ftype  = string_type;
	s->value  = (void *)newp;
	s->count  = tlen;	
	return s;	
}

PFRTAny allocGlobalCharType(int v) {
	PFRTTypeG 	c0 = (PFRTTypeG) malloc(sizeof (struct FRTTypeG));
	c0->fsig = global_signature;
	PFRTAny 	c = (PFRTAny) &c0->fclass;
	c->fclass = scalar_class;
	c->ftype  = character_type;
	c->count  = 1;
	c->value  = (void *) (ft) v;
	return c;
}

PFRTAny allocGlobalIntegerType(long long v) {
	PFRTTypeG 	c0 = (PFRTTypeG) malloc(sizeof (struct FRTTypeG));
	c0->fsig = global_signature;
	PFRTAny 	c = (PFRTAny) &c0->fclass;
	c->fclass = scalar_class;
	c->ftype  = integer_type;
	c->count  = 0;
	c->value  = (void *) (ft) v;
	return c;
}

//	IO Channels

PFRTIOChannel 	allocIOChannel(ft ftype,PFRTAny name) {
	PFRTIOChannel ioc = foidl_xall(sizeof (struct FRTIOChannel));
	ioc->fclass = io_class;
	ioc->ftype  = ftype;
	ioc->name   = name;
	return ioc;
}

PFRTIOBuffer 	allocIONoBuffer() {
	PFRTIOBuffer b = foidl_xall(sizeof (struct FRTIOBuffer));
	b->fclass = io_class;
	b->ftype  = no_buffer;
	b->buffersize =
    b->previous_read_offset =
    b->current_read_offset =
    b->previous_write_offset =
    b->current_write_offset =
    b->max_read_position = 
    b->current_line = 
    b->current_pos = 0;
	return b;
}

PFRTIOBuffer 	allocIOMMapBuffer(void *mmap,ft sz) {
	PFRTIOBuffer b = allocIONoBuffer();
	b->current_line = 1;
	b->ftype  = mem_map_buffer;
	b->buffersize = b->max_read_position = sz;
	b->bufferPtr = (char *) mmap;
	return b;
}

PFRTIOBuffer 	allocIOBlockBuffer(ft sz) {
	PFRTIOBuffer b = allocIONoBuffer();
	b->ftype = mem_block_buffer;
	b->current_line = 1;
	b->buffersize = b->max_read_position = sz;
	b->bufferPtr = (char *) foidl_xall(sz);
	return b;
}


//	Function reference instance

PFRTFuncRef2 allocFuncRef2(void *fn, ft maxarg, invoke_funcptr ifn) {
	PFRTFuncRef2 fr = foidl_xall(sizeof (struct FRTFuncRef2));
	fr->fclass = function_class;
	fr->ftype  = funcinst_type;
	fr->mcount = maxarg;
	fr->fnptr  = fn;
	fr->args   = foidl_vector_inst_bang();
	fr->invokefnptr = ifn;
	return fr;
}

//	Collection related

PFRTLinkNode   allocLinkNode() {
	PFRTLinkNode l = (PFRTLinkNode) foidl_xall(sizeof(struct FRTLinkNode));
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
	PFRTList 	l = (PFRTList) foidl_xall(sizeof(struct FRTList));
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
	PFRTVector a = (PFRTVector) foidl_xall(sizeof(struct FRTVector));
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
	PFRTSet a = (PFRTSet) foidl_xall(sizeof(struct FRTSet));
	a->fclass = collection_class;
	a->ftype  = set2_type;
	a->count  = cnt;
	a->root   = root;	
	a->shift  = shift;
	a->hash   = 0;
	return a;
}

PFRTMap allocMap(ft cnt, ft shift, PFRTBitmapNode root) {
	PFRTMap a = (PFRTMap) foidl_xall(sizeof(struct FRTMap));
	a->fclass = collection_class;
	a->ftype  = map2_type;
	a->count  = cnt;
	a->root   = root;	
	a->shift  = shift;
	a->hash   = 0;
	return a;
}

PFRTMapEntry allocMapEntryWith(PFRTAny key, PFRTAny value) {
	PFRTMapEntry me = (PFRTMapEntry) foidl_xall(sizeof(struct FRTMapEntry));
	me->fclass = collection_class;
	me->ftype  = mapentry_type;
	me->key    = key;
	me->value  = value;
	return me;
}

PFRTSeries 	allocSeries() {
	PFRTSeries s = (PFRTSeries) foidl_xall(sizeof(struct FRTSeries));
	s->fclass = collection_class;
	s->ftype  = series_type;
	return s;
}

//	Generic array allocator

PFRTAny 	*allocRawAnyArray(ft cnt) {
	PFRTAny *res = (PFRTAny *) foidl_xall(cnt * sizeof(PFRTAny));
	for(ft i = 0; i < cnt;i++) res[i] = end;
	return res;
}

PFRTBitmapNode allocNode() {
	PFRTBitmapNode a = (PFRTBitmapNode) 
		foidl_xall(sizeof(struct FRTBitmapNode));
	a->fclass = bitmapnode_class;
	a->datamap = 0;
	a->nodemap = 0;
	a->slots = (PFRTAny *) emtndarray;	
	return a;
}

PFRTHamtNode allocHamtNode() {
	PFRTHamtNode a = (PFRTHamtNode) foidl_xall(sizeof(struct FRTHamtNode));
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

PFRTIterator allocTrieIterator(PFRTAssocType base,itrNext next) {
	uint32_t node_arity = nodeArity(base->root);
	uint32_t payload_arity = payloadArity(base->root);
	PFRTTrie_Iterator i = foidl_xall(sizeof(struct FRTTrie_Iterator));
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
		foidl_xall(sizeof(struct FRTVector_Iterator));
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
		foidl_xall(sizeof(struct FRTList_Iterator));
	li->fclass = iterator_class;
	li->ftype  = vector_iterator_type;
	li->next   = next;
	li->list   = l;
	li->node   = l->root;
	return (PFRTIterator) li;
}

PFRTIterator allocSeriesIterator(PFRTSeries s, itrNext next) {
	PFRTSeries_Iterator li = (PFRTSeries_Iterator)
		foidl_xall(sizeof(struct FRTSeries_Iterator));
	li->fclass = iterator_class;
	li->ftype  = series_iterator_type;
	li->next   = next;
	li->counter = -1;
	li->initialValue = nil;
	li->lastValue = nil;
	li->series = s;	
	return (PFRTIterator) li;
}

PFRTIterator allocChannelIterator(PFRTIOBuffer cb, itrNext next) {
	PFRTChannel_Iterator ci = (PFRTChannel_Iterator)
		foidl_xall(sizeof(struct FRTChannel_Iterator));
	ci->fclass = iterator_class;
	ci->ftype  = channel_iterator_type;
	ci->next   = next;
	ci->buffer = cb;
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
	foidl_xdel(fref);
	if(pv != empty_vector) {
		if(pv->tail != empty_node)
			foidl_xdel(pv->tail);
		if(pv->root != empty_node)
			foidl_xdel(pv->root);
		foidl_xdel(pv);
	}
}

void deallocChannelBuffer(PFRTIOChannel chan) {
	PFRTIOBuffer b = chan->bufferptr;
	switch(b->ftype) {
		case 	mem_map_buffer:
			foidl_deallocate_mmap(b->bufferPtr,b->buffersize);
			break;
		case 	mem_block_buffer:
			foidl_xdel(b->bufferPtr);
			break;
	}
	foidl_xdel(b);
	chan->bufferptr = (PFRTIOBuffer) nil;
}