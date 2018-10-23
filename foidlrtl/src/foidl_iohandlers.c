/*
	foidl_iohandlers.c
	Library string type management

	Copyright Frank V. Castellucci
	All Rights Reserved
*/

#define IOHANDLERS_IMPL
#include <foidlrt.h>
#include <stdio.h>

#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif

#include <unistd.h>

const char 	*trueStr = "true";
const char 	*falseStr = "false";
const char  *nilStr = "nil";
const char  *endStr = "end";
const char  *fnStr = "Fn reference";
const char  *chanStr = "Channel reference";
const char  *infSeriesStr = "Infinite series";
const char  *seriesStr = "Series reference";

typedef struct Counters {
	uint32_t 	processed;
	uint32_t 	line;
	uint32_t 	pos;
} *PCounters;

#define counter(lhs, mb) \
	struct Counters lhs = {0,mb->current_line,mb->current_pos};

static uint32_t isCharWS_qmark(char v) {
	return v == 0x20 ? 1 : v == 0x09 ? 1 : v == 0x0D ? 1 : 0;
}

//	Buffered readers

PFRTAny io_byteReader(PFRTIOChannel chn) {
	return nil;
}

PFRTAny io_nextByteReader(PFRTIterator chn) {
	return nil;
}

PFRTAny charBlockBuffRead(PFRTIOBuffer b) {
	PFRTAny result = nil;
	if(b->current_read_offset < b->max_read_position) {
		char v = b->bufferPtr[b->current_read_offset];
		switch(v) {
			case 0x0A:
			{
				++b->current_line;
				b->current_pos = 0;
			}
			break;
			case 0x009:
				b->current_pos += 4;
				break;
			default:
				++b->current_pos;
				break;
		}
		result = allocCharWithValue((ft) v);
		b->previous_read_offset = b->current_read_offset;
		++b->current_read_offset;
	}
	return result;
}

PFRTAny charMMapBuffRead(PFRTIOBuffer b) {
	PFRTAny result = nil;
	if(b->current_read_offset < b->max_read_position) {
		char v = b->bufferPtr[b->current_read_offset];
		switch(v) {
			case 0x0A:
			{
				++b->current_line;
				b->current_pos = 0;
			}
			break;
			case 0x009:
				b->current_pos += 4;
				break;
			default:
				++b->current_pos;
				break;
		}
		result = allocCharWithValue((ft) v);
		b->previous_read_offset = b->current_read_offset;
		++b->current_read_offset;
	}
	return result;
}

static uint32_t countMMapBufferToChar(PFRTIOBuffer b, char c, PCounters pc) {
	char *p = &b->bufferPtr[b->current_read_offset];

	for(char v = *p;
		v != c &&
			(b->current_read_offset + pc->processed) < b->max_read_position;
		++pc->processed) {
		switch(v) {
			case 0x0A:
			{
				++pc->line;
				pc->pos = 0;
			}
			break;
			case 0x009:
				pc->pos += 4;
				break;
			default:
				++pc->pos;
				break;
		}
		++p;
		v = *p;
	}
	return pc->processed;
}

static uint32_t countMMapBufferWhileChar(PFRTIOBuffer b, char c, PCounters pc) {
	char *p = &b->bufferPtr[b->current_read_offset];
	uint32_t processed = 0;
	for(char v = *p;
		v == c &&
			(b->current_read_offset + processed) < b->max_read_position;
		++pc->processed) {
		switch(v) {
			case 0x0A:
			{
				++pc->line;
				pc->pos = 0;
			}
			break;
			case 0x009:
				pc->pos += 4;
				break;
			default:
				++pc->pos;
				break;
		}
		++p;
		v = *p;
	}
	return pc->processed;
}

