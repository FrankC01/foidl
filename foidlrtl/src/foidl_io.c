/*
	foidl_io.c
	Library support for IO channels

	Copyright Frank V. Castellucci
	All Rights Reserved
*/

#define IO_IMPL
#include	<foidlrt.h>
#include 	<stdio.h>
#ifdef _MSC_VER
#include <io.h>
#else
#include 	<unistd.h>
#endif
#include 	<sys/stat.h>
//#include 	<stdio.h>


//
//	Well known channel open keywords
//

// Keys and values

constKeyword(channelKW,":channel"); 	//	Refers to string

constKeyword(openKW,":open"); 			//	Refers to how to open channel
constKeyword(openReadKW,":r");
constKeyword(openWriteKW,":w");
constKeyword(openReadWriteKW,":wr");
constKeyword(openAppendKW,":a");

constKeyword(bufferKW,":buffer"); 		//	Refers to buffer type for channel
constKeyword(bufferMmapKW,":mmap");
constKeyword(bufferBlockKW,":block");
constKeyword(bufferNoneKW,":none");

constKeyword(readHandlerKW,":read_handler");
constKeyword(writeHandlerKW,":write_handler");

constKeyword(byteKW,":byte");		//	For both read and write channel control
constKeyword(charKW,":char"); 	//	ditto
constKeyword(stringKW,":string"); //  ditto


struct chanFlags {
	ft 	oflag;
	ft  mflag;
	ft 	hflag;
};

static struct chanFlags setupChanFromDecl(PFRTAny decl) {
	struct chanFlags flags = {.oflag  = -1, .mflag = -1,.hflag = -1};

	if(decl->fclass == collection_class && decl->ftype == map2_type &&
		decl->count != 0) {
		if( map_contains_qmark(decl,channelKW) == false ) {
			writeCerrNl(missing_channel);
			foidl_fail();
		}
		if( map_contains_qmark(decl,openKW) == false ) {
			writeCerrNl(missing_open);
			foidl_fail();
		}
		else {
			//	Open condition
			PFRTAny oval = map_get(decl,openKW);
			if(scalar_equality(oval,openReadKW) == true)
				flags.oflag = open_read_only;
			else if(scalar_equality(oval,openWriteKW) == true)
				flags.oflag = open_write_only;
			else if(scalar_equality(oval,openReadWriteKW) == true)
				flags.oflag = open_read_write;
			else if(scalar_equality(oval,openAppendKW) == true)
				flags.oflag = open_write_append;
			else {
				writeCerr(unknown_open_flag);
				writeCerrNl(oval);
				foidl_fail();
			}
		}
			// Read type
		if( map_contains_qmark(decl,readHandlerKW) == true ) {
			PFRTAny oval = map_get(decl,readHandlerKW);
			if(scalar_equality(oval,byteKW) == true)
				flags.hflag = read_byte;
			else if(scalar_equality(oval,charKW) == true)
				flags.hflag = read_char;
			else if(scalar_equality(oval,stringKW) == true)
				flags.hflag = read_string;
			else {
				writeCerr(unknown_reader_flag);
				writeCerrNl(oval);
				flags.hflag = read_char;
			}
		}
			//	Write type
		else if( map_contains_qmark(decl,writeHandlerKW) == true ) {
			PFRTAny oval = map_get(decl,writeHandlerKW);
			if(scalar_equality(oval,byteKW) == true)
				flags.hflag = write_byte;
			else if(scalar_equality(oval,charKW) == true)
				flags.hflag = write_char;
			else if(scalar_equality(oval,stringKW) == true)
				flags.hflag = write_string;
			else {
				writeCerr(unknown_writer_flag);
				writeCerrNl(oval);
				flags.hflag = write_char;
			}

		}
			//	Buffer should check r/w flags to support
		if( map_contains_qmark(decl,bufferKW) == true ) {
			PFRTAny oval = map_get(decl,bufferKW);
			if(scalar_equality(oval,bufferMmapKW) == true)
				flags.mflag = mem_map_buffer;
			else if(scalar_equality(oval,bufferBlockKW) == true)
				flags.mflag = mem_block_buffer;
			else if(scalar_equality(oval,bufferNoneKW) == true)
				flags.mflag = no_buffer;
			else {
				writeCerr(unknown_buffer_flag);
				writeCerrNl(oval);
				foidl_fail();
			}
		}
		else {
			switch(flags.oflag) {
				case 	open_read_write:
				case 	open_read_only:
					flags.mflag = mem_map_buffer;
					break;
				case 	open_write_only:
				case 	open_write_append:
					flags.mflag = mem_block_buffer;
					break;
				default:
					flags.mflag = no_buffer;
					break;
			}
		}
	}
	else {
		writeCerrNl(requires_map);
		foidl_fail();
	}
	return flags;
}

