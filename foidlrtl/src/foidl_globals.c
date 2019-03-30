/*
	foidl_globals.c
	Library common/globals

	Copyright Frank V. Castellucci
	All Rights Reserved
*/

#define GLOBALS_IMPL
#include <foidlrt.h>

//	Well known types
globalScalarConst(true,boolean_type,(void *) 1,1);
globalScalarConst(false,boolean_type,(void *) 0,1);
globalScalarConst(nil,nil_type,(void *) 0,1);
globalScalarConst(end,end_type,(void *) 0,1);

// String types
globalScalarConst(empty_string,string_type,(void *) "",0);
globalScalarConst(space_string,string_type,(void *) " ",1);

constString(cinstr,"cin");
constString(coutstr,"cout");
constString(cerrstr,"cerr");
constString(nilstr,"nil");
constString(truestr,"true");
constString(falsestr,"false");
constString(endstr,"end");
constString(fnstr,"function reference");
constString(chanstr,"channel reference");
constString(infserstr,"infinite series");
constString(seriesstr,"serries reference");

// Character types

globalScalarConst(chr0,character_type,(void *) 0x30,1);
globalScalarConst(chr1,character_type,(void *) 0x31,1);
globalScalarConst(chr2,character_type,(void *) 0x32,1);
globalScalarConst(chr3,character_type,(void *) 0x33,1);
globalScalarConst(chr4,character_type,(void *) 0x34,1);
globalScalarConst(chr5,character_type,(void *) 0x35,1);
globalScalarConst(chr6,character_type,(void *) 0x36,1);
globalScalarConst(chr7,character_type,(void *) 0x37,1);
globalScalarConst(chr8,character_type,(void *) 0x38,1);
globalScalarConst(chr9,character_type,(void *) 0x39,1);


PFRTAny	lparen,rparen,lbracket,rbracket,langle,rangle,comma,lbrace,rbrace;
PFRTAny meta,atchr,pctchr,tildchr,prchr;
PFRTAny tbchr,nlchr,crchr,spchr;
PFRTAny semichr,colonchr,sqchr,dqchr,eschr,exchr, carchr,qmchr,orchr;
PFRTAny eqchr, tmchr, addchr, subchr, divchr;
PFRTAny boundarySet;
PFRTAny boundarySetLite;
PFRTAny whitespace;

PFRTAny zero,one,two,three,four,five,six,seven,eight;
PFRTAny nine,ten,eleven,twelve,thirteen,fourteen,fifteen,sixteen;

#define genchar(lsym,v) lsym = allocCharWithValue((ft) v)

#define gencharToSet(lset,lsym,v) \
	genchar(lsym,v); \
	foidl_set_extend_bang(lset,lsym)

#define addCharToSet(lset,lsym) foidl_set_extend_bang(lset,lsym)