static uint32_t countMMapBufferToWhen(PFRTIOBuffer b, PFRTAny c,PCounters pc) {
	char *p = &b->bufferPtr[b->current_read_offset];
	PFRTAny v = allocCharWithValue(*p);
	while(set_get(c,v) == nil) {
		switch(*p) {
			case 0x0A:
			{
				++pc->line;
				pc->pos = 0;
			}
			break;
			case 0x009:
				pc->pos += 4;
				break;
			default:
				++pc->pos;
				break;
		}
		++pc->processed;
		v = allocCharWithValue( *(++p));
	}
	return pc->processed;
}

static uint32_t countMMapBufferWhile(PFRTIOBuffer b, PFRTAny c,PCounters pc) {
	char *p = &b->bufferPtr[b->current_read_offset];
	PFRTAny v = allocCharWithValue(*p);
	while(set_get(c,v) != nil) {
		switch(*p) {
			case 0x0A:
			{
				++pc->line;
				pc->pos = 0;
			}
			break;
			case 0x009:
				pc->pos += 4;
				break;
			default:
				++pc->pos;
				break;
		}
		++pc->processed;
		v = allocCharWithValue( *(++p));
	}
	return pc->processed;
}

static uint32_t countMMapBufferString(PFRTIOBuffer b) {
	uint32_t processed = 0;
	char *p = &b->bufferPtr[b->current_read_offset];
	int  x = 1;
	++p;
	++processed;
	while(x && (b->current_read_offset + processed ) < b->buffersize) {
		//	If escape char, jump it and next byte
		if(*p == '\\') {
			p += 2;
			processed += 2;
		}
		else if(*p == '"') {
			x = 0;
			++processed;
		}
		else {
			++p;
			++processed;
		}

	}
	return processed;
}

static uint32_t countMMapBufferQChar(PFRTIOBuffer b) {
	uint32_t processed = 0;
	char *p = &b->bufferPtr[b->current_read_offset];
	int  x = 1;
	++p;
	++processed;
	while(x && (b->current_read_offset + processed ) < b->buffersize) {
		//	If escape char, jump it and next byte
		if(*p == 0x27) {
			x = 0;
			++processed;
		}
		else {
			++p;
			++processed;
		}

	}
	return processed;
}

static void advanceMMapBufferToChar(PFRTIOBuffer b, char c) {
	counter(pc,b);
	countMMapBufferToChar(b,c,&pc);
	b->previous_read_offset = b->current_read_offset;
	b->current_read_offset += pc.processed;
	b->current_line = pc.line;
	b->current_pos = pc.pos;
}

static void advanceMMapBufferToWhen(PFRTIOBuffer b, PFRTAny c) {
	counter(pc,b);
	countMMapBufferToWhen(b,c,&pc);
	b->previous_read_offset = b->current_read_offset;
	b->current_read_offset += pc.processed;
	b->current_pos 			= pc.pos;
	b->current_line 		= pc.line;
}

static void advanceMMapBufferWhileChar(PFRTIOBuffer b, char c) {
	counter(pc,b);
	countMMapBufferWhileChar(b,c,&pc);
	b->previous_read_offset = b->current_read_offset;
	b->current_read_offset += pc.processed;
	b->current_pos 			= pc.pos;
	b->current_line 		= pc.line;
}

static void advanceMMapBufferWhile(PFRTIOBuffer b, PFRTAny c) {
	counter(pc,b);
	countMMapBufferWhile(b,c,&pc);
	b->previous_read_offset = b->current_read_offset;
	b->current_read_offset += pc.processed;
	b->current_pos 			= pc.pos;
	b->current_line 		= pc.line;
}

PFRTAny io_charReader(PFRTIOChannel chn) {
	PFRTAny 	result = nil;
	switch(chn->bufferptr->ftype) {
		case 	no_buffer:
			unknown_handler();
			break;
		case 	mem_block_buffer:
			result = charBlockBuffRead(chn->bufferptr);
		case 	mem_map_buffer:
			result = charMMapBuffRead(chn->bufferptr);
			break;
	}
	return result;
}

