/*
	foidl_set.c
	Library support for CHAMP set

	Copyright Frank V. Castellucci
	All Rights Reserved
*/

#define  SET_IMPL
#include <foidlrt.h>
#include <stdio.h>

//	Global things

const struct FRTSetG _empty_set = {
	global_signature,
	collection_class,
	set2_type,
	0,0,
	(PFRTBitmapNode) &_empty_champ_node.fclass,SHIFT};

PFRTSet const empty_set = (PFRTSet) &_empty_set.fclass;

//	Utilities

static PFRTAny getKey(PFRTBitmapNode node, uint32_t index) {
	return node->slots[TUPLELENSFT *index];
}

uint32_t set_nodelength(PFRTBitmapNode node) {
	return (data_count(node) + node_count(node)) ;
}

PFRTBitmapNode set_getNode(PFRTBitmapNode node, uint32_t index) {
	return (PFRTBitmapNode) node->slots[set_nodelength(node) - 1 - index];
}


PFRTAny setGetDefault(PFRTAny node, uint32_t index) {
	return getKey((PFRTBitmapNode) node,index);
}

//  Node and Array utilities

static PFRTAny *entryToNode(PFRTAny *src,uint32_t olen,uint32_t idx, uint32_t idxNew,
	PFRTAny node) {
	uint32_t newlen = (olen - TUPLELENSFT) + TUPLELENSFT;
	PFRTAny *dst  = allocRawAnyArray(newlen);

	//printf("Copying from 0 for %u\n",idxOld);
	arraycopy(src,dst,idx);
	//debugArray("New array 1",dst,newlen);
	arraycopy(&src[idx + TUPLELENSFT],&dst[idx],idxNew - idx);
	dst[idxNew] = node;
	//debugArray("New array 2",dst,newlen);
	arraycopy(&src[idxNew+TUPLELENSFT],&dst[idxNew+TUPLELENSFT],olen - idxNew - TUPLELENSFT);
	return dst;
}

static PFRTAny *insertK(PFRTAny *src,uint32_t olen,uint32_t idx,
	uint32_t pos, PFRTAny k) {
	PFRTAny *dst  = allocRawAnyArray(olen+TUPLELENSFT);
	if(src != emtndarray) {
		arraycopy(src,dst,idx);
		arraycopy(&src[idx],&dst[idx+TUPLELENSFT],olen-idx);
	}
	dst[idx+KEYPOS] = k;
	return dst;
}

static PFRTAny  *nodeToEntry(PFRTAny *src,uint32_t oldlen,uint32_t idxOld,uint32_t idxNew,
	PFRTAny k) {

 	PFRTAny  *dst = allocRawAnyArray(oldlen - TUPLELENSFT + TUPLELENSFT);
 	arraycopy(src,dst,idxNew);
 	dst[idxNew+KEYPOS] = k;
    arraycopy(&src[idxNew],&dst[idxNew+1],idxOld - idxNew);
	arraycopy(&src[idxOld+1],&dst[idxOld+1],oldlen - idxOld - 1);
	return dst;
}


//	Create new node with immediate two children or push down and try again

static PFRTBitmapNode mergeTwoKeyValPairs(PFRTAny cKey,  PFRTAny key,  ft shift) {
	PFRTBitmapNode node;
	//printf("mtkvp shift = %llu const shift = %llu\n", shift,WCNT);
	//printf("Merge two key pairs ");
	//printf("	Merge cKey %s key %s shift %llu\n",(char *)cKey->value,(char *)key->value,shift);
	if (shift >= WCNT) {
        // throw new
        // IllegalStateException("Hash collision not yet fixed.");
        printf("SET: Can't handle collisions yet on shift %llu\n",shift);
        printf("	Current Key %s cKeyHash 0x%04X Key %s keyHash 0x%04X\n",
        	(char*)cKey->value,cKey->hash,(char*)key->value,key->hash);
        //exit(-1);
        unknown_handler();
        //return new HashCollisionMapNode<>(keyHash0, (K[]) new Object[]{key0, key1},
        //    (V[]) new Object[]{val0, val1});
	}

	uint32_t mask0 = mask(hash(cKey), shift);
	uint32_t mask1 = mask(hash(key), shift);

	//	Can add somehow
	if(mask0 != mask1) {
		//printf("	Simple shot\n");
		node = allocNodeWith(bit_pos(mask0) | bit_pos(mask1),0,2);
		if(mask0 < mask1) {
			node->slots[0] = cKey;
			node->slots[1] = key;
		}
		else {
			node->slots[0] = key;
			node->slots[1] = cKey;
		}
	}
	else {
		//printf("	Recursive shot\n");
		node = allocNodeWith(0,bit_pos(mask0),1);
		node->slots[0] = (PFRTAny) mergeTwoKeyValPairs(cKey,key,shift+SHIFT);
	}
	return node;
}