void foidl_rtl_init_globals() {


	//	Setup some character sets

	whitespace = foidl_set_inst_bang();
	boundarySet = foidl_set_inst_bang();
	boundarySetLite = foidl_set_inst_bang();

	gencharToSet(boundarySet, lbracket,'[');
	gencharToSet(boundarySet, rbracket,']');
	gencharToSet(boundarySet, langle,'<');
	gencharToSet(boundarySet, rangle,'>');
	gencharToSet(boundarySet, comma,',');
	gencharToSet(boundarySet, lbrace,'{');
	gencharToSet(boundarySet, rbrace,'}');
	gencharToSet(boundarySet, lparen, '(');
	gencharToSet(boundarySet, rparen, ')');
	gencharToSet(boundarySet, meta,'#');
	gencharToSet(boundarySet, semichr,';');
	gencharToSet(boundarySet, tildchr,'~'); 	//	Defer
	genchar(colonchr,':');
	gencharToSet(boundarySet, orchr,'|');
	genchar(qmchr,'?');
	genchar(tmchr,'*');
	genchar(prchr, '.');
	gencharToSet(boundarySet, carchr,'^');
	gencharToSet(boundarySet, eqchr,'=');
	gencharToSet(boundarySet, addchr,'+');
	gencharToSet(boundarySet, subchr,'-');
	gencharToSet(boundarySet, divchr,'/');
	gencharToSet(boundarySet, exchr,'!');
	gencharToSet(boundarySet, atchr,'@');
	gencharToSet(boundarySet, pctchr,'%');

	gencharToSet(boundarySet, tbchr,0x09);	//	tab
#ifdef _MSC_VER
	nlchr = allocCharWithValue(0x0A0D);
	nlchr->count = 2;
	foidl_set_extend_bang(boundarySet,nlchr);
	// gencharToSet(boundarySet, nlchr,0x0A0D); //	windows newline/carriage return
#else
	gencharToSet(boundarySet, nlchr,0x0A); 	//	newline
#endif
	gencharToSet(boundarySet, crchr,0x0D); 	//	carriage return
	gencharToSet(boundarySet, spchr,0x20); 	//	space
	gencharToSet(boundarySet, dqchr,0x22); 	//  quote
	gencharToSet(boundarySet, sqchr,0x27); 	//  apostrophe
	gencharToSet(boundarySet, eschr,0x5C); 	// 	backslash

	addCharToSet(boundarySetLite, lbracket);
	addCharToSet(boundarySetLite, rbracket);
	addCharToSet(boundarySetLite, langle);
	addCharToSet(boundarySetLite, rangle);
	addCharToSet(boundarySetLite, comma);
	addCharToSet(boundarySetLite, lbrace);
	addCharToSet(boundarySetLite, rbrace);
	addCharToSet(boundarySetLite, lparen);
	addCharToSet(boundarySetLite, rparen);
	addCharToSet(boundarySetLite, meta);
	addCharToSet(boundarySetLite, semichr);
	addCharToSet(boundarySetLite, orchr);
	addCharToSet(boundarySetLite, carchr);
	addCharToSet(boundarySetLite, eqchr);
	addCharToSet(boundarySetLite, divchr);
	addCharToSet(boundarySetLite, exchr);
	addCharToSet(boundarySetLite, atchr);
	addCharToSet(boundarySetLite, pctchr);
	addCharToSet(boundarySetLite, tildchr);
	addCharToSet(boundarySetLite, tbchr);	//	tab
	addCharToSet(boundarySetLite, nlchr); 	//	newline
	addCharToSet(boundarySetLite, crchr); 	//	carriage return
	addCharToSet(boundarySetLite, spchr); 	//	space
	addCharToSet(boundarySetLite, dqchr); 	//  quote
	addCharToSet(boundarySetLite, sqchr); 	//  apostrophe
	addCharToSet(boundarySetLite, eschr); 	// 	backslash

	addCharToSet(whitespace, tbchr);	//	tab
	addCharToSet(whitespace, nlchr); 	//	newline
	addCharToSet(whitespace, crchr); 	//	carriage return
	addCharToSet(whitespace, spchr); 	//	space

	//	Whitespace set?
	//	Newline set?
	//	Tab set?
}

// Integer types

//	IO Reader Functions types

extern PFRTAny 	io_consReader(PFRTIOChannel);
extern PFRTAny 	io_byteReader(PFRTIOChannel);
extern PFRTAny 	io_charReader(PFRTIOChannel);
extern PFRTAny 	io_charWriter(PFRTIOChannel);
extern PFRTAny 	io_stringReader(PFRTIOChannel);

globalFuncConst(cin_reader,1,io_consReader);
globalFuncConst(byte_reader,1,io_byteReader);
globalFuncConst(char_reader,1,io_charReader);
globalFuncConst(string_reader,1,io_stringReader);
globalFuncConst(char_writer,1,io_charWriter);

//	IO Writer Functions types

extern PFRTAny 	io_consWriter(PFRTIOChannel, PFRTAny);
globalFuncConst(cout_writer,2,io_consWriter);

uint32_t const cbsize = 4096;

static char cinbuffer[cbsize];

const struct FRTIOBufferG _cin_buffer = {
	global_signature,
	io_class,
	mem_block_buffer,
	cbsize,
	0,0,0,0,0,0,0,
	cinbuffer
};

PFRTIOBuffer const cin_buffer = (PFRTIOBuffer) & _cin_buffer.fclass;

const struct FRTIOChannelG _cin_base = {
	global_signature,
	io_class,
	cin_type,
	nil,
	cinstr,
	open_read_only,
	mem_block_buffer,
	0,
	cin_reader,
	(PFRTFuncRef) nil,
	cin_buffer
};

PFRTIOChannel const cin = (PFRTIOChannel) &_cin_base.fclass;

const struct FRTIOChannelG _cout_base = {
	global_signature,
	io_class,
	cout_type,
	nil,
	coutstr,
	open_write_only,
	no_buffer,
	1,
	(PFRTFuncRef) nil,
	cout_writer,
	(PFRTIOBuffer) nil
};

PFRTIOChannel const cout = (PFRTIOChannel) &_cout_base.fclass;

const struct FRTIOChannelG _cerr_base = {
	global_signature,
	io_class,
	cerr_type,
	nil,
	cerrstr,
	open_write_only,
	no_buffer,
	2,
	(PFRTFuncRef) nil,
	cout_writer,
	(PFRTIOBuffer) nil
};

PFRTIOChannel const cerr = (PFRTIOChannel) &_cerr_base.fclass;
