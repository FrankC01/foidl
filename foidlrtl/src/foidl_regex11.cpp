/*
    foidl_regexe.cpp
    Library support for C++11 regular expressions

    Copyright Frank V. Castellucci
    All Rights Reserved
*/

#include <regex>
using namespace std;

static regex sympattern("[a-zA-Z]{1}[a-zA-Z0-9_.\\?\\!]*$");
static regex callpattern("[a-zA-Z]{1}[a-zA-Z0-9_.\\?\\!]*:$");
static regex kwdpattern (":[a-zA-Z0-9]{1}[a-zA-Z0-9_.\\?\\!]*$");
static regex intpattern ("^[-+]?[0-9][0-9]*$");
static regex realpattern("^[-+]?[0-9]*\\.{1,1}[0-9]+$");
static regex hexpattern ("^0[xX]{1,1}[a-fA-F0-9]+$");
static regex bitpattern ("^0[bB]{1,1}[0-1]+$");


extern "C" int _is_symbol(const char* s) {
    return regex_match(s, sympattern);
}

extern "C" int _is_keyword(const char* s) {
    return regex_match(s, kwdpattern);
}

extern "C" int _is_number(const char* s) {
     if( regex_match(s, intpattern)) return 1;
     if( regex_match(s, realpattern)) return 1;
     if( regex_match(s, hexpattern)) return 1;
     if( regex_match(s, bitpattern)) return 1;
     return 0;
}