static PFRTBitmapNode migrateFromEntryToNode(PFRTBitmapNode o, uint32_t bitpos,
	PFRTBitmapNode node) {

	uint32_t idxOld = TUPLELENSFT * dataIndex(o->datamap,bitpos);
	uint32_t oldlen =set_nodelength(o);
	uint32_t idxNew = (oldlen - TUPLELENSFT) - nodeIndex(o->nodemap,bitpos);
	return allocNodeWithAll(o->datamap ^ bitpos,o->nodemap | bitpos,
		entryToNode(o->slots,oldlen,idxOld, idxNew, (PFRTAny) node));
}

static PFRTBitmapNode migrateFromNodeToEntry(PFRTBitmapNode o, uint32_t pos,
	PFRTBitmapNode node) {
	uint32_t oldlen =set_nodelength(o);
	uint32_t idxOld = oldlen - TUPLELENSFT - nodeIndex(o->nodemap,pos);
	uint32_t idxNew = TUPLELENSFT * dataIndex(o->datamap,pos);
	return allocNodeWithAll(o->datamap | pos, o->nodemap ^ pos,
		nodeToEntry(o->slots,oldlen,idxOld,idxNew,getKey(node,0)));
}

//	Called on persist operation to update a slot for a node

static PFRTBitmapNode copyAndSetNode (PFRTBitmapNode src,
	uint32_t bitpos,PFRTBitmapNode node) {

	if(src->slots == emtndarray) {
		//printf("MAJOR CONDITION FAULT src->slots == emtndarray copyAndSetNode\n");
		//exit(-1);
		unknown_handler();
	}
	ft slen =set_nodelength(src);
	uint32_t idx = slen - 1 - nodeIndex(src->nodemap,bitpos);
	PFRTBitmapNode dst = allocNodeClone(src,slen);
	dst->slots[idx+0] = (PFRTAny) node;
	return dst;
}

static PFRTBitmapNode copyAndSetNode_m (PFRTBitmapNode src,
	uint32_t bitpos,PFRTBitmapNode node) {

	//printf("	Copy/Set node with addy 0x%08llX\n",(ft) node);
	if(src->slots == emtndarray) {
		//printf("MAJOR POSITION FAULT copyAndSetNode_m\n");
		//exit(-1);
		unknown_handler();
	}

	ft slen =set_nodelength(src);
	uint32_t idx = slen - 1 - nodeIndex(src->nodemap,bitpos);
	src->slots[idx] = (PFRTAny) node;
	return src;
}
//	Return node copy with inserted new K/V pair

static PFRTBitmapNode copyAndInsertValue (PFRTBitmapNode src,uint32_t bitpos,
	PFRTAny key) {

	//printf("	Insert/Adjust value %s at index %u of node 0x%08llX\n",(char *)key->value,idx,(ft) src);
	return allocNodeWithAll(src->datamap | bitpos,
		src->nodemap,
		insertK(src->slots,
			nodelength(src),
			TUPLELENSFT * dataIndex(src->datamap,bitpos),
			bitpos,
			key));
}

static PFRTBitmapNode copyAndRemoveValue (PFRTBitmapNode src,uint32_t bitpos) {
	uint32_t idxOld = TUPLELENSFT * dataIndex(src->datamap,bitpos);
	uint32_t oldlen =set_nodelength(src);
	PFRTAny  *dst = allocRawAnyArray(oldlen-1);
	arraycopy(src->slots,dst,idxOld);
	arraycopy(&src->slots[idxOld+1],&dst[idxOld],oldlen - idxOld - 1);
	return allocNodeWithAll(src->datamap ^ bitpos,src->nodemap,dst);
}

static PFRTBitmapNode copyAndInsertValue_m (PFRTBitmapNode src,uint32_t bitpos,
	PFRTAny key) {
	//printf("	Insert/Adjust value %s at index %u of node 0x%08llX\n",(char *)key->value,idx,(ft) src);
	PFRTAny *old = src->slots;
	//printf("Old node length = %u \n",set_nodelength(src));
	src->slots = insertK(old,
		set_nodelength(src),
		TUPLELENSFT * dataIndex(src->datamap,bitpos),
		bitpos,
		key);

	src->datamap = src->datamap | bitpos;
	//printf("New node length = %u \n",set_nodelength(src));

	if(old != emtndarray)
		foidl_xdel(old);
	return src;
}

