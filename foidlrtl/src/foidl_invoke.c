/*
	foidl_invoke.c
	Library support for partials, lambdas and evaluation
	
	Copyright Frank V. Castellucci
	All Rights Reserved
*/

#define 	INVOKE_IMPL
#include 	<foidlrt.h>


PFRTAny 	invoke0 (PFRTFuncRef2 a) {
	return ((PFRTAny (*) ()) a->fnptr) ();
}

PFRTAny 	invoke1 (PFRTFuncRef2 a) {
	return ((PFRTAny (*)
			(PFRTAny)
			) a->fnptr)
		(
			foidl_get(a->args, zero)
		);
}

PFRTAny 	invoke2 (PFRTFuncRef2 a) {
	return ((PFRTAny (*)
			(
				PFRTAny, 	//	1
				PFRTAny 	//	2
				)
			) a->fnptr)
		(	foidl_get(a->args, zero),
			foidl_get(a->args, one)
		);
}

PFRTAny 	invoke3 (PFRTFuncRef2 a) {
	return ((PFRTAny (*)
			(
				PFRTAny, 	// 1
				PFRTAny,	// 2
				PFRTAny 	// 3
				)
			) a->fnptr)
		(	foidl_get(a->args, zero),
			foidl_get(a->args, one),
			foidl_get(a->args, two)
		);
}

PFRTAny 	invoke4 (PFRTFuncRef2 a) {
	return ((PFRTAny (*)
			(
				PFRTAny, 	// 1
				PFRTAny,	// 2
				PFRTAny, 	// 3
				PFRTAny 	// 4
				)
			) a->fnptr)
		(	foidl_get(a->args, zero),
			foidl_get(a->args, one),
			foidl_get(a->args, two),
			foidl_get(a->args, three)
		);
}

PFRTAny 	invoke5 (PFRTFuncRef2 a) {
	return ((PFRTAny (*)
			(
				PFRTAny, 	// 1
				PFRTAny,	// 2
				PFRTAny, 	// 3
				PFRTAny, 	// 4
				PFRTAny 	// 5
				)
			) a->fnptr)
		(	foidl_get(a->args, zero),
			foidl_get(a->args, one),
			foidl_get(a->args, two),
			foidl_get(a->args, three),
			foidl_get(a->args, four)
		);
}

PFRTAny 	invoke6 (PFRTFuncRef2 a) {
	return ((PFRTAny (*)
			(
				PFRTAny, 	// 1
				PFRTAny,	// 2
				PFRTAny, 	// 3
				PFRTAny, 	// 4
				PFRTAny, 	// 5
				PFRTAny 	// 6
				)
			) a->fnptr)
		(	foidl_get(a->args, zero),
			foidl_get(a->args, one),
			foidl_get(a->args, two),
			foidl_get(a->args, three),
			foidl_get(a->args, four),
			foidl_get(a->args, five)
		);
}

PFRTAny 	invoke7 (PFRTFuncRef2 a) {
	return ((PFRTAny (*)
			(
				PFRTAny, 	// 1
				PFRTAny,	// 2
				PFRTAny, 	// 3
				PFRTAny, 	// 4
				PFRTAny, 	// 5
				PFRTAny, 	// 6
				PFRTAny 	// 7
				)
			) a->fnptr)
		(	foidl_get(a->args, zero),
			foidl_get(a->args, one),
			foidl_get(a->args, two),
			foidl_get(a->args, three),
			foidl_get(a->args, four),
			foidl_get(a->args, five),
			foidl_get(a->args, six)
		);
}

PFRTAny 	invoke8 (PFRTFuncRef2 a) {
	return ((PFRTAny (*)
			(
				PFRTAny, 	// 1
				PFRTAny,	// 2
				PFRTAny, 	// 3
				PFRTAny, 	// 4
				PFRTAny, 	// 5
				PFRTAny, 	// 6
				PFRTAny, 	// 7
				PFRTAny 	// 8
				)
			) a->fnptr)
		(	foidl_get(a->args, zero),
			foidl_get(a->args, one),
			foidl_get(a->args, two),
			foidl_get(a->args, three),
			foidl_get(a->args, four),
			foidl_get(a->args, five),
			foidl_get(a->args, six),
			foidl_get(a->args, seven)			
		);
}

