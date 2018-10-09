/*
	foidl_character.c
	Library ascii and UTF-8 character
	
	Copyright Frank V. Castellucci
	All Rights Reserved
*/

#define CHARACTER_IMPL
#include <foidlrt.h>

#define	ASCII_MAX_RANGE 128
PFRTAny foidl_ascii_chars[ASCII_MAX_RANGE];

void foidl_rtl_init_chars() {
	for(int i=0; i< 127;i++) foidl_ascii_chars[i] = allocGlobalCharType(i);
}

PFRTAny allocCharWithValue(ft v) {
	PFRTAny c = nil;
	if((long long) v >= 0 && (long long) v < ASCII_MAX_RANGE) {
		c = foidl_ascii_chars[v];
	}
	else {
		c = allocAny(scalar_class,character_type,(void *)v);
	}
	return c;
}

PFRTAny  foidl_reg_character(ft i) {
	return allocCharWithValue(i);
}

PFRTAny allocECharWithValue(ft v, ft count) {	
	PFRTAny c = (PFRTAny) allocAny(scalar_class, character_type,(void *)v);
	//c->fclass = scalar_class;
	//;c->ftype  = character_type;
	c->count  = count;
	//c->value  = (void *)v;	
	return c;
}


