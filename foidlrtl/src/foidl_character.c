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
	c->count  = count;
	return c;
}

PFRTAny  foidl_reg_char(char *p) {
	ft l = strlen(p);
	if(l == 3)
		return allocCharWithValue((ft) p[1]);
	else if(l == 4)
		return allocECharWithValue((p[1] | p[2] << 8),2);
	else if(l == 5)
		return allocECharWithValue((p[1] | p[2] << 8 | p[3] << 16), 3);
	else if(l == 6)
		return allocECharWithValue((p[1] | p[2] << 8 | p[3] << 16 | p[4] << 24), 4);
	else {
		unknown_handler();
	}
	return nil;
}


