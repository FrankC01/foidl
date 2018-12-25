/*
	foidl_entrypoint.c
	Library main entry

	Copyright Frank V. Castellucci
	All Rights Reserved
*/

#define ENTRYPOINT_IMPL
#include <foidlrt.h>
#include <string.h>
#include <stdio.h>

static PFRTAny 	foidl_argv;
PFRTAny 		foidl_env;
static PFRTAny 	foidl_rtl_initialized = (PFRTAny) & _false.fclass;

//	RTL Entrypoint

static void genEnvMap(char **es) {
	foidl_env = foidl_map_inst_bang();
	while(*es) {
		char 	*estr = *es++;
		#ifdef _MSC_VER
		char 	*temp = _strdup(estr);
		#else
		char 	*temp = strdup(estr);
		#endif
		char 	*tk = strtok(temp,"=");
		char 	*tv = strtok(NULL,"=");
		if( tk != NULL && tv != NULL)
			foidl_map_extend_bang(foidl_env,
				allocGlobalString(tk),
				allocGlobalString(tv));
	}
}

PFRTAny 	foidl_rtl_init() {

	if(foidl_rtl_initialized == false) {
		//foidl_heap_setup();
		foidl_gc_init();
		foidl_rtl_init_chars();
		foidl_rtl_init_ints();
		foidl_rtl_init_globals();
		foildl_rtl_init_strings();
		foidl_rtl_init_types();
		foidl_rtl_init_series();
		foidl_rt_init_regex();

		foidl_rtl_initialized = true;
	}
	return true;
}

//	Application entry point

PFRTAny 	foidl_convert_mainargs(int argc, char **as, char **es) {
	genEnvMap(es);
	foidl_argv = vector_from_argv(argc,as);
	return foidl_argv;
}

PFRTAny 	foidl_system_getenv(PFRTAny el) {
	return map_get(foidl_env,el);
}

//
//	Interface Entry points
//

PFRTAny 	foidl_count(PFRTAny el) {
	if(foidl_extendable_qmark(el) == false)
		return nil;

	PFRTAny 	res=zero;
	if(el->fclass == collection_class || el->ftype == string_type || el->ftype == keyword_type)
		res = allocIntegerWithValue(el->count);
	else if(el->fclass == io_class)
		res = foidl_io_count(el);
	return res;
}

PFRTAny foidl_countto(PFRTAny coll, PFRTAny exp) {
	unknown_handler();
	PFRTAny 	res=zero;
	if(coll->fclass == io_class)
		res = foidl_io_countto(coll,exp);
	else
		unknown_handler();
	return res;
}

PFRTAny foidl_countnotto(PFRTAny coll, PFRTAny exp) {
	unknown_handler();
	PFRTAny 	res=zero;
	if(coll->fclass == io_class)
		res = foidl_io_countnotto(coll,exp);
	else
		unknown_handler();
	return res;
}


//	Gets the element from collection or string
//	for string: el == index
// 	for vector: el == index
//	for list: el == index
//	for map: el == key
//	for set: el == value

PFRTAny 	foidl_get(PFRTAny coll, PFRTAny el) {
	PFRTAny 	result = nil;
	if(foidl_extendable_qmark(coll) == false)
		return result;

	if(el != nil) {
		switch(coll->ftype) {
			case 	vector2_type:
				return vector_get(coll,el);
			case 	map2_type:
				return map_get(coll,el);
			case 	set2_type:
				return set_get(coll,el);
			case 	list2_type:
				return list_get(coll,el);
			case 	string_type:
				return string_get(coll,el);
			default:
				unknown_handler();
		}
	}
	return result;
}

//	Get with default

PFRTAny 	foidl_getd(PFRTAny coll, PFRTAny el, PFRTAny def) {
	PFRTAny 	result = nil;
	if(foidl_extendable_qmark(coll) == false)
		return result;
	if(el != nil) {

		switch(coll->ftype) {
			case 	vector2_type:
				result = vector_get_default(coll,el,def);
				break;
			case 	map2_type:
				result = map_get_default(coll,el,def);
				break;
			case 	set2_type:
				result = set_get_default(coll,el,def);
				break;
			case 	list2_type:
				result = list_get_default(coll,el,def);
				break;
			case 	string_type:
				result = string_get_default(coll,el,def);
				break;
			default:
				unknown_handler();
		}
	}
	return result;
}