PFRTAny 	invoke9 (PFRTFuncRef2 a) {
	return ((PFRTAny (*)
			(
				PFRTAny, 	// 1
				PFRTAny,	// 2
				PFRTAny, 	// 3
				PFRTAny, 	// 4
				PFRTAny, 	// 5
				PFRTAny, 	// 6
				PFRTAny, 	// 7
				PFRTAny, 	// 8
				PFRTAny 	// 9
				)
			) a->fnptr)
		(	foidl_get(a->args, zero),
			foidl_get(a->args, one),
			foidl_get(a->args, two),
			foidl_get(a->args, three),
			foidl_get(a->args, four),
			foidl_get(a->args, five),
			foidl_get(a->args, six),
			foidl_get(a->args, seven),
			foidl_get(a->args, eight)
		);
}

PFRTAny 	invoke10 (PFRTFuncRef2 a) {
	return ((PFRTAny (*)
			(
				PFRTAny, 	// 1
				PFRTAny,	// 2
				PFRTAny, 	// 3
				PFRTAny, 	// 4
				PFRTAny, 	// 5
				PFRTAny, 	// 6
				PFRTAny, 	// 7
				PFRTAny, 	// 8
				PFRTAny, 	// 9
				PFRTAny 	// 10
				)
			) a->fnptr)
		(	foidl_get(a->args, zero),
			foidl_get(a->args, one),
			foidl_get(a->args, two),
			foidl_get(a->args, three),
			foidl_get(a->args, four),
			foidl_get(a->args, five),
			foidl_get(a->args, six),
			foidl_get(a->args, seven),
			foidl_get(a->args, eight),
			foidl_get(a->args, nine)
		);
}

PFRTAny 	invoke11 (PFRTFuncRef2 a) {
	return ((PFRTAny (*)
			(
				PFRTAny, 	// 1
				PFRTAny,	// 2
				PFRTAny, 	// 3
				PFRTAny, 	// 4
				PFRTAny, 	// 5
				PFRTAny, 	// 6
				PFRTAny, 	// 7
				PFRTAny, 	// 8
				PFRTAny, 	// 9
				PFRTAny, 	// 10
				PFRTAny 	// 11
				)
			) a->fnptr)
		(	foidl_get(a->args, zero),
			foidl_get(a->args, one),
			foidl_get(a->args, two),
			foidl_get(a->args, three),
			foidl_get(a->args, four),
			foidl_get(a->args, five),
			foidl_get(a->args, six),
			foidl_get(a->args, seven),
			foidl_get(a->args, eight),
			foidl_get(a->args, nine),
			foidl_get(a->args, ten)
		);
}
PFRTAny 	invoke12 (PFRTFuncRef2 a) {
	return ((PFRTAny (*)
			(
				PFRTAny, 	// 1
				PFRTAny,	// 2
				PFRTAny, 	// 3
				PFRTAny, 	// 4
				PFRTAny, 	// 5
				PFRTAny, 	// 6
				PFRTAny, 	// 7
				PFRTAny, 	// 8
				PFRTAny, 	// 9
				PFRTAny, 	// 10
				PFRTAny, 	// 11
				PFRTAny 	// 12
				)
			) a->fnptr)
		(	foidl_get(a->args, zero),
			foidl_get(a->args, one),
			foidl_get(a->args, two),
			foidl_get(a->args, three),
			foidl_get(a->args, four),
			foidl_get(a->args, five),
			foidl_get(a->args, six),
			foidl_get(a->args, seven),
			foidl_get(a->args, eight),
			foidl_get(a->args, nine),
			foidl_get(a->args, ten),
			foidl_get(a->args, eleven)
		);
}
PFRTAny 	invoke13 (PFRTFuncRef2 a) {
	return ((PFRTAny (*)
			(
				PFRTAny, 	// 1
				PFRTAny,	// 2
				PFRTAny, 	// 3
				PFRTAny, 	// 4
				PFRTAny, 	// 5
				PFRTAny, 	// 6
				PFRTAny, 	// 7
				PFRTAny, 	// 8
				PFRTAny, 	// 9
				PFRTAny, 	// 10
				PFRTAny, 	// 11
				PFRTAny, 	// 12
				PFRTAny 	// 13
				)
			) a->fnptr)
		(	foidl_get(a->args, zero),
			foidl_get(a->args, one),
			foidl_get(a->args, two),
			foidl_get(a->args, three),
			foidl_get(a->args, four),
			foidl_get(a->args, five),
			foidl_get(a->args, six),
			foidl_get(a->args, seven),
			foidl_get(a->args, eight),
			foidl_get(a->args, nine),
			foidl_get(a->args, ten),
			foidl_get(a->args, eleven),
			foidl_get(a->args, twelve)
		);
}
PFRTAny 	invoke14 (PFRTFuncRef2 a) {
	return ((PFRTAny (*)
			(
				PFRTAny, 	// 1
				PFRTAny,	// 2
				PFRTAny, 	// 3
				PFRTAny, 	// 4
				PFRTAny, 	// 5
				PFRTAny, 	// 6
				PFRTAny, 	// 7
				PFRTAny, 	// 8
				PFRTAny, 	// 9
				PFRTAny, 	// 10
				PFRTAny, 	// 11
				PFRTAny, 	// 12
				PFRTAny, 	// 13
				PFRTAny 	// 14
				)
			) a->fnptr)
		(	foidl_get(a->args, zero),
			foidl_get(a->args, one),
			foidl_get(a->args, two),
			foidl_get(a->args, three),
			foidl_get(a->args, four),
			foidl_get(a->args, five),
			foidl_get(a->args, six),
			foidl_get(a->args, seven),
			foidl_get(a->args, eight),
			foidl_get(a->args, nine),
			foidl_get(a->args, ten),
			foidl_get(a->args, eleven),
			foidl_get(a->args, twelve),
			foidl_get(a->args, thirteen)
		);
}
PFRTAny 	invoke15 (PFRTFuncRef2 a) {
	return ((PFRTAny (*)
			(
				PFRTAny, 	// 1
				PFRTAny,	// 2
				PFRTAny, 	// 3
				PFRTAny, 	// 4
				PFRTAny, 	// 5
				PFRTAny, 	// 6
				PFRTAny, 	// 7
				PFRTAny, 	// 8
				PFRTAny, 	// 9
				PFRTAny, 	// 10
				PFRTAny, 	// 11
				PFRTAny, 	// 12
				PFRTAny, 	// 13
				PFRTAny, 	// 14
				PFRTAny 	// 15
				)
			) a->fnptr)
		(	foidl_get(a->args, zero),
			foidl_get(a->args, one),
			foidl_get(a->args, two),
			foidl_get(a->args, three),
			foidl_get(a->args, four),
			foidl_get(a->args, five),
			foidl_get(a->args, six),
			foidl_get(a->args, seven),
			foidl_get(a->args, eight),
			foidl_get(a->args, nine),
			foidl_get(a->args, ten),
			foidl_get(a->args, eleven),
			foidl_get(a->args, twelve),
			foidl_get(a->args, thirteen),
			foidl_get(a->args, fourteen)
		);
}

