/*
	foidl_regex.c
	Library support for regular expressions

	Copyright Frank V. Castellucci
	All Rights Reserved
*/

#define REGEX_IMPL
#include <foidlrt.h>
#include <foidl_regex11.hpp>
#include <stdio.h>

// Symbol compiled regex

constKeyword(unknownKW,":unknown");
constKeyword(typeKW,":type");
constKeyword(regexKW,":regex");
constKeyword(errorKW,":error");
constKeyword(strKW,":string");
constKeyword(token_typeKW,":token_type");
constKeyword(token_strKW,":token_str");
constKeyword(linenoKW,":lineno");
constKeyword(colnoKW,":colno");


// Take input foidl string and return foidl_regex type

PFRTAny	foidl_regex(PFRTAny s) {
	if(s->fclass == scalar_class && s->ftype == string_type) {
		return allocRegex(s,_string_to_regex(s->value));
	}
	else {
		printf("Skipped for pattern processing 0x%08llx\n",s->ftype);
	}
	return nil;
}


PFRTAny 	foidl_split(PFRTAny s, PFRTAny pattern) {
	if(s->ftype != string_type || pattern->ftype != regex_type)
		unknown_handler();

	PFRTAny rlist = foidl_list_inst_bang();
	PFRTRegEx rex = (PFRTRegEx) pattern;
	_string_split(rlist, s->value, rex->regex);
	return rlist;
}


PFRTAny foidl_regex_match_qmark(PFRTAny s, PFRTAny pattern) {
	// If s is, indeed, a string
	if(s->fclass == scalar_class && s->ftype == string_type) {
		// And pattern is a string
		if(pattern->fclass == scalar_class && pattern->ftype == string_type) {
			if (_is_match(s->value, pattern->value)) return true;
		}
		// Otherwise if pattern is a regex
		else if(pattern->fclass == scalar_class && pattern->ftype == regex_type) {
			PFRTRegEx rex = (PFRTRegEx) pattern;
			if (_is_matchp(s->value, rex->regex)) return true;
		}
		else {
			return false;
		}
	}
	return false;
}

PFRTAny 	foidl_format(PFRTAny s, PFRTAny coll) {
	if(s->ftype == string_type &&
		(coll->ftype == vector2_type || coll->ftype == list2_type)) {
		if(coll->count == 0)
			return s;
		PFRTAny entry = nil;
		PFRTIterator li = iteratorFor(coll);
		char **sarray  = foidl_xall(coll->count * sizeof(ft));
		char *delarray = foidl_xall(coll->count * sizeof(int));
		int count  = 0;
		int delcnt = 0;
		while((entry = iteratorNext(li)) != end) {
			switch(entry->ftype) {
				case 	string_type:
				case 	keyword_type:
					sarray[count] = entry->value;
					break;
				case 	regex_type:
					sarray[count] = ((PFRTRegEx) entry)->value->value;
					break;
				case 	character_type:
					{
						char *p = foidl_xall(entry->count + 1);
						strncpy(p, (char *)entry->value, entry->count);
						sarray[count] = p;
						delarray[delcnt]=count;
						++delcnt;
					}
					break;
				case integer_type:
					{
						char *p = foidl_xall(50);
						sprintf(p, "%lld", (ft) entry->value );
						sarray[count] = p;
						delarray[delcnt]=count;
						++delcnt;
					}
					break;
				case nil_type:
					sarray[count] = nilstr->value;
					break;
				case boolean_type:
					if(((ft) entry->value) == 1)
						sarray[count] = truestr->value;
					else
						sarray[count] = falsestr->value;
					break;
				default:
					unknown_handler();
			}

			count++;
		}
		PFRTAny result = _format_string((const char*) s->value, sarray, count);
		for(int x = 0; x < delcnt; x++)
			foidl_xdel(sarray[delarray[x]]);
		foidl_xdel(sarray);
		foidl_xdel(delarray);
		return result;
	}
	else {
		unknown_handler();
	}

	return s;
}

PFRTAny 	foidl_format_bang(PFRTAny s, PFRTAny coll) {
	PFRTAny result = foidl_format(s, coll);
	if(coll->ftype == list2_type && coll != empty_list)
		release_list_bang(coll);
	return result;
}