PFRTAny foidl_indexof(PFRTAny coll, PFRTAny arg) {
	PFRTAny result = nil;
	if(foidl_extendable_qmark(coll) == false)
		return result;
	switch(coll->ftype) {
		case 	vector2_type:
			// result = vector_first(a);
			break;
		case 	set2_type:
			// result = set_first(a);
			break;
		case 	map2_type:
			// result = map_first(a);
			break;
		case 	list2_type:
			result = list_index_of(coll, arg);
			break;
		case 	string_type:
			// result = string_first(a);
			break;
	}
	return result;
}

//	Gets first element of collection or string

PFRTAny 	foidl_first(PFRTAny a) {
	PFRTAny result = nil;
	if(foidl_extendable_qmark(a) == false)
		return result;

	if(a->fclass == io_class)
		return foidl_io_first(a);

	switch(a->ftype) {
		case 	vector2_type:
			result = vector_first(a);
			break;
		case 	set2_type:
			result = set_first(a);
			break;
		case 	map2_type:
			result = map_first(a);
			break;
		case 	list2_type:
			result = list_first(a);
			break;
		case 	string_type:
			result = string_first(a);
			break;
	}
	return result;
}

//	Gets second element of collection or string

PFRTAny foidl_second(PFRTAny a) {
	PFRTAny result = nil;
	if(foidl_extendable_qmark(a) == false)
		return result;

	if(a->fclass == io_class)
		return foidl_io_second(a);

	switch(a->ftype) {
		case 	vector2_type:
			result = vector_second(a);
			break;
		case 	set2_type:
			result = set_second(a);
			break;
		case 	map2_type:
			result = map_second(a);
			break;
		case 	list2_type:
			result = list_second(a);
			break;
		case 	string_type:
			result =  string_second(a);
			break;
	}

	return result;
}

//	Gets all but first element of collection or string

PFRTAny 	foidl_rest(PFRTAny a) {
	PFRTAny result = a;
	if(foidl_extendable_qmark(a) == false)
		return result;

	if((foidl_collection_qmark(a) || a->ftype == string_type) &&
		a->count > 0) {
		switch(a->ftype) {
			case 	vector2_type:
				result = vector_rest(a);
				break;
			case 	set2_type:
				result = set_rest(a);
				break;
			case 	map2_type:
				result = map_rest(a);
				break;
			case 	list2_type:
				result = list_rest(a);
				break;
			case 	string_type:
				result =  string_rest(a);
				break;
		}
	}
	return result;
}

//	Gets last element of collection or string

PFRTAny 	foidl_last(PFRTAny a) {
	PFRTAny result = a;
	if((foidl_collection_qmark(a) || a->ftype == string_type) &&
		a->count > 0) {
		switch(a->ftype) {
			case 	vector2_type:
				unknown_handler();
				break;
			case 	set2_type:
				unknown_handler();
				break;
			case 	map2_type:
				unknown_handler();
				break;
			case 	list2_type:
				result =  list_last(a);
				break;
			case 	string_type:
				result =  string_last(a);
				break;
		}
	}
	return result;
}


//	Extend associates additional data into a collection
//	For vectors: Adds value to rear
//	For lists: Adds value to rear
//	For sets: Adds in associative structure
//	For maps: element is a 2 value container (k,v) to add
//  Note: For sets and maps if the element or key exist by
//	hash, it reverts to an update type operation

PFRTAny 	foidl_extendKV(PFRTAny coll, PFRTAny key, PFRTAny value) {
	PFRTAny res = nil;
	if(coll->ftype == map2_type)
		res = map_extend(coll,key,value);
	else
		unknown_handler();
	return res;
}

PFRTAny 	foidl_extendKV_bang(PFRTAny coll, PFRTAny key, PFRTAny value) {
	PFRTAny res = nil;
	if(coll->ftype == map2_type)
		res = foidl_map_extend_bang(coll,key,value);
	else
		unknown_handler();
	return res;
}

PFRTAny 	foidl_extend(PFRTAny coll, PFRTAny element) {
	PFRTAny result=nil;
	switch(coll->ftype) {
		case 	set2_type:
			result = set_extend(coll,element);
			break;
		case 	vector2_type:
			result = vector_extend(coll,element);
			break;
		case 	map2_type:
			if(element->fclass == collection_class && element->count == 2) {
				result =  map_extend(coll,
						foidl_first(element),
						foidl_second(element));
			}
			else {
				foidl_ep_excp(extend_map_two_arg);
			}
			break;
		case 	list2_type:
			result = list_extend(coll,element);
			break;
		case 	keyword_type:
		case 	string_type:
			result = string_extend(coll,element);
			break;
	}
	return result;
}

