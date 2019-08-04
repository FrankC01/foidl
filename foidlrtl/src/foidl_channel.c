/*
; foidl_channel.c
;   Generic channel RTL
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

#define CHANNEL_IMPL
#include <foidlrt.h>
#include <stdio.h>

// Exensible type keys
constKeyword(channel_ext,":channel_extension");
constKeyword(channel_ext_init,":channel_ext_init");
constKeyword(channel_ext_open,":channel_ext_open");
constKeyword(channel_ext_read,":channel_ext_read");
constKeyword(channel_ext_write,":channel_ext_write");
constKeyword(channel_ext_close,":channel_ext_close");
constKeyword(channel_ext_iterator,":channel_ext_iterator");
constKeyword(channel_ext_iterator_next,":channel_ext_iterator_next");


PFRTAny     foidl_channel_extension(PFRTAny descriptor) {
    return register_extension(descriptor);
}

PFRTAny     foidl_channel_get_extension(PFRTAny esubtype) {
    printf("Channel extension subtype invoked\n");
    return nil;
}

// Indirect call to underlying channel type 'open!'

PFRTAny     foidl_open_channel_bang(PFRTAny chan_args) {
    PFRTAny chan_t = foidl_get(chan_args, chan_type);
    if( chan_t == chan_file) {
        return foidl_open_file_bang(chan_args);
    }
    else if( chan_t == chan_http) {
        return foidl_open_http_bang(chan_args);
    }
    return nil;
}

// Indirect call to underlying channel 'read!'

PFRTAny     foidl_channel_read_bang(PFRTAny chan) {
    if( foidl_io_qmark(chan) == true) {
        PFRTAny chan_t = foidl_channel_type_qmark(chan);
        if( chan_t == chan_file ) {
            return foidl_channel_file_read_bang(chan);
        }
        else if( chan_t == chan_http) {
            return foidl_channel_http_read_bang(chan);
        }
    }
    return nil;
}

// Indirect call to underlying channel 'write!'

PFRTAny     foidl_channel_write_bang(PFRTAny chan, PFRTAny data) {
    PFRTAny res = nil;
    if( foidl_io_qmark(chan) == true) {
        PFRTAny chan_t = foidl_channel_type_qmark(chan);
        if( chan_t == chan_file ) {
            res = foidl_channel_file_write_bang(chan, data);
        }
        else if( chan_t == chan_http) {
            res = foidl_channel_http_write_bang(chan, data);
        }
    }
    return res;
}

// Indirect call to underlying channel 'close!'

PFRTAny     foidl_channel_close_bang(PFRTAny chan) {
    if( foidl_io_qmark(chan) == true) {
        PFRTAny chan_t = foidl_channel_type_qmark(chan);
        if( chan_t == chan_file ) {
            return foidl_channel_file_close_bang(chan);
        }
        else if( chan_t == chan_http) {
            return foidl_channel_http_close_bang(chan);
        }
    }
    return nil;
}

