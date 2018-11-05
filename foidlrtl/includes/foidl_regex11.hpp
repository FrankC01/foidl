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

int _is_symbol(const char* s);
int _is_keyword(const char* s);
int _is_number(const char* s);
int _is_match(const char* s, const char* pattern);

#ifdef __cplusplus
}
#endif

#endif