PFRTAny io_nextCharReader(PFRTIterator itr) {
	PFRTChannel_Iterator citr = (PFRTChannel_Iterator) itr;
	PFRTAny result = nil;
	if(citr->currRef < citr->buffer->max_read_position) {
		result = allocCharWithValue(
			(ft) citr->buffer->bufferPtr[citr->currRef]);
		++citr->currRef;
	}
	return result;
}


PFRTAny io_stringReader(PFRTIOChannel chn) {
	return nil;
}

PFRTAny io_nextStringReader(PFRTIterator chn) {
	return nil;
}

PFRTAny io_consReader(PFRTIOChannel chn) {
	chn->bufferptr->bufferPtr[0] = 0;
	int x = read(chn->handle,
		chn->bufferptr->bufferPtr,
		chn->bufferptr->buffersize);
	if(x < 0)
		unknown_handler();
	return allocStringWithCopy(chn->bufferptr->bufferPtr);
}

static uint32_t hasmore(PFRTIOBuffer b,uint32_t off) {
	return (b->current_read_offset + off) < b->buffersize ? 1 : 0;
}

PFRTAny io_first(PFRTIOChannel chn) {
	PFRTIOBuffer b = chn->bufferptr;
	if(chn->ftype == cin_type)
		unknown_handler();
	else
	switch(chn->bufferptr->ftype) {
		case 	no_buffer:
			unknown_handler();
			break;
		case 	mem_block_buffer:
			unknown_handler();
		case 	mem_map_buffer:
			if(hasmore(chn->bufferptr,0))
				return allocCharWithValue(
					(ft) b->bufferPtr[b->current_read_offset]);
			else
				unknown_handler();
			break;
	}
	return nil;
}

PFRTAny io_second(PFRTIOChannel chn) {
	PFRTIOBuffer b = chn->bufferptr;
	if(chn->ftype == cin_type)
		unknown_handler();
	else
	switch(chn->bufferptr->ftype) {
		case 	no_buffer:
			unknown_handler();
			break;
		case 	mem_block_buffer:
			unknown_handler();
		case 	mem_map_buffer:
			if(hasmore(chn->bufferptr,1))
				return allocCharWithValue(
					(ft) b->bufferPtr[b->current_read_offset]);
			else
				unknown_handler();
			break;
	}
	return nil;
}

PFRTAny io_peek_match(PFRTIOChannel chn, PFRTAny pat) {
	PFRTIOBuffer b = chn->bufferptr;
	PFRTAny 	result;
	if(chn->ftype == cin_type)
		result = false;
	else
	switch(chn->bufferptr->ftype) {
		case 	no_buffer:
			result = false;
			break;
		case 	mem_block_buffer:
			result = false;
		case 	mem_map_buffer:
			if(hasmore(chn->bufferptr,pat->count)) {
				if(isCharWS_qmark(b->bufferPtr[b->current_read_offset+pat->count])) {
					PFRTAny arg = allocStringWithCopyCnt(pat->count,
							&b->bufferPtr[b->current_read_offset]);
					result = foidl_regex_cmp_qmark(pat,arg);
					foidl_xdel(arg->value);
					foidl_xdel(arg);
				}
				else
					result = false;
			}
			else
				result = false;
			break;
	}
	return result;
}

PFRTIOChannel io_skipto(PFRTIOChannel chn, PFRTAny exp) {
	if(chn->ftype == cin_type)
		unknown_handler();
	else
	switch(chn->bufferptr->ftype) {
		case 	no_buffer:
			unknown_handler();
			break;
		case 	mem_block_buffer:
			unknown_handler();
		case 	mem_map_buffer:
			if(exp->fclass == scalar_class && exp->ftype == character_type)
				advanceMMapBufferToChar(chn->bufferptr, (char) exp->value);
			else if(exp->fclass == collection_class && exp->ftype == set2_type)
				advanceMMapBufferToWhen(chn->bufferptr, exp);
			else
				unknown_handler();
			break;
	}
	return chn;
}

