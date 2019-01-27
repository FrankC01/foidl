/*
	foidl_series.c
	Library series support

	Copyright Frank V. Castellucci
	All Rights Reserved
*/

#define SERIES_IMPL
#include <foidlrt.h>

/*
Series - a specialized data type that lazily serves up the
'next' element in the series.

Infinite - A specific series with a start of 0, step of 1 and
has no end

Constructs:
infinite = series: nil, nil, nil
finite = series: 0, 16, 1 (0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15)
finite = series: 16, -1, -1 (reverse of above)
*/


static PFRTAny 	ss_default_step(PFRTAny s, PFRTAny l) {
	return foidl_add(s,l);
}

globalFuncConst(ss_defstep,2,ss_default_step);

static PFRTAny 	ss_default_end(PFRTAny e,PFRTAny last) {
	return foidl_gteq_qmark(e, last);
	//return foidl_equal_qmark(e,last);
}

globalFuncConst(ss_defend,2,ss_default_end);

static PFRTAny 	ss_never_end(PFRTAny e,PFRTAny last) {
	return false;
}

globalFuncConst(ss_nevend,2,ss_never_end);


struct FRTSeriesG _inf_series = {
	global_signature,
	collection_class,
	series_type,
	0,0,
	0,0,0 	//	Populated by series RTL init (see below)
};

PFRTSeries const infinite = (PFRTSeries) &_inf_series.fclass;

//	Setup from RTL init

void foidl_rtl_init_series() {
	_inf_series.start = zero;
	_inf_series.step = one;
	_inf_series.stop = false;
}

//	API

PFRTAny 	foidl_series(PFRTAny start, PFRTAny stop, PFRTAny step) {

	if(start == nil && end == nil && step == nil)
		return (PFRTAny) infinite;

	PFRTSeries 	result = allocSeries();
	if(start == nil) {
		result->start = zero;
	}
	else if(foidl_function_qmark(start) == true &&
		function_strict_arg(start,zero)) {
		result->start = start;
	}
	else if(start->ftype == number_type){
		result->start = start;
	}
	else
		unknown_handler();

	if(step == nil) {
		result->step = one;
	}
	else if(foidl_function_qmark(step) == true &&
		function_strict_arg(step,one)) {
		result->step = step;
	}
	else if(step->ftype == number_type) {
		result->step = step;
	}
	else
		unknown_handler();

	if(stop == nil) {
		result->stop  = false;
	}
	else if(foidl_function_qmark(stop) == true &&
		function_strict_arg(stop,one)) {
		result->stop = stop;
	}
	else if(stop->ftype == number_type) {
		result->stop  = stop;
	}
	else
		unknown_handler();

	return (PFRTAny) result;
}