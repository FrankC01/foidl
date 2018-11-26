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

// Predefined patterns. Should move to compiler?

static regex *NLCR = _new_regex("(\r\n|[\r\n])");
static regex *simple_str = _new_regex("\"(.|\n)*?\"");
static regex *frmtstr = _new_regex("\\{.*?\\}");

// Core functions to avoid cleaning up foidlrt.h at the moment
EXTERNC void * foidl_xall(uint32_t sz);
EXTERNC void foidl_xdel(void *v);
EXTERNC void * foidl_reg_string(const char *i);

const char *astring = ":string";

ptoken _build_token(string &word, int ti, int lc, int sp ) {
    ptoken _tok = (ptoken) foidl_xall(sizeof(token));
    _tok->type_index = ti;
    _tok->colno = sp;
    _tok->lineno = lc;
    _tok->word = (char *) foidl_reg_string(word.c_str());
    return _tok;
}

// EXTERNC int _is_symbol(const char* s) {
//     return regex_match(s, sympattern);
// }

// EXTERNC int _is_keyword(const char* s) {
//     return regex_match(s, kwdpattern);
// }

// EXTERNC int _is_number(const char* s) {
//      if( regex_match(s, intpattern)) return 1;
//      if( regex_match(s, realpattern)) return 1;
//      if( regex_match(s, hexpattern)) return 1;
//      if( regex_match(s, bitpattern)) return 1;
//      return 0;
// }

// Match with conversion of pattern string to regex
// Expensive!
EXTERNC int _is_match(const char* s, const char* pattern) {
    return regex_match(s,regex(pattern));
}

// Match with assumption 'pattern' is actually a regex*
// see _string_to_regex below
EXTERNC int _is_matchp(const char* s, void* pattern) {
    regex* rpattern = static_cast<regex*>(pattern);
    return regex_match(s,*rpattern);
}

regex* _new_regex(const char* s) {
    return new regex(s);
}

// Compile a regex from string
EXTERNC void* _string_to_regex(const char* s) {
    // cout << " compiling regex " << s << endl;
    regex* re = _new_regex(s);
    return static_cast<void *>(re);
}

EXTERNC void* _format_string(const char *strng, char **rep, int rcnt) {
    string base(strng);
    string result;
    smatch m;
    int pos=0;

    while (regex_search (base,m,*frmtstr)) {
        result += m.prefix().str() + rep[pos];
        base = m.suffix().str();
        ++pos;
    }
    if( pos != rcnt ) {
        cerr << "Count found does not match inbound match set" << endl;
    }
    result += base;
    return foidl_reg_string(result.c_str());
}


EXTERNC void _reduce_tokens(const char*s, ptoken_block block) {
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
                lc += 1;
                sp = 1;
                content.erase(sm.position(), sm.length());
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
                lc += count(word.begin(),word.end(),'\n');;
                content.erase(sm.position(), sm.length());
                hit = 1;
            }
        }
        if(!res)
            if(!hit) {
                for (auto&& i : hitlist) {
                    foidl_xdel(i);
                }
                hitlist.erase(hitlist.begin(), hitlist.end());

                string errorstr("Error: No match for '");
                errorstr += content.substr(0,1);
                hitlist.push_back(_build_token(errorstr, -2, lc, sp));
                break;
            }
    }
    if(hitlist.size() > 0) {
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