PFRTIOChannel io_skipwhile(PFRTIOChannel chn, PFRTAny exp) {
	if(chn->ftype == cin_type)
		unknown_handler();
	else
	switch(chn->bufferptr->ftype) {
		case 	no_buffer:
			unknown_handler();
			break;
		case 	mem_block_buffer:
			unknown_handler();
		case 	mem_map_buffer:
			if(exp->fclass == scalar_class && exp->ftype == character_type)
				advanceMMapBufferWhileChar(chn->bufferptr, (char) exp->value);
			else if(exp->fclass == collection_class && exp->ftype == set2_type)
				advanceMMapBufferWhile(chn->bufferptr, exp);
			else
				unknown_handler();
			break;
	}
	return chn;
}

PFRTAny io_count(PFRTIOChannel chn) {
	uint32_t 	vcnt = 0;
	if(chn->ftype == cin_type)
		unknown_handler();
	else
	switch(chn->bufferptr->ftype) {
		case 	no_buffer:
			unknown_handler();
			break;
		case 	mem_block_buffer:
			unknown_handler();
		case 	mem_map_buffer:
			vcnt = chn->bufferptr->buffersize - chn->bufferptr->current_read_offset;
			break;
	}
	return (vcnt > 0) ? allocIntegerWithValue(vcnt) : zero;
}

PFRTAny io_countto(PFRTIOChannel chn, PFRTAny exp) {
	uint32_t 	vcnt = 0;
	counter(pc,chn->bufferptr);
	if(chn->ftype == cin_type)
		unknown_handler();
	else
	switch(chn->bufferptr->ftype) {
		case 	no_buffer:
			unknown_handler();
			break;
		case 	mem_block_buffer:
			unknown_handler();
		case 	mem_map_buffer:
			if(exp->fclass == scalar_class && exp->ftype == character_type)
				vcnt = countMMapBufferToChar(chn->bufferptr, (char) exp->value,&pc);
			else if(exp->fclass == collection_class && exp->ftype == set2_type)
				vcnt = countMMapBufferToWhen(chn->bufferptr, exp,&pc);
			else
				unknown_handler();
			break;
	}
	return (vcnt > 0) ? allocIntegerWithValue(vcnt) : zero;
}

PFRTIOChannel io_unread(PFRTIOChannel chn) {
	if(chn->ftype == cin_type)
		unknown_handler();
	else
	switch(chn->bufferptr->ftype) {
		case 	no_buffer:
			unknown_handler();
			break;
		case 	mem_block_buffer:
		case 	mem_map_buffer:
			if(chn->bufferptr->current_read_offset
				> chn->bufferptr->previous_read_offset) {
					chn->bufferptr->current_read_offset
						= chn->bufferptr->previous_read_offset;
					--chn->bufferptr->current_pos;
				}
			else
				;
			break;
	}
	return  chn;
}

PFRTAny io_take_until(PFRTIOChannel chn, PFRTAny pred) {
	PFRTAny result = empty_string;
	uint32_t 	vcnt = 0;
	counter(pc,chn->bufferptr);
	if(chn->ftype == cin_type)
		unknown_handler();
	else
	switch(chn->bufferptr->ftype) {
		case 	no_buffer:
			unknown_handler();
			break;
		case 	mem_block_buffer:
			unknown_handler();
			break;
		case 	mem_map_buffer:
			if(pred->fclass == scalar_class && pred->ftype == character_type)
				vcnt = countMMapBufferToChar(chn->bufferptr, (char) pred->value,&pc);
			else if(pred->fclass == collection_class && pred->ftype == set2_type)
				vcnt = countMMapBufferToWhen(chn->bufferptr, pred,&pc);
			else
				unknown_handler();
			break;
	}
	if(vcnt) {
		result = allocStringWithCopyCnt(vcnt,
			&chn->bufferptr->bufferPtr[chn->bufferptr->current_read_offset]);
		chn->bufferptr->previous_read_offset = chn->bufferptr->current_read_offset;
		chn->bufferptr->current_read_offset += vcnt;
	}
	return result;
}