size_t fileSizeInfo(PFRTAny name) {
	#if _MSC_VER
	struct _stat64 buffer;
	int status = _stat64(name->value, &buffer);
	#else
	struct stat buffer;
	int status = stat(name->value, &buffer);
	#endif
	//printf("fileSizeInfo for %s\n",name->value );
	if( status == - 1) {
		perror("io open error: ");
		foidl_fail();
	}
	#ifdef _MSC_VER
	else if (_S_IFDIR & buffer.st_mode) {
	#else
	else if (S_ISDIR(buffer.st_mode)) {
	#endif
		writeCerrNl(file_is_directory);
		foidl_fail();
	}
	#ifdef _MSC_VER
	else if (! (_S_IFREG & buffer.st_mode) ) {
	#else
	else if (! S_ISREG(buffer.st_mode)) {
	#endif
		foidl_fail();
	}
	else {
		;
	}

	return buffer.st_size;
}

PFRTAny foidl_fexists_qmark(PFRTAny s) {
	PFRTAny 	result = true;
	if(string_type_qmark(s) == false)
		return false;
	#if _MSC_VER
	struct _stat64 buffer;
	int status = _stat64(s->value, &buffer);
	#else
	struct stat buffer;
	int status = stat(s->value, &buffer);
	#endif
	if( status == - 1)
		return false;
	result = true;
	#ifdef _MSC_VER
	if (_S_IFDIR & buffer.st_mode) {
	#else
	if (S_ISDIR(buffer.st_mode)) {
	#endif
		writeCerrNl(file_is_directory);
		foidl_fail();
	}
	#ifdef _MSC_VER
	else if (! (_S_IFREG & buffer.st_mode) ) {
	#else
	else if (! S_ISREG(buffer.st_mode)) {
	#endif
		foidl_fail();
	}
	else {
		;
	}
	return result;
}


static PFRTIOChannel openFileRead(PFRTIOChannel chn) {
	//printf("Opening file for reading = %s\n", chn->name->value);
	size_t sz = fileSizeInfo(chn->name);
	switch(chn->buffertype) {
		case mem_map_buffer:
			{
				void *mm = foidl_open_ro_mmap_file(chn->name->value, sz);
				chn->bufferptr = allocIOMMapBuffer(mm, sz);
			}
			break;
		case mem_block_buffer:
			{
				chn->handle = foidl_fopen_read_only(chn->name->value);
				chn->bufferptr = allocIOBlockBuffer((ft) sz);
			}
			break;
		case no_buffer:
			{
				chn->handle = foidl_fopen_read_only(chn->name->value);
				chn->bufferptr = allocIONoBuffer();
			}
			break;
	}
	return chn;
}

static const ft defblocksz = 4096;
static PFRTIOChannel openFileWrite(PFRTIOChannel chn) {
	//printf("Opening file for reading = %s\n", chn->name->value);
	switch(chn->buffertype) {
		case mem_map_buffer:
				unknown_handler();
				//chn->bufferptr = allocIOMMapBuffer(open_ro_mmap_file(chn->name->value));
			break;
		case mem_block_buffer:
			{
				chn->handle = foidl_fopen_create_truncate(chn->name->value);
				chn->bufferptr = allocIOBlockBuffer(defblocksz);
			}
			break;
		case no_buffer:
			{
				chn->handle = foidl_fopen_create_truncate(chn->name->value);
				chn->bufferptr = allocIONoBuffer();
			}
			break;
	}
	return chn;
}

static PFRTIOChannel filePrepare(PFRTIOChannel chn) {
	switch(chn->openflag) {
		case open_read_only:
			openFileRead(chn);
			break;
		case open_write_only:
			openFileWrite(chn);
			break;
		case open_read_write:
			unknown_handler();
			break;
		case open_write_append:
			unknown_handler();
			break;
	}
	return chn;
}

//	Create the channel, set the handlers

static PFRTIOChannel fileChannel(PFRTAny name,struct chanFlags flags) {
	PFRTIOChannel chn = allocIOChannel(file_type,name);
	chn->openflag = flags.oflag;
	chn->buffertype = flags.mflag;
	//	Setup the read/write handlers
	switch(chn->openflag) {
		case open_read_only:
			switch(flags.hflag) {
				case 	read_byte:
					chn->readhandler = byte_reader;
					break;
				case 	read_char:
					chn->readhandler = char_reader;
					break;
				case 	read_string:
					chn->readhandler = string_reader;
					break;
			}
			break;
		case open_write_only:
			switch(flags.hflag) {
				case 	write_char:
					chn->writehandler = char_writer;
					break;
				default:
					unknown_handler();
					break;
			}
			break;
		default:
			unknown_handler();
			break;
	}

	return filePrepare(chn);
}


//	TODO: Drop file assumption, extend for
//	http(s), IP, memory

PFRTAny 	foidl_write_bang(PFRTIOChannel,PFRTAny);
PFRTAny 	foidl_open_bang(PFRTAny with) {
	//foidl_write_bang(cerr,with);
	struct chanFlags flags = setupChanFromDecl(with);
	PFRTIOChannel channel = fileChannel(map_get(with,channelKW),flags);
	channel->channelMap = with;
	return (PFRTAny) channel;
}

void release_io(PFRTAny a) {

}

PFRTAny io_drop(PFRTAny a) {
	return a;
}

//
//	Channel Reading
//

static PFRTAny 	canRead(PFRTIOChannel chan) {
	if(chan->fclass == io_class
		&& (chan->openflag == open_read_only
			|| chan->openflag == open_read_write)
		&& chan->ftype != closed_type)
		return true;
	else
		return false;
}

PFRTAny 	foidl_read_bang(PFRTIOChannel chan) {
	if(canRead(chan) == true)
		return ((io_readFuncPtr) chan->readhandler->fnptr)(chan);
	else {
		writeCerrNl(not_read_channel);
	}
	return nil;
}

PFRTAny 	foidl_unread_bang(PFRTIOChannel chan) {
	if(canRead(chan) == true)
		return io_unread(chan);
	else {
		writeCerrNl(not_read_channel);
	}
	return nil;
}

PFRTAny io_take(PFRTAny chn, PFRTAny x) {
	return chn;
}

PFRTAny foidl_take_until_bang(PFRTIOChannel chan,PFRTAny pred) {
	if(canRead(chan) == true)
		return io_take_until(chan,pred);
	else {
		writeCerrNl(not_read_channel);
	}
	return nil;
}

PFRTAny foidl_take_string_bang(PFRTIOChannel chan) {
	if(canRead(chan) == true)
		return io_take_string(chan);
	else {
		writeCerrNl(not_read_channel);
	}
	return nil;
}

PFRTAny foidl_take_qchar_bang(PFRTIOChannel chan) {
	if(canRead(chan) == true)
		return io_take_qchar(chan);
	else {
		writeCerrNl(not_read_channel);
	}
	return nil;
}

PFRTAny foidl_skipto_bang(PFRTIOChannel chan, PFRTAny exp) {
	if(canRead(chan) == true)
		return io_skipto((PFRTIOChannel) chan,exp);
	else
		writeCerrNl(not_read_channel);
	return nil;
}

PFRTAny foidl_skipwhile_bang(PFRTIOChannel chan, PFRTAny exp) {
	if(canRead(chan) == true)
		return io_skipwhile(chan,exp);
	else
		writeCerrNl(not_read_channel);
	return nil;
}

PFRTAny foidl_io_count(PFRTIOChannel chan) {
	if(canRead(chan) == true)
		return io_count(chan);
	else
		return nil;
}

PFRTAny foidl_io_countto(PFRTIOChannel chan, PFRTAny exp) {
	if(canRead(chan) == true)
		return io_countto(chan,exp);
	else
		return nil;
}

PFRTAny foidl_io_countnotto(PFRTIOChannel chan, PFRTAny exp) {
	return zero;
}

PFRTAny foidl_io_first(PFRTIOChannel chan) {
	if(canRead(chan) == true)
		return io_first(chan);
	else
		return nil;
}

PFRTAny foidl_io_second(PFRTIOChannel chan) {
	if(canRead(chan) == true)
		return io_second(chan);
	else
		return nil;
}

PFRTAny foidl_io_line(PFRTIOChannel chan) {
	if(canRead(chan) == true)
		return foidl_reg_intnum(chan->bufferptr->current_line);
	else
		return nil;
}

PFRTAny foidl_io_pos(PFRTIOChannel chan) {
	if(canRead(chan) == true)
		return foidl_reg_intnum(chan->bufferptr->current_pos);
	else
		return nil;
}

PFRTAny foidl_peek_qmark(PFRTIOChannel chan, PFRTAny arg) {
	if(canRead(chan) == true && arg->ftype == string_type)
		return io_peek_match(chan, arg);
	return false;
}


//
// Channel writing
//


static PFRTAny 	canWrite(PFRTIOChannel chan) {
	if(chan->openflag == open_write_only ||
		chan->openflag == open_read_write ||
		chan->openflag == open_write_append)
		return true;
	else
		return false;
}

PFRTAny 	foidl_write_bang(PFRTIOChannel chan, PFRTAny value) {
	if(chan->fclass == io_class
		&& chan->ftype != closed_type
		&& canWrite(chan) == true)
		return ((io_writeFuncPtr) chan->writehandler->fnptr)(chan,value);
	else
		writeCerrNl(not_write_channel);
	return nil;
}

PFRTAny 	foidl_close_bang(PFRTIOChannel chan) {
	if(chan->fclass == io_class
		&& chan->ftype != closed_type
		&& (chan != cin || chan != cout || chan != cerr)) {
			chan->ftype = closed_type;
			if(chan->openflag == open_write_only) {
				io_blockBufferClose(chan);
				foidl_fclose(chan->handle);
			}
			deallocChannelBuffer(chan);
	}
	else
		writeCerrNl(already_closed_channel);

	return (PFRTAny) chan;
}

PFRTAny foidl_io_quaf_bang(PFRTIOChannel chan) {
	if(canRead(chan) == true) {
		PFRTIOBuffer b = chan->bufferptr;
		if(b->buffersize == 0)
			return empty_string;
		else {
			return allocStringWithCopyCnt(b->buffersize, b->bufferPtr);
		}
	}
	else {
		return nil;
	}
}

