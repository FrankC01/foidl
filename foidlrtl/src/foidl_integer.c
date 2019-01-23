/*
	foidl_integer.c
	Library integer model

	Copyright Frank V. Castellucci
	All Rights Reserved
*/

#define INTEGER_IMPL
#include <foidlrt.h>
#include <stdio.h>
#include <stdlib.h>
#include <m_apm.h>

static PFRTAny intMap;

void 	foidl_rtl_init_ints() {
	intMap = foidl_map_inst_bang();
	FRTAny 	anyInt = {scalar_class,integer_type,0,0,0};
	for(long long x = -100; x < 101; x++) {
		anyInt.value = (void *) x;
		PFRTAny res = allocGlobalIntegerType(x);
		foidl_map_extend_bang(intMap,res,res);
	}
}

PFRTAny  foidl_reg_integer(long long i) {
	printf("Exception foidl_reg_integer is deprecated...\n");
	unknown_handler();
	return nil; //res;
}

int foidl_return_code(PFRTAny rc) {
	if(rc->ftype == integer_type) {
		return (int) rc->value;
	}
	else {
		printf("Attempting to return non integer from main\n");
		unknown_handler();
	}
	return 0;
}

PFRTAny allocIntegerWithValue(long long v) {
	FRTAny 	anyInt = {scalar_class,integer_type,0,0,(void *) v};
	PFRTAny res = map_get(intMap,&anyInt);
	if(res == nil) {
		res = (PFRTAny) allocAny(scalar_class,integer_type,(void *)v);
	}
	return res;
}

PFRTAny foidl_add_ints(PFRTAny arg1, PFRTAny arg2) {
	return allocIntegerWithValue((long long) arg1->value
		+ (long long) arg2->value);
}

PFRTAny foidl_sub_ints(PFRTAny arg1, PFRTAny arg2) {
	return allocIntegerWithValue((long long) arg1->value
		- (long long) arg2->value);
}

PFRTAny foidl_div_ints(PFRTAny arg1, PFRTAny arg2) {
	return allocIntegerWithValue((long long) arg1->value
		/ (long long) arg2->value);
}

PFRTAny foidl_mul_ints(PFRTAny arg1, PFRTAny arg2) {
	return allocIntegerWithValue((long long) arg1->value
			* (long long) arg2->value);
}

PFRTAny foidl_mod_ints(PFRTAny arg1, PFRTAny arg2) {
	return allocIntegerWithValue((long long) arg1->value
			% (long long) arg2->value);
}

PFRTAny foidl_c2i(PFRTAny el) {
	PFRTAny res = nil;
	if(el->ftype == character_type)
		res = allocIntegerWithValue((long long) el->value);
	return res;
}

