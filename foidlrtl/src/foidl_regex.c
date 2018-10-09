/*
	foidl_regex.c
	Library support for regular expressions
	
	Copyright Frank V. Castellucci
	All Rights Reserved
*/

#define REGEX_IMPL
#include <foidlrt.h>


#include <regex.h>
#include <stdio.h>

// Symbol compiled regex

static regex_t recall;
static regex_t resymbol;
static regex_t rekeyword;
static regex_t reinteger;
static regex_t rereal;
static regex_t rehex;

static const char *sympattern 	= "[a-zA-Z]{1}[a-zA-Z0-9_.\\?\\*]*$";
static const char *callpattern	= "[a-zA-Z]{1}[a-zA-Z0-9_.\\?\\*]*:$";
static const char *kwdpattern 	= ":[a-zA-Z0-9]{1}[a-zA-Z0-9_.\\?\\*]*$";
static const char *intpattern 	= "^[-+]?[0-9][0-9]*$";
static const char *realpattern 	= "^[-+]?[0-9]*\\.{1,1}[0-9]+$";
static const char *hexpattern 	= "^0[xX]{1,1}[a-fA-F0-9]+$";

constKeyword(callKW,":call");
constKeyword(symbolKW,":symbol");
constKeyword(letKW,":let");
constKeyword(matchKW,":match");
constKeyword(integerKW,":integer");
constKeyword(realKW,":real");
constKeyword(hexidecimalKW,":hexidecimal");
constKeyword(unknownKW,":unknown");

/*
static void testregex(char *p,regex_t* pattern) {
	int result = regexec (pattern, p, 0, NULL, 0);
	if(! result )
		printf("%s matches \n",p);
	else 
		printf("No match on %s\n",p);
}
*/

void foidl_rt_init_regex() {
	int result = 0;
	result = regcomp(&resymbol,sympattern,REG_NOSUB | REG_EXTENDED);
	if(result)
		unknown_handler(); 	//	Throw run time error

	result = regcomp(&rekeyword,kwdpattern,REG_NOSUB | REG_EXTENDED);
	if(result)
		unknown_handler(); 	//	Throw run time error

	result = regcomp(&reinteger,intpattern,REG_NOSUB | REG_EXTENDED);
	if(result)
		unknown_handler(); 	//	Throw run time error

	result = regcomp(&rereal,realpattern,REG_NOSUB | REG_EXTENDED);
	if(result)
		unknown_handler(); 	//	Throw run time error

	result = regcomp(&rehex,hexpattern,REG_NOSUB | REG_EXTENDED);
	if(result)
		unknown_handler(); 	//	Throw run time error

	result = regcomp(&recall,callpattern,REG_NOSUB | REG_EXTENDED);
	if(result)
		unknown_handler(); 	//	Throw run time error

	/*
	//	Symbol testing
	testregex("",sympattern);
	testregex("h",sympattern);
	testregex("hello",sympattern);
	testregex("hello1234*",sympattern);
	testregex("hello_???4?",sympattern);
	testregex("a._..b._1234",sympattern);
	*/
	/*
	//	Keyword testing
	testregex("",&rekeyword);
	testregex(":",&rekeyword);
	testregex(":foo",&rekeyword);
	testregex("hello_???4?",&rekeyword);
	testregex(":hello_???4?",&rekeyword);
	testregex("hello_???4?:",&rekeyword);
	*/
	/*
	//	Integer Testing
	testregex("0",&reinteger);
	testregex("-0",&reinteger);
	testregex("+0",&reinteger);
	testregex("1",&reinteger);
	testregex("-1",&reinteger);
	testregex("+1",&reinteger);
	testregex("21987322901",&reinteger);
	testregex("21987F22901",&reinteger);
	*/

	// Hex testing
	/*
	testregex("0x",&rehex);
	testregex("0X",&rehex);
	testregex("0Xg",&rehex);
	testregex("0x739FABCD",&rehex);
	testregex("0x0000ABCD",&rehex);
	*/
}

PFRTAny foidl_valid_symbol(PFRTAny s) {
	if(s->fclass == scalar_class && s->ftype == string_type) {		
		if(!regexec(&resymbol,s->value, 0, NULL, 0))
			return true;
		else 
			return false;
	}
	return false;
}

PFRTAny foidl_valid_keyword(PFRTAny s) {
	if(s->fclass == scalar_class && 
		( s->ftype == keyword_type
		|| s->ftype == string_type)) {		
		if(!regexec(&rekeyword,s->value, 0, NULL, 0)) {
			//printf("%s passed keyword\n", s->value);
			return true;
		}
		else {
			//printf("%s did not pass keyword\n", s->value);
			return false;
		}
	}
	else {
		printf("%s not tested\n", s->value);
	}
	return false;
}

PFRTAny foidl_valid_number(PFRTAny s) {
	if(s->fclass == scalar_class && s->ftype == string_type) {		
		if(!regexec(&reinteger,s->value, 0, NULL, 0))			
			return true;
		if(!regexec(&rereal,s->value, 0, NULL, 0))			
			return true;
		if(!regexec(&rehex,s->value, 0, NULL, 0))			
			return true;		
		else 
			return false;		
	}
	else {
		printf("%s not tested\n", s->value);
	}
	return false;
}

PFRTAny foidl_categorize_num(PFRTAny s) {
	if(s->fclass == scalar_class && s->ftype == string_type) {		
		if(!regexec(&reinteger,s->value, 0, NULL, 0))			
			return integerKW;
		if(!regexec(&rereal,s->value, 0, NULL, 0))			
			return realKW;
		if(!regexec(&rehex,s->value, 0, NULL, 0))			
			return hexidecimalKW;		
		else 
			unknown_handler();		
	}
	else {
		printf("%s not tested\n", s->value);
		unknown_handler();
	}
	return false;

}
PFRTAny foidl_regex_cmp_qmark(PFRTAny pattern, PFRTAny arg) {
	PFRTAny res = false;
	int result;
	regex_t regpat;	
	result = regcomp(&regpat,pattern->value,REG_NOSUB | REG_EXTENDED);
	if(!result) {
		if(!regexec(&regpat,arg->value, 0, NULL, 0))
			res = true;
		regfree(&regpat);
	}
	return res;
}

PFRTAny foidl_categorize(PFRTAny s) {
	PFRTAny res = unknownKW;
	if(s->fclass == scalar_class && s->ftype == string_type) {
		if(!regexec(&resymbol,s->value, 0, NULL, 0))
			return symbolKW;
		if(!regexec(&recall,s->value, 0, NULL, 0))
			return callKW;

	}
	return res;
}
