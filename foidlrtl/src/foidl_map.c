
/*
	foidl_map.c
	Library support for CHAMP map
	
	Copyright Frank V. Castellucci
	All Rights Reserved
*/

#define  MAP_IMPL

#include <foidlrt.h>
#include <stdio.h>

//	Global constructs

const struct FRTMapG _empty_map = {
	global_signature,
	collection_class,
	map2_type,
	0,0,
	(PFRTBitmapNode) &_empty_champ_node.fclass,SHIFT};

PFRTMap const empty_map = (PFRTMap) &_empty_map.fclass;

static PFRTAny getKey(PFRTBitmapNode node, uint32_t index) {
	return node->slots[TUPLELEN *index];
}

static PFRTAny getValue(PFRTBitmapNode node, uint32_t index) {
	return node->slots[(TUPLELEN *index) + TUPLELENSFT];
}

static PFRTMapEntry getEntry(PFRTBitmapNode node, uint32_t index) {
	return allocMapEntryWith(node->slots[TUPLELEN * index], 
		node->slots[TUPLELEN * index + 1]);
}

PFRTAny mapGetDefault(PFRTAny node, uint32_t index) {
	return (PFRTAny) getEntry((PFRTBitmapNode) node,index);
}

const char *lineno = ":lineno";
const char *colnum = ":colnum";
const char *tokens = ":tokens";

static void debugMap(char *title,PFRTBitmapNode node,PFRTAny el,uint32_t bpos) {
	return;
	char *pv = (char *) el->value;

	if(el->ftype == keyword_type && 
		(!strcmp(lineno, pv)
		|| !strcmp(colnum, pv)
		|| !strcmp(tokens,pv))) {
		printf("%s key %s ", title,(char *) pv);	
		printf("bpos = 0x%04X dmap = 0x%04X nmap = 0x%04X \n", 
			bpos,node->datamap,node->nodemap);	
	}
	
}

//
//	Map Iterator (probably refactor to trie iterator)
//

//	Insert a new KV pair to slot array

static PFRTAny *insertKV(PFRTAny *src,uint32_t olen,uint32_t idx, uint32_t pos, 
	PFRTAny k, PFRTAny v) {	
	PFRTAny * dst  = allocRawAnyArray(olen+TUPLELEN);
	if(src != emtndarray) {
		arraycopy(src,dst,idx);	
		arraycopy(&src[idx],&dst[idx+TUPLELEN],olen-idx);
	}
	dst[idx+KEYPOS] = k;
	dst[idx+VALPOS] = v;		
	return dst;
}

//	Convert location at idx to a node (shrink the array)

static PFRTAny *entryToNode(PFRTAny *src,uint32_t olen,uint32_t idx, uint32_t idxNew, 
	PFRTAny node) {
	uint32_t newlen = (olen - TUPLELEN) + TUPLELENSFT;
	PFRTAny *dst  = allocRawAnyArray(newlen);	

	//printf("Copying from 0 for %u\n",idxOld);
	arraycopy(src,dst,idx);
	//debugArray("New array 1",dst,newlen);
	arraycopy(&src[idx + TUPLELEN],&dst[idx],idxNew - idx);
	dst[idxNew] = node;	
	//debugArray("New array 2",dst,newlen);
	arraycopy(&src[idxNew+TUPLELEN],&dst[idxNew+TUPLELENSFT],olen - idxNew - 2);
	
	return dst;
}

// Convert location from node to Key/Value entry (expand the array)

static PFRTAny  *nodeToEntry(PFRTAny *src,uint32_t oldlen,uint32_t idxOld,uint32_t idxNew, 
	PFRTAny k, PFRTAny v) {

 	PFRTAny  *dst = allocRawAnyArray(oldlen - TUPLELENSFT + TUPLELEN); 	
 	arraycopy(src,dst,idxNew);
 	dst[idxNew+KEYPOS] = k;
 	dst[idxNew+VALPOS] = v;
    arraycopy(&src[idxNew],&dst[idxNew+2],idxOld - idxNew);  
	arraycopy(&src[idxOld+1],&dst[idxOld+2],oldlen - idxOld - 1);    
	return dst;
}

