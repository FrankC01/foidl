/*
    foidl_http_channel.c
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

struct Chunk {
  char *memory;
  size_t size;
};

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

PFRTAny foidl_channel_http_read_bang(PFRTAny channel) {
    PFRTIOHttpChannel http = (PFRTIOHttpChannel)channel;
    CURL *curl = http->value;
    CURLcode cres;
    struct Chunk mem;
    mem.memory = foidl_xall(1);
    mem.size = 0;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_read_handler);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&mem);
    cres = curl_easy_perform(curl);
    return allocStringWithCptr(mem.memory,strlen(mem.memory));
}

PFRTAny foidl_open_http_bang(PFRTAny name, PFRTAny mode, PFRTAny args) {
    PFRTAny res = nil;
    if( name == nil || mode == nil ) {
        printf("Exception: Requires :target and :mode to open channel\n");
        foidl_error_exit(-1);
    }
    CURL    *curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, name->value);
        res = (PFRTAny) allocHttpChannel(name,mode);
        res->value = (void *)curl;
    }
    return res;
}

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