PFRTAny 	foidl_extend_bang(PFRTAny coll, PFRTAny element) {
	PFRTAny result=nil;

	switch(coll->ftype) {
		case 	set2_type:
			result = foidl_set_extend_bang(coll,element);
			break;
		case 	vector2_type:
			result = foidl_vector_extend_bang(coll,element);
			break;
		case 	map2_type:
			if(element->fclass == collection_class &&
				((PFRTCollection)element)->count % 2 == 0) {
				PFRTCollection c = (PFRTCollection) element;
				if(c->count == 2) {
					result = foidl_map_extend_bang(coll,
						foidl_first(element),
						foidl_second(element));
				}
				else
					unknown_handler();
			}
			else {
				unknown_handler();
			}
			break;
		case 	list2_type:
			result = list_push_bang(coll,element);
			break;
		case 	string_type:
			string_extend_bang(coll,element);
			break;
	}

	return result;
}

PFRTAny 	foidl_take(PFRTAny arg, PFRTAny coll) {
	return coll;
}

PFRTAny 	foidl_drop(PFRTAny arg, PFRTAny coll) {
	return coll;
}

PFRTAny 	foidl_drop_bang(PFRTAny coll, PFRTAny arg) {
	if(coll->ftype == vector2_type)
		return vector_drop_bang(coll,arg);
	else if (coll->ftype == list2_type)
		return list_drop_bang(coll, arg);
	else
		unknown_handler();
	return coll;
}

PFRTAny 	foidl_dropFor(PFRTAny coll, PFRTAny cnt) {
	return coll;
}

PFRTAny 	foidl_dropLast(PFRTAny coll) {
	if(coll->ftype == string_type)
		return string_droplast(coll);
	else
		unknown_handler();
	return coll;
}

PFRTAny 	foidl_droplast_bang(PFRTAny coll) {
	if(coll->ftype == string_type)
		return string_droplast_bang(coll);
	else if(coll->ftype == vector2_type)
		return vector_dropLast_bang(coll);
	else
		unknown_handler();
	return coll;
}

//	Update takes three args: Collection location and value
//	For all, value may be a function otherwise a new value
//	For vectors: location is a numeric index
//	For lists: location is a numeric index
//	For maps: location is a key value
//	For sets: location is a value
//	For all, v can be a value or an expression (Fn, Ln)

static PFRTAny validateIndexable(PFRTAny coll,PFRTAny indx) {
	PFRTAny 	isIndex = indx->ftype == integer_type ? true : false;
	if(coll->ftype == list2_type || coll->ftype == vector2_type
		|| coll->ftype == string_type || coll->ftype == keyword_type) {
		if(isIndex == false)
			foidl_ep_excp(update_not_integral);
		else if((ft) indx->value >= coll->count)
			foidl_ep_excp(index_out_of_bounds);
		}

	return isIndex;
}

PFRTAny 	foidl_update(PFRTAny coll, PFRTAny k, PFRTAny v) {
	PFRTAny 	result = nil;

	if(foidl_extendable_qmark(coll) == false)
		return result;
	validateIndexable(coll,k);

	PFRTAny 	finalValue =
		foidl_function_qmark(v) == false ? v : dispatch1(v, foidl_get(coll,k));
		switch(coll->fclass) {
			case 	collection_class:
				switch(coll->ftype) {
					case 	list2_type:
						result = list_update(coll,k,finalValue);
						break;
					case 	map2_type:
						result = map_update(coll,k,finalValue);
						break;
					case 	vector2_type:
						result = vector_update(coll,k,finalValue);
						break;
					case 	set2_type:
						unknown_handler();
						break;
				}
				break;
			case 	scalar_class:
				switch(coll->ftype) {
					case 	string_type:
						result = string_update(coll,k,finalValue);
						break;
				}
				break;
		}
	return result;
}


PFRTAny 	foidl_update_bang(PFRTAny coll, PFRTAny k, PFRTAny v) {
	PFRTAny 	result = nil;
	if(foidl_extendable_qmark(coll) == false)
		return result;
	validateIndexable(coll,k);
	PFRTAny 	finalValue =
		foidl_function_qmark(v) == false ? v : dispatch1(v, foidl_get(coll,k));
		switch(coll->fclass) {
			case 	collection_class:
				switch(coll->ftype) {
					case 	list2_type:
						result = list_update_bang(coll,k,finalValue);
						break;
					case 	map2_type:
						unknown_handler();
						break;
					case 	vector2_type:
						result = vector_update_bang(coll,k,finalValue);
						break;
					case 	set2_type:
						unknown_handler();
						break;
				}
				break;
			case 	scalar_class:
				switch(coll->ftype) {
					case 	string_type:
						result = string_update_bang(coll,k,finalValue);
						break;
				}
				break;
		}
	return result;
}