PFRTAny io_take_string(PFRTIOChannel chn) {
	PFRTAny result = empty_string;
	uint32_t 	vcnt = 0;

	if(chn->ftype == cin_type)
		unknown_handler();
	else
	switch(chn->bufferptr->ftype) {
		case 	no_buffer:
			unknown_handler();
			break;
		case 	mem_block_buffer:
			unknown_handler();
			break;
		case 	mem_map_buffer:
			if(chn->bufferptr->bufferPtr[chn->bufferptr->current_read_offset]
				== '"')
				vcnt = countMMapBufferString(chn->bufferptr);
			else
				unknown_handler();
			break;
	}
	if(vcnt) {
		result = allocStringWithCopyCnt(vcnt-2,
			&chn->bufferptr->bufferPtr[chn->bufferptr->current_read_offset+1]);
		chn->bufferptr->previous_read_offset = chn->bufferptr->current_read_offset;
		chn->bufferptr->current_read_offset += vcnt;
	}
	return result;
}


PFRTAny io_take_qchar(PFRTIOChannel chn) {
	PFRTAny result = nil;
	uint32_t 	vcnt = 0;

	if(chn->ftype == cin_type)
		unknown_handler();
	else
	switch(chn->bufferptr->ftype) {
		case 	no_buffer:
			unknown_handler();
			break;
		case 	mem_block_buffer:
			unknown_handler();
			break;
		case 	mem_map_buffer:
			if(chn->bufferptr->bufferPtr[chn->bufferptr->current_read_offset]
				== 0x27)
				vcnt = countMMapBufferQChar(chn->bufferptr);
			else
				unknown_handler();
			break;
	}
	if(vcnt > 2) {
		char *p = &chn->bufferptr->bufferPtr[chn->bufferptr->current_read_offset];
		++p;
		ft 	res = 0;
		//	One char
		if(vcnt == 3)
			result = allocCharWithValue((ft) *p);
		//	UTF-8 (2)
		else if(vcnt == 4) {
			res = p[0] | p[1] << 8;
			result = allocECharWithValue(res,2);
		}
		//	UTF-8 (3)
		else if(vcnt == 5) {
			res = p[0] | p[1] << 8 | p[2] << 16;
			result = allocECharWithValue(res,3);
		}
		//	UTF-8 (4)
		else if(vcnt == 6) {
			res = p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
			result = allocECharWithValue(res,4);
		}
		else {
			unknown_handler();
		}
		chn->bufferptr->previous_read_offset = chn->bufferptr->current_read_offset;
		chn->bufferptr->current_read_offset += vcnt;
	}
	return result;
}

typedef struct _ioscalarinfo {
	char 		*pdata;
	uint32_t 	bytes;
	uint32_t 	genned;
} IOScalarInfo, *PIOScalarInfo;

static void scalarIOInfo(PFRTAny el,PIOScalarInfo psi) {
	switch (el->ftype) {
		case 	keyword_type:
		case 	string_type:
			{
				psi->pdata = el->value;
				psi->bytes = el->count;
				psi->genned = 0;
			}
			break;
		case 	integer_type:
			{
				char *tmp;
				if (asprintf(&tmp,"%lld",(ft) el->value) == -1) {
					perror("asprintf:");
					unknown_handler();
				}
				psi->pdata = tmp;
				psi->bytes = strlen(tmp);
				psi->genned = 1;
			}
			break;
		case 	character_type:
			{
				psi->pdata = (char *) &el->value;
				psi->bytes = el->count;
				psi->genned = 0;
			}
			break;
		case 	boolean_type:
			if((ft) el->value == 0) {
				psi->pdata = (char *) falseStr;
				psi->bytes = 5;
				psi->genned = 0;
			}
			else {
				psi->pdata = (char *) trueStr;
				psi->bytes = 4;
				psi->genned = 0;
			}
			break;
		case 	nil_type:
			{
				psi->pdata = (char *) (char *) nilStr;
				psi->bytes = 3;
				psi->genned = 0;
			}
			break;
		case 	end_type:
			{
				psi->pdata = (char *) endStr;
				psi->bytes = 3;
				psi->genned = 0;
			}
			break;
		default:
			unknown_handler();
			break;
	}

}

