/*
	foidl_errors.c
	Library main entry

	Copyright Frank V. Castellucci
	All Rights Reserved
*/

#define ERRORS_IMPL
#include <foidlrt.h>

//
// Const error strings
//

//	General

constString(unknown_error,": call to exit unconditionally");

//	Entry point
constString(rteprefix,"RTE: ");
constString(unsupported,":function not supporting yet");
constString(index_out_of_bounds,"index out of bounds");
constString(update_not_integral, "update: expects a positive number as second argument");
constString(extend_map_two_arg,"extend: extending a map needs argument of two element iterable");
constString(fold_requires_function,"fold: call expects a function reference as 1st argument");
constString(fold_requires_collection,"fold: call expects a iterable reference as 3rd argument");
constString(reduce_requires_function,"reduce: call expects a function reference as 1st argument");
constString(reduce_requires_collection,"reduce: call expects a iterable reference as 2rd argument");

// IO Module errors

constString(not_read_channel,"io: Attempting to read data from invalid or read only channel");
constString(not_write_channel,"io: Attempting to write data to invalid or read only channel");
constString(already_closed_channel,"io: Attempting to close an invalid or already closed channel");
constString(requires_map,"io open: argument must be of non-empty datatype map");
constString(missing_channel,"io open: missing :channel");
constString(missing_open,"io open: missing :open");
constString(unknown_open_flag,"io open: unrecognized :open flag = ");
constString(unknown_buffer_flag,"io open: unrecognized :buffer flag = ");
constString(unknown_reader_flag,"io open: unrecognized :read_handler flag = ");
constString(unknown_writer_flag,"io open: unrecognized :write_handler flag = ");
constString(file_is_directory,"io open: directory operations not supported ");

PFRTAny 	foidl_fail() {
	foidl_error_exit(-1);
	return nil;
}

PFRTAny 	writeCerr(PFRTAny el) {
	return foidl_channel_write_bang((PFRTAny) cerr,el);
}

PFRTAny 	writeCerrNl(PFRTAny el) {
	foidl_channel_write_bang((PFRTAny)cerr,el);
	return foidl_channel_write_bang((PFRTAny)cerr,nlchr);
}

void 	foidl_ep_excp(PFRTAny estring) {
	foidl_channel_write_bang((PFRTAny)cerr,rteprefix);
	writeCerrNl(estring);
	foidl_exit();
}

void 	foidl_ep_excp2(PFRTAny hdr,PFRTAny estring) {
	foidl_channel_write_bang((PFRTAny)cerr,rteprefix);
	foidl_channel_write_bang((PFRTAny)cerr,hdr);
	writeCerrNl(estring);
	foidl_exit();
}

void unknown_handler() {
	foidl_ep_excp(unknown_error);
}