static void gen_block(PFRTAny patterns,PFRTAny ignores, token_block *block) {
	ft pcnt = ((PFRTList) patterns)->count;
	ft icnt = ((PFRTList) ignores)->count;

	PFRTAny entry = nil;
	PFRTAny ttype;
	PFRTRegEx tregex;
	PFRTAny  spare;
	if(pcnt > 0) {
		block->pattern_cnt = pcnt;
		block->regex_array = foidl_xall(pcnt * sizeof(ft));
		block->type_array = foidl_xall(pcnt * sizeof(ft));
		uint32_t count=0;
		PFRTIterator li = iteratorFor(patterns);
		while((entry = iteratorNext(li)) != end) {
			ttype = foidl_get(entry, typeKW);
			// spare = foidl_get(entry, regexKW);
			tregex = (PFRTRegEx)foidl_get(entry, regexKW);
			// printf("Type in pattern processing 0x%08x\n",spare->ftype);
			block->regex_array[count] = tregex->regex;
			block->type_array[count] = (char *) ttype->value;
			count++;
		}
		foidl_xdel(li);
	}
	else {
		block->pattern_cnt = 0;
	}
	if(icnt > 0) {
		block->ignore_cnt = icnt;
		block->ig_regex_array = foidl_xall(icnt * sizeof(ft));
		uint32_t count=0;
		PFRTIterator li = iteratorFor(ignores);
		while((entry = iteratorNext(li)) != end) {
			tregex = (PFRTRegEx)foidl_get(entry, regexKW);
			block->ig_regex_array[count] = tregex->regex;
			count++;
		}
		foidl_xdel(li);
	}
	else {
		block->ignore_cnt = 0;
	}

	return;
}

PFRTAny foidl_tokenize(PFRTAny s, PFRTAny patterns, PFRTAny ignores) {
	// 1. Get size of list and create array(s) of pointers for
	//	the patterns and ignores
	// 2. Verify or create regexes for each pattern and ignore
	// 3. call underylying regex searches
	// 4. Convert the return
	PFRTAny mylist = empty_list;
	if( s == empty_string || s->count < 1)
		return mylist;

	token_block	block;
	block.token_cnt = 0;
	block.tokens = 0;
	gen_block(patterns, ignores, &block);
	_reduce_tokens(s->value,  &block);
	// Drop the ignores array
	if(block.ignore_cnt > 0) {
		foidl_xdel(block.ig_regex_array);
	}
	if(block.token_cnt > 0) {
		mylist = foidl_list_inst_bang();
		for(int i=0; i<block.token_cnt;i++) {
			PFRTAny mymap = foidl_map_inst_bang();
			ptoken res = block.tokens[i];
			foidl_map_extend_bang(
				mymap,
				token_strKW,
				(PFRTAny) res->word);
			foidl_map_extend_bang(
				mymap,
				linenoKW,
				allocIntegerWithValue((long long) res->lineno));
			foidl_map_extend_bang(
				mymap,
				colnoKW,
				allocIntegerWithValue((long long) res->colno));
			if(res->type_index == -1) {
				foidl_map_extend_bang(
					mymap,
					token_typeKW,
					strKW);
			}
			else if(res->type_index == -2) {
				foidl_map_extend_bang(
					mymap,
					token_typeKW,
					errorKW);
			}
			else {
				PFRTAny index = allocIntegerWithValue((long long) res->type_index);
				PFRTAny lmap = foidl_get(patterns, index);
				PFRTAny tkw = foidl_get(lmap, typeKW);
				foidl_map_extend_bang(mymap,token_typeKW,tkw);
			}
			foidl_list_extend_bang(mylist,mymap);
			// Drop the token structure from the C++ allocation
			foidl_xdel(block.tokens[i]);
		}
		// Drop the token list from the C++ allocation
		foidl_xdel(block.tokens);

	}
	// Drop the regex and type arrays
	if(block.pattern_cnt > 0) {
		foidl_xdel(block.regex_array);
		foidl_xdel(block.type_array);
	}

	return mylist;
}


PFRTAny foidl_regex_cmp_qmark(PFRTAny pattern, PFRTAny arg) {
	PFRTAny res = false;
	// int result;
	// regex_t regpat;
	// result = regcomp(&regpat,pattern->value,REG_NOSUB | REG_EXTENDED);
	// if(!result) {
	// 	if(!regexec(&regpat,arg->value, 0, NULL, 0))
	// 		res = true;
	// 	regfree(&regpat);
	// }
	return res;
}

void foidl_rt_init_regex() {
}
