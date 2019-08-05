/*
;    foidl_http_channel.c
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


#define HTTP_CHANNEL_IMPL
#include    <foidlrt.h>
#ifdef _MSC_VER
#else
#include    <curl/curl.h>
#endif

static int inited = 0;

static PFRTAny chttp_desc;
static PFRTAny curl_http_type_identifier;
static ft curl_http_type_hash;

typedef struct   FRTIOHttpChannel {
    ft          fclass;
    ft          ftype;
    ft          count;
    uint32_t    hash;
    void        *value;         // Maps to curl handle
    PFRTAny     ctype;
    PFRTAny     settings;
    PFRTAny     name;
    PFRTAny     mode;
    PFRTAny     render;
} *PFRTIOHttpChannel;

static PFRTAny allocHttpChannel(PFRTAny name, PFRTAny args) {
    PFRTIOHttpChannel fc = foidl_alloc(sizeof(struct FRTIOHttpChannel));
    fc->fclass = io_class;
    fc->ftype  = curl_http_type_hash;
    fc->ctype  = curl_http_type_identifier;
    fc->name   = name;
    fc->settings = args;
    return (PFRTAny) fc;
}

// Used in http_read_handler operations

struct Chunk {
  char *memory;
  size_t size;
};

// http_header_handler
// Read header callback
// TODO
// Data of potential persistence (map)
// Status Code (number)
// Content-Type (list)
// Content-Length (number)

static size_t http_header_handler(char* b, size_t size, size_t nitems, void *userdata) {
    size_t numbytes = size * nitems;
    //printf("Hdr Size: %zu\n", size);
    //printf("Hdr Size: %zu\n", nitems);
    //printf("Header: %.*s\n", (int) numbytes, b);
    return numbytes;
}

//  http_read_handler
//  Read data callback, reads data in chunks into memory

static size_t http_read_handler(
    void *contents, size_t size,size_t nmemb, void *chunk) {
    size_t realsize = size * nmemb;
    struct Chunk *mem = (struct Chunk *)chunk;
    char *ptr = foidl_xreall(mem->memory, mem->size + realsize + 1);
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}

PFRTAny foidl_channel_http_write_bang(PFRTAny channel, PFRTAny data) {
    PFRTAny result = nil;
    PFRTIOHttpChannel http = (PFRTIOHttpChannel)channel;
    CURL *curl = http->value;
    CURLcode cres;
    struct Chunk mem;
    mem.memory = foidl_xall(1);
    mem.size = 0;
    curl_easy_setopt(curl, CURLOPT_URL, http->name->value);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&mem);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data->value);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)data->count);
    //curl_easy_setopt(curl, CURLOPT_NOBODY, 1); // For debugging header only
    cres = curl_easy_perform(curl);

    if(!cres) {
        result = (PFRTAny)
            allocResponse(
                http->ftype,
                (PFRTAny) allocStringWithCptr(mem.memory,strlen(mem.memory)));
    }
    else {
        ;
    }
    return result;
}
localFunc(curl_ch_write,2,foidl_channel_http_write_bang);

// foidl_channel_http_read_bang (<- foidl_channel_http_read! <- reads!)
// Entry point for reading from http channel

PFRTAny foidl_channel_http_read_bang(PFRTAny channel) {
    PFRTAny result = nil;
    PFRTIOHttpChannel http = (PFRTIOHttpChannel)channel;
    CURL *curl = http->value;
    CURLcode cres;
    struct Chunk mem;
    mem.memory = foidl_xall(1);
    mem.size = 0;
    curl_easy_setopt(curl, CURLOPT_URL, http->name->value);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&mem);
    //curl_easy_setopt(curl, CURLOPT_NOBODY, 1); // For debugging header only
    cres = curl_easy_perform(curl);

    if(!cres) {
        result = (PFRTAny)
            allocResponse(
                http->ftype,
                (PFRTAny) allocStringWithCptr(mem.memory,strlen(mem.memory)));
        //printf("Download size: %u\n", (int) mem.size);
    }
    else {
        ;
    }
    return result;
}
localFunc(curl_ch_read,1,foidl_channel_http_read_bang);

// foidl_open_http_bang (<- foidl_open_http! <- opens!)
// Entry point for opening http channel

PFRTAny foidl_open_http_bang(PFRTAny args) {
    PFRTAny res = nil;
    PFRTAny name = foidl_get(args, chan_target);
    if( name == nil ) {
        printf("Exception: Requires chan_target to open channel\n");
        foidl_error_exit(-1);
    }
    CURL    *curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_read_handler);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, http_header_handler);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
        res = allocHttpChannel(name,args);
        res->value = (void *)curl;
    }
    return res;
}
localFunc(curl_ch_open,1,foidl_open_http_bang);

// foidl_channel_http_close_bang (<- foidl_channel_http_close! <- closes!)
// Entry point for closing a http channel

PFRTAny foidl_channel_http_close_bang(PFRTAny channel) {
    PFRTAny res = true;
    if(channel->ftype == closed_type) {
        ;
    }
    else {
        channel->ftype = closed_type;
        curl_easy_cleanup((CURL*) channel->value);
        channel->value = (void *) 0;
    }
    return res;
}
localFunc(curl_ch_close,1,foidl_channel_http_close_bang);

// Setup curl initialization

void foidl_rtl_init_http_channel() {
#ifdef _MSC_VER
#else
     curl_global_init(CURL_GLOBAL_ALL);
#endif
}

// Setup the extension

static void setup_desc(PFRTAny tname) {
    // Setup function indirects
    PFRTAny func_map = foidl_map_inst_bang();
    foidl_map_extend_bang(func_map, channel_ext_open, (PFRTAny) curl_ch_open);
    foidl_map_extend_bang(func_map, channel_ext_read, (PFRTAny) curl_ch_read);
    foidl_map_extend_bang(func_map, channel_ext_write, (PFRTAny) curl_ch_write);
    foidl_map_extend_bang(func_map, channel_ext_close, (PFRTAny) curl_ch_close);

    // Build descriptor
    chttp_desc = foidl_map_inst_bang();
    foidl_map_extend_bang(chttp_desc, ext_type, channel_ext);
    foidl_map_extend_bang(chttp_desc, ext_subtype, tname);
    foidl_map_extend_bang(chttp_desc, ext_interface, nil);
    foidl_map_extend_bang(chttp_desc, ext_functions, func_map);
    //writeCoutNl(chttp_desc);
    return;
}

// Register extension
PFRTAny     foidl_register_curl_http(PFRTAny tname) {
    if( ! inited ) {
        inited=1;
        curl_http_type_identifier = tname;
        curl_http_type_hash = (ft) hash(tname);
        foidl_rtl_init_http_channel();
        setup_desc(tname);
        foidl_channel_extension(chttp_desc);
    }
    else {
        printf("Curl http already initialized\n");
    }
    return chttp_desc;
}