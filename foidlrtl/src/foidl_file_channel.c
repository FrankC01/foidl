/*
;    foidl_file_channel.c
;    Library support for IO channels
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

#define FILE_CHANNEL_IMPL
#include    <foidlrt.h>
#include    <stdio.h>
#include    <stdlib.h>
#ifdef _MSC_VER
#include <io.h>
#else
#include    <unistd.h>
#endif
#include    <sys/stat.h>


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

globalScalarConst(file_eof,byte_type,(void *) 0xF,1);


static char *stuff[] = {"r", "rb", "w", "wb","w+","wb+","a","ab","a+","ab+"};

// stdin, stdout and stderr

struct FRTIOFileChannelG _cin2_base = {
    global_signature,
    io_class,
    cin_type,
    0,
    0,
    0,
    NULL,//(void *)stdin,
    NULL,
    NULL,
    open_r,
    NULL
};

PFRTAny const cin = (PFRTAny) &_cin2_base.fclass;

struct FRTIOFileChannelG _cout2_base = {
    global_signature,
    io_class,
    cout_type,
    0,
    0,
    0,
    NULL,//(void *)stdin,
    NULL,
    NULL,
    open_w,
    NULL
};

PFRTAny const cout = (PFRTAny) &_cout2_base.fclass;

struct FRTIOFileChannelG _cerr2_base = {
    global_signature,
    io_class,
    cerr_type,
    0,
    0,
    0,
    NULL,//(void *)stdin,
    NULL,
    NULL,
    open_w,
    NULL
};

PFRTAny const cerr = (PFRTAny) &_cerr2_base.fclass;

void foidl_rtl_init_file_channel() {
    _cin2_base.value = (void *) stdin;
    _cin2_base.name = cinstr;
    _cin2_base.ctype = chan_file;
    _cin2_base.render = render_line;

    _cout2_base.value = (void *) stdout;
    _cout2_base.name = coutstr;
    _cout2_base.ctype = chan_file;
    _cout2_base.render = render_line;

    _cerr2_base.value = (void *) stderr;
    _cerr2_base.ctype = chan_file;
    _cerr2_base.name = cerrstr;
    _cerr2_base.render = render_line;
}

// Utility

PFRTAny is_file_read(PFRTIOFileChannel channel) {
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

PFRTAny is_file_text(PFRTIOFileChannel channel) {
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

// Get size of file from file/stream descriptor

static size_t file_size_desc(FILE *fptr) {
    size_t off = 0;
    if(fseek(fptr, 0L, SEEK_END) == -1) {
        unknown_handler();
    }
    off = ftell(fptr);
    if( off == (long) - 1) {
        unknown_handler();
    }
    if(fseek(fptr, 0L, SEEK_SET) == -1) {
        unknown_handler();
    }
    return off;

}

PFRTAny     writeCerrNl(PFRTAny el);

PFRTAny foidl_fexists_qmark(PFRTAny s) {
    PFRTAny     result = true;
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


// Read a line into a string

static PFRTAny read_txt_line(FILE *fptr) {
    PFRTAny reof = file_eof;
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
    return file_eof;
}

// Read a single character

static PFRTAny read_txt_char(FILE *fptr) {
    int ch;
    PFRTAny reof = file_eof;
    if((ch=fgetc(fptr)) != EOF) {
        reof = allocCharWithValue((ft) ch);
    }
    return reof;
}

static PFRTAny read_bin_char(FILE *fptr) {
    return file_eof;
}

// Read a single byte

static PFRTAny read_txt_byte(FILE *fptr) {
    int ch;
    PFRTAny reof = file_eof;
    if((ch=fgetc(fptr)) != EOF) {
        reof = allocAny(scalar_class,byte_type,(void *)(ft)ch);
    }
    return reof;
}

static PFRTAny read_bin_byte(FILE *fptr) {
    return file_eof;
}

static PFRTAny render_txt_read(PFRTIOFileChannel channel) {
    int render = (int) channel->render->value;
    FILE *fp = (FILE *)channel->value;
    PFRTAny feof = file_eof;
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
    PFRTAny feof = file_eof;
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
    return file_eof;
}

static PFRTAny foidl_channel_read_cin(PFRTIOFileChannel channel) {
    char    buffer[1024];
    PFRTAny res = empty_string;
    if(fgets(buffer, sizeof(buffer), (FILE *) channel->value) != NULL) {
        res = allocStringWithCopy(buffer);
    }
    else {
        unknown_handler();
    }
    return res;
}


// Read entry point

PFRTAny foidl_channel_file_read_bang(PFRTAny channel) {
    PFRTAny res = nil;
    PFRTIOFileChannel chan = (PFRTIOFileChannel) channel;
    if(chan->ftype == file_type) {
        res = foidl_channel_readfile(chan);
    }
    else if(chan->ftype == cin_type) {
        res = foidl_channel_read_cin(chan);
    }
    else {
        unknown_handler();
    }
    return res;
}

PFRTAny file_channel_read_next(PFRTIterator i) {
    PFRTAny res = foidl_channel_readfile(
        (PFRTIOFileChannel)((PFRTChannel_Iterator)i)->channel);
    if(res == file_eof)
        res = end;
    return res;
}

PFRTAny foidl_channel_quaf_bang(PFRTAny channel) {

    PFRTAny res = empty_string;
    PFRTIOFileChannel chan = (PFRTIOFileChannel) channel;
    if(chan->ftype == file_type) {
        size_t  buffsize = file_size_desc((FILE *)chan->value);
        char *s = foidl_xall(buffsize+1);
        int x = fread(s,buffsize,1,(FILE *)chan->value);
        res = allocStringWithCptr(s,buffsize);
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
            if(el == nlchr)
                fprintf(chn,"\n");
            else
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

PFRTAny foidl_channel_file_write_bang(PFRTAny channel, PFRTAny el);

static PFRTAny io_file_compound_txt_writer(PFRTAny channel, PFRTAny el) {
    switch(el->ftype) {
        case    vector2_type:
            write_vector(channel, el, foidl_channel_file_write_bang);
            break;
        case    list2_type:
            write_list(channel, el, foidl_channel_file_write_bang);
            break;
        case    set2_type:
            write_set(channel, el, foidl_channel_file_write_bang);
            break;
        case    map2_type:
            write_map(channel, el, foidl_channel_file_write_bang);
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
                foidl_channel_file_write_bang(channel,((PFRTMapEntry)el)->key);
                io_file_scalar_txt_writer(channel->value,spchr);
                foidl_channel_file_write_bang(channel,((PFRTMapEntry)el)->value);
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
        else if(el->fclass == function_class) {
            io_file_scalar_txt_writer(channel->value, fnstr);
        }
        else if(el->fclass == response_class) {
            foidl_channel_writefile(channel, (PFRTAny) el->value);
        }
        else if(el->fclass == worker_class) {
            if(el->ftype == thrdpool_type) {
                io_file_scalar_txt_writer(channel->value, poolstr);
            }
            else if(el->ftype == worker_type) {
                io_file_scalar_txt_writer(channel->value, workstr);
            }
            else {
                unknown_handler();
            }
        }
        else {
            unknown_handler();
        }
    }
    else {
        //return render_bin_read(channel);
    }
    return file_eof;
}

// Writes entry point

PFRTAny foidl_channel_file_write_bang(PFRTAny channel, PFRTAny el) {
    PFRTAny res = nil;
    if(channel->ftype == file_type ) {
        if(is_file_read((PFRTIOFileChannel)channel) == false) {
            foidl_channel_writefile(channel, el);
        }
    }
    else if(channel->ftype == cout_type || channel->ftype == cerr_type) {
        foidl_channel_writefile(channel, el);
    }
    else {
        unknown_handler();
    }
    return res;
}

// File Channel open entry point

PFRTAny foidl_open_file_bang(PFRTAny args) {
    PFRTAny fc2 = nil; //(PFRTIOFileChannel) allocFileChannel(name,mode);
    PFRTAny name = foidl_get(args, chan_target);
    PFRTAny mode = foidl_get(args, chan_mode);
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
        PFRTIOFileChannel fc1 = (PFRTIOFileChannel) allocFileChannel(name,mode,args);
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

PFRTAny foidl_channel_file_close_bang(PFRTAny channel) {
    PFRTAny res = true;
    if(channel->ftype == closed_type)
        return true;

    if(channel->ftype == file_type)
        res = close_file((PFRTIOFileChannel) channel);
    else
        unknown_handler();
    return res;
}

//  Console shortcuts used in RTL

PFRTAny     writeCout(PFRTAny el) {
    return foidl_channel_file_write_bang((PFRTAny) cout,el);
}

PFRTAny     writeCoutNl(PFRTAny el) {
    foidl_channel_file_write_bang((PFRTAny) cout,el);
    return foidl_channel_file_write_bang((PFRTAny) cout,nlchr);
}

PFRTAny     writeCerr(PFRTAny el) {
    return foidl_channel_file_write_bang((PFRTAny) cerr,el);
}

PFRTAny     writeCerrNl(PFRTAny el) {
    foidl_channel_file_write_bang((PFRTAny) cerr,el);
    return foidl_channel_file_write_bang((PFRTAny) cerr,nlchr);
}
