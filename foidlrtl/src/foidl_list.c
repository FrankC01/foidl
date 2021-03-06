/*
	foidl_list.c
	Library support for List

	Copyright Frank V. Castellucci
	All Rights Reserved
*/

/*
	Using first and rest (1 and many) vs. first and list (many and 1)
	Regardless, first and rest are linked by first
	push - Insert a new first
		take original first and set it to rest
		insert new first with link to old rest
	pop - Removes first
		Copy first rest and link to remainder
		Make that first
	append/extend - Add to end
		copying required?
	drop - same as pop
*/

#define  LIST_IMPL

#include <foidlrt.h>

const struct FRTLinkNodeG _empty_link = {
	global_signature,
	collection_class,
	linknode_type,
	(PFRTAny) &_end.fclass,
	(PFRTLinkNode) (PFRTAny) &_end.fclass
};

PFRTLinkNode const empty_link = (PFRTLinkNode) &_empty_link.fclass;

const struct FRTListG _empty_list = {
	global_signature,
	collection_class,
	list2_type,
	0,
	0,
	empty_link,
	empty_link
};

PFRTList const empty_list = (PFRTList) &_empty_list.fclass;

PFRTAny 	list_pop_bang(PFRTAny l) {
	PFRTList 	list = (PFRTList) l;
	if( list->count > 0 ) {
		PFRTLinkNode oldNode = list->root;
		uint32_t 	keyhash = hash(oldNode->data);
		list->hash -= keyhash;
		--list->count;
		if( list->count == 0 ) {
			list->root = list->rest = empty_link;
		}
		else if( list->count == 1 )
			list->root = list->rest;
		else {
			list->root = oldNode->next;
			list->rest = list->root->next;
		}

		if(oldNode != empty_link) foidl_xdel(oldNode);
	}
	return l;
}

PFRTAny 	list_pop(PFRTAny l) {
	PFRTList oldList = (PFRTList) l;
	PFRTList newList = allocList(oldList->count,oldList->rest);

	if(newList->count == 0) {
		return (PFRTAny) newList;
	}
	else {
		newList->hash = oldList->hash;
		newList->hash -= hash(oldList->root->data);
		--newList->count;
		if(newList->count)
			newList->rest = newList->root->next;
	}
	return (PFRTAny) newList;
}

PFRTAny  list_drop_bang(PFRTAny src,PFRTAny cnt) {
	PFRTList pv = (PFRTList) src;
	ft val = 0;
	if(cnt->ftype != number_type)
		unknown_handler();
	val = number_toft(cnt);
	if(pv->count >= val) {
		for(ft x = 0; x < val; ++x)
			list_pop_bang(src);
	}
	return src;
}

static PFRTAny 	list_prepend_bang(PFRTAny l, PFRTAny v) {
	uint32_t 	keyhash = hash(v);
	PFRTLinkNode newNode = allocLinkNodeWith(v,empty_link);
	PFRTList 	list =
		(PFRTList) l == empty_list ? (PFRTList) allocList(0,empty_link)
		: (PFRTList) l;

	// If count == 0, make newNode root
	if(list->count > 0) {
		newNode->next = list->rest = list->root;
	}
	list->root = newNode;
	list->hash += keyhash;
	++list->count;
	return (PFRTAny) list;
}

PFRTAny 	list_extend(PFRTAny l, PFRTAny v) {
	// If empty: list->root = newNode, newNode->next = empty_link
	// list->rest = empty_link
	if((PFRTList) l == empty_list) {
		return list_prepend_bang(l,v);
	}
	PFRTList 		oldList = (PFRTList) l;
	uint32_t 		keyhash = hash(v);
	PFRTLinkNode 	newNode = allocLinkNodeWith(v,empty_link);
	PFRTList 		newList = allocList(oldList->count,newNode);

	newList->root->next = newList->rest = oldList->root;
	newList->hash = oldList->hash + keyhash;
	++newList->count;

	return (PFRTAny) newList;
}

