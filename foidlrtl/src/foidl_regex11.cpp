/*
    foidl_regexe.cpp
    Library support for C++11 regular expressions

    Copyright Frank V. Castellucci
    All Rights Reserved
*/
#include <foidl_regex11.hpp>
#include <stdio.h>
#include <iostream>
#include <regex>
#include <list>

using namespace std;

regex* _new_regex(const char*);

static regex sympattern("[a-zA-Z]{1}[a-zA-Z0-9_.\\?\\!]*$");
static regex callpattern("[a-zA-Z]{1}[a-zA-Z0-9_.\\?\\!]*:$");
static regex kwdpattern (":[a-zA-Z0-9]{1}[a-zA-Z0-9_.\\?\\!]*$");
static regex intpattern ("^[-+]?[0-9][0-9]*$");
static regex realpattern("^[-+]?[0-9]*\\.{1,1}[0-9]+$");
static regex hexpattern ("^0[xX]{1,1}[a-fA-F0-9]+$");
static regex bitpattern ("^0[bB]{1,1}[0-1]+$");
static regex *NLCR = _new_regex("\r?[\n]");
static regex *simple_str = _new_regex("\".*\"");
// static regex *comment = _new_regex("\"([\\s\\S]+?)\"");

extern "C" void * foidl_xall(uint32_t sz);
extern "C" void foidl_xdel(void *v);
extern "C" void * foidl_reg_string(const char *i);

const char *astring = ":string";

ptoken _build_token(string &word, int ti, int lc, int sp ) {
    ptoken _tok = (ptoken) foidl_xall(sizeof(token));
    _tok->type_index = ti;
    _tok->colno = sp;
    _tok->lineno = lc;
    _tok->word = (char *) foidl_reg_string(word.c_str());
    return _tok;
}

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

// Match with conversion of pattern string to regex
// Expensive!
extern "C" int _is_match(const char* s, const char* pattern) {
    return regex_match(s,regex(pattern));
}

// Match with assumption 'pattern' is actually a regex*
// see _string_to_regex below
extern "C" int _is_matchp(const char* s, void* pattern) {
    regex* rpattern = static_cast<regex*>(pattern);
    return regex_match(s,*rpattern);
}

regex* _new_regex(const char* s) {
    return new regex(s);
}

// Compile a regex from string
extern "C" void* _string_to_regex(const char* s) {
    // cout << " compiling regex " << s << endl;
    return _new_regex(s);
}

extern "C" void _reduce_tokens(const char*s, ptoken_block block) {
    list<ptoken> hitlist;
    string content(s);
    smatch sm;
    int lc =  1;
    int sp =  1;
    int nomatcherr = 0;
    while(!content.empty()) {
        int  err=0;
        int  res=0;
        int  hit=0;
        regex* regexp;
        //  Process ignores
        for(int i=0; i<block->ignore_cnt; i++) {
            regexp = static_cast<regex*>(block->ig_regex_array[i]);
            res = regex_search(content, sm,
                *regexp, regex_constants::match_continuous);
            if (res) {
                // cout << "Skipping ignore " << endl;
                sp += sm.length();
                content.erase(sm.position(), sm.length());
                break;
            }

        }
        // Process NL
        if(!res) {
            res = regex_search(content, sm,
                *NLCR,regex_constants::match_continuous);
            if (res) {
                // cout << "Newline " << endl;
                lc += 1;
                sp = 1;
                content.erase(sm.position(), sm.length());
            }
        }
        // Process quoted string
        if(!res) {
            res = regex_search(content, sm,
                *simple_str,regex_constants::match_continuous);
            if (res) {
                string word = content.substr(
                    sm.position(), sm.length());
                // cout << "Found type [:string] at {"
                // << lc << "}:{" << sp
                // << "} " << word << endl;
                hitlist.push_back(_build_token(word, -1, lc, sp));
                sp += sm.length();
                content.erase(sm.position(), sm.length());
                hit = 1;
            }
        }
        // Process tokens
        if(!res) {
            for(int i=0; i<block->pattern_cnt;i++) {
                regexp = static_cast<regex*>(block->regex_array[i]);
                res = regex_search(content, sm,
                    *regexp, regex_constants::match_continuous);
                // cout << "Testing "
                // << content.substr(0,1)
                // << " with type ["
                // << block->type_array[i] << "]" << endl;
                if(res) {
                    string word = content.substr(
                        sm.position(), sm.length());
                    // cout << "Found type ["
                    // << block->type_array[i]
                    // << "] at {"
                    // << lc << "}:{" << sp
                    // << "} = " << word << endl;
                    hitlist.push_back(_build_token(word, i, lc, sp));
                    sp += sm.length();
                    content.erase(sm.position(), sm.length());
                    hit = 1;
                    break;
                }
            }
        }
        if(!res)
            if(!hit) {
                cout << "Error: No match for '" << content.substr(0,1)
                    << "' in '" << content
                    << "' at line " << lc << " column " << sp << endl;
                nomatcherr = 1;
                break;
            }
    }
    if(nomatcherr == 1) {
        for (auto&& i : hitlist) {
            foidl_xdel(i);
        }
    }
    else if(hitlist.size() > 0) {
        block->token_cnt = hitlist.size();
        block->tokens = (ptoken *) foidl_xall(hitlist.size() * sizeof(ptoken));
        int index = 0;
        for (auto&& i : hitlist) {
            block->tokens[index] = i;
            ++index;
        }
    }

    return;
}