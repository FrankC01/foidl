
/*
	foidl_vector.c
	Library support for HAMT vector

	Copyright Frank V. Castellucci
	All Rights Reserved
*/

#define  VECTOR_IMPL
#include <foidlrt.h>

// Externs (may move to headers)

struct FRTVectorG _empty_vector =
	{	global_signature,
		collection_class,
		vector2_type,
		0,
		0,
		(PFRTHamtNode) &_nil,
		(PFRTHamtNode) &_nil,
		SHIFT};

static struct FRTHamtNode _empty_node = {hamptnode_class};

PFRTVector 	const	empty_vector = (PFRTVector) &_empty_vector.fclass;
PFRTHamtNode const	empty_node = &_empty_node;

//
//	Vector utilities
//

static PFRTHamtNode cloneNode(PFRTHamtNode src) {
	PFRTHamtNode 	dest = allocHamtNode();
	for(ft i=0; i < 32; i++)
		dest->slots[i] = src->slots[i];
	return dest;
}

//	Tail offset calculation
static ft tailOffset(PFRTVector pv) {
	if ( pv->count < 32)
		return 0;
	else
		return ((( pv->count - 1 ) >> SHIFT) << SHIFT);
}

//	Retrieves the HAMT node array where index resolves to
static PFRTHamtNode nodeFor(PFRTVector v, ft index){
	PFRTHamtNode res;
	long long i = (long long) index;
	if(i >= 0 && i < v->count) {
		if(i >= tailOffset(v))
			res = v->tail;
		else {
			PFRTHamtNode node = v->root;
			for(ft level = v->shift; level > 0; level -= 5)
				node = (PFRTHamtNode) node->slots[(i >> level) & MASK];
			res = node;
			}
	}
	else {
		//	Should be run-time exception?
		res = empty_node;
	}
	return res;
}

PFRTAny vectorGetDefault(PFRTAny v, uint32_t index) {
	return (PFRTAny) nodeFor((PFRTVector) v,(ft) index);
}

//	Retrieves value at index i
PFRTAny 	vector_nth(PFRTVector v, ft i) {
	PFRTHamtNode p = nodeFor(v,i);
	if( p == empty_node) {
		return  nil;
	}
	return p->slots[i&MASK];
}


//	Utilitity functions

static PFRTHamtNode newPath(ft level, PFRTHamtNode node){
	if(level == 0)
		return node;
	PFRTHamtNode ret = allocHamtNode();
	ret->slots[0] = (PFRTAny) newPath(level - SHIFT, node);
	return ret;
}

static PFRTHamtNode pushTail(ft cnt, ft level, PFRTHamtNode parent, PFRTHamtNode tailnode){
	//if parent is leaf, insert node,
	// else does it map to an existing child? -> nodeToInsert = pushNode one more level
	// else alloc new path
	//return  nodeToInsert placed in copy of parent

	int subidx = ((cnt - 1) >> level) & MASK;
	PFRTHamtNode ret = cloneNode(parent);
	PFRTHamtNode nodeToInsert;
	if(level == 5)
		{
		nodeToInsert = tailnode;
		}
	else
		{
		PFRTAny child = parent->slots[subidx];
		nodeToInsert = ( child != end)?
		                pushTail(cnt, (level-5), (PFRTHamtNode) child, tailnode)
		                : newPath((level-5), tailnode);
		}
	ret->slots[subidx] = (PFRTAny) nodeToInsert;
	return ret;
}

//	Get persistent vector item at index

PFRTAny vector_get(PFRTVector src, PFRTAny index) {
	PFRTAny res = nil;
	if (index->ftype == number_type) {
		ft val = number_toft(index);
		if(src->count > val)
			res = vector_nth(src,val);
		else
			;
	}
	else {
		unknown_handler();
		// Out of bounds exception
	}

	return res;
}

PFRTAny vector_get_default(PFRTVector src,PFRTAny index, PFRTAny def) {
	PFRTAny res = nil;
	if (index->ftype == number_type) {
		ft val = number_toft(index);
		if(src->count > val)
			res = vector_nth(src,val);
		else {
			if(foidl_function_qmark(def) == true)
				res = dispatch2(def,(PFRTAny) src,index);
			else
				res = def;
		}
	}
	else {
		unknown_handler();
	}

	return res;
}