//	remove takes collection and key to remove
//	map: key
//	set: key
//	list: index
//	vector: index

PFRTAny 	foidl_remove(PFRTAny coll, PFRTAny key) {
	PFRTAny 	result = nil;
	if(foidl_extendable_qmark(coll) == false)
		return result;
	validateIndexable(coll,key);
	return coll;
}

//	removes may take a function as a key or collection of
//	elements to remove commiserate of collection type
//	map: Collection of keys
//	set: Collection of keys
//	list: collection of diminishing indexes.
//	vector: collection of dinishing indexes.
//	If key is a function, it expects to take 1 argument (coll element)
//	and return true to remove, false otherwise

PFRTAny 	foidl_removes(PFRTAny coll, PFRTAny key) {
	return coll;
}

PFRTAny 	foidl_empty_bang(PFRTAny coll) {
	switch(coll->ftype) {
		case 	list2_type:
			return empty_list_bang(coll);
			break;
		default:
			unknown_handler();
	}
	return coll;
}

//	Pop is only valid for:
//		lists, removes head
//		vector, removes tail

PFRTAny 	foidl_pop(PFRTAny coll) {
	switch(coll->ftype) {
		case 	list2_type:
			return list_pop(coll);
			break;
		case 	vector2_type:
			return vector_pop(coll);
			break;
		default:
			unknown_handler();
	}

	return coll;
}

PFRTAny 	foidl_pop_bang(PFRTAny coll) {
	switch(coll->ftype) {
		case 	vector2_type:
			return vector_pop_bang(coll);
			break;
		case 	list2_type:
			return list_pop_bang(coll);
			break;
		default:
			unknown_handler();
	}

	return coll;
}

// Push is only valid for lists, puts new root

PFRTAny foidl_push(PFRTAny coll, PFRTAny value) {
	if(coll->ftype == list2_type)
		return list_push(coll,value);
	else
		unknown_handler();
	return coll;
}

PFRTAny foidl_push_bang(PFRTAny coll,PFRTAny value) {
	if(coll->ftype == list2_type)
		return list_push_bang(coll,value);
	else
		unknown_handler();
	return coll;
}

// Reduced gens object to hopefully halt a reduction

PFRTAny 	foidl_reduced(PFRTAny el) {
	return allocAny(collection_class,reduced_type,(void *)el);
}


//	Applies fn to each element of coll. Fn must take 2 argument

PFRTAny 	foidl_apply(PFRTAny fn, PFRTAny coll) {
	return coll;
}

//	Map fn to each element of coll. Fn must take 1 argument
//	result of Fn are stored in vector

PFRTAny 	foidl_map(PFRTAny fn, PFRTAny coll) {
	return coll;
}

//
//	Reducers and folders
//

static void verifyFold(PFRTAny fn, PFRTAny fnerr, PFRTAny coll, PFRTAny collerr) {
	if(foidl_function_qmark(fn) == true) {
		if(foidl_collection_qmark(coll) == true || coll->ftype == string_type)
			return;
		else {
			foidl_ep_excp(collerr);
		}
	}
	else {
		foidl_ep_excp(fnerr);
	}
}

static PFRTAny 	reduction(PFRTAny fn, PFRTAny accum, PFRTIterator rI) {
	PFRTAny result = accum;
	PFRTAny iNext;
	while((iNext = iteratorNext(rI)) != end) {
		PFRTFuncRef2 fref = (PFRTFuncRef2) foidl_fref_instance(fn);
		foidl_imbue((PFRTAny) fref,result);
		result = foidl_imbue((PFRTAny) fref,iNext);
		deallocFuncRef2(fref);
		if(result->ftype == reduced_type) {
			PFRTAny redVal = result;
			result = (PFRTAny) redVal->value;
			foidl_xdel(redVal);
			break;
		}
	}
	foidl_xdel(rI);
	return result;
}

static PFRTAny 	reduction_bang(PFRTAny fn, PFRTAny accum, PFRTIterator rI) {
	PFRTAny result = accum;
	PFRTAny iNext;
	while((iNext = iteratorNext(rI)) != end) {
		PFRTFuncRef2 fref = (PFRTFuncRef2) foidl_fref_instance(fn);
		foidl_imbue((PFRTAny) fref,result);
		PFRTAny result2 = foidl_imbue((PFRTAny) fref,iNext);
		deallocFuncRef2(fref);
		if(result2->ftype == reduced_type) {
			PFRTAny redVal = result2;
			foidl_xdel(result);
			result = (PFRTAny) redVal->value;
			foidl_xdel(redVal);
			break;
		}
		else {
			foidl_xdel(result);
			result = result2;
		}
	}
	foidl_xdel(rI);
	return result;
}