PFRTAny 	list_push_bang(PFRTAny l, PFRTAny v) {
	return list_prepend_bang(l,v);
}

PFRTAny 	list_push(PFRTAny l, PFRTAny v) {
	return list_extend(l,v);
}

//	This is for constructors only that preserve
//	ordering as declared, all othr operations for list
//	generally do so at the head

PFRTAny 	foidl_list_extend_bang(PFRTAny l, PFRTAny v) {
	uint32_t 	keyhash = hash(v);
	PFRTLinkNode newNode = allocLinkNodeWith(v,empty_link);
	PFRTList 	list =
		(PFRTList) l == empty_list ? (PFRTList) allocList(0,empty_link)
		: (PFRTList) l;

	// If count == 0, make newNode root
	if(list->count == 0) {
		list->root = newNode;
	}
	else if(list->count == 1) {
		list->root->next = list->rest = newNode;
	}
	//	Otherwise, the new node is 'appended' to end
	else {
		PFRTLinkNode lnode = list->rest;
		while(lnode->next != empty_link)
			lnode = lnode->next;
		lnode->next = newNode;
	}

	list->hash += keyhash;
	++list->count;
	return (PFRTAny) list;
}

PFRTAny 	foidl_list_inst_bang() {
	return (PFRTAny) allocList(0,empty_link);
}

PFRTAny 	list_update(PFRTAny l, PFRTAny i, PFRTAny v) {
	PFRTList 	newList = empty_list;
	if(i->ftype != number_type)
		unknown_handler();
	ft 	index = number_toft(i);
	//	If in range
	if(l->count > 0 && index < l->count) {
		PFRTList 	 	oldList = (PFRTList) l;
		PFRTLinkNode 	newNode = allocLinkNodeWith(v,empty_link);
		uint32_t 		vhash = hash(v);
		newList = allocList(l->count,empty_link);

		//	0 (head) location

		if(index == 0) {
			newList->root = newNode;
		 	newList->root->next = newList->rest = oldList->rest;
		 	newList->hash = (oldList->hash - hash(oldList->root->data))+vhash;
		 }
		 else {
		 	PFRTLinkNode rnode =
		 		allocLinkNodeWith(oldList->root->data,empty_link);
		 	PFRTLinkNode node = oldList->root->next;
		 	newList->root = rnode;
		 	for(uint32_t i=1; i<index;i++) {
		 		rnode->next = allocLinkNodeWith(node->data,empty_link);
		 		rnode = rnode->next;
		 		node = node->next;
		 	}
		 	//	Node should be one after replaced
		 	rnode->next = newNode;
		 	newNode->next = node->next;
		 	newList->rest = newList->root->next;
		 	newList->hash = (oldList->hash - hash(node->data))+vhash;
		 }
	}
	else
		unknown_handler();
	return (PFRTAny) newList;
}

static PFRTLinkNode getListLinkNode(PFRTList list, ft indx) {
	PFRTLinkNode p  = empty_link;
	if(indx == 0)
		p = list->root;
	else {
		p = list->root->next;
		ft i=1;
		while(i<indx) {
			p = p->next;
			i++;
		}
	}
	return p;
}

PFRTAny list_get(PFRTAny l, PFRTAny index) {
	if(index->ftype == number_type) {
		return getListLinkNode((PFRTList) l, number_toft(index))->data;
	}
	else {
		unknown_handler();
	}
	return nil;
}

PFRTAny list_index_of(PFRTAny list, PFRTAny el) {
	PFRTIterator li = iteratorFor(list);
	PFRTAny 	result = nil;
	PFRTAny 	entry = nil;
	ft 		 	mcount=((PFRTList) list)->count;
	if(mcount > 0) {
		uint64_t count=0;
		--mcount;
		while((entry = iteratorNext(li)) != end) {
			if(foidl_equal_qmark(entry,el) == true) {
				result = foidl_reg_intnum(count);
				break;
			}
			++count;
		}
		foidl_xdel(li);
	}
	return result;
}