//	Called on persist object to update a slot 'key's value

static PFRTBitmapNode copyAndSetValue (PFRTBitmapNode src,uint32_t pos, PFRTAny value) {
	PFRTBitmapNode dst = (PFRTBitmapNode) nil;	
	if(src->slots == emtndarray) {
		//printf("MAJOR CONDITION FAULT src->slots == emtndarray copyAndSetValue\n");
		//exit(-1);
		unknown_handler();
	}
	else {		
		ft idx = TUPLELEN * dataIndex(src->datamap,pos) + 1;
		dst = allocNodeClone(src,nodelength(src));		
		dst->slots[idx+0] = value;
	}
	return dst;
}

//	Called on mutable object to update a slot 'key's value

static PFRTBitmapNode copyAndSetValue_m (PFRTBitmapNode src,uint32_t pos, PFRTAny value) {
	PFRTBitmapNode dst = src;
	if(src->slots == emtndarray) {
		//printf("MAJOR POSITION FAULT copyAndSetValue_m\n");
		//exit(-1);
		unknown_handler();
	}
	else {
		ft idx = TUPLELEN * dataIndex(src->datamap,pos) + 1;
		dst->slots[idx] = value;
	}
	return dst;
}

//	Called on persist object to remove a key/value

static PFRTBitmapNode copyAndRemoveValue(PFRTBitmapNode node, uint32_t bitpos,
	PMapNodeResult details) {

	uint32_t idx = TUPLELEN * dataIndex(node->datamap,bitpos);
	uint32_t oldlen = nodelength(node);	
	uint32_t newlen = oldlen - 2;
	PFRTAny  *dst = allocRawAnyArray(newlen);
	arraycopy(node->slots,dst,idx);
	arraycopy(&node->slots[idx+2],&dst[idx],(oldlen - idx) - 2);
	return allocNodeWithAll(node->datamap ^ bitpos, node->nodemap,dst);
}

//	Called on mutable object to remove a key/value

static PFRTBitmapNode copyAndRemoveValue_m(PFRTBitmapNode node, uint32_t bitpos,
	PMapNodeResult details) {

	uint32_t idx = TUPLELEN * dataIndex(node->datamap,bitpos);
	uint32_t oldlen = nodelength(node);	
	uint32_t newlen = oldlen - 2;
	PFRTAny  *dst = allocRawAnyArray(newlen);
	PFRTAny  *old = node->slots;
	arraycopy(node->slots,dst,idx);
	arraycopy(&node->slots[idx+2],&dst[idx],(oldlen - idx) - 2);
	node->datamap = node->datamap ^ bitpos;	
	node->slots = dst;
	foidl_xdel(old);
	return node;
}


//	Called on persist operation to update a slot for a node 

static PFRTBitmapNode copyAndSetNode (PFRTBitmapNode src,
	uint32_t pos,PFRTBitmapNode node) {	
	PFRTBitmapNode dst = (PFRTBitmapNode) nil;	
	if(src->slots == emtndarray) {
		//printf("MAJOR CONDITION FAULT src->slots == emtndarray copyAndSetNode\n");
		//exit(-1);
		unknown_handler();
	}
	else {
		ft slen = nodelength(src); 
		uint32_t idx = slen - 1 - nodeIndex(src->nodemap,pos);
		dst = allocNodeClone(src,slen);	
		dst->slots[idx+0] = (PFRTAny) node;
	}
	return dst;
}

static PFRTBitmapNode copyAndSetNode_m (PFRTBitmapNode src,
	uint32_t pos,PFRTBitmapNode node) {
	
	//printf("	Copy/Set node with addy 0x%08llX\n",(ft) node);
	if(src->slots == emtndarray) {
		//printf("MAJOR POSITION FAULT copyAndSetNode_m\n");
		//exit(-1);
		unknown_handler();
	}
	else {
		ft slen = nodelength(src); 		
		uint32_t idx = slen - 1 - nodeIndex(src->nodemap,pos);		
		src->slots[idx] = (PFRTAny) node;
	}
	return src;
}

//	Return node copy with inserted new K/V pair

