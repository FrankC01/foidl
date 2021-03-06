/*
;	foidl_globals.c
;	Library common/globals
;
; Copyright (c) Frank V. Castellucci
; All Rights Reserved
;
; Licensed under the Apache License, Version 2.0 (the "License");
; you may not use this file except in compliance with the License.
; You may obtain a copy of the License at
;
;     http://www.apache.org/licenses/LICENSE-2.0
;
; Unless required by applicable law or agreed to in writing, software
; distributed under the License is distributed on an "AS IS" BASIS,
; WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; See the License for the specific language governing permissions and
; limitations under the License.
*/

#define GLOBALS_IMPL
#include <foidlrt.h>

//	Well known types
globalScalarConst(true,boolean_type,(void *) 1,1);
globalScalarConst(false,boolean_type,(void *) 0,1);
globalScalarConst(nil,nil_type,(void *) 0,1);
globalScalarConst(end,end_type,(void *) 0,1);

// IO Channel types
constKeyword(chan_file,":channel_file");
globalScalarConst(chan_unknown,byte_type,(void *) 16,1);

// For file channel read rendering

globalScalarConst(render_byte,byte_type,(void *) 0,1);
globalScalarConst(render_char,byte_type,(void *) 1,1);
globalScalarConst(render_line,byte_type,(void *) 2,1);
globalScalarConst(render_file,byte_type,(void *) 3,1);
globalScalarConst(render_string,byte_type,(void *) 4,1);

constKeyword(chan_target,":target");
constKeyword(chan_type,":type");
constKeyword(chan_render,":render");
constKeyword(chan_mode,":mode");

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
constString(seriesstr,"series reference");
constString(poolstr,"thread pool reference");
constString(workstr,"worker reference");

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