PFRTAny 	invoke16 (PFRTFuncRef2 a) {
	return ((PFRTAny (*)
			(
				PFRTAny, 	// 1
				PFRTAny,	// 2
				PFRTAny, 	// 3
				PFRTAny, 	// 4
				PFRTAny, 	// 5
				PFRTAny, 	// 6
				PFRTAny, 	// 7
				PFRTAny, 	// 8
				PFRTAny, 	// 9
				PFRTAny, 	// 10
				PFRTAny, 	// 11
				PFRTAny, 	// 12
				PFRTAny, 	// 13
				PFRTAny, 	// 14
				PFRTAny, 	// 15
				PFRTAny 	// 16
				) 
			) a->fnptr)
		(	foidl_get(a->args, zero),
			foidl_get(a->args, one),
			foidl_get(a->args, two),
			foidl_get(a->args, three),
			foidl_get(a->args, four),
			foidl_get(a->args, five),
			foidl_get(a->args, six),
			foidl_get(a->args, seven),
			foidl_get(a->args, eight),
			foidl_get(a->args, nine),
			foidl_get(a->args, ten),
			foidl_get(a->args, eleven),
			foidl_get(a->args, twelve),
			foidl_get(a->args, thirteen),
			foidl_get(a->args, fourteen),
			foidl_get(a->args, fifteen)
						
		);
}

//	Invocation lookup table

static const invoke_funcptr iptrs[17] =
		{invoke0,invoke1,invoke2,invoke3,invoke4,invoke5,invoke6,invoke7,
			invoke8,invoke9,invoke10,invoke11,invoke12,invoke13,invoke14,
			invoke15,invoke16};

PFRTAny 	foidl_tofuncref(void *fref, PFRTAny acnt) {
	ft 	cnt = 0;
	if(acnt->ftype == string_type )
		cnt = atoi(acnt->value);
	else if (acnt->ftype == integer_type)
		cnt = (ft) acnt->value;
	else 
		unknown_handler();
	return (PFRTAny) allocFuncRef2(fref,(ft) cnt,iptrs[cnt]);
}
//	Instantiate a new or unique function reference instance