static PFRTBitmapNode copyAndInsertValue (PFRTBitmapNode src,uint32_t pos,
	PFRTAny key, PFRTAny value) {	

	//printf("	Insert/Adjust value %s at index %u of node 0x%08llX\n",(char *)key->value,idx,(ft) src);
	return allocNodeWithAll(src->datamap | pos, 
		src->nodemap,
		insertKV(src->slots,
			nodelength(src),
			TUPLELEN * dataIndex(src->datamap,pos),
			pos,
			key,
			value));	
}

static PFRTBitmapNode copyAndInsertValue_m (PFRTBitmapNode src,uint32_t pos,
	PFRTAny key, PFRTAny value) {	
	//printf("	Insert/Adjust value %s at index %u of node 0x%08llX\n",(char *)key->value,idx,(ft) src);
	PFRTAny *old = src->slots;
	src->slots = insertKV(old,
		nodelength(src),
		TUPLELEN * dataIndex(src->datamap,pos),
		pos,
		key,
		value);

	src->datamap = src->datamap | pos;

	if(old != emtndarray) foidl_xdel(old);
	return src;
}

static PFRTBitmapNode mergeTwoKeyValPairs(PFRTAny cKey, PFRTAny cValue,
	PFRTAny key, PFRTAny value, ft shift) {
	PFRTBitmapNode node;	
	//printf("Merge two key pairs ");
	//printf("	Merge cKey %s key %s shift %llu\n",(char *)cKey->value,(char *)key->value,shift);
	if (shift >= WCNT) {
        // throw new
        // IllegalStateException("Hash collision not yet fixed.");
        //printf("Can't handle collisions yet on shift %llu\n",shift);
        //printf("Current Key %s cKeyHash 0x%04X Key %s keyHash 0x%04X\n", 
        //	(char*)cKey->value,cKey->hash,(char*)key->value,key->hash);
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
		node = allocNodeWith(bit_pos(mask0) | bit_pos(mask1),0,4);
		if(mask0 < mask1) {			
			node->slots[0] = cKey;
			node->slots[1] = cValue;
			node->slots[2] = key;
			node->slots[3] = value;
		}
		else {
			node->slots[0] = key;
			node->slots[1] = value;
			node->slots[2] = cKey;
			node->slots[3] = cValue;
		}
	}
	else {
		//printf("	Recursive shot\n");
		node = allocNodeWith(0,bit_pos(mask0),1);
		node->slots[0] = (PFRTAny) mergeTwoKeyValPairs(cKey,cValue,key,value,shift+SHIFT);		
	}
	return node;	
}

static PFRTBitmapNode migrateFromEntryToNode(PFRTBitmapNode o, uint32_t pos, 
	PFRTBitmapNode node) {
	
	uint32_t idxOld = TUPLELEN * dataIndex(o->datamap,pos);
	uint32_t oldlen = nodelength(o);	
	uint32_t idxNew = (oldlen - TUPLELEN) - nodeIndex(o->nodemap,pos);	
	
	return allocNodeWithAll(o->datamap ^ pos,o->nodemap | pos,
		entryToNode(o->slots,oldlen,idxOld, idxNew, (PFRTAny) node));
}

static PFRTBitmapNode migrateFromEntryToNode_m(PFRTBitmapNode o, uint32_t pos, PFRTBitmapNode node) {

	uint32_t idxOld = TUPLELEN * dataIndex(o->datamap,pos);
	uint32_t oldlen = nodelength(o);	
	uint32_t idxNew = (oldlen - TUPLELEN) - nodeIndex(o->nodemap,pos);	
	
	o->datamap = o->datamap ^ pos;
	o->nodemap = o->nodemap | pos;
	PFRTAny *old = o->slots;	
	o->slots   = entryToNode(old,oldlen,idxOld, idxNew, (PFRTAny) node);
	foidl_xdel(old);

	return o;
}

static PFRTBitmapNode migrateFromNodeToEntry(PFRTBitmapNode o, uint32_t pos, 
	PFRTBitmapNode node) {
	
	uint32_t idxNew = TUPLELEN * dataIndex(o->datamap,pos);
	uint32_t oldlen = nodelength(o);	
	uint32_t idxOld = oldlen - TUPLELENSFT - nodeIndex(o->nodemap,pos);	
	return allocNodeWithAll(o->datamap | pos, o->nodemap ^ pos,
		nodeToEntry(o->slots,oldlen,idxOld,idxNew,getKey(node,0),getValue(node,0)));
}

