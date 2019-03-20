/*
    foidl_io2.c
    Library support for IO channels

    Copyright Frank V. Castellucci
    All Rights Reserved

    The IO capability encapsulates interactions mainly with
    the outside world via channels.

    A channel can be used to interact with:
        Files
        Http(s)
        Strings
        Memory

    How the individual is interacted with is through an
    assoication with some kind of handler
        bytes
        characters
        lines (as strings)

    Opening a channel and it's characteristics determines
    which implemention of a channel is used.

*/

#define IO_IMPL2
#include    <foidlrt.h>
#include    <stdio.h>
#ifdef _MSC_VER
#else
#endif

// Common global flags

constKeyword(chan_target,":target");
constKeyword(chan_type,":type");
constKeyword(chan_render,":render");
constKeyword(chan_mode,":mode");

globalScalarConst(chan_file,byte_type,(void *) 0,1);

// For file disposition
globalScalarConst(open_r,byte_type,(void *) 0x0,1);
globalScalarConst(open_rb,byte_type,(void *) 0x1,1);
globalScalarConst(open_w,byte_type,(void *) 0x2,1);
globalScalarConst(open_wb,byte_type,(void *) 0x3,1);
globalScalarConst(open_rw,byte_type,(void *) 0x4,1);
globalScalarConst(open_rwb,byte_type,(void *) 0x5,1);
globalScalarConst(open_a,byte_type,(void *) 0x6,1);
globalScalarConst(open_ab,byte_type,(void *) 0x7,1);
globalScalarConst(open_ar,byte_type,(void *) 0x8,1);
globalScalarConst(open_arb,byte_type,(void *) 0x9,1);

globalScalarConst(eof,byte_type,(void *) 0xF,1);


static char *stuff[] = {"r", "rb", "w", "wb","w+","wb+","a","ab","a+","ab+"};

// For read rendering

globalScalarConst(render_byte,byte_type,(void *) 0,1);
globalScalarConst(render_char,byte_type,(void *) 1,1);
globalScalarConst(render_line,byte_type,(void *) 2,1);
globalScalarConst(render_file,byte_type,(void *) 3,1);

typedef struct _ffile_st {
    long int spos;
    long int epos;
    int      skip;
    int      ahead;
    int      eof;
} FileStat;

static FileStat count_to_nl(FILE *fptr) {
    int ch;
    FileStat ffs = {ftell(fptr),0,0,0,0};
    while((ch = fgetc(fptr)) != EOF) {
        if(ch == 0x0d || ch == 0x0a) {
#ifdef _MSC_VER
            ffs.skip = ffs.ahead=2;
#else
            ffs.skip = ffs.ahead=1;
            if((ch=fgetc(fptr)) != EOF) {
                if(ch == 0x0a || ch == 0x0d) {
                    ffs.skip = ffs.ahead=2;
                }
                else {
                    ffs.ahead = 2;
                }
            }
#endif
            break;
        }
        else {
            ++ffs.epos;
        }
    }
    ffs.eof = feof(fptr);

    return ffs;
}

static PFRTAny readline(FILE *fptr) {
    PFRTAny reof = eof;
    FileStat ffs = count_to_nl(fptr);
    if(ffs.epos) {
        fseek(fptr, 0 - (ffs.epos + ffs.ahead), SEEK_CUR);
        char *s = foidl_xall(ffs.epos+1);
        int x = fread(s,ffs.epos,1,fptr);
        fseek(fptr, ffs.skip, SEEK_CUR);
        reof = allocStringWithCptr(s,ffs.epos);
    }
    return reof;
}

static PFRTAny readchar(FILE *fptr) {
    int ch;
    PFRTAny reof = eof;
    if((ch=fgetc(fptr)) != EOF) {
        reof = allocCharWithValue((ft) ch);
    }
    return reof;
}

static PFRTAny readbyte(FILE *fptr) {
    int ch;
    PFRTAny reof = eof;
    if((ch=fgetc(fptr)) != EOF) {
        reof = allocAny(scalar_class,byte_type,(void *)(ft)ch);
    }
    return reof;
}

static PFRTAny foidl_channel_readfile(PFRTIOFileChannel channel) {
    int render = (int) channel->render->value;
    FILE *fp = (FILE *)channel->value;
    PFRTAny feof = eof;
    switch(render) {
        case    0:
            feof = readbyte(fp);
            break;
        case    1:
            feof = readchar(fp);
            break;
        case    2:
            feof = readline(fp);
            break;
        case    3:
            unknown_handler();
            break;
        default:
            unknown_handler();
            break;

    }
    return feof;
}

// Opens and return channels
// File

PFRTAny foidl_open_file_bang(PFRTAny name, PFRTAny mode, PFRTAny args) {
    PFRTAny fc2 = nil; //(PFRTIOFileChannel) allocFileChannel(name,mode);
    FILE    *fptr;
    ft      imode = (ft) mode->value;
    if(imode >= 0 && imode <= 9) {
        if((imode == 0 || imode == 1) && (foidl_fexists_qmark(name) == false)) {
            return fc2;
        }
        fptr = fopen(name->value, stuff[imode]);
        if(fptr == NULL)
            unknown_handler();
        PFRTIOFileChannel fc1 = (PFRTIOFileChannel) allocFileChannel(name,mode);
        fc1->value = (void *) fptr;
        fc1->render = foidl_getd(args, chan_render, render_line);
        fc2 = (PFRTAny) fc1;
    }
    else {
        unknown_handler();
    }

    return fc2;
}

// Reads from channel

PFRTAny foidl_channel_read_bang(PFRTAny channel) {
    PFRTAny res = nil;
    if(channel->ftype == file_type)
        res = foidl_channel_readfile((PFRTIOFileChannel) channel);
    else
        unknown_handler();
    return res;
}