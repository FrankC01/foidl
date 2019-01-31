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

#ifdef _MSC_VER
#endif // _MSC_VER

static PFRTAny strMap;
static PFRTAny kwMap;

// TODO: Separate strings from keywords

typedef PFRTAny (*_regskalloc)(char*);

void foildl_rtl_init_strings() {
	strMap = foidl_map_inst_bang();
	kwMap = foidl_map_inst_bang();
}

/*
	Utility to replace format characters with actual types
*/

static PFRTAny replace_with(char el) {
	switch(el) {
		case 't':
			return tbchr;
		case 'q':
			return sqchr;
		case 'd':
			return dqchr;
		case 'n':
			return nlchr;
	}
	return nil;
}

static char* escape_scan(char *i) {

	if (strchr(i,'`') != NULL) {
		char *dupe = foidl_xall(strlen(i)+1);
		int s = 0;
		int d = 0;
		while(i[s] != 0) {
			if(i[s] == '`' && i[s-1] != '\\' && (i[s+1] != 0 || i[s+1] != ' ')) {
				PFRTAny rep = replace_with(i[s+1]);
				if(rep != nil) {
					dupe[d] = ((unsigned char*) (&rep->value))[0];
					++d;
					if(rep->count == 2) {
						dupe[d] = ((unsigned char*) (&rep->value))[1];
						++d;
					}
				}
				s+=2;
			}
			else {
				dupe[d] = i[s];
				++d;
				++s;
			}
		}
		return dupe;
	}
	else {
		return (char *) NULL;
	}
}

char* escape_descan(char *i) {
	int32_t len = strlen(i);
	char 	*scratch = foidl_xall(len*2);
	char 	*result = i;
	int32_t pos=0;
	int32_t opos=0;
	int32_t hit=0;
	while(i[pos] != 0) {
		if(i[pos] == 0x0d) {
			++hit;
			scratch[opos++] = '`';
			scratch[opos] = 'n';
			if(i[pos+1] == 0x0a) {
				printf("	and then 0x0a\n");
				pos += 1;
			}
		}
		else if(i[pos] == 0x0a) {
			++hit;
			scratch[opos++] = '`';
			scratch[opos] = 'n';
			if(i[pos+1] == 0x0d) {
				printf("	and then 0x0d\n");
				pos += 1;
			}
		}
		else
			scratch[opos] = i[pos];
		pos += 1;
		++opos;
	}
	if(hit != 0) {
		scratch[opos] = 0;
		result = foidl_xall(strlen(scratch)+1);
		strcpy(result, scratch);
	}
	foidl_xdel(scratch);

	return result;
}

/*
	Takes a string and descans embedded
	nl/cr characters
*/

PFRTAny	foidl_descan_string(PFRTAny s) {
	PFRTAny news = s;
	char *result = escape_descan(s->value);
	if(result != s->value) {
		news = allocAny(scalar_class, string_type, result);
		news->count = strlen(result);
	}
	return news;
}

/*
	Registers strings in map, expand any control
	statements and eliminate duplicates
*/

PFRTAny  foidl_reg_string(char *i) {
	FRTAny 	anyStr = {scalar_class,string_type,strlen(i),0,i};
	PFRTAny res = map_get(strMap,&anyStr);
	if(res == nil) {
		PFRTAny sval = nil;
		PFRTAny skey = allocGlobalStringCopy(anyStr.value);
		char *es = escape_scan(i);
		if(es != NULL) {
			sval = allocGlobalString(es);
		}
		else {
			sval = skey;
		}
		foidl_map_extend_bang(strMap,skey,sval);
		res = sval;
	}
	return res;
}

/*
	Registers keywords in map and eliminate duplicates
*/

PFRTAny  foidl_reg_keyword(char *i) {
	FRTAny 	anyKW = {scalar_class,keyword_type,strlen(i),0,i};
	PFRTAny res = map_get(kwMap,&anyKW);
	if(res == nil) {
		res = allocGlobalStringCopy(anyKW.value);
		foidl_map_extend_bang(kwMap,res,res);
	}
	return res;
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
	if(index->ftype != number_type)
		unknown_handler();
	ft val = number_toft(index);
	if(strlen((char *)s->value) > val)
		res = allocCharWithValue(((char *)s->value)[val]);
	return res;
}

//	Gets character at index, otherwise default

PFRTAny 	string_get_default(PFRTAny s, PFRTAny index,PFRTAny def) {
	PFRTAny res = nil;
	ft 	    val = 0;
	if(index->ftype == number_type) {
		val = number_toft(index);
	}
	else {
		unknown_handler();
	}
	if(s->count > val)
		res = allocCharWithValue(((char *)s->value)[val]);
	else {
		if(foidl_function_qmark(def) == true)
			res=dispatch2(def,s,index);
		else
			res = def;
	}
	return res;
}

// Extend a string
// TODO: Possibly leverage 'format'

PFRTAny 	string_extend(PFRTAny s, PFRTAny v) {
	char 	 *p1=0;
	int 	 delp = 0;
	uint32_t bcnt=0;
	uint32_t tcnt = s->count;
	switch(v->ftype) {
		// Should checck for control characters (tab, nl, quote, dbl quote)
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
		case 	number_type:
			{
				p1 = number_tostring(v);
				delp = 1;
				tcnt += bcnt = strlen(p1);
			}
			break;
		// Does this make sense?
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
	if(delp == 1)
		foidl_xdel(p1);

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

PFRTAny 	release_string(PFRTAny s) {
	PFRTTypeG  rs = ANYTOG(s);
	if(rs->fsig == global_signature)
		return s;
	foidl_xdel(s->value);
	foidl_xdel(s);
	return nil;
}