PFRTAny 	foidl_fold(PFRTAny fn, PFRTAny accum, PFRTAny coll) {
	verifyFold(fn,fold_requires_function, coll, fold_requires_collection);
	return reduction(fn,accum,iteratorFor(coll));
}

PFRTAny 	foidl_fold_bang(PFRTAny fn, PFRTAny accum, PFRTAny coll) {
	verifyFold(fn,fold_requires_function, coll, fold_requires_collection);
	return reduction_bang(fn,accum,iteratorFor(coll));
}

globalFuncConst(str_builder,2,string_extend);

PFRTAny 	foidl_strb (PFRTAny a) {
	PFRTAny 	res = a;
	switch(a->fclass) {
		case 	scalar_class:
			switch(a->ftype) {
				case 	string_type:
					break;
				default:
					res = string_extend(empty_string,a);
					break;
			}
			break;
		case 	collection_class:
			res = foidl_fold_bang((PFRTAny) str_builder,empty_string,a);
			break;
		default:
			unknown_handler();
	}
	return res;
}
PFRTAny 	foidl_reduce(PFRTAny fn, PFRTAny coll) {
	verifyFold(fn,reduce_requires_function, coll, reduce_requires_collection);
	PFRTIterator  cI = iteratorFor(coll);
	return reduction(fn,iteratorNext(cI),cI);
}

//	Flatten takes any nested combinations of collections and
//	returns a vector of their contents

static PFRTAny 	internal_fold_flatten(PFRTAny accum, PFRTAny v);
localFunc(i_flat,2,internal_fold_flatten);

static PFRTAny 	internal_fold_flatten(PFRTAny accum, PFRTAny v) {
	if(foidl_collection_qmark(v) == true)
		return reduction((PFRTAny) i_flat,accum,iteratorFor(v));
	else if(v->ftype == mapentry_type)
		return internal_fold_flatten(
			internal_fold_flatten(accum,((PFRTMapEntry)v)->key),
			((PFRTMapEntry)v)->value);
	else
		return foidl_extend_bang(accum,v);
}

PFRTAny 	foidl_flatten(PFRTAny coll) {
	if(foidl_collection_qmark(coll) == true && coll->count != 0)
		return foidl_fold((PFRTAny) i_flat,foidl_vector_inst_bang(),coll);
	else
		return (PFRTAny) empty_vector;
}

//	Coercion routine

PFRTAny 	foidl_coerce(PFRTAny type, PFRTAny data) {
	return data;
}

PFRTAny 	foidl_failWith(PFRTAny str) {
	return nil;
}

PFRTAny 	foidl_split(PFRTAny s, PFRTAny d) {
	return s;
}

//	Math routines

PFRTAny foidl_add(PFRTAny arg1, PFRTAny arg2) {
	if(arg1->ftype == integer_type && arg2->ftype == integer_type)
		return foidl_add_ints(arg1,arg2);
	else
		unknown_handler();
	return nil;
}

PFRTAny foidl_sub(PFRTAny arg1, PFRTAny arg2) {
	if(arg1->ftype == integer_type && arg2->ftype == integer_type)
		return foidl_sub_ints(arg1,arg2);
	else
		unknown_handler();
	return nil;
}

PFRTAny foidl_div(PFRTAny arg1, PFRTAny arg2) {
	if(arg1->ftype == integer_type && arg2->ftype == integer_type)
		return foidl_div_ints(arg1,arg2);
	else
		unknown_handler();
	return nil;
}

PFRTAny foidl_mul(PFRTAny arg1, PFRTAny arg2) {
	if(arg1->ftype == integer_type && arg2->ftype == integer_type)
		return foidl_mul_ints(arg1,arg2);
	else
		unknown_handler();
	return nil;
}

PFRTAny foidl_mod(PFRTAny arg1, PFRTAny arg2) {
	if(arg1->ftype == integer_type && arg2->ftype == integer_type)
		return foidl_mod_ints(arg1,arg2);
	else
		unknown_handler();
	return nil;
}
PFRTAny foidl_inc(PFRTAny el) {
	if(el->ftype == integer_type)
		return foidl_add(el,one);
	else
		unknown_handler();
	return nil;
}

PFRTAny foidl_dec(PFRTAny el) {
	if(el->ftype == integer_type)
		return foidl_sub(el,one);
	else
		unknown_handler();
	return nil;
}