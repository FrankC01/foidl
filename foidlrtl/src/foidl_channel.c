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


// Register a channel extension
PFRTAny     foidl_channel_extension(PFRTAny descriptor) {
    return register_extension(descriptor);
}

// Resolve function lookup in extension
static PFRTAny channel_get_extended_function(PFRTAny esubtype, PFRTAny func_type) {
    PFRTAny func_map = extension_functions_for(channel_ext, esubtype);
    if( func_map == nil ) {
        printf("No channel extension exists for %s\n", esubtype->value);
        unknown_handler();
    }
    PFRTAny func_ref = map_get(func_map, func_type);
    if( func_ref == nil ) {
        printf("Function %s not defined in channel extensions for %s\n",
            func_type->value,
            esubtype->value);
        unknown_handler();
    }
    return func_ref;
}

// Invoke single argument indirect function
static PFRTAny call_extension_1(PFRTAny subtype, PFRTAny func_type, PFRTAny arg1) {
    PFRTAny ext_func = channel_get_extended_function(subtype, func_type);
    return dispatch1i(ext_func, arg1);
}

// Invoke two argument indirect function
static PFRTAny call_extension_2(PFRTAny subtype, PFRTAny func_type,
    PFRTAny arg1, PFRTAny arg2) {
    PFRTAny ext_func = channel_get_extended_function(subtype, func_type);
    return dispatch2i(ext_func, arg1, arg2);
}

// Call to underlying channel type 'open!'

PFRTAny     foidl_open_channel_bang(PFRTAny chan_args) {
    PFRTAny chan_t = foidl_get(chan_args, chan_type);
    PFRTAny result = nil;
    if( chan_t == chan_file) {
        result = foidl_open_file_bang(chan_args);
    }
    else {
        result = call_extension_1(chan_t, channel_ext_open, chan_args);
    }
    return result;
}

// Call to underlying channel 'read!'

PFRTAny     foidl_channel_read_bang(PFRTAny chan) {
    PFRTAny result = nil;
    if( foidl_io_qmark(chan) == true) {
        PFRTAny chan_t = foidl_channel_type_qmark(chan);
        if( chan_t == chan_file ) {
            result = foidl_channel_file_read_bang(chan);
        }
        else {
            result = call_extension_1(chan_t, channel_ext_read, chan);
        }
    }
    return result;
}

// Call to underlying channel 'write!'

PFRTAny     foidl_channel_write_bang(PFRTAny chan, PFRTAny data) {
    PFRTAny result = nil;
    if( foidl_io_qmark(chan) == true) {
        PFRTAny chan_t = foidl_channel_type_qmark(chan);
        if( chan_t == chan_file ) {
            result = foidl_channel_file_write_bang(chan, data);
        }
        else {
            result = call_extension_2(chan_t, channel_ext_write, chan, data);
        }
    }
    return result;
}

// Call to underlying channel 'close!'

PFRTAny     foidl_channel_close_bang(PFRTAny chan) {
    PFRTAny result = nil;
    if( foidl_io_qmark(chan) == true) {
        PFRTAny chan_t = foidl_channel_type_qmark(chan);
        if( chan_t == chan_file ) {
            result = foidl_channel_file_close_bang(chan);
        }
        else {
            result = call_extension_1(chan_t, channel_ext_close, chan);
        }
    }
    return result;
}

