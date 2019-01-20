/*
    foidl_regexe.cpp
    Library support for C++11 regular expressions

    Copyright Frank V. Castellucci
    All Rights Reserved
*/
#include <foidl_regex11.hpp>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <regex>
#include <list>

using namespace std;

regex* _new_regex(const char*);

// Predefined patterns. Should move to compiler?

static regex *NLCR = _new_regex("(\r\n|[\r\n])");
static regex *simple_str = _new_regex("\"(.|\n)*?\"");
static regex *frmtstr = _new_regex("\\{.*?\\}");

// Core functions not exposed in header
EXTERNC void * foidl_reg_string(const char *i);
EXTERNC void * list_extend_bang(void *, void *);

const char *astring = ":string";

ptoken _build_token(string &word, int ti, int lc, int sp ) {
    ptoken _tok = (ptoken) foidl_xall(sizeof(token));
    _tok->type_index = ti;
    _tok->colno = sp;
    _tok->lineno = lc;
    _tok->word = (char *) foidl_reg_string(word.c_str());
    return _tok;
}


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

// Split a string
EXTERNC void _string_split(void *rlist, const char *strng, void* pattern) {
    string base(strng);
    regex* rpattern = static_cast<regex*>(pattern);
    sregex_token_iterator it(base.begin(), base.end(), *rpattern, -1);
    sregex_token_iterator reg_end;
    for (; it != reg_end; ++it) {
        string hs = it->str();
        list_extend_bang(rlist, allocStringWithCopy((char *) hs.c_str()));
    }
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

static PFRTAny fend = (PFRTAny) &_end.fclass;
static void _ttstr(ostringstream &ost, PFRTAny e);

static void _single_coll(ostringstream &ost, PFRTAny coll) {
    PFRTAny      entry;
    PFRTIterator li = iteratorFor(coll);
    int cnt = 0;
    while((entry = iteratorNext(li)) != fend) {
        if( cnt > 0 && cnt < coll->count) {
            ost << ",";
        }
        ++cnt;
        _ttstr(ost, entry);
    }

}
static void _assoc_coll(ostringstream &ost, PFRTAny coll) {
    PFRTAny      entry;
    PFRTIterator li = iteratorFor(coll);
    int cnt = 0;
    while((entry = iteratorNext(li)) != fend) {
        PFRTMapEntry e = (PFRTMapEntry) entry;
        _ttstr(ost, e->key);
        ost << "->";
        _ttstr(ost, e->value);
        ++cnt;
        if( cnt > 0 && cnt < coll->count) {
            ost << ",";
        }
    }

}

static void _ttstr(ostringstream &ost, PFRTAny e) {
    int count = 0;
    switch(e->ftype) {
        case    string_type:
        case    keyword_type:
            ost << (char *)e->value;
            break;
        case    integer_type:
            ost << (ft) e->value;
            break;
        case nil_type:
            ost << (char *)nilstr->value;
            break;
        case boolean_type:
            if(((ft) e->value) == 1)
                ost << (char *) truestr->value;
            else
                ost << (char *) falsestr->value;
            break;
        case list2_type:
            {
                ost << "[";
                _single_coll(ost, e);
                ost << "]";
            }
            break;
        case vector2_type:
            {
                ost << "#[";
                _single_coll(ost, e);
                ost << "]";
            }
            break;
        case map2_type:
            {
                ost << "{";
                _assoc_coll(ost, e);
                ost << "}";
            }
            break;
        case set2_type:
            {
                ost << "#{";
                _single_coll(ost, e);
                ost << "}";
            }
            break;
        default:
            unknown_handler();
            break;

    }
    return;
}

EXTERNC PFRTAny foidl_format(PFRTAny s, PFRTAny coll) {
    if(s->ftype == string_type &&
        (coll->ftype == vector2_type || coll->ftype == list2_type)) {
        if(coll->count == 0)
            return s;
        PFRTAny entry;
        PFRTIterator li = iteratorFor(coll);
        vector<string *> hitlist;

        while((entry = iteratorNext(li)) != fend) {
            ostringstream ost;
            _ttstr(ost, entry);
            hitlist.push_back(new string(ost.str()));
        }
        string base((char *) s->value);
        string result;
        smatch m;
        int pos=0;

        while (regex_search (base,m,*frmtstr)) {
            result += m.prefix().str() + hitlist[pos]->c_str();
            base = m.suffix().str();
            ++pos;
        }
        for (auto&& i : hitlist) {
            delete i;
        }
        result += base;
        return allocStringWithCopy((char *) result.c_str());

        // Get rid of surplus strings
    }
    else {
        unknown_handler();
    }

    return s;
}