static void io_scalarRawWriter(PFRTIOChannel chn, PFRTAny el) {
	switch(el->ftype) {
		case 	keyword_type:
		case 	string_type:
			write(chn->handle,el->value,el->count);
			break;
		case 	integer_type:
			{
				char *tmp;
				asprintf(&tmp,"%lld",(ft) el->value);
				write(chn->handle,tmp,strlen(tmp));
				free(tmp);
			}
			break;
		case 	character_type:
			write(chn->handle,&el->value,el->count);
			break;
		case 	nil_type:
			write(chn->handle,nilStr,3);
			break;
		case 	end_type:
			write(chn->handle,endStr,3);
			break;
		case 	boolean_type:
			if((ft) el->value == 0)
				write(chn->handle,falseStr,5);
			else
				write(chn->handle,trueStr,4);
			break;
		default:
			unknown_handler();
			break;
	}

}

//	Character/string writer

PFRTAny io_charWriter(PFRTIOChannel chn, PFRTAny el) {

	if(el->fclass == scalar_class) {
		IOScalarInfo si = {0,0,0};
		scalarIOInfo(el,&si);
		PFRTIOBuffer pb = chn->bufferptr;
		if(pb->current_write_offset + si.bytes < pb->buffersize) {
			strncpy(&pb->bufferPtr[pb->current_write_offset],si.pdata,si.bytes);
			pb->previous_write_offset = pb->current_write_offset;
			pb->current_write_offset += si.bytes;
		}
		else {
			write(chn->handle,pb->bufferPtr,pb->current_write_offset);
			strncpy(pb->bufferPtr,si.pdata,si.bytes);
			pb->previous_write_offset = 0;
			pb->current_write_offset = si.bytes;
		}
		if(si.genned == 1)
			free(si.pdata);
	}
	else
		unknown_handler();
	return (PFRTAny) chn;
}

void 	io_blockBufferClose(PFRTIOChannel chn) {
	if(chn->openflag == open_write_only &&
		chn->bufferptr->current_write_offset > 0) {
		PFRTIOBuffer pb = chn->bufferptr;
		write(chn->handle,pb->bufferPtr,pb->current_write_offset);
		pb->current_write_offset = pb->previous_write_offset = 0;
	}
}

//	Console writer (cout,cerr)

PFRTAny io_consWriter(PFRTIOChannel chn, PFRTAny el) {
	if(el->fclass == scalar_class) {
		io_scalarRawWriter(chn, el);
	}
	else if(el->fclass == collection_class) {
		switch(el->ftype) {
			case 	vector2_type:
				vector_print(chn,el);
				break;
			case 	list2_type:
				list_print(chn,el);
				break;
			case 	set2_type:
				set_print(chn,el);
				break;
			case 	map2_type:
				map_print(chn,el);
				break;
			case 	series_type:
				{
					if((PFRTSeries) el == infinite)
						write(chn->handle,infSeriesStr,15);
					else
						write(chn->handle,seriesStr,16);
				}
				break;
			case 	mapentry_type:
				{
					io_consWriter(chn,lbracket);
					io_consWriter(chn,((PFRTMapEntry)el)->key);
					io_consWriter(chn,comma);
					io_consWriter(chn,((PFRTMapEntry)el)->value);
					io_consWriter(chn,rbracket);
				}
				break;
			default:
				unknown_handler();
				break;
		}

	}
	else if(el->fclass == function_class) {
		write(chn->handle,fnStr,12);
	}
	else {
		unknown_handler();
	}
	return nil;
}