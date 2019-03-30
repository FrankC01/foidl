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
#include    <stdlib.h>


// Common global flags

constKeyword(chan_target,":target");
constKeyword(chan_type,":type");
constKeyword(chan_render,":render");
constKeyword(chan_mode,":mode");

globalScalarConst(chan_file,byte_type,(void *) 0,1);
globalScalarConst(chan_http,byte_type,(void *) 1,1);

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

// Utility

static PFRTAny is_file_read(PFRTIOFileChannel channel) {
    switch((ft) channel->mode->value) {
        case 0:
        case 1:
        case 4:
        case 5:
        case 8:
        case 9:
            return true;
    }
    return false;
}

static PFRTAny is_file_text(PFRTIOFileChannel channel) {
    switch((ft) channel->mode->value) {
        case 0:
        case 2:
        case 4:
        case 6:
        case 8:
            return true;
    }
    return false;
}

// Line reader to nl information structure
typedef struct _ffile_st {
    long int spos;
    long int epos;
    int      skip;
    int      ahead;
    int      eof;
} FileStat;

// Counts characters to nl or EOF

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


// Read a line into a string

static PFRTAny read_txt_line(FILE *fptr) {
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

static PFRTAny read_bin_line(FILE *fptr) {
    return eof;
}

// Read a single character

static PFRTAny read_txt_char(FILE *fptr) {
    int ch;
    PFRTAny reof = eof;
    if((ch=fgetc(fptr)) != EOF) {
        reof = allocCharWithValue((ft) ch);
    }
    return reof;
}

static PFRTAny read_bin_char(FILE *fptr) {
    return eof;
}

// Read a single byte

static PFRTAny read_txt_byte(FILE *fptr) {
    int ch;
    PFRTAny reof = eof;
    if((ch=fgetc(fptr)) != EOF) {
        reof = allocAny(scalar_class,byte_type,(void *)(ft)ch);
    }
    return reof;
}

static PFRTAny read_bin_byte(FILE *fptr) {
    return eof;
}

static PFRTAny render_txt_read(PFRTIOFileChannel channel) {
    int render = (int) channel->render->value;
    FILE *fp = (FILE *)channel->value;
    PFRTAny feof = eof;
    switch(render) {
        case    0:
            feof = read_txt_byte(fp);
            break;
        case    1:
            feof = read_txt_char(fp);
            break;
        case    2:
            feof = read_txt_line(fp);
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

static PFRTAny render_bin_read(PFRTIOFileChannel channel) {
    int render = (int) channel->render->value;
    FILE *fp = (FILE *)channel->value;
    PFRTAny feof = eof;
    switch(render) {
        case    0:
            feof = read_bin_byte(fp);
            break;
        case    1:
            feof = read_bin_char(fp);
            break;
        case    2:
            feof = read_bin_line(fp);
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
// General read-file function

static PFRTAny foidl_channel_readfile(PFRTIOFileChannel channel) {
    if( is_file_text(channel) ) {
        return render_txt_read(channel);
    }
    else {
        return render_bin_read(channel);
    }
    return eof;
}

// Read entry point

PFRTAny foidl_channel_read_bang(PFRTAny channel) {
    PFRTAny res = nil;
    PFRTIOFileChannel chan = (PFRTIOFileChannel) channel;
    if(chan->ftype == file_type) {
        res = foidl_channel_readfile(chan);
    }
    else {
        unknown_handler();
    }
    return res;
}

// Scalar writer

static void io_file_scalar_txt_writer(FILE *chn, PFRTAny el) {
    switch(el->ftype) {
        case    keyword_type:
        case    string_type:
            fprintf(chn,"%s", el->value);
            break;
        case    regex_type:
            {
                PFRTRegEx rel = (PFRTRegEx)el;
                fprintf(chn,"%s", rel->value->value);
            }
            break;
        case    character_type:
            fprintf(chn, "%c", (int) el->value );
            break;
        case    byte_type:
            {
                char buffer[5];
                snprintf(buffer, 5, "%d", (int)el->value);
                fprintf(chn, "%s", buffer);
            }
            break;
        case    nil_type:
            fprintf(chn,"%s", nilstr->value);
            break;
        case    end_type:
            fprintf(chn,"%s", endstr->value);
            break;
        case    boolean_type:
            if((ft) el->value == 0)
                fprintf(chn,"%s", falsestr->value);
            else
                fprintf(chn,"%s", truestr->value);
            break;
        case    number_type:
            {
                char *tmp = number_tostring(el);
                fprintf(chn,"%s", tmp);
                foidl_xdel(tmp);
            }
            break;
        default:
            unknown_handler();
            break;
    }

}

PFRTAny foidl_channel_write_bang(PFRTAny channel, PFRTAny el);

static PFRTAny io_file_compound_txt_writer(PFRTAny channel, PFRTAny el) {
    switch(el->ftype) {
        case    vector2_type:
            write_vector(channel, el, foidl_channel_write_bang);
            break;
        case    list2_type:
            write_list(channel, el, foidl_channel_write_bang);
            break;
        case    set2_type:
            write_set(channel, el, foidl_channel_write_bang);
            break;
        case    map2_type:
            write_map(channel, el, foidl_channel_write_bang);
            break;
        case    series_type:
            if((PFRTSeries) el == infinite)
                io_file_scalar_txt_writer(channel->value, infserstr);
            else
                io_file_scalar_txt_writer(channel->value, seriesstr);
            break;
        case    mapentry_type:
            {
                io_file_scalar_txt_writer(channel->value,lbracket);
                foidl_channel_write_bang(channel,((PFRTMapEntry)el)->key);
                io_file_scalar_txt_writer(channel->value,spchr);
                foidl_channel_write_bang(channel,((PFRTMapEntry)el)->value);
                io_file_scalar_txt_writer(channel->value,rbracket);
            }
            break;
        default:
            unknown_handler();
            break;
    }
    return nil;
}

// Write file

static PFRTAny foidl_channel_writefile(PFRTAny channel, PFRTAny el) {
    if( is_file_text((PFRTIOFileChannel)channel) ) {
        //return render_txt_read(channel);
        if(el->fclass == scalar_class) {
            io_file_scalar_txt_writer(channel->value, el);
        }
        else if(el->fclass == collection_class) {
            io_file_compound_txt_writer(channel, el);
        }
        else {
            unknown_handler();
        }
    }
    else {
        //return render_bin_read(channel);
    }
    return eof;
}

// Writes entry point

PFRTAny foidl_channel_write_bang(PFRTAny channel, PFRTAny el) {
    PFRTAny res = nil;
    if(channel->ftype == file_type) {
        if(is_file_read((PFRTIOFileChannel)channel) == false) {
            foidl_channel_writefile(channel, el);
        }
    }
    else {
        ;
    }
    return res;
}

// Channel open entry point

PFRTAny foidl_open_file_bang(PFRTAny name, PFRTAny mode, PFRTAny args) {
    PFRTAny fc2 = nil; //(PFRTIOFileChannel) allocFileChannel(name,mode);
    FILE    *fptr;
    ft      imode = (ft) mode->value;
    if( name == nil || mode == nil ) {
        printf("Exception: Requires :target and :mode to open channel\n");
        foidl_error_exit(-1);
    }
    if(imode >= 0 && imode <= 9) {
        if((imode == 0 || imode == 1) && (foidl_fexists_qmark(name) == false)) {
            return fc2;
        }
        fptr = fopen(name->value, stuff[imode]);
        if(fptr == NULL)
            unknown_handler();
        PFRTIOFileChannel fc1 = (PFRTIOFileChannel) allocFileChannel(name,mode);
        fc1->value = (void *) fptr;
        // If reading, check for render statement
        if( imode == 0 || imode == 1) {
            fc1->render = foidl_getd(args, chan_render, render_line);
        }
        fc2 = (PFRTAny) fc1;
    }
    else {
        unknown_handler();
    }

    return fc2;
}

// Close file

static PFRTAny close_file(PFRTIOFileChannel fc) {
    PFRTAny res = true;
    if( fclose((FILE *)fc->value) == EOF) {
        res = false;
    }
    fc->ftype = closed_type;
    return res;
}

// Close channel entry point

PFRTAny foidl_channel_close_bang(PFRTAny channel) {
    PFRTAny res = true;
    if(channel->ftype == closed_type)
        return true;

    if(channel->ftype == file_type)
        res = close_file((PFRTIOFileChannel) channel);
    else
        unknown_handler();
    return res;
}

