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

PFRTAny     foidl_channel_extension(PFRTAny descriptor) {
    printf("Channel extension invoked\n");
    return nil;
}

// Indirect call to underlying channel type 'open!'

PFRTAny     foidl_open_channel_bang(PFRTAny chan_args) {
    PFRTAny chan_t = foidl_get(chan_args, chan_type);
    if( chan_t == chan_file) {
        return foidl_open_file_bang(
            foidl_get(chan_args, chan_target),
            foidl_get(chan_args, chan_mode),
            chan_args);
    }
    else if( chan_t == chan_http) {
        return foidl_open_http_bang(
            foidl_get(chan_args, chan_target),
            chan_args);
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
    if( foidl_io_qmark(chan) == true) {
        PFRTAny chan_t = foidl_channel_type_qmark(chan);
        if( chan_t == chan_file ) {
            return foidl_channel_file_write_bang(chan, data);
        }
        else if( chan_t == chan_http) {
            return foidl_channel_http_write_bang(chan, data);
        }
    }
    return nil;
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

