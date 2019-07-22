/*
; foidl_extension.c
;   Routines to extend run time
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
    Extend the RTL for polymorphism and general extensions
    Each extension requires an extension description:
    * What is being extended (i.e. channel)
    * What is it's unique type identifier (i.e. fixed identifier)
    * What are it's extension points, in other words, a subsumption of
        the method(s) being extended

    The extension can use the RTL and guarentee that it observes
    data persistence
*/

#define EXTENSION_IMPL
#include <foidlrt.h>
#include <stdio.h>

// Register an extension

// Descriptor keys

// ext_type describes the core type (e.g. channel)
globalScalarConst(ext_type,byte_type,(void *) 0,1);
// ext_subtype describes the unique descriptor (e.g. http_chan)
globalScalarConst(ext_subtype,byte_type,(void *) 1,1);
// ext_interface describes the interface calling type
globalScalarConst(ext_interface,byte_type,(void *) 2,1);
// ext_functions describes the interface functions
globalScalarConst(ext_functions,byte_type,(void *) 3,1);

// Exensible types
globalScalarConst(channel,byte_type,(void *) 0,1);

PFRTAny register_extension(PFRTAny descriptor) {
    printf("register_extension called\n");
    return nil;
}