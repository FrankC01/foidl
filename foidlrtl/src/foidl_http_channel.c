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

/*
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

#define IO_HTTP_CHANNEL
#include    <foidlrt.h>
#ifdef _MSC_VER
#else
#include    <curl/curl.h>
#endif

void foidl_rtl_init_http_channel() {
#ifdef _MSC_VER
#else
     curl_global_init(CURL_GLOBAL_ALL);
#endif
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
    printf("Header: %.*s\n", (int) numbytes, b);
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

// foidl_channel_http_read_bang (<- foidl_channel_http_read! <- reads!)
// Entry point for reading from http channel

PFRTAny foidl_channel_http_read_bang(PFRTAny channel) {
    if(channel->ftype == http_type) {
        PFRTIOHttpChannel http = (PFRTIOHttpChannel)channel;
        CURL *curl = http->value;
        CURLcode cres;
        struct Chunk mem;
        mem.memory = foidl_xall(1);
        mem.size = 0;
        curl_easy_setopt(curl, CURLOPT_URL, http->name->value);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_read_handler);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&mem);
        //curl_easy_setopt(curl, CURLOPT_NOBODY, 1); // For debugging header only
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, http_header_handler);
        cres = curl_easy_perform(curl);

        if(!cres) {
            //printf("Download size: %u\n", (int) mem.size);
        }
        else {
            return nil;
        }
        // TODO: Read handler (e.g. string, json-map, json-list, html-map, html-list)

        return allocStringWithCptr(mem.memory,strlen(mem.memory));
    }
    else {
        printf("Attempting to http read from closed channel\n");
        unknown_handler();
        return nil;
    }
}

// foidl_open_http_bang (<- foidl_open_http! <- opens!)
// Entry point for opening http channel

PFRTAny foidl_open_http_bang(PFRTAny name, PFRTAny mode, PFRTAny args) {
    PFRTAny res = nil;
    if( name == nil || mode == nil ) {
        printf("Exception: Requires :target and :mode to open channel\n");
        foidl_error_exit(-1);
    }
    CURL    *curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        res = (PFRTAny) allocHttpChannel(name,mode);
        res->value = (void *)curl;
    }
    return res;
}

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