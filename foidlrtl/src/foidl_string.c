/*
	foidl_string.c
	Library string type management
	
	Copyright Frank V. Castellucci
	All Rights Reserved
*/

#define STRING_IMPL
#include <foidlrt.h>
#include <string.h>
#include <stdio.h>

static PFRTAny strkwMap;

typedef PFRTAny (*_regskalloc)(char*);

void foildl_rtl_init_strings() {
	strkwMap = foidl_map_inst_bang();
}

static PFRTAny  registerMember(PFRTAny k,_regskalloc fn) {
	PFRTAny res = map_get(strkwMap,k);	
	if(res == nil) {		
		res = fn(k->value);
		foidl_map_extend_bang(strkwMap,res,res);
	}
	return res;
}

PFRTAny  foidl_reg_string(char *i) {
	FRTAny 	anyStr = {scalar_class,string_type,strlen(i),0,i};
	return registerMember(&anyStr, allocGlobalStringCopy);
}

PFRTAny  foidl_reg_keyword(char *i) {
	FRTAny 	anyKW = {scalar_class,keyword_type,strlen(i),0,i};
	return registerMember(&anyKW,allocGlobalKeywordCopy);
}

PFRTAny 	string_from_carray(char p[]) {
	return empty_string;
}

PFRTAny 	string_first(PFRTAny s) {
	return allocCharWithValue((ft) *((char *)s->value));	
}

PFRTAny 	string_second(PFRTAny s) {
	return allocCharWithValue((ft) ((char *)s->value)[1]);	
}

PFRTAny 	string_rest(PFRTAny s) {
	PFRTAny res = empty_string;		
	if(s != space_string) {
		char* v = (char *)s->value;
		res = allocStringWithCopyCnt(s->count,&v[1]);		
	}	
	return res;	
}

PFRTAny 	string_last(PFRTAny s) {
	PFRTAny res = nil;	
	if(s != empty_string)
		res = allocCharWithValue(((char *)s->value)[strlen((char *)s->value) - 1]);
	return res;	
}

//	Gets character at index

PFRTAny 	string_get(PFRTAny s, PFRTAny index) {
	PFRTAny res = nil;	
	if(strlen((char *)s->value) > (ft) index->value)
		res = allocCharWithValue(((char *)s->value)[(ft) index->value]);
	return res;	
}

//	Gets character at index, otherwise default

PFRTAny 	string_get_default(PFRTAny s, PFRTAny index,PFRTAny def) {
	PFRTAny res = nil;	
	if(s->count > (ft) index->value)
		res = allocCharWithValue(((char *)s->value)[(ft) index->value]);
	else {
		if(foidl_function_qmark(def) == true)
			res=dispatch2(def,s,index);
		else 
			res = def;
	}
	return res;	
}

PFRTAny 	string_extend(PFRTAny s, PFRTAny v) {	
	char 	 *p1=0;
	uint32_t bcnt=0;
	uint32_t tcnt = s->count;
	switch(v->ftype) {
		case 	character_type:
			{
			p1 = (char *) &v->value;
			tcnt += bcnt = 1;
			}
			break;
		case 	keyword_type:
		case 	string_type:
			{
			p1 = (char *) v->value;
			tcnt += bcnt = v->count;
			}
			break;
		case 	integer_type:
			{
				asprintf(&p1,"%lld",(ft) v->value);	   				
				tcnt += bcnt = strlen(p1);
			}
			break;
		case 	nil_type:
			p1 = (char *) nilstr->value;
			tcnt += bcnt = nilstr->count;			
			break;
		default:
			unknown_handler();
	}

	PFRTAny res = allocAndConcatString(tcnt,s->value,s->count,p1,bcnt);
	if(s->ftype == keyword_type)
		res->ftype = keyword_type;

	return res;
}

PFRTAny foidl_strcatnc(PFRTAny s, PFRTAny cnt, PFRTAny chc) {
	ft 		nlen = s->count + (uint32_t) cnt->value + 1;
	PFRTAny res = allocAny(scalar_class,string_type,(void *) nlen);
	res->count = nlen -1;
	res->value = strcpy(foidl_xall(nlen),s->value);	
	char 	val = (char) chc->value;
	for(ft i = 0; i< (ft) cnt->value; i++ ) ((char *)res->value)[s->count+i] = val;
	return res;
}

PFRTAny foidl_strcatns(PFRTAny s, PFRTAny cnt, PFRTAny chs) {
	ft 		nlen = s->count + ((ft) cnt->value * chs->count) + 1;
	PFRTAny res = allocAny(scalar_class,string_type,(void *) nlen);
	res->count = nlen -1;
	res->value = strcpy(foidl_xall(nlen),s->value);	
	char 	*str = (char*) chs->value;
	for(ft i = 0; i< (ft) cnt->value; i++ ) 
		strcat((char *)res->value,str);
	
	return res;
}

PFRTAny 	string_extend_bang(PFRTAny s, PFRTAny v) {
	PFRTAny sn = s;
	return sn;
}

PFRTAny 	string_update(PFRTAny s, PFRTAny index, PFRTAny v) {
	PFRTAny sn = s;
	if(v->fclass == scalar_class && v->ftype == character_type) {
		sn = allocStringWithCopy(s->value);				
		((char *) sn->value)[(uint32_t)index->value] = (char)v->value;
	}
	return sn;
}

PFRTAny 	string_update_bang(PFRTAny s, PFRTAny index, PFRTAny v) {
	PFRTAny sn = s;
	if(v->fclass == scalar_class && v->ftype == character_type ) {
		((char *) sn->value)[(uint32_t)index->value] = (char)v->value;		
	}
	return sn;
}


PFRTAny 	string_droplast(PFRTAny s) {
	if(s->count == 1)
		unknown_handler();
	PFRTAny sn = allocStringWithCopy(s->value);
	--sn->count;
	((char *) sn->value)[sn->count] = 0;	
	return sn;
}

PFRTAny 	string_droplast_bang(PFRTAny s) {
	if( s->count ) {
		((char *) s->value)[s->count-1] = 0;	
		--s->count;	
	}
	return s;
}

PFRTAny 	number_to_string(PFRTAny t) {
	return t;
}

PFRTAny 	char_to_string(PFRTAny t) {
	return t;
}

PFRTAny 	coerce_to_string(PFRTAny t) {
	return t;
}

PFRTAny 	release_string(PFRTAny s) {
	PFRTTypeG  rs = ANYTOG(s);
	if(rs->fsig == global_signature) 
		return s; 
	foidl_xdel(s->value);
	foidl_xdel(s);
	return nil;
}

PFRTAny 	foidl_hasChar(PFRTAny s, PFRTAny c) {
	return true;
}

PFRTAny 	foidl_collc2str(PFRTAny v) {
	ft 	nlen = v->count+1;
	PFRTAny res = allocAny(scalar_class,string_type,(void *) nlen);
	res->count = nlen -1;
	res->value = foidl_xall(nlen);
	PFRTIterator mi = iteratorFor(v);
	PFRTAny 	entry;
	uint32_t 	i = 0;
	while((entry = iteratorNext(mi)) != end) {
		((char *) res->value)[i] = (char) entry->value;
		++i;
		}
	
	return res;
}

PFRTAny 	foidl_string_from(PFRTAny s, PFRTAny d) {
	return s;
}

PFRTAny 	foidl_trim(PFRTAny s) {
	return s;
}