/*
    foidl_regex11.hpp
    C++11 support for regular expressions

    Copyright Frank V. Castellucci
    All Rights Reserved
*/

#ifndef REGEX11_IMPL
#define REGEX11_IMPL

#include    <foidlrt.h>

typedef struct {
    int     type_index;
    char    *word;
    int     lineno;
    int     colno;
} token, *ptoken;

typedef struct {
    int     pattern_cnt;
    void    **regex_array;
    char    **type_array;
    int     ignore_cnt;
    void    **ig_regex_array;
    int     token_cnt;
    ptoken  *tokens;
} token_block, *ptoken_block;


EXTERNC void*   _string_to_regex(const char* s);
EXTERNC int     _is_match(const char* s, const char* pattern);
EXTERNC int     _is_matchp(const char* s, void* pattern);
EXTERNC void    _reduce_tokens(const char*s, ptoken_block);
EXTERNC void    _string_split(void *rlist, const char *strng, void* pattern);
EXTERNC PFRTAny foidl_format(PFRTAny bstr, PFRTAny bcoll);
#endif
