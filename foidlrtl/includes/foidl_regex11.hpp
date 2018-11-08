/*
    foidl_regex11.hpp
    C++11 support for regular expressions

    Copyright Frank V. Castellucci
    All Rights Reserved
*/

#ifndef REGEX11_IMPL
#define REGEX11_IMPL

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int     pattern_cnt;
    void    **regex_array;
    char    **type_array;
    int     ignore_cnt;
    void    **ig_regex_array;
} token_block;

typedef struct {
    int     type_index;
    char    *word;
    int     lineno;
    int     colno;
} token, *ptoken;

int     _is_symbol(const char* s);
int     _is_keyword(const char* s);
int     _is_number(const char* s);

void*   _string_to_regex(const char* s);
int     _is_match(const char* s, const char* pattern);
int     _is_matchp(const char* s, void* pattern);

#ifdef __cplusplus
}
#endif

#endif