static PFRTBitmapNode migrateFromEntryToNode_m(PFRTBitmapNode o, uint32_t bitpos,
	PFRTBitmapNode node) {

	uint32_t idxOld = TUPLELENSFT * dataIndex(o->datamap,bitpos);
	uint32_t nidx = nodeIndex(o->nodemap,bitpos);
	uint32_t oldlen = set_nodelength(o);
	uint32_t idxNew = (oldlen - TUPLELENSFT) - nidx;

	o->datamap = o->datamap ^ bitpos;
	o->nodemap = o->nodemap | bitpos;
	PFRTAny *old = o->slots;
	o->slots   = entryToNode(old,oldlen,idxOld, idxNew, (PFRTAny) node);
	foidl_xdel(old);

	return o;
}

static PFRTBitmapNode set_extend_i(PFRTBitmapNode node, PSetNodeResult details,
	ft shift,uint32_t keyHash, PFRTAny key) {
	uint32_t 		keyMask = mask(keyHash,shift);
	uint32_t 		bitpos = bit_pos(keyMask);
	PFRTBitmapNode 	newNode = node;

	if((node->datamap & bitpos) != 0) {
		uint32_t dindex = dataIndex(node->datamap,bitpos);
		PFRTAny currentKey = getKey(node,dindex);
		if(foidl_equal_qmark(key, currentKey) == true) {
			newNode = node;
		}
		else {
			details->isModified = true;
			++details->deltaSize;
			details->deltaHashCode += keyHash;
			newNode = migrateFromEntryToNode(node,bitpos,
						mergeTwoKeyValPairs(currentKey,key,shift + SHIFT));
		}

	}
	else if((node->nodemap & bitpos) != 0) {
		PFRTBitmapNode subNode = set_getNode(node,nodeIndex(node->nodemap,bitpos));
		PFRTBitmapNode subNewNode = set_extend_i(subNode,details,shift + SHIFT,keyHash,key);
		if(details->isModified == true) {
			newNode = copyAndSetNode(node,bitpos,subNewNode);
		}

	}
	else {
		details->isModified = true;
		++details->deltaSize;
		details->deltaHashCode += keyHash;
		newNode = copyAndInsertValue(node,bitpos,key);
	}
	return newNode;
}

static PFRTBitmapNode set_remove_i(PFRTBitmapNode node, PSetNodeResult details,
	ft shift,uint32_t keyHash, PFRTAny key) {
	uint32_t 	keyMask = mask(keyHash,shift);
	uint32_t 	bitpos = bit_pos(keyMask);
	if((node->datamap & bitpos) != 0) {
		uint32_t dataIdx = dataIndex(node->datamap,bitpos);
		PFRTAny currentKey = getKey(node,dataIdx);
		if( foidl_equal_qmark (currentKey,key) == true) {
			details->isModified = true;
			details->deltaSize = -1;
			details->deltaHashCode = -keyHash;
			if(payloadArity(node) == 2 && nodeArity(node) == 0) {
				uint32_t dataMap = (shift == 0) ? node->datamap ^ bitpos :
					bit_pos(mask(keyHash,0));
				PFRTAny  *newSlots = allocRawAnyArray(1);
				newSlots[0] = (dataIdx == 0) ? getKey(node,1) : getKey(node,0);
				return allocNodeWithAll(dataMap,0,newSlots);
			}
			else {
				return copyAndRemoveValue(node,bitpos);
			}
		}
		else {
			return node;
		}
	}
	else if((node->nodemap & bitpos) != 0) {
		PFRTBitmapNode subNode = set_getNode(node,nodeIndex(node->nodemap,bitpos));
		PFRTBitmapNode subNewNode = set_remove_i(subNode,details,shift+SHIFT,keyHash,key);
		if(details->isModified == false) {
			return node;
		}
		switch(sizePredicate(subNewNode)) {
			case 0:
				//printf("Node is empty which is no good in map_remove\n");
				//exit(-1);
				unknown_handler();
			case 1:
				if(payloadArity(node) == 0 && nodeArity(node) == 1)
					return subNewNode;
				else
					return migrateFromNodeToEntry(node,bitpos,subNewNode);
			default:
				return copyAndSetNode(node,bitpos,subNewNode);
		}

	}

	return node;
}