//
//	Vector public API
//

PFRTAny 	vector_first(PFRTAny v) {
	return vector_nth((PFRTVector) v,0);
}

PFRTAny 	vector_second(PFRTAny v) {
	return vector_nth((PFRTVector)v,1);
}


//	Append new element to vector
//	Note: This is the cons equiv

static PFRTAny vector_extend_i(PFRTVector src, PFRTAny value) {

	PFRTHamtNode 	newTail;
	ft 				tailcnt = src->count - tailOffset(src);

	//	Append to the tail if room
	if( tailcnt < 32 ) {
		newTail = cloneNode(src->tail);
		newTail->slots[tailcnt] = value;
		return (PFRTAny) allocVector(src->count + 1, src->shift, src->root, newTail);
	}

	// Tail full, push into tree and do path work
	PFRTHamtNode 	newRoot;
	PFRTHamtNode 	tailnode = src->tail;
	ft 				newshift = src->shift;

	//	Root overflow, make root child of new root,
	if (( src->count >> SHIFT) > (1 << src->shift) ) {
		newRoot = allocHamtNode();
		newRoot->slots[0] = (PFRTAny) src->root;
		newRoot->slots[1] = (PFRTAny) newPath(src->shift, tailnode);
		newshift += SHIFT;
	}
	//	Otherwise just push the tail
	else
		newRoot = pushTail(src->count,src->shift,src->root,tailnode);

	newTail = allocHamtNode();
	newTail->slots[0] = value;

	return (PFRTAny) allocVector(src->count + 1, newshift, newRoot, newTail);
}

PFRTAny vector_extend(PFRTVector src, PFRTAny value) {
	return vector_extend_i(src,value);
}

//	Update to a persistent vector for a specific index
//	Note: This is the assoc equiv

static PFRTHamtNode vector_doupdate(ft level, PFRTHamtNode node, ft index, PFRTAny item) {
	PFRTHamtNode 	ret = cloneNode(node);
	if( level == 0 ) {
		ret->slots[index & MASK] = item;
	}
	else {
		ft 	subidx = (index >> level) & MASK;
		ret->slots[subidx] = (PFRTAny)
			vector_doupdate(level - SHIFT,(PFRTHamtNode) ret->slots[subidx],index, item);
	}
	return ret;
}

PFRTAny vector_update(PFRTVector src, PFRTAny index, PFRTAny item) {
	if( index->fclass == scalar_class && index->ftype == integer_type) {
		ft 	i = (ft) index->value;
		ft  cnt = src->count;
		if((long long) i >= 0 && i < cnt) {
			if(i >= tailOffset(src)) {
				PFRTHamtNode newTail = cloneNode(src->tail);
				newTail->slots[i & MASK] = item;
				return (PFRTAny)
					allocVector(src->count, src->shift, src->root, newTail);
			}
			return (PFRTAny)
				allocVector(cnt,src->shift,
					vector_doupdate(src->shift, src->root, i, item), src->tail);
		}
		if(i == cnt)
			return vector_extend(src,item);

	}
	return nil;
}

//	Drop last element from vector

static PFRTHamtNode popTail(ft cnt,ft level, PFRTHamtNode node) {
	ft subidx = ((cnt-2) >> level) & MASK;
	PFRTHamtNode ret;

	if(level > SHIFT) {
		PFRTHamtNode newchild = popTail(cnt,level - 5,(PFRTHamtNode) node->slots[subidx]);
		if((PFRTAny) newchild == end && subidx == 0)
			ret = (PFRTHamtNode) &_nil;
		else {
			ret = cloneNode(node); // new Node(root.edit, node.array.clone());
			ret->slots[subidx] = (PFRTAny) newchild;
			}
		}
	else if(subidx == 0)
		ret = (PFRTHamtNode) &_nil;
	else {
		ret = cloneNode(node); // new Node(root.edit, node.array.clone());
		ret->slots[subidx] = end;
		}
	return ret;
}