PFRTAny foidl_fref_instance(PFRTAny fsrc) {
	PFRTAny res = nil;	
	
	if(fsrc->ftype == funcref_type) {
		PFRTFuncRef fref = (PFRTFuncRef) fsrc;
		res = (PFRTAny) allocFuncRef2(fref->fnptr,fref->argcount,
			iptrs[fref->argcount]);
	}
	else if(fsrc->ftype == lambref_type) {
		PFRTLambdaRef lref = (PFRTLambdaRef) fsrc;
		res = (PFRTAny) 
			allocFuncRef2(lref->ffuncref->fnptr,lref->ffuncref->argcount,
				iptrs[lref->ffuncref->argcount]);
	}
	else if(fsrc->ftype == funcinst_type) {
		PFRTFuncRef2 iref  = (PFRTFuncRef2) fsrc;
		PFRTVector 	 ivec  = (PFRTVector) iref->args;
		PFRTFuncRef2 iref2 = allocFuncRef2(iref->fnptr,
			iref->mcount,iptrs[iref->mcount]);
		PFRTVector 	 ivec2 = (PFRTVector) iref2->args;
		if(ivec->count > 0)
			for(ft i = 0; i < ivec->count;i++)
				foidl_vector_extend_bang((PFRTAny) ivec2,
					vector_nth(ivec,i));			
		res = (PFRTAny) iref2;
	}
	else 
		unknown_handler();

	return (PFRTAny) res;
}

PFRTAny foidl_imbue(PFRTAny fref, PFRTAny val) {
	PFRTAny res = fref;
	if(fref->ftype == funcinst_type) {
		PFRTFuncRef2 iref  = (PFRTFuncRef2) fref;
		foidl_vector_extend_bang(iref->args,val);
		PFRTVector ivec = (PFRTVector) iref->args;
		if(ivec->count == iref->mcount)
			res = ((invoke_funcptr)iref->invokefnptr)(iref);
	}
	return res;
}

PFRTAny dispatch0i(PFRTAny fn) {

	_d0 fni = (_d0) (fn->ftype == funcref_type ? ((PFRTFuncRef) fn)->fnptr :
		((PFRTLambdaRef) fn)->ffuncref->fnptr);
	return fni();
}

PFRTAny dispatch0(PFRTAny fn) {

	if(fn->ftype == funcref_type || fn->ftype == lambref_type)
		return dispatch0i(fn);

	PFRTAny 	res = nil;
	PFRTFuncRef2 fref =(PFRTFuncRef2) foidl_fref_instance(fn);
	if(fref->mcount == 0) {
		res = invoke0(fref);
		deallocFuncRef2(fref);
	}
	else {
		deallocFuncRef2(fref);
		unknown_handler();
	}
	return res;
}

PFRTAny foidl_dispatch0 (PFRTAny fn) {
	return dispatch0(fn);
}

PFRTAny dispatch1i(PFRTAny fn, PFRTAny arg1) {
	_d1 fni = (_d1) (fn->ftype == funcref_type ? ((PFRTFuncRef) fn)->fnptr :
		((PFRTLambdaRef) fn)->ffuncref->fnptr);
	return fni(arg1);
}

PFRTAny dispatch1(PFRTAny fn, PFRTAny arg1) {
	if(fn->ftype == funcref_type || fn->ftype == lambref_type)
		return dispatch1i(fn,arg1);

	PFRTAny 	res = nil;
	PFRTFuncRef2 fref =(PFRTFuncRef2) foidl_fref_instance(fn);

	if(fref->mcount >= 1) {		
		res = foidl_imbue((PFRTAny) fref,arg1);
		deallocFuncRef2(fref);
	}
	else {
		deallocFuncRef2(fref);
		unknown_handler();
	}
	return res;
}

//	Go through motions of dispatching a two arg function
PFRTAny dispatch2i(PFRTAny fn, PFRTAny arg1, PFRTAny arg2) {
	_d2 fni = (_d2) (fn->ftype == funcref_type ? ((PFRTFuncRef) fn)->fnptr :
		((PFRTLambdaRef) fn)->ffuncref->fnptr);
	return fni(arg1,arg2);

}

PFRTAny dispatch2(PFRTAny fn, PFRTAny arg1, PFRTAny arg2) {
	if(fn->ftype == funcref_type || fn->ftype == lambref_type)
		return dispatch2i(fn,arg1,arg2);

	PFRTAny 	res = nil;
	PFRTFuncRef2 fref = (PFRTFuncRef2) foidl_fref_instance(fn);
	if(fref->mcount >= 2) {
		foidl_imbue((PFRTAny) fref,arg1);
		res = foidl_imbue((PFRTAny) fref,arg2);
		deallocFuncRef2(fref);
	}
	else {
		deallocFuncRef2(fref);
		unknown_handler();
	}
	return res;
}