PFRTAny list_get_default(PFRTAny l, PFRTAny index, PFRTAny def) {
	PFRTAny  result = nil;
	PFRTList list = (PFRTList) l;
	if(index->ftype == number_type) {
		ft val = number_toft(index);
		if(list->count > val) {
			if(val == 0)
				result = list->root->data;
			else {
				PFRTLinkNode p = list->root->next;
				ft i=1;
				while(i<val) {
					p = p->next;
					i++;
				}
				result = p->data;
			}
		}
		else {
			if(foidl_function_qmark(def) == true)
				result = dispatch2(def,l,index);
			else
				result = def;
		}
	}
	else {
		//	Not an index
		unknown_handler();
	}

	return result;
}

PFRTAny 	list_update_bang(PFRTAny l, PFRTAny i, PFRTAny v) {
	PFRTList list = (PFRTList) l;
	if(i->ftype != number_type)
		unknown_handler();

	getListLinkNode(list, number_toft(i))->data = v;
	return l;
}

PFRTAny	list_first(PFRTAny l) {
	PFRTList list = (PFRTList) l;
	if( list->count == 0)
		return nil;
	else
		return list->root->data;
}

PFRTAny	list_second(PFRTAny l) {
	PFRTList list = (PFRTList) l;
	if( list->count < 2)
		return nil;
	else
		return list->root->next->data;
}

PFRTAny 	list_last(PFRTAny l) {
	PFRTList list = (PFRTList) l;
	ft 		 lcnt = list->count;
	if(lcnt == 0)
		return nil;
	--lcnt;
	return getListLinkNode(list, lcnt)->data;
}

// Takes all but the last element in the
// list

PFRTAny list_droplast(PFRTAny src) {
	if( src == (PFRTAny) empty_list)
		return src;
	else {
		PFRTList l1 = (PFRTList) foidl_list_inst_bang();
		PFRTList list = (PFRTList) src;
		ft 		 lcnt = list->count - 1;
		ft 		 count = 0;
		while(count < lcnt) {
			foidl_list_extend_bang((PFRTAny) l1,
				getListLinkNode(list, count)->data);
			++count;
		}
		return (PFRTAny) l1;
	}
}

PFRTAny list_rest(PFRTAny src) {
	PFRTAny result = (PFRTAny) empty_list;
	if(src->count > 1) {
		PFRTList l1 = (PFRTList) foidl_list_inst_bang();
		result = (PFRTAny) l1;
		l1->root = ((PFRTList) src)->rest;
		l1->rest = l1->root->next;

		uint32_t s1hsh = hash(list_first(src));
		l1->hash = src->hash - s1hsh;
		l1->count = src->count - 1;
	}
	return result;
}

PFRTAny write_list(PFRTAny channel, PFRTAny list, channel_writer writer) {
	PFRTIterator li = iteratorFor(list);
	PFRTAny entry = nil;
	writer(channel,lbracket);
	ft 		 mcount=((PFRTList) list)->count;
	if(mcount > 0) {
		uint32_t count=0;
		--mcount;
		while((entry = iteratorNext(li)) != end) {
			writer(channel,entry);
			if(mcount > count++) writer(channel,comma);
		}
		foidl_xdel(li);
	}

	writer(channel,rbracket);
	return nil;
}

PFRTAny coerce_to_list(PFRTAny stemplate, PFRTAny src) {
	unknown_handler();
	return src;
}

PFRTAny empty_list_bang(PFRTAny s) {
	PFRTList list;
	if (s->ftype != list2_type)
		unknown_handler();
	else
		list = (PFRTList) s;

	while(list->count > 0)
		list_pop_bang(s);
	return s;
}

PFRTAny release_list_bang(PFRTAny s) {
	foidl_xdel(empty_list_bang(s));
	return nil;
}