PFRTAny set_remove(PFRTAny s, PFRTAny k) {
	PFRTSet 	 			set = (PFRTSet) s;
	struct SetNodeResult 	details = {end,false,false,0,0};
	uint32_t 				keyhash = hash(k);
	uint32_t 				ohash  = set->hash;
	uint32_t 				ocount = set->count;
	PFRTBitmapNode 			newRoot = set_remove_i(set->root,&details,0,keyhash,k);
	PFRTSet 				newSet = set;
	if(details.isModified == true) {
		newSet = allocSet(ocount-1,SHIFT,newRoot);
		newSet->hash  = ohash - keyhash;
	}
	return (PFRTAny) newSet;
}


static PFRTBitmapNode set_extend_bang_i(PFRTBitmapNode node, PSetNodeResult details,
	ft shift,uint32_t keyHash, PFRTAny key) {
	uint32_t 		keyMask = mask(keyHash,shift);
	uint32_t 		bitpos = bit_pos(keyMask);
	PFRTBitmapNode 	newNode = node;
	//printf("sebi mask = 0x%04X in pos %u\n",keyMask,bitpos);

	if((node->datamap & bitpos) != 0) {
		uint32_t dindex = dataIndex(node->datamap,bitpos);
		PFRTAny currentKey = getKey(node,dindex);
		//printf("	datamap hit at index %u with value %c\n",dindex,
		//	(char) (ft) currentKey->value);
		if(foidl_equal_qmark(key, currentKey) == true) {
			newNode = node;
		}
		else {
			details->isModified = true;
			++details->deltaSize;
			details->deltaHashCode += keyHash;
			newNode = migrateFromEntryToNode_m(node,bitpos,
						mergeTwoKeyValPairs(currentKey,key,shift + SHIFT));
		}

	}
	else if((node->nodemap & bitpos) != 0) {
		//printf("	nodemap hit\n");
		PFRTBitmapNode subNode = set_getNode(node,nodeIndex(node->nodemap,bitpos));
		PFRTBitmapNode subNewNode = set_extend_bang_i(subNode,details,shift + SHIFT,keyHash,key);
		if(details->isModified == true) {
			newNode = copyAndSetNode_m(node,bitpos,subNewNode);
		}

	}
	else {
		details->isModified = true;
		++details->deltaSize;
		details->deltaHashCode += keyHash;
		newNode = copyAndInsertValue_m(node,bitpos,key);
	}
	return newNode;
}

PFRTAny 	foidl_set_inst_bang() {
	return (PFRTAny) allocSet(0,SHIFT,allocNode());
}

PFRTAny 	foidl_set_extend_bang(PFRTAny s, PFRTAny k) {
	struct SetNodeResult 	details = {end,false,false,0,0};
	uint32_t keyhash = hash(k);
	PFRTSet  set = ((PFRTSet) s != empty_set) ? (PFRTSet) s
		: (PFRTSet) foidl_set_inst_bang();
	uint32_t ohash  = set->hash;

	//printf("Adding element %c to set with hash 0x%4X\n",(char) (ft) k->value, keyhash);

	set_extend_bang_i(set->root,&details,0,keyhash,k);
	PFRTSet 				newSet = set;

	if(details.isModified == true) {
		newSet->hash  = ohash + keyhash;
		++newSet->count;
	}

	return (PFRTAny) newSet;
}

PFRTAny set_extend(PFRTAny s, PFRTAny k) {
	struct SetNodeResult 	details = {end,false,false,0,0};
	uint32_t 				keyhash = hash(k);

	if((PFRTSet) s == empty_set)
		return foidl_set_extend_bang(s,k);
	PFRTSet  		set = (PFRTSet) s;
	uint32_t 		ohash  = set->hash;
	uint32_t 		ocount = set->count;
	PFRTBitmapNode 	newRoot = set_extend_i(set->root,&details,0,keyhash,k);
	PFRTSet 		newSet = set;

	if(details.isModified == true) {
		newSet = allocSet(ocount,SHIFT,newRoot);
		newSet->hash  = ohash + keyhash;
		++newSet->count;
	}
	return (PFRTAny) newSet;
}