static PFRTBitmapNode migrateFromNodeToEntry_m(PFRTBitmapNode o, uint32_t pos, 
	PFRTBitmapNode node) {
	
	uint32_t idxNew = TUPLELEN * dataIndex(o->datamap,pos);
	uint32_t oldlen = nodelength(o);	
	uint32_t idxOld = oldlen - TUPLELENSFT - nodeIndex(o->nodemap,pos);
	PFRTAny  *old = o->slots;	
	o->slots = nodeToEntry(old,oldlen,idxOld,idxNew,getKey(node,0),getValue(node,0));
	o->datamap = o->datamap | pos;
	o->nodemap = o->nodemap ^ pos;
	foidl_xdel(old);
	return o;
}

//	Extend (insert, associate) map with new key/value pair

static PFRTBitmapNode map_extend_i(PFRTBitmapNode srcNode,PMapNodeResult details, 
	ft shift, uint32_t keyHash, PFRTAny key, PFRTAny value) {
	uint32_t 		keyMask = mask(keyHash,shift);	
	uint32_t 		bitpos = bit_pos(keyMask);

	PFRTBitmapNode 	newNode = (PFRTBitmapNode) nil;

	//printf("Map Extend with key %s at shift %llu \n", (char *) key->value,shift);
	
	if((srcNode->datamap & bitpos) != 0) {		
		uint32_t dindex = dataIndex(srcNode->datamap,bitpos);
		PFRTAny currentKey = getKey(srcNode,dindex);
		PFRTAny currentValue = getValue(srcNode,dindex);
		//	If keys are the same then replace value for key
		if(foidl_equal_qmark(key,currentKey) == true) {
			//printf("	Extend: key match, replace value\n");			
			details->isModified = true;
			details->isReplaced = true;
			details->replacedValue = currentValue;
			return copyAndSetValue(srcNode,bitpos,value);
		}
		//	We have another kv at position
		else {
			//printf("	Extend: key no match, push to node\n");
			details->isModified = true;			
			newNode = mergeTwoKeyValPairs(currentKey,currentValue,key,value,shift + SHIFT);			
			return migrateFromEntryToNode(srcNode,bitpos,newNode);
		}
	}
	else if((srcNode->nodemap & bitpos) != 0) {
		//printf("	Extend: ((node->nodemap & bitpos) != 0) recursing...\n");
		PFRTBitmapNode subNode = getNode(srcNode,nodeIndex(srcNode->nodemap,bitpos));
		PFRTBitmapNode subNewNode = map_extend_i(subNode,details,shift + SHIFT,keyHash,key,value);
		if(details->isModified)	{
			newNode = copyAndSetNode(srcNode,bitpos,subNewNode);
		}
	}
	else {		
		//printf("	Extend: No data or node positions match...\n");
		details->isModified = true;
		newNode = copyAndInsertValue(srcNode,bitpos,key,value);
	}
	return newNode;		
}

//	Map contains predicate

static PFRTAny map_contains_qmark_i(PFRTBitmapNode node,uint32_t khash,PFRTAny key,
	uint32_t shift) {

	uint32_t mask0 = mask(khash,shift);
	uint32_t bpos = bit_pos(mask0);
	if((node->datamap & bpos) != 0) {
		uint32_t idx = dataIndex(node->datamap,bpos);		
		return foidl_equal_qmark(getKey(node,idx),key);
	}

	if((node->nodemap & bpos) != 0) {
		uint32_t idx = nodeIndex(node->nodemap,bpos);
		return map_contains_qmark_i(getNode(node,idx),khash,key,shift + SHIFT);
	}

	return false;
}

PFRTAny map_contains_key_qmark(PFRTAny m,PFRTAny el) {
	PFRTMap map = (PFRTMap) m;
	return map_contains_qmark_i(map->root,hash(el),el,0);
}

