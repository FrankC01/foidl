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

PFRTAny foidl_channel_http_read_bang(PFRTAny channel) {
    PFRTIOHttpChannel http = (PFRTIOHttpChannel)channel;
    CURL *curl = http->value;
    CURLcode cres;
    cres = curl_easy_perform(curl);
    return nil;
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