static PGetResult find_by_key(PFRTBitmapNode node, PFRTAny key, uint32_t keyHash,
	ft shift,PGetResult results) {
	uint32_t 		keyMask = mask(keyHash,shift);
	uint32_t 		bitpos = bit_pos(keyMask);
	//printf("set: find_by_key val: %c\n",(char) (ft) key->value);
	if((node->datamap & bitpos) != 0) {
		uint32_t index = dataIndex(node->datamap,bitpos);
		PFRTAny  fkey = getKey(node,index);
		//printf("	datamap hit on %c\n",(char) (ft) fkey->value );
		if(foidl_equal_qmark(fkey,key) == true) {
			//printf("	keys match\n");
			results->isFound = true;
			results->result = fkey;
			return results;
		}
		return results;
	}
	if((node->nodemap & bitpos) != 0) {
		//printf("	nodemap hit on %c\n",(char) (ft) key->value );
		PFRTBitmapNode subnode = set_getNode(node,nodeIndex(node->nodemap,bitpos));
		return find_by_key(subnode,key,keyHash,shift + SHIFT,results);
	}

	return results;
}

PFRTAny set_get(PFRTAny s,PFRTAny el) {
	PFRTSet set = (PFRTSet) s;
	struct GetResult res = {nil,nil,nil,false};
	find_by_key(set->root,el,hash(el),0,&res);
	return (res.isFound == true ? res.result : nil);
}

PFRTAny set_get_default(PFRTAny s,PFRTAny el, PFRTAny def) {
	PFRTSet set = (PFRTSet) s;
	struct GetResult res = {nil,nil,nil,false};
	find_by_key(set->root,el,hash(el),0,&res);
	if(res.isFound == true) {
		return res.result;
	}
	else {
		if(foidl_function_qmark(def) == true)
			 return dispatch2(def,s,el);
		else
			return def;
	}
	return nil;
}

PFRTAny	set_first(PFRTAny set) {
	PFRTAny	result = nil;
	PFRTIterator si = iteratorFor(set);
	result = iteratorNext(si);
	foidl_xdel(si);
	result = (result == end) ? nil : result;
	return result;
}

PFRTAny	set_second(PFRTAny set) {
	PFRTAny	result = nil;
	PFRTIterator si = iteratorFor(set);
	result = iteratorNext(si);
	if(result != end) {
		result = iteratorNext(si);
	}
	foidl_xdel(si);
	result = (result == end) ? nil : result;
	return result;
}

PFRTAny set_rest(PFRTAny src) {
	PFRTAny result = (PFRTAny) empty_set;
	if(src->count > 1) {
		PFRTAny 		itm = nil;
		PFRTAny 		s1 = foidl_set_inst_bang();
		PFRTIterator	si = iteratorFor(src);
		result = s1;
		iteratorNext(si);
		while((itm = iteratorNext(si)) != end)
			foidl_set_extend_bang(s1,itm);
		foidl_xdel(si);
		uint32_t s1hsh = hash(set_first(src));
		s1->hash = src->hash - s1hsh;
		s1->count = src->count - 1;
	}
	return result;
}


PFRTAny set_print(PFRTIOChannel chn,PFRTAny set) {
	PFRTIterator mi = iteratorFor(set);
	PFRTAny entry;
	ft 		 mcount=((PFRTSet) set)->count;
	io_writeFuncPtr	cfn = chn->writehandler->fnptr;
	cfn(chn,meta);
	cfn(chn,lbrace);
	if(mcount > 0) {
		uint32_t count=0;
		--mcount;
		while ((entry = iteratorNext(mi)) != end) {
			cfn(chn,entry);
			if(mcount > count++) cfn(chn,comma);
		}
		foidl_xdel(mi);
	}
	cfn(chn,rbrace);
	return nil;
}

PFRTAny write_set(PFRTAny channel, PFRTAny set, channel_writer writer) {
	PFRTIterator mi = iteratorFor(set);
	PFRTAny entry;
	ft 		 mcount=((PFRTSet) set)->count;
	writer(channel,meta);
	writer(channel,lbrace);
	if(mcount > 0) {
		uint32_t count=0;
		--mcount;
		while ((entry = iteratorNext(mi)) != end) {
			writer(channel,entry);
			if(mcount > count++) writer(channel,comma);
		}
		foidl_xdel(mi);
	}
	writer(channel,rbrace);
	return nil;
}

PFRTAny coerce_to_set(PFRTAny stemplate, PFRTAny src) {
	unknown_handler();
	return src;
}

void  release_set(PFRTAny s) {
	unknown_handler();
}