PFRTAny map_contains_qmark(PFRTAny m,PFRTAny el) {
	return map_contains_key_qmark(m,el);
}

//	Remove element identified by key (if found) from map

static PFRTBitmapNode map_remove_i(PFRTBitmapNode node,PMapNodeResult details, 
	ft shift, uint32_t keyHash, PFRTAny key) {

	uint32_t 		keyMask = mask(keyHash,shift);	
	uint32_t 		bitpos = bit_pos(keyMask);
	if((node->datamap & bitpos) != 0) {
		uint32_t idx = dataIndex(node->datamap,bitpos);		
		if(foidl_equal_qmark(getKey(node,idx),key) == true) {
			details->replacedValue = getValue(node,idx);
			details->isModified = true;
			if(payloadArity(node) == 2 && nodeArity(node) == 0) {
				uint32_t newDataMap = 
				(shift == 0) ? (node->datamap ^ bitpos) : bit_pos(mask(keyHash,0));
				PFRTAny *dst = allocRawAnyArray(2);
				if(idx == 0) {
					dst[0] = getKey(node,1);
					dst[1] = getValue(node,1);
					return allocNodeWithAll(newDataMap,0,dst);
				}
				else {
					dst[0] = getKey(node,0);
					dst[1] = getValue(node,0);
					return allocNodeWithAll(newDataMap,0,dst);					
				}
			}
			else {
				return copyAndRemoveValue(node,bitpos,details);
			}
		}
		else {
			return node;
		}
	}
	else if((node->nodemap & bitpos) != 0) {
		PFRTBitmapNode subNode = getNode(node,nodeIndex(node->nodemap,bitpos));
		PFRTBitmapNode subNewNode = map_remove_i(subNode,details,shift + SHIFT,keyHash,key);
		if(details->isModified != true)	{
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

PFRTAny map_remove(PFRTAny m, PFRTAny k) {
	PFRTMap 	 			map = (PFRTMap) m;	
	struct MapNodeResult 	details = {end,false,false};
	uint32_t 				keyhash = hash(k);	
	uint32_t 				ohash  = map->hash;
	uint32_t 				ocount = map->count;
	PFRTBitmapNode 			newRoot = map_remove_i(map->root,&details,0,keyhash,k);
	PFRTMap 				newMap = map;

	if(details.isModified == true) {		
		newMap = allocMap(ocount-1,SHIFT,newRoot);
		uint32_t valhash = hash(details.replacedValue);
		newMap->hash  = ohash - ((keyhash ^ valhash));		
	}
	
	return (PFRTAny) newMap;
}


//	Map get
static void map_get_i(PFRTBitmapNode node, PFRTAny el,uint32_t khash,uint32_t shift, 
	PGetResult res) {	
	uint32_t mask0 = mask(khash,shift);
	uint32_t bpos = bit_pos(mask0);
	debugMap("Get",node,el,bpos);
	if((node->datamap & bpos) != 0) {
		uint32_t idx = dataIndex(node->datamap,bpos);
		if(foidl_equal_qmark(getKey(node,idx),el) == true) {
			res->result = getValue(node,idx);
			res->isFound = true;
			res->elementSearchedOn = el;
			res->nodeFoundIn = (PFRTAny) node;
			return;
		}
	}

	if((node->nodemap & bpos) != 0) {
		uint32_t idx = nodeIndex(node->nodemap,bpos);
		map_get_i(getNode(node,idx),el,khash,shift + SHIFT,res);
	}
}

PFRTAny map_get(PFRTAny m,PFRTAny el) {
	PFRTMap map = (PFRTMap) m;
	struct GetResult res = {nil,nil,nil,false};
	map_get_i(map->root,el,hash(el),0, &res);	
	return (res.isFound == true ? res.result : nil);
}

PFRTAny map_get_default(PFRTAny m,PFRTAny el, PFRTAny def) {
	PFRTMap map = (PFRTMap) m;
	struct GetResult res = {nil,nil,nil,false};
	map_get_i(map->root,el,hash(el),0, &res);	
	if(res.isFound == true) {
		return res.result;
	}
	else {
		if(foidl_function_qmark(def) == true)
			 return dispatch2(def,m,el);
		else 
			return def;
	}
	return nil;
}


//
//	Core map transient API
//


static PFRTBitmapNode map_extend_bang_i(PFRTBitmapNode node,PMapNodeResult details, 
	ft shift, uint32_t keyHash, PFRTAny key, PFRTAny value) {	

	uint32_t 		keyMask = mask(keyHash,shift);	
	uint32_t 		bitpos = bit_pos(keyMask);
	PFRTBitmapNode 	newNode = node;

	//printf("Map Extend with key %s with hash 0x%04X at shift %llu \n", (char *) key->value,keyHash,shift);
	
	if((node->datamap & bitpos) != 0) {		
		uint32_t dindex = dataIndex(node->datamap,bitpos);
		PFRTAny currentKey = getKey(node,dindex);
		PFRTAny currentValue = getValue(node,dindex);
		//	If keys are the same then replace value for key
		if(foidl_equal_qmark(key,currentKey) == true) {
			debugMap("Inplace update before ",node,key,bitpos);
			details->isModified = true;
			details->isReplaced = true;
			details->replacedValue = currentValue;
			newNode = copyAndSetValue_m(node,bitpos,value);
			debugMap("Inplace update after ",node,key,bitpos); 
		}
		//	We have another kv at position
		else {
			//if( key->ftype == keyword_type )
			//	printf("	Extend: key %s no match, push to node\n",(char *) key->value);
			details->isModified = true;			
			newNode = mergeTwoKeyValPairs(currentKey,currentValue,key,value,shift + SHIFT);			
			return migrateFromEntryToNode_m(node,bitpos,newNode);
		}
	}
	else if((node->nodemap & bitpos) != 0) {		
		debugMap("	Node shift update before ",node,key,bitpos);
		PFRTBitmapNode subNode = getNode(node,nodeIndex(node->nodemap,bitpos));
		PFRTBitmapNode subNewNode = map_extend_bang_i(subNode,details,shift + SHIFT,keyHash,key,value);
		if(details->isModified)	{
			newNode = copyAndSetNode_m(node,bitpos,subNewNode);
			debugMap("	Node shift update after ",node,key,bitpos);
		}

	}
	else {		
		//printf("	Extend: No data or node positions match...\n");
		debugMap("	All new before ",node,key,bitpos);
		details->isModified = true;
		newNode = copyAndInsertValue_m(node,bitpos,key,value);
		debugMap("	All new after ",node,key,bitpos);
	}
	return newNode;
}

PFRTAny foidl_map_extend_bang(PFRTAny m, PFRTAny k, PFRTAny v) {
	struct MapNodeResult 	details = {end,false,false};
	uint32_t 				keyhash = hash(k);
	uint32_t 				valhash = hash(v);	
	PFRTMap  map = ((PFRTMap) m != empty_map) ? (PFRTMap) m  
					: allocMap(0,SHIFT,allocNode());		
	uint32_t 				ohash  = map->hash;
	map_extend_bang_i(map->root,&details,0,keyhash,k,v);
	PFRTMap 				newMap = map;

	if(details.isModified == true) {
		if(details.replacedValue != end) {
			uint32_t oldvhash = hash(details.replacedValue);						
			newMap->hash  = ohash + ((keyhash ^ valhash)) - ((keyhash ^ oldvhash));
		}
		else {
			++newMap->count;
			newMap->hash = ohash + ((keyhash ^ valhash));
		}
	}
	
	return (PFRTAny) newMap;
}

PFRTAny map_extend(PFRTAny m, PFRTAny k, PFRTAny v) {
	
	struct MapNodeResult 	details = {end,false,false};
	uint32_t 				keyhash = hash(k);
	uint32_t 				valhash = hash(v);
	if((PFRTMap) m == empty_map)
		return foidl_map_extend_bang(m,k,v);
	PFRTMap 		map = (PFRTMap) m;

	uint32_t 				ocount = map->count;
	uint32_t 				ohash  = map->hash;
	PFRTBitmapNode 			newRoot = map_extend_i(map->root,&details,0,keyhash,k,v);
	PFRTMap 				newMap = map;

	if(details.isModified == true) {
		newMap = allocMap(ocount,SHIFT,newRoot);		
		if(details.replacedValue != end) {
			uint32_t oldvhash = hash(details.replacedValue);						
			newMap->hash  = ohash + ((keyhash ^ valhash)) - ((keyhash ^ oldvhash));
		}
		else {
			++newMap->count;
			newMap->hash = ohash + ((keyhash ^ valhash));
		}
	}
	else {
		newMap->count = ocount;
		newMap->hash  = ohash;
	}
	
	return (PFRTAny) newMap;
}

PFRTAny map_update(PFRTAny m, PFRTAny k, PFRTAny v) {
	return map_extend(m,k,v);
}

static PFRTBitmapNode map_remove_bang_i(PFRTBitmapNode node,PMapNodeResult details, 
	ft shift, uint32_t keyHash, PFRTAny key) {

	uint32_t 		keyMask = mask(keyHash,shift);	
	uint32_t 		bitpos = bit_pos(keyMask);
	if((node->datamap & bitpos) != 0) {
		uint32_t idx = dataIndex(node->datamap,bitpos);		
		if(foidl_equal_qmark(getKey(node,idx),key) == true) {
			details->replacedValue = getValue(node,idx);
			details->isModified = true;
			if(payloadArity(node) == 2 && nodeArity(node) == 0) {
				uint32_t newDataMap = 
				(shift == 0) ? (node->datamap ^ bitpos) : bit_pos(mask(keyHash,0));
				PFRTAny *dst = allocRawAnyArray(2);
				if(idx == 0) {
					dst[0] = getKey(node,1);
					dst[1] = getValue(node,1);
					return allocNodeWithAll(newDataMap,0,dst);
				}
				else {
					dst[0] = getKey(node,0);
					dst[1] = getValue(node,0);
					return allocNodeWithAll(newDataMap,0,dst);					
				}
			}
			else {
				return copyAndRemoveValue_m(node,bitpos,details);
			}
		}
		else {
			return node;
		}
	}
	else if((node->nodemap & bitpos) != 0) {
		PFRTBitmapNode subNode = getNode(node,nodeIndex(node->nodemap,bitpos));
		PFRTBitmapNode subNewNode = map_remove_bang_i(subNode,details,shift + SHIFT,keyHash,key);
		if(details->isModified != true)	{
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
					return migrateFromNodeToEntry_m(node,bitpos,subNewNode);
			default:
				return copyAndSetNode_m(node,bitpos,subNewNode);
		}
	}

	return node;
}

PFRTAny map_remove_bang(PFRTAny m, PFRTAny k) {
	PFRTMap 	 			map = (PFRTMap) m;	
	struct MapNodeResult 	details = {end,false,false};
	uint32_t 				keyhash = hash(k);	
	uint32_t 				ohash  = map->hash;
	map_remove_bang_i(map->root,&details,0,keyhash,k);
	PFRTMap 				newMap = map;

	if(details.isModified == true) {		
		uint32_t valhash = hash(details.replacedValue);
		newMap->hash  = ohash - ((keyhash ^ valhash));
		--newMap->count;
	}
	
	return (PFRTAny) newMap;
}

PFRTAny foidl_map_inst_bang() {
	return (PFRTAny) allocMap(0,SHIFT,allocNode());
}

PFRTAny	map_first(PFRTAny map) {
	PFRTAny	result = nil;
	PFRTIterator mi = iteratorFor(map);
	result = iteratorNext(mi);
	foidl_xdel(mi);
	result = (result == end) ? nil : ((PFRTMapEntry) result)->value;
	return result;
}

PFRTAny	map_second(PFRTAny map) {
	PFRTAny	result = nil;
	PFRTIterator mi = iteratorFor(map);
	result = iteratorNext(mi);
	if(result != end) {
		result = iteratorNext(mi);
	}
	foidl_xdel(mi);
	result = (result == end) ? nil : ((PFRTMapEntry) result)->value;	
	return result;
}

PFRTAny map_rest(PFRTAny src) {	
	PFRTAny result = (PFRTAny) empty_map;
	if(src->count > 1) {
		PFRTAny 		itm = nil;
		PFRTAny 		m1 = foidl_map_inst_bang();
		PFRTIterator	mi = iteratorFor(src);
		result = m1;
		iteratorNext(mi);
		while((itm = iteratorNext(mi)) != end) {
			PFRTMapEntry e = (PFRTMapEntry) itm;
			foidl_map_extend_bang(m1,e->key,e->value);
		}
		foidl_xdel(mi);
		uint32_t s1hsh = hash(map_first(src));
		m1->hash = src->hash - s1hsh;
		m1->count = src->count - 1;
	}
	return result;
}


PFRTAny map_print(PFRTIOChannel chn,PFRTAny map) {
	PFRTIterator mi = iteratorFor(map);
	PFRTAny entry;
	ft 		 mcount=((PFRTMap) map)->count;
	io_writeFuncPtr	cfn = chn->writehandler->fnptr;
	cfn(chn,lbrace);
	if(mcount > 0) {
		uint32_t count=0;
		--mcount;
		while ((entry = iteratorNext(mi)) != end) {			
			PFRTMapEntry e = (PFRTMapEntry) entry;
			cfn(chn,e->key);
			cfn(chn,comma);
			cfn(chn,e->value);
			if(mcount > count++) cfn(chn,comma);
			foidl_xdel(e);			
		}
		foidl_xdel(mi);
	} 
	cfn(chn,rbrace);
	return nil;
}

//	Use by compiler to understand literal metrics
localKeyword(flms_subtype,":subtype");
localKeyword(flms_inttype,":integer");
localKeyword(flms_strtype,":string");
localKeyword(flms_kwdtype,":keyword");
localKeyword(flms_fname,":fname");

localKeyword(flms_icnt,":intcnt");
localKeyword(flms_scnt,":strcnt");
localKeyword(flms_smlen,":strmlen");
localKeyword(flms_ucnt,":unkncnt");

PFRTAny foidl_literal_map_stats(PFRTAny m) {
	PFRTAny result = nil;
	uint32_t icnt;
	uint32_t skwcnt;
	uint32_t maxskwlen;
	uint32_t unknowncnt;
	icnt = skwcnt = maxskwlen = unknowncnt = 0;
	if(m->fclass == collection_class && m->ftype == map2_type && m->count > 0) {
		PFRTIterator mi = iteratorFor(m);
		PFRTAny entry;
		while((entry = iteratorNext(mi)) != end) {
			PFRTMapEntry e = (PFRTMapEntry) entry;
			PFRTAny stype = foidl_get(e->value,flms_subtype);
			if( foidl_equal_qmark(stype,flms_strtype) == true ||
				foidl_equal_qmark(stype,flms_kwdtype) == true ) {
				PFRTAny fname = foidl_get(e->value,flms_fname);
				if(fname->count > maxskwlen) 
					maxskwlen = fname->count;
				++skwcnt;
			}
			else if( foidl_equal_qmark(stype,flms_inttype) == true) {
				++icnt;
			}
			else 
				++unknowncnt;			
			
			foidl_xdel(e);			
		}
		foidl_xdel(mi);
		result = foidl_map_inst_bang();
		foidl_map_extend_bang(result,flms_icnt,allocIntegerWithValue(icnt));
		foidl_map_extend_bang(result,flms_scnt,allocIntegerWithValue(skwcnt));
		foidl_map_extend_bang(result,flms_smlen,allocIntegerWithValue(maxskwlen));
		foidl_map_extend_bang(result,flms_ucnt,allocIntegerWithValue(unknowncnt));
	}	
	return result;
}


PFRTAny coerce_to_map(PFRTAny mtemplate, PFRTAny src) {
	unknown_handler();
	return src;
}

PFRTAny foidl_key(PFRTAny me) {
	if(me->ftype == mapentry_type)
		return ((PFRTMapEntry) me)->key;
	else 
		unknown_handler();
	return nil;
}

PFRTAny foidl_value(PFRTAny me) {
	if(me->ftype == mapentry_type)
		return ((PFRTMapEntry) me)->value;
	else 
		unknown_handler();
	return nil;
}

void release_map(PFRTAny m) {
	unknown_handler();
}