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
constKeyword(ext_type,":extension_type");
// ext_subtype describes the unique descriptor (e.g. http_chan)
constKeyword(ext_subtype,":extension_subtype");
// ext_interface describes the interface calling type
constKeyword(ext_interface,":extension_interface");
// ext_functions describes the interface functions
constKeyword(ext_functions,":extension_functions");

/*
    Collection 'registrar' is a map containing keyed
    typed entries with value being a map of subtypes e.g.:
    {(ext_type)'channel': map of channel subtypes}
*/

static PFRTMap registrar;

// Get's the registrar subtype map for type argument
// If does not exist, creates the type entry with empty
// map to hold subtypes

static PFRTAny subtype_map_for_type(PFRTAny type_extension) {
    PFRTAny result = nil;
    result = map_get((PFRTAny)registrar, type_extension);
    if(result == nil) {
        result = foidl_map_inst_bang();
        foidl_map_extend_bang((PFRTAny)registrar,type_extension,result);
    }
    return result;
}

// Registers or balks at request to added extension to the
// registrar

PFRTAny register_extension(PFRTAny descriptor) {

    return nil;
}

// Pre-main initializer

void foidl_rtl_init_extensions() {
    registrar = (PFRTMap) foidl_map_inst_bang();
}