PFRTAny vector_pop(PFRTVector src) {
	PFRTAny	ret = (PFRTAny) empty_vector;

	// If not 0 or 1 process, otherwise return empty node

	if( src->count > 1 ) {
		ft 	tailcnt = src->count - tailOffset(src);
		if ( tailcnt > 1) {
			PFRTHamtNode 	newTail = cloneNode(src->tail);
			newTail->slots[tailcnt - 1] = end;
			ret = (PFRTAny) allocVector(src->count - 1, src->shift,src->root,newTail);
		}
		else {
			PFRTHamtNode newTail = nodeFor(src, src->count - 2);
			PFRTHamtNode newRoot = popTail(src->count,src->shift,src->root);
			ft 	newshift = src->shift;
			if( (PFRTAny) newRoot == nil) {
				newRoot = allocHamtNode();
			}
			if(src->shift > SHIFT && newRoot->slots[1] == end) {
				newRoot =  (PFRTHamtNode) newRoot->slots[0];
				newshift = newshift - SHIFT;
			}
			ret = (PFRTAny) allocVector(src->count - 1, newshift,newRoot,newTail);
		}
	}
	return ret;
}

PFRTAny	vector_droplast(PFRTAny src) {
	return vector_pop((PFRTVector) src);
}

//	Get the last element of the vector

PFRTAny vector_peek(PFRTVector src) {
	if(src->count > 0)
		return vector_nth(src,src->count - 1);
	return nil;
}


//
//	Transient API
//

//	Transient vector tail transition to root tree

static PFRTHamtNode tailToTree(ft cnt, ft level, PFRTHamtNode parent, PFRTHamtNode tailnode){
	//if parent is leaf, insert node,
	// else does it map to an existing child? -> nodeToInsert = pushNode one more level
	// else alloc new path
	//return  nodeToInsert placed in copy of parent

	int subidx = ((cnt - 1) >> level) & MASK;
	PFRTHamtNode ret = parent;
	PFRTHamtNode nodeToInsert;
	if(level == 5)
		{
		nodeToInsert = tailnode;
		}
	else
		{
		PFRTAny child = parent->slots[subidx];
		nodeToInsert = ( child != end)?
		                tailToTree(cnt, (level-5), (PFRTHamtNode) child, tailnode)
		                : newPath((level-5), tailnode);
		}
	ret->slots[subidx] = (PFRTAny) nodeToInsert;
	return ret;
}

//	Transient vector population of value, used during creation
//	Count is the key to the HAMT tree and/or tail calculations

PFRTAny vector_extend_bang_i(PFRTAny v,PFRTAny e) {
	PFRTVector 	vi = (PFRTVector) v;
	ft 			cnt = vi->count;

	if ( (cnt - tailOffset(vi)) < 32 ) {
		vi->tail->slots[cnt & MASK] = e;
		++vi->count;
		return v;
	}

	PFRTHamtNode 	newRoot;
	PFRTHamtNode 	tailnode = vi->tail;
	ft 				newshift = vi->shift;

	vi->tail = allocHamtNode();
	vi->tail->slots[0] = e;

	if (( cnt >> 5) > (1 << vi->shift) ) {
		newRoot = allocHamtNode();
		newRoot->slots[0] = (PFRTAny) vi->root;
		newRoot->slots[1] = (PFRTAny) newPath(vi->shift, tailnode);
		newshift += SHIFT;
	}
	else {
		newRoot = tailToTree(vi->count,vi->shift, vi->root,tailnode);
	}

	vi->root  = newRoot;
	vi->shift = newshift;
	++vi->count;
	return v;
}

PFRTAny foidl_vector_extend_bang(PFRTAny v,PFRTAny e) {
	if((PFRTVector) v == empty_vector)
		return vector_extend_bang_i(
			(PFRTAny) allocVector(0,SHIFT,allocHamtNode(), allocHamtNode()),
			e);
	else
		return vector_extend_bang_i(v,e);
}

PFRTAny vector_update_bang(PFRTVector src, PFRTAny index, PFRTAny item) {
	ft 	i = (ft) index->value;
	nodeFor(src,i)->slots[i&MASK] = item;
	return (PFRTAny) src;
}

//	Mutative drop-last
// 	TODO: Is there a memory leak when droping HAMT Nodes

static PFRTHamtNode popTail_bang(ft cnt,ft level, PFRTHamtNode node) {
	ft subidx = ((cnt-2) >> level) & MASK;
	PFRTHamtNode ret;

	if(level > SHIFT) {
		PFRTHamtNode newchild = popTail_bang(cnt,level - 5,(PFRTHamtNode) node->slots[subidx]);
		if((PFRTAny) newchild == end && subidx == 0)
			ret = (PFRTHamtNode) &_nil;
		else {
			ret = node;
			ret->slots[subidx] = (PFRTAny) newchild;
			}
		}
	else if(subidx == 0)
		ret = (PFRTHamtNode) &_nil;
	else {
		ret = node;
		ret->slots[subidx] = end;
		}
	return ret;
}

PFRTAny vector_pop_bang(PFRTVector src) {

	//	Only one element is in the tail
	if (src->count == 1) {
		src->tail->slots[0] = end;
		--src->count;
		return (PFRTAny) src;
	}

	//	Possible tail location
	ft 	intail = ((src->count - 1) & MASK);
	if ( intail > 0) {
		src->tail->slots[intail] = end;
		--src->count;
		return (PFRTAny) src;
	}

	//	Heavier lifting

	PFRTHamtNode newTail  = nodeFor(src, src->count - 2);
	PFRTHamtNode newRoot  = popTail_bang(src->count,src->shift,src->root);
	ft 			 newShift = src->shift;

	if( newRoot == (PFRTHamtNode) &_nil ) {
		newRoot = allocHamtNode();
	}
	if(src->shift > SHIFT && newRoot->slots[1] == end) {
		newRoot =  (PFRTHamtNode) newRoot->slots[0];
		newShift = newShift - SHIFT;
	}
	src->root = newRoot;
	src->tail = newTail;
	src->shift = newShift;
	--src->count;
	return (PFRTAny) src;
}

PFRTAny	vector_dropLast_bang(PFRTAny src) {
		return vector_pop_bang((PFRTVector) src);
}

PFRTAny	vector_drop_bang(PFRTAny src, PFRTAny cnt) {
	PFRTVector pv = (PFRTVector) src;
	if(pv->count > (ft) cnt->value) {
		for(ft x = 0; x < (ft) cnt->value; ++x)
			vector_pop_bang(pv);
	}
	return src;
}

//	Creates a new instance (assume during building of declared datatypes)
PFRTAny foidl_vector_inst_bang() {
	return (PFRTAny) allocVector(0,SHIFT,allocHamtNode(), allocHamtNode());
}


PFRTAny vector_rest(PFRTAny src) {
	PFRTAny result = (PFRTAny) empty_vector;
	if(src->count > 1) {
		PFRTAny 		itm = nil;
		PFRTAny 		v1 = foidl_vector_inst_bang();
		PFRTIterator	vi = iteratorFor(src);
		result = v1;
		iteratorNext(vi);
		while((itm = iteratorNext(vi)) != end)
			vector_extend_bang_i((PFRTAny) v1,itm);
		foidl_xdel(vi);
		uint32_t s1hsh = hash(vector_first(src));
		v1->hash = src->hash - s1hsh;
		v1->count = src->count - 1;

	}
	return result;
}

//	Transmutes the command line into a foidl vector
PFRTAny vector_from_argv(int cnt, char** argv) {
	PFRTVector res = (PFRTVector) foidl_vector_inst_bang();
	for(int i = 0; i < cnt;)
		vector_extend_bang_i((PFRTAny) res,allocGlobalString(argv[i++]));
	return (PFRTAny) res;
}

//
// 	Iteration support
//

PFRTAny vector_print(PFRTIOChannel chn,PFRTAny v) {
	io_writeFuncPtr	cfn = chn->writehandler->fnptr;
	cfn(chn,meta);
	cfn(chn,lbracket);
	ft 			max = ((PFRTVector) v)->count;
	if(max > 0) {
		PFRTAny 		res = nil;
		PFRTIterator	vi = iteratorFor(v);
		ft 				i = 0;
		--max;
		while((res = iteratorNext(vi)) != end) {
			cfn(chn,res);
			if(max > i++) cfn(chn,comma);
		}
		foidl_xdel(vi);
	}
	cfn(chn,rbracket);
	return nil;
}

PFRTAny coerce_to_vector(PFRTAny mtemplate, PFRTAny src) {
	unknown_handler();
	return src;
}

//	Memory recovery

void release_vector(PFRTVector v) {
	unknown_handler();
}
