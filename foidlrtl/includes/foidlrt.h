/*

//	Reader handling flags

	Master header file for C run-time definitions
	Copyright Frank V. Castellucci
	All Rights Reserved
*/

#ifndef FOIDL_MAIN

#define FOIDL_MAIN
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#ifdef __cplusplus
    #define EXTERNC extern "C"
#else
    #define EXTERNC extern
#endif

typedef unsigned long long ft;
typedef long long lt;

//
// Class identifiers
//

static const ft 	global_signature = 0xefefefef00000000;
static const ft 	scalar_class 	 = 0xfffffffffffffff1;
static const ft 	collection_class = 0xfffffffffffffff2;
static const ft 	function_class 	 = 0xfffffffffffffff3;
static const ft 	io_class 		 = 0xfffffffffffffff4;
static const ft 	hamptnode_class  = 0xfffffffffffffff5;
static const ft 	bitmapnode_class = 0xfffffffffffffff6;

static const ft 	iterator_class	 = 0xfffffffffffffffe;

//
//	Type identifiers
//

//	Support types

static const ft 	nil_type 		= 0xffffffffe00000ad;
static const ft 	end_type 		= 0xffffffffe00000aa;

//	Scalar types
static const ft 	byte_type   	= 0xffffffff100000ac;
static const ft 	keyword_type    = 0xffffffff100000ab;
static const ft 	string_type 	= 0xffffffff100000aa;
static const ft 	boolean_type    = 0xffffffff100000a9;
static const ft 	character_type  = 0xffffffff100000a8;
static const ft     regex_type      = 0xffffffff100000a7;

static const ft 	integer_type 	= 0xffffffff1000a8a9; // Deprecated for number_type
static const ft     number_type     = 0xffffffff1000a8a8;

//	Collection types

static const ft 	list2_type      = 0xffffffff100000cf;
static const ft 	vector2_type 	= 0xffffffff100000ce;
static const ft 	set2_type       = 0xffffffff100000cd;
static const ft 	map2_type       = 0xffffffff100000cc;

static const ft 	mapentry_type   = 0xffffffff100000cb;
static const ft 	linknode_type   = 0xffffffff100000ca;

static const ft 	series_type     = 0xffffffff100000c7;
static const ft 	reduced_type    = 0xffffffff100000c6;

//	Iterator types

static const ft 	vector_iterator_type = 0xffffffff100000c5;
static const ft 	map_iterator_type 	 = 0xffffffff100000c4;
static const ft 	set_iterator_type 	 = 0xffffffff100000c3;
static const ft 	list_iterator_type   = 0xffffffff100000c2;
static const ft 	series_iterator_type = 0xffffffff100000c1;
static const ft 	channel_iterator_type = 0xffffffff100000c0;

//	Function/Lambda types

static const ft 	funcref_type  = 0xffffffff100000ef;
static const ft 	lambref_type  = 0xffffffff100000ee;
static const ft 	funcinst_type = 0xffffffff100000ed;

//	IO types

static const ft 	file_type   = 0xffffffff100000bf;
static const ft 	url_type    = 0xffffffff100000be;
static const ft 	ip_type     = 0xffffffff100000bd;
static const ft 	mem_type  	= 0xffffffff100000bc;
static const ft 	cin_type    = 0xffffffff100000bb;
static const ft 	cout_type   = 0xffffffff100000ba;
static const ft 	cerr_type   = 0xffffffff100000b9;
static const ft 	closed_type = 0xffffffff100000b8;

// IO buffer types

static const ft 	mem_map_buffer   = 0xffffffff100000b6;
static const ft 	mem_block_buffer = 0xffffffff100000b5;
static const ft 	no_buffer        = 0xffffffff100000b4;

//	IO Flags

static const ft 	open_read_only    = 0x0000000000000000;
static const ft 	open_write_only   = 0x0000000000000001;
static const ft 	open_read_write   = 0x0000000000000002;
static const ft 	open_write_append = 0x0000000000000008;
static const ft 	open_create   	  = 0x0000000000000200;
static const ft 	open_truncate 	  = 0x0000000000000400;

//	Reader/writer handling flags

static const ft 	read_byte   = 0x0000000000000000;
static const ft 	read_char   = 0x0000000000000001;
static const ft 	read_string = 0x0000000000000002;

static const ft 	write_byte   = 0x0000000000000003;
static const ft 	write_char   = 0x0000000000000004;
static const ft 	write_string = 0x0000000000000005;

//	Other constants

static const ft SHIFT = 5; 		//	For HAMT/CHAMP
static const ft MASK  = 0x1f; 	//	For HAMT/CHAMP
static const ft WCNT  = 0x20; 	//	For HAMT/CHAMP

static const ft TUPLELENSFT = 1;	// For CHAMP Map/Set
static const ft TUPLELEN 	= 2;	// For CHAMP Map/Set
static const ft KEYPOS 		= 0;	// For CHAMP Map/Set
static const ft VALPOS 		= 1;	// For CHAMP Map/Set
static const ft MAX_I_DEPTH	= 7;	// For CHAMP MAP/Set

typedef struct _MEMSIG {
	uint16_t 	bytes;
	uint16_t 	refs;
	uint16_t 	cntrl;
	uint16_t 	used;
} MEMSIG;

typedef struct FRTTypeG {
	ft 			fsig;
	ft 			fclass;
	ft 			ftype;
	ft 			count;
	uint32_t	hash;
	void 		*value;	 	// Maps to count on collections
} *PFRTTypeG;

typedef struct FRTType {
	ft 			fclass;
	ft 			ftype;
	ft 			count;
	uint32_t	hash;
	void 		*value;	 	// Maps to count on collections
} FRTAny, *PFRTType, *PFRTAny,*PFRTCollection;

#define ANYTOG(s) (PFRTTypeG) ((void *)s - sizeof(ft))

// Special Types

typedef struct FRTRegExG {
    ft          fsig;
    ft          fclass;
    ft          ftype;
    ft          count;      // Copy over from string basis
    uint32_t    hash;       // Copy over from string basis
    PFRTAny     value;      // Initial String
    void        *regex;     // Compiled regex
} *PFRTRegExG;

typedef struct FRTRegEx {
    ft          fclass;
    ft          ftype;
    ft          count;      // Copy over from string basis
    uint32_t    hash;       // Copy over from string basis
    PFRTAny     value;      // Initial String
    void        *regex;     // Compiled regex
} *PFRTRegEx;

//
// Collections
//

//	Vector structures

typedef struct FRTHamtNode {
	ft 					fclass;
	PFRTAny 			slots[32];
} *PFRTHamtNode;

typedef struct FRTVectorG {
	ft 				fsig;
	ft 				fclass; 	//	FOIDL Class - Collection
	ft 				ftype;		//	FOIDL Type - Vector
    ft 				count;    	//	Element count
	uint32_t 		hash;
    PFRTHamtNode 	root; 		// 	Root HAMT node
    PFRTHamtNode 	tail; 		//	Tail collection
    ft 				shift;		//	Dynamic
} *PFRTVectorG;

typedef struct FRTVector {
	ft 				fclass; 	//	FOIDL Class - Collection
	ft 				ftype;		//	FOIDL Type - Vector
    ft 				count;    	//	Element count
	uint32_t 		hash;
    PFRTHamtNode 	root; 		// 	Root HAMT node
    PFRTHamtNode 	tail; 		//	Tail collection
    ft 				shift;		//	Dynamic
} *PFRTVector;

//	List structures

typedef struct FRTLinkNode *PFRTLinkNode;

typedef struct FRTLinkNodeG {
	ft 				fsig;
	ft 				fclass; 	//	FOIDL Class - collection_class
	ft 				ftype;		//	FOIDL Type - linknode_type
    PFRTAny 		data;
    PFRTLinkNode 	next;
} *PFRTLinkNodeG;

typedef struct FRTLinkNode {
	ft 				fclass; 	//	FOIDL Class - collection_class
	ft 				ftype;		//	FOIDL Type - linknode_type
    PFRTAny 		data;
    PFRTLinkNode 	next;
} *PFRTLinkNode;

typedef struct FRTListG {
	ft 				fsig;
	ft 				fclass; 	//	FOIDL Class - Collection
	ft 				ftype;		//	FOIDL Type - Vector
    ft 				count;    	//	Element count
	uint32_t 		hash;
    PFRTLinkNode 	root; 		// 	Root
    PFRTLinkNode 	rest; 		// 	Last
} *PFRTListG;

typedef struct FRTList {
	ft 				fclass; 	//	FOIDL Class - Collection
	ft 				ftype;		//	FOIDL Type - Vector
    ft 				count;    	//	Element count
	uint32_t 		hash;
    PFRTLinkNode 	root; 		// 	Root HAMT node
    PFRTLinkNode 	rest; 		// 	Last
} *PFRTList;


//	Associative structures

typedef struct FRTBitmapNodeG {
	ft 					fsig;
	ft 					fclass;
	uint32_t 			datamap;
	uint32_t 			nodemap;
	PFRTAny 			*slots;
} *PFRTBitmapNodeG;

typedef struct FRTBitmapNode {
	ft 					fclass;
	uint32_t 			datamap;
	uint32_t 			nodemap;
	PFRTAny 			*slots;
} *PFRTBitmapNode;

//	Map

typedef struct FRTMapG {
	ft 				fsig;
	ft 				fclass; 	//	FOIDL Class - Collection
	ft 				ftype;		//	FOIDL Type - Map
    ft 				count;    	//	Element count
	uint32_t 		hash;
    PFRTBitmapNode 	root; 		// 	Root HAMT node
    ft 				shift;		//	Dynamic
} *PFRTMapG;

typedef struct FRTMap {
	ft 				fclass; 	//	FOIDL Class - Collection
	ft 				ftype;		//	FOIDL Type - Map
    ft 				count;    	//	Element count
	uint32_t 		hash;
    PFRTBitmapNode 	root; 		// 	Root HAMT node
    ft 				shift;		//	Dynamic
} *PFRTMap,*PFRTAssocType;

typedef struct FRTMapEntry {
	ft 				fclass; 	//	FOIDL Class - Collection
	ft 				ftype;		//	FOIDL Type - Map
    PFRTAny 		key;
    PFRTAny 		value;
} *PFRTMapEntry;

//  Set

typedef struct FRTSetG {
	ft 				fsig;
	ft 				fclass; 	//	FOIDL Class - Collection
	ft 				ftype;		//	FOIDL Type - Set
    ft 				count;    	//	Element count
	uint32_t 		hash;
    PFRTBitmapNode 	root; 		// 	Root CHAMP node
    ft 				shift;		//	Dynamic
} *PFRTSetG;

typedef struct FRTSet {
	ft 				fclass; 	//	FOIDL Class - Collection
	ft 				ftype;		//	FOIDL Type - Set
    ft 				count;    	//	Element count
	uint32_t 		hash;
    PFRTBitmapNode 	root; 		// 	Root HAMT node
    ft 				shift;		//	Dynamic
} *PFRTSet;

//	Function and Lambda Structures

typedef struct   FRTFuncRefG {
	ft 			fsig;
	ft 			fclass;
	ft 			ftype;
    ft 			argcount;
	uint32_t 	spare;
    void 		*fnptr;
} *PFRTFuncRefG;

typedef struct   FRTFuncRef {
	ft 			fclass;
	ft 			ftype;
    ft 			argcount;
	uint32_t 	spare;
    void 		*fnptr;
} *PFRTFuncRef;

typedef struct   FRTLambdaRef {
	ft 			fclass;
	ft 			ftype;
	PFRTFuncRef ffuncref;
	ft 			closures; 		//	Count of closed over args
	//ft 			hash;
	ft 			spare;
	//void 		*closureptrs;
} *PFRTLambdaRef;

typedef struct FRTFuncRef2 {
	ft 			fclass;
	ft 			ftype;
	ft  		mcount;
	uint32_t 	spare;
	void 		*fnptr;
	PFRTAny 	args; 	//	Could be a vector as well
	void 		*invokefnptr;
} *PFRTFuncRef2;

//	Series

typedef struct FRTSeriesG {
	ft 			fsig;
	ft 			fclass;
	ft 			ftype;
	ft 			spare1;
	uint32_t 	spare2;
	PFRTAny 	start;
	PFRTAny 	stop;
	PFRTAny 	step;
} *PFRTSeriesG;

typedef struct FRTSeries {
	ft 			fclass;
	ft 			ftype;
	ft 			spare1;
	uint32_t 	spare2;
	PFRTAny 	start;
	PFRTAny 	stop;
	PFRTAny 	step;
} *PFRTSeries;

//	IO Structures

typedef	struct   FRTIOBufferG {
	ft 				fsig;		//
	ft 				fclass; 	//	io_buffer_class
	ft 				ftype;		//	block_type, mmem_type
	uint32_t 		buffersize;
    uint32_t 		previous_read_offset;
    uint32_t 		current_read_offset;
    uint32_t 		previous_write_offset;
    uint32_t 		current_write_offset;
    uint32_t 		max_read_position;
    uint32_t 		current_line;
    uint32_t 		current_pos;
	char 			*bufferPtr;
} *PFRTIOBufferG;

typedef	struct   FRTIOBuffer {
	ft 				fclass; 	//	io_buffer_class
	ft 				ftype;		//	block_type, mmem_type
	uint32_t 		buffersize;
    uint32_t 		previous_read_offset;
    uint32_t 		current_read_offset;
    uint32_t 		previous_write_offset;
    uint32_t 		current_write_offset;
    uint32_t 		max_read_position;
    uint32_t 		current_line;
    uint32_t 		current_pos;
	char 			*bufferPtr;
} *PFRTIOBuffer;


typedef	struct   FRTIOChannelG {
	ft 				fsig;
	ft 				fclass; 	//	FOIDL_CLASS_IO
	ft 				ftype;		//	FOIDL_FILE_REF, etc.
	PFRTAny 		channelMap; //	Initiating structure
	PFRTAny 		name;
    ft 				openflag;
    ft 				buffertype;
    ft 				handle;
    PFRTFuncRef 	readhandler;
    PFRTFuncRef 	writehandler;
    PFRTIOBuffer 	bufferptr;
} *PFRTIOChannelG;

typedef	struct   FRTIOChannel {
	ft 				fclass; 	//	FOIDL_CLASS_IO
	ft 				ftype;		//	FOIDL_FILE_REF, etc.
	PFRTAny 		channelMap; //	Initiating structure
	PFRTAny 		name;
    ft 				openflag;
    ft 				buffertype;
    ft 				handle;
    PFRTFuncRef 	readhandler;
    PFRTFuncRef 	writehandler;
    PFRTIOBuffer 	bufferptr;
} *PFRTIOChannel;


//	Forward decls and macros

#define globalScalar(insym,gtype,val,cnt) \
	static struct FRTTypeG _ ## insym = {global_signature,scalar_class, \
		gtype,cnt,0,val}; \
	static PFRTAny const insym = (PFRTAny) & _ ## insym.fclass

#define globalScalarConst(insym,gtype,val,cnt) \
	const struct FRTTypeG _ ## insym = {global_signature,scalar_class, \
		gtype,cnt,0,val}; \
	PFRTAny const insym = (PFRTAny) & _ ## insym.fclass

#define constString(insym,str) \
	char* const _str ## insym = str; \
	const struct FRTTypeG _ ## insym = {global_signature,scalar_class, \
		string_type,sizeof(str) - 1,0,_str ## insym}; \
	PFRTAny const insym = (PFRTAny) & _ ## insym.fclass

#define localString(insym,str) \
	char* const _str ## insym = str; \
	static const struct FRTTypeG _ ## insym = {global_signature,scalar_class, \
		string_type,sizeof(str) - 1,0,_str ## insym}; \
	static PFRTAny const insym = (PFRTAny) & _ ## insym.fclass

#define constKeyword(insym,str) \
	char* const _str ## insym = str; \
	const struct FRTTypeG _ ## insym = {global_signature,scalar_class, \
		keyword_type,sizeof(str) - 1,0,_str ## insym}; \
	PFRTAny const insym = (PFRTAny) & _ ## insym.fclass

#define localKeyword(insym,str) \
	char* const _str ## insym = str; \
	static const struct FRTTypeG _ ## insym = {global_signature,scalar_class, \
		keyword_type,sizeof(str) - 1,0,_str ## insym}; \
	static PFRTAny const insym = (PFRTAny) & _ ## insym.fclass

#define localFunc(insym,acount,val) \
	static struct FRTFuncRefG _ ## insym  = {global_signature,function_class, \
		funcref_type, acount,0,val}; \
	static PFRTFuncRef const insym = (PFRTFuncRef) & _ ## insym.fclass;

#define globalFuncConst(insym,acount,val) \
	static const struct FRTFuncRefG _ ## insym  = {global_signature,function_class, \
		funcref_type, acount,0,val}; \
	PFRTFuncRef const insym = (PFRTFuncRef) & _ ## insym.fclass;

typedef struct FRTIterator *PFRTIterator;

typedef PFRTAny (*itrNext)(PFRTIterator);
typedef PFRTAny (*typeGetter)(PFRTAny,uint32_t);
typedef PFRTAny (*io_readFuncPtr)(PFRTIOChannel);
typedef PFRTAny (*io_writeFuncPtr)(PFRTIOChannel, PFRTAny);
typedef PFRTAny (*invoke_funcptr)(PFRTFuncRef2);
typedef PFRTAny (*_d0)();
typedef PFRTAny (*_d1)(PFRTAny);
typedef PFRTAny (*_d2)(PFRTAny,PFRTAny);

//  Iterators

typedef struct FRTIterator {
	ft 				fclass; 	//	FOIDL Class - Iterator
	ft 				ftype;		//	FOIDL Type - Vector
	itrNext 		next;
	typeGetter 		get;
} *PFRTIterator;

typedef struct FRTVector_Iterator {
	ft 				fclass; 	//	FOIDL Class - Iterator
	ft 				ftype;		//	vector_iterator_type
	itrNext 		next;
	typeGetter 		get;
    PFRTVector 		vector; 	// 	Base vector
	ft 				hash; 		//	Vector hash (TBD)
    PFRTHamtNode 	node; 		//	Current Node
    ft 				base;		//	Base offset
	ft 				index; 		//	Last fetched element
} *PFRTVector_Iterator;

typedef struct FRTList_Iterator {
	ft 				fclass; 	//	FOIDL Class - Iterator
	ft 				ftype;		//	list_iterator_type
	itrNext 		next;
	typeGetter 		get;
	PFRTList 		list; 		//	Base list
	PFRTLinkNode 	node; 		//  Current Untapped Node
} *PFRTList_Iterator;

typedef struct FRTTrie_Iterator {
	ft 				fclass; 	//	FOIDL Class - Iterator
	ft 				ftype;		//	map_iterator_type
	itrNext 		next;
	typeGetter 		get;
	int 			currentValueCursor;
	int 			currentValueLength;
	PFRTBitmapNode 	currentValueNode;
	int 			currentStackLevel;
	int 			nodeCursorAndLength[MAX_I_DEPTH*2];
	PFRTBitmapNode  nodes[MAX_I_DEPTH];
} *PFRTTrie_Iterator,*PFRTMap_Iterator,*PFRTSet_Iterator;

typedef struct FRTSeries_Iterator {
	ft 				fclass; 	//	FOIDL Class - Iterator
	ft 				ftype;		//	series_iterator_type
	itrNext 		next;
	ft 				counter;
	PFRTSeries 		series;
	PFRTAny 		initialValue;
	PFRTAny 		lastValue;
} *PFRTSeries_Iterator;

typedef struct FRTChannel_Iterator {
	ft 				fclass; 	//	FOIDL Class - Iterator
	ft 				ftype;		//	channel_iterator_type
	itrNext 		next;
	uint32_t 		currRef;
	PFRTIOBuffer 	buffer;
} *PFRTChannel_Iterator;

//	Local use structures

//	Local structures

typedef struct MapNodeResult {
	PFRTAny 	replacedValue;
	PFRTAny 	isModified;
	PFRTAny 	isReplaced;
} *PMapNodeResult;

typedef struct SetNodeResult {
	PFRTAny 	replacedValue;
	PFRTAny 	isModified;
	PFRTAny 	isReplaced;
	uint32_t 	deltaSize;
	uint32_t 	deltaHashCode;
} *PSetNodeResult;

typedef struct GetResult {
	PFRTAny 	nodeFoundIn;
	PFRTAny 	elementSearchedOn;
	PFRTAny 	result;
	PFRTAny 	isFound;
} *PGetResult;

//
//	Global Expansions
//

//	langcore function references

// RTL Asm file/memory operations
void foidl_heap_setup();
ft 	foidl_fopen_read_only(char *);
ft  foidl_fopen_create_truncate(char *);
void foidl_deallocate_mmap(char*, ft);
void *foidl_open_ro_mmap_file(char *, size_t);
ft 	foidl_fclose(ft);
void foidl_exit();
void foidl_error_exit(int);


EXTERNC void *foidl_alloc(ft);
EXTERNC void foidl_release(void *);

// RTL Asm Error functions

EXTERNC void unknown_handler();

//	RTL Asm Math functions

#ifndef MATH_IMPL
EXTERNC PFRTAny 	foidl_add(PFRTAny, PFRTAny);
#endif

#ifndef GLOBALS_IMPL
EXTERNC void foidl_rtl_init_globals();
EXTERNC PFRTAny end;

#ifndef __cplusplus
EXTERNC PFRTAny  nil,true,false;
EXTERNC PFRTIOChannel    cin,cout,cerr;
#endif

EXTERNC struct FRTTypeG _end;
EXTERNC struct FRTTypeG _nil;
EXTERNC struct FRTTypeG _false;
EXTERNC struct FRTTypeG _true;
EXTERNC struct FRTTypeG _one;
EXTERNC struct FRTTypeG _zero;
EXTERNC PFRTAny 	langle,rangle;
EXTERNC PFRTAny 	lbrace,rbrace;
EXTERNC PFRTAny 	lbracket,rbracket;
EXTERNC PFRTAny 	tbchr,nlchr,crchr,spchr,semichr;
EXTERNC PFRTAny     semichr,colonchr,sqchr,dqchr,eschr;
EXTERNC PFRTAny 	meta,comma;
EXTERNC PFRTAny 	zero, one,two,three,four,five,six,seven,eight;
EXTERNC PFRTAny 	nine,ten,eleven,twelve,thirteen,fourteen,fifteen,sixteen;
EXTERNC PFRTAny 	empty_string, space_string;

EXTERNC PFRTAny 	nilstr, truestr, falsestr;
#endif

#ifndef TYPE_IMPL
EXTERNC void foidl_rtl_init_types();
#endif

#ifndef ENTRYPOINT_IMPL
EXTERNC PFRTAny 	foidl_count(PFRTAny);
EXTERNC PFRTAny 	foidl_get(PFRTAny,PFRTAny);
EXTERNC PFRTAny 	foidl_getd(PFRTAny,PFRTAny,PFRTAny);
EXTERNC PFRTAny 	foidl_first(PFRTAny);
EXTERNC PFRTAny 	foidl_second(PFRTAny);
EXTERNC PFRTAny 	foidl_last(PFRTAny);
EXTERNC PFRTAny 	foidl_rest(PFRTAny);
EXTERNC PFRTAny 	foidl_remove(PFRTAny,PFRTAny);
EXTERNC PFRTAny 	foidl_dropFor(PFRTAny,PFRTAny);
EXTERNC PFRTAny 	foidl_dropLast(PFRTAny);
EXTERNC PFRTAny 	foidl_extend(PFRTAny, PFRTAny);
EXTERNC PFRTAny 	foidl_extendKV(PFRTAny, PFRTAny,PFRTAny);
EXTERNC PFRTAny 	foidl_extend_bang(PFRTAny, PFRTAny);
EXTERNC PFRTAny 	foidl_update(PFRTAny,PFRTAny,PFRTAny);
EXTERNC PFRTAny 	foidl_update_bang(PFRTAny,PFRTAny,PFRTAny);
EXTERNC PFRTAny 	foidl_pop(PFRTAny);
EXTERNC PFRTAny 	foidl_pop_bang(PFRTAny);
EXTERNC PFRTAny 	foidl_push(PFRTAny, PFRTAny);
EXTERNC PFRTAny 	foidl_push_bang(PFRTAny, PFRTAny);
EXTERNC PFRTAny  foidl_apply(PFRTAny, PFRTAny);
EXTERNC PFRTAny  foidl_map(PFRTAny, PFRTAny);
EXTERNC PFRTAny 	foidl_fold(PFRTAny, PFRTAny, PFRTAny);
EXTERNC PFRTAny 	foidl_reduce(PFRTAny, PFRTAny, PFRTAny);
EXTERNC PFRTAny  foidl_reduced(PFRTAny);
EXTERNC PFRTAny  foidl_split(PFRTAny,PFRTAny); 	//	May move to string
#endif

#ifndef F_ASPRINTF
#ifdef _MSC_VER
#include <stdarg.h>
EXTERNC int vasprintf(char **strp, const char *format, va_list ap);
EXTERNC int asprintf(char **strp, const char *format, ...);
#elif defined __linux__
#include <stdio.h>
EXTERNC int vasprintf(char **strp, const char *format, va_list ap);
EXTERNC int asprintf(char **strp, const char *format, ...);
#endif
#endif

#ifndef PREDICATE_IMPL
EXTERNC PFRTAny scalar_equality(PFRTAny,PFRTAny);
EXTERNC PFRTAny foidl_equal_qmark(PFRTAny,PFRTAny);
EXTERNC PFRTAny foidl_falsey_qmark(PFRTAny);
EXTERNC PFRTAny foidl_truthy_qmark(PFRTAny);
EXTERNC PFRTAny foidl_not_equal_qmark(PFRTAny,PFRTAny);
EXTERNC PFRTAny foidl_function_qmark(PFRTAny);
EXTERNC PFRTAny foidl_number_qmark(PFRTAny);
EXTERNC PFRTAny foidl_collection_qmark(PFRTAny);
EXTERNC PFRTAny foidl_extendable_qmark(PFRTAny);
EXTERNC PFRTAny function_strict_arg(PFRTAny, PFRTAny);
EXTERNC PFRTAny string_type_qmark(PFRTAny);
#endif

#ifndef HASH_IMPL
EXTERNC  uint32_t murmur3_32(const uint8_t*, size_t, uint32_t);
EXTERNC 	uint32_t hash(PFRTAny);
#endif

#ifndef	ALLOC_IMPL

EXTERNC const PFRTAny emtndarray[];
EXTERNC void 			foidl_gc_init();
EXTERNC void *			foidl_xall(uint32_t sz);
EXTERNC void 			foidl_xdel(void *);
//	Scalar types
EXTERNC PFRTAny 			*allocRawAnyArray(ft count);
EXTERNC PFRTAny 			allocAny(ft fclass,ft ftype,void *value);
EXTERNC PFRTAny 			allocGlobalString(char *);
EXTERNC PFRTAny 			allocGlobalStringCopy(char *);
EXTERNC PFRTAny 			allocGlobalKeywordCopy(char *);

EXTERNC PFRTAny 			allocStringWithBufferSize(uint32_t);
EXTERNC PFRTAny 			allocStringWithCopy(char *);
EXTERNC PFRTAny 			allocStringWithCopyCnt(uint32_t, char *);
EXTERNC PFRTAny 			allocAndConcatString(uint32_t,char *, uint32_t, char *, uint32_t);

EXTERNC PFRTAny 			allocGlobalCharType(int);
EXTERNC PFRTAny 			allocGlobalIntegerType(long long v);

EXTERNC PFRTAny          allocRegex(PFRTAny sbase, void* regex);

// IO Types
EXTERNC PFRTIOChannel 	allocIOChannel(ft,PFRTAny);
EXTERNC PFRTIOBuffer 	allocIOMMapBuffer(void *, size_t);
EXTERNC PFRTIOBuffer 	allocIOBlockBuffer(ft);
EXTERNC PFRTIOBuffer 	allocIONoBuffer();
EXTERNC void 			deallocChannelBuffer(PFRTIOChannel);

// Function types
EXTERNC PFRTFuncRef2 	allocFuncRef2(void *fn, ft maxarg,invoke_funcptr ifn);
EXTERNC void 			deallocFuncRef2(PFRTFuncRef2);

// Collection types
EXTERNC PFRTList   		allocList(ft, PFRTLinkNode);
EXTERNC PFRTLinkNode   	allocLinkNode();
EXTERNC PFRTLinkNode     allocLinkNodeWith(PFRTAny data, PFRTLinkNode nextNode);
EXTERNC PFRTSet 			allocSet(ft,ft,PFRTBitmapNode);
EXTERNC PFRTMap 			allocMap(ft,ft,PFRTBitmapNode);
EXTERNC PFRTVector 		allocVector(ft,ft,PFRTHamtNode,PFRTHamtNode);
EXTERNC PFRTMapEntry 	allocMapEntryWith(PFRTAny, PFRTAny);
EXTERNC PFRTSeries 		allocSeries();
EXTERNC PFRTBitmapNode 	allocNode();
EXTERNC PFRTHamtNode 	allocHamtNode();
EXTERNC PFRTBitmapNode 	allocNodeWith(uint32_t, uint32_t, ft);
EXTERNC PFRTBitmapNode 	allocNodeClone(PFRTBitmapNode, ft);
EXTERNC PFRTBitmapNode 	allocNodeWithAll(uint32_t,uint32_t,PFRTAny *);

// Iterator types
EXTERNC PFRTIterator 	allocTrieIterator(PFRTAssocType,itrNext);
EXTERNC PFRTIterator 	allocVectorIterator(PFRTVector,itrNext);
EXTERNC PFRTIterator 	allocListIterator(PFRTList,itrNext);
EXTERNC PFRTIterator 	allocSeriesIterator(PFRTSeries,itrNext);
EXTERNC PFRTIterator 	allocChannelIterator(PFRTIOBuffer,itrNext);

#endif

// Errors
#ifndef ERRORS_IMPL
EXTERNC PFRTAny foidl_fail();
EXTERNC PFRTAny writeCerr(PFRTAny);
EXTERNC PFRTAny writeCerrNl(PFRTAny);
EXTERNC void    foidl_ep_excp(PFRTAny);
EXTERNC void    foidl_ep_excp2(PFRTAny,PFRTAny);
EXTERNC PFRTAny const unsupported;
EXTERNC PFRTAny const index_out_of_bounds;
EXTERNC PFRTAny const update_not_integral;
EXTERNC PFRTAny const extend_map_two_arg;
EXTERNC PFRTAny const fold_requires_function;
EXTERNC PFRTAny const fold_requires_collection;
EXTERNC PFRTAny const reduce_requires_function;
EXTERNC PFRTAny const reduce_requires_collection;
EXTERNC PFRTAny const not_read_channel;
EXTERNC PFRTAny const not_write_channel;
EXTERNC PFRTAny const already_closed_channel;
EXTERNC PFRTAny const requires_map;
EXTERNC PFRTAny const missing_channel;
EXTERNC PFRTAny const missing_open;
EXTERNC PFRTAny const unknown_open_flag;
EXTERNC PFRTAny const unknown_buffer_flag;
EXTERNC PFRTAny const unknown_reader_flag;
EXTERNC PFRTAny const unknown_writer_flag;
EXTERNC PFRTAny const file_is_directory;
#endif

// Characters
#ifndef CHARACTER_IMPL
EXTERNC void foidl_rtl_init_chars();
EXTERNC PFRTAny 	allocCharWithValue(ft);
EXTERNC PFRTAny 	allocECharWithValue(ft, ft);
#endif

#ifndef NUMBER_IMPL
EXTERNC void        foidl_rtl_init_numbers();
EXTERNC PFRTAny     foidl_reg_number(char *);
EXTERNC PFRTAny     foidl_reg_intnum(ft);
EXTERNC ft          number_tostring_buffersize(PFRTAny);
EXTERNC char*       number_tostring(PFRTAny);
EXTERNC long long   number_tolong(PFRTAny);
EXTERNC ft          number_toft(PFRTAny);
EXTERNC PFRTAny     foidl_num_equal(PFRTAny flhs, PFRTAny frhs);
EXTERNC PFRTAny     foidl_num_nequal(PFRTAny flhs, PFRTAny frhs);
EXTERNC PFRTAny     foidl_num_lt(PFRTAny flhs, PFRTAny frhs);
EXTERNC PFRTAny     foidl_num_gt(PFRTAny flhs, PFRTAny frhs);
EXTERNC PFRTAny     foidl_num_lteq(PFRTAny flhs, PFRTAny frhs);
EXTERNC PFRTAny     foidl_num_gteq(PFRTAny flhs, PFRTAny frhs);
EXTERNC PFRTAny     foidl_num_odd(PFRTAny flhs);
EXTERNC PFRTAny     foidl_num_even(PFRTAny flhs);
EXTERNC PFRTAny     foidl_num_add(PFRTAny flhs, PFRTAny frhs);
EXTERNC PFRTAny     foidl_num_sub(PFRTAny flhs, PFRTAny frhs);
EXTERNC PFRTAny     foidl_num_mul(PFRTAny flhs, PFRTAny frhs);
EXTERNC PFRTAny     foidl_num_div(PFRTAny flhs, PFRTAny frhs);
EXTERNC PFRTAny     foidl_num_mod(PFRTAny flhs, PFRTAny frhs);

#endif

// String
#ifndef STRING_IMPL
EXTERNC 	void foildl_rtl_init_strings();
EXTERNC 	PFRTAny string_get(PFRTAny,PFRTAny);
EXTERNC 	PFRTAny string_get_default(PFRTAny, PFRTAny,PFRTAny);
EXTERNC  PFRTAny string_first(PFRTAny);
EXTERNC  PFRTAny string_second(PFRTAny);
EXTERNC  PFRTAny string_rest(PFRTAny);
EXTERNC  PFRTAny string_last(PFRTAny);
EXTERNC 	PFRTAny string_update(PFRTAny, PFRTAny, PFRTAny);
EXTERNC 	PFRTAny string_update_bang(PFRTAny, PFRTAny, PFRTAny);
EXTERNC  PFRTAny string_extend(PFRTAny, PFRTAny);
EXTERNC  PFRTAny string_extend_bang(PFRTAny, PFRTAny);
EXTERNC  PFRTAny string_droplast(PFRTAny);
EXTERNC  PFRTAny string_droplast_bang(PFRTAny);
EXTERNC  PFRTAny release_string(PFRTAny s);
//EXTERNC 	PFRTAny charPtrToString(char *);
#endif

#ifndef INVOKE_IMPL
EXTERNC PFRTAny 	foidl_fref_instance(PFRTAny);
EXTERNC PFRTAny 	foidl_imbue(PFRTAny,PFRTAny);
EXTERNC PFRTAny 	dispatch0(PFRTAny fn);
EXTERNC PFRTAny 	dispatch1(PFRTAny fn, PFRTAny arg1);
EXTERNC PFRTAny 	dispatch2(PFRTAny fn, PFRTAny arg1, PFRTAny arg2);
EXTERNC PFRTAny 	dispatch0i(PFRTAny fn);
EXTERNC PFRTAny 	dispatch1i(PFRTAny fn, PFRTAny arg1);
EXTERNC PFRTAny 	dispatch2i(PFRTAny fn, PFRTAny arg1, PFRTAny arg2);
#endif

#ifndef ITERATORS_IMPL
EXTERNC PFRTIterator iteratorFor(PFRTAny);
EXTERNC PFRTAny 		iteratorNext(PFRTIterator);
#endif

//	Node functions
#ifndef NODE_IMPL
EXTERNC const struct FRTBitmapNode _empty_champ_node;
EXTERNC PFRTBitmapNode   const empty_champ_node;
EXTERNC uint32_t 		bit_count(uint32_t);
EXTERNC uint32_t 		bit_pos(uint32_t);
EXTERNC uint32_t 		data_count(PFRTBitmapNode);
EXTERNC uint32_t 		node_count(PFRTBitmapNode);
EXTERNC uint32_t 		nodelength(PFRTBitmapNode);
EXTERNC uint32_t 		dataIndex(uint32_t, uint32_t);
EXTERNC uint32_t 		nodeIndex(uint32_t, uint32_t);
EXTERNC uint32_t 		payloadArity(PFRTBitmapNode);
EXTERNC uint32_t 		nodeArity(PFRTBitmapNode);
EXTERNC uint32_t 		mask(uint32_t, ft);
EXTERNC uint32_t 		sizePredicate(PFRTBitmapNode);
EXTERNC PFRTBitmapNode 	getNode(PFRTBitmapNode, uint32_t);
EXTERNC void 			arraycopy(PFRTAny *, PFRTAny *,uint32_t);
#endif

//  Vector functions
#ifndef VECTOR_IMPL
EXTERNC  PFRTHamtNode const empty_node;
EXTERNC 	PFRTVector 	 const empty_vector;
EXTERNC 	PFRTAny foidl_vector_inst_bang();
EXTERNC  PFRTAny foidl_vector_extend_bang(PFRTAny,PFRTAny);
EXTERNC  PFRTAny vector_from_argv(int, char**);
EXTERNC  PFRTAny vector_nth(PFRTVector, ft);
EXTERNC 	PFRTAny vector_first(PFRTAny);
EXTERNC 	PFRTAny vector_second(PFRTAny);
EXTERNC 	PFRTAny vector_rest(PFRTAny);
EXTERNC 	PFRTAny vector_extend(PFRTAny,PFRTAny);
EXTERNC 	PFRTAny vector_update(PFRTAny,PFRTAny,PFRTAny);
EXTERNC 	PFRTAny vector_update_bang(PFRTAny,PFRTAny,PFRTAny);
EXTERNC 	PFRTAny vector_pop(PFRTAny);
EXTERNC 	PFRTAny vector_pop_bang(PFRTAny);
EXTERNC 	PFRTAny vector_get(PFRTAny,PFRTAny);
EXTERNC 	PFRTAny vector_get_default(PFRTAny, PFRTAny,PFRTAny);
EXTERNC  PFRTAny	vector_drop_bang(PFRTAny,PFRTAny);
EXTERNC  PFRTAny	vector_dropLast_bang(PFRTAny);
EXTERNC  PFRTAny vector_droplast(PFRTAny);
EXTERNC 	PFRTAny vectorGetDefault(PFRTAny v, uint32_t index);
EXTERNC 	PFRTAny vector_print(PFRTIOChannel,PFRTAny);
#endif

//	Set function
#ifndef SET_IMPL
EXTERNC PFRTAny  const empty_set;
EXTERNC PFRTAny 	foidl_set_inst_bang();
EXTERNC PFRTAny 	foidl_set_extend_bang(PFRTAny s, PFRTAny k);
EXTERNC PFRTAny 	set_extend(PFRTAny,PFRTAny);
EXTERNC PFRTAny 	set_remove(PFRTAny,PFRTAny);
EXTERNC PFRTAny 	set_first(PFRTAny);
EXTERNC PFRTAny 	set_second(PFRTAny);
EXTERNC PFRTAny 	set_rest(PFRTAny);
EXTERNC PFRTAny 	set_get(PFRTAny,PFRTAny);
EXTERNC PFRTBitmapNode set_getNode(PFRTBitmapNode, uint32_t);
EXTERNC PFRTAny 	set_get_default(PFRTAny, PFRTAny,PFRTAny);
EXTERNC PFRTAny 	setGetDefault(PFRTAny node, uint32_t index);
EXTERNC PFRTAny 	set_print(PFRTIOChannel,PFRTAny);
#endif

//	Map functions
#ifndef MAP_IMPL
EXTERNC PFRTAny const empty_map;
EXTERNC PFRTAny 	foidl_map_inst_bang();
EXTERNC PFRTAny 	foidl_map_extend_bang(PFRTAny,PFRTAny,PFRTAny);
EXTERNC PFRTAny 	map_first(PFRTAny);
EXTERNC PFRTAny 	map_second(PFRTAny);
EXTERNC PFRTAny 	map_rest(PFRTAny);
EXTERNC PFRTAny  map_get(PFRTAny, PFRTAny);
EXTERNC PFRTAny  map_get_default(PFRTAny, PFRTAny,PFRTAny);
EXTERNC PFRTAny 	map_extend(PFRTAny,PFRTAny,PFRTAny);
EXTERNC PFRTAny 	map_update(PFRTAny,PFRTAny,PFRTAny);
EXTERNC PFRTAny  map_remove(PFRTAny, PFRTAny);
EXTERNC PFRTAny  map_contains_qmark(PFRTAny, PFRTAny);
EXTERNC PFRTAny 	mapGetDefault(PFRTAny node, uint32_t index);
EXTERNC PFRTAny 	map_print(PFRTIOChannel,PFRTAny);
#endif

// List
#ifndef LIST_IMPL
EXTERNC  PFRTAny  const empty_list;
EXTERNC  PFRTLinkNode empty_link;
EXTERNC 	PFRTAny  foidl_list_inst_bang();
EXTERNC 	PFRTAny  foidl_list_extend_bang(PFRTAny,PFRTAny);
EXTERNC 	PFRTAny  list_extend(PFRTAny,PFRTAny);
EXTERNC  PFRTAny  list_prepend_bang(PFRTAny,PFRTAny);
EXTERNC 	PFRTAny  list_get(PFRTAny,PFRTAny);
EXTERNC  PFRTAny  list_get_default(PFRTAny, PFRTAny, PFRTAny);
EXTERNC  PFRTAny  list_index_of(PFRTAny,PFRTAny);
EXTERNC 	PFRTAny  list_update(PFRTAny,PFRTAny,PFRTAny);
EXTERNC 	PFRTAny  list_update_bang(PFRTAny,PFRTAny,PFRTAny);
EXTERNC 	PFRTAny  list_pop(PFRTAny);
EXTERNC 	PFRTAny  list_pop_bang(PFRTAny);
EXTERNC  PFRTAny  list_drop_bang(PFRTAny,PFRTAny);
EXTERNC  PFRTAny  list_droplast(PFRTAny);
EXTERNC 	PFRTAny  list_push(PFRTAny,PFRTAny);
EXTERNC 	PFRTAny  list_push_bang(PFRTAny,PFRTAny);
EXTERNC 	PFRTAny  list_first(PFRTAny);
EXTERNC 	PFRTAny  list_second(PFRTAny);
EXTERNC 	PFRTAny  list_rest(PFRTAny);
EXTERNC  PFRTAny  list_last(PFRTAny);
EXTERNC 	PFRTAny  list_print(PFRTIOChannel,PFRTAny);
EXTERNC  PFRTAny  empty_list_bang(PFRTAny);
EXTERNC  PFRTAny  release_list_bang(PFRTAny);
#endif

// IO
#ifndef IO_IMPL
EXTERNC 	PFRTAny  writeCout(PFRTAny);
EXTERNC 	PFRTAny  writeCoutNl(PFRTAny);
EXTERNC 	PFRTAny  foidl_open_bang(PFRTAny);
EXTERNC 	PFRTAny  foidl_write_bang(PFRTIOChannel,PFRTAny);
EXTERNC 	PFRTAny  io_get(PFRTAny,PFRTAny);
EXTERNC 	PFRTAny  foidl_io_count(PFRTAny);
EXTERNC  PFRTAny  foidl_io_first(PFRTAny);
EXTERNC  PFRTAny  foidl_io_second(PFRTAny);
EXTERNC  PFRTAny  foidl_io_countto(PFRTAny,PFRTAny);
EXTERNC  PFRTAny  foidl_io_countnotto(PFRTAny,PFRTAny);
#endif

// IO Handlers
#ifndef IOHANDLERS_IMPL
EXTERNC PFRTAny 	io_nextByteReader(PFRTIterator);
EXTERNC PFRTAny 	io_nextCharReader(PFRTIterator);
EXTERNC PFRTAny 	io_nextStringReader(PFRTIterator);
EXTERNC PFRTFuncRef const byte_reader;
EXTERNC PFRTFuncRef const char_reader;
EXTERNC PFRTFuncRef const string_reader;
EXTERNC PFRTFuncRef const char_writer;
EXTERNC PFRTAny 	io_skipto(PFRTIOChannel, PFRTAny);
EXTERNC PFRTAny 	io_skipwhile(PFRTIOChannel, PFRTAny);
EXTERNC PFRTAny 	io_countto(PFRTIOChannel, PFRTAny);
EXTERNC PFRTAny 	io_take_until(PFRTIOChannel, PFRTAny);
EXTERNC PFRTAny 	io_take_string(PFRTIOChannel);
EXTERNC PFRTAny 	io_take_qchar(PFRTIOChannel);
EXTERNC PFRTAny 	io_unread(PFRTIOChannel);
EXTERNC PFRTAny 	io_count(PFRTIOChannel);
EXTERNC PFRTAny 	io_first(PFRTIOChannel);
EXTERNC PFRTAny 	io_second(PFRTIOChannel);
EXTERNC PFRTAny 	io_peek_match(PFRTIOChannel,PFRTAny);
EXTERNC void 	io_blockBufferClose(PFRTIOChannel);
#endif

// Series

#ifndef SERIES_IMPL
EXTERNC  const PFRTSeries infinite;
EXTERNC 	void foidl_rtl_init_series();
EXTERNC 	PFRTAny foidl_series(PFRTAny,PFRTAny,PFRTAny);
EXTERNC  PFRTAny ss_defstep;
EXTERNC 	PFRTAny ss_defend;
#endif

//	Regex
#ifndef REGEX_IMPL
EXTERNC void foidl_rt_init_regex();
EXTERNC PFRTAny foidl_regex_cmp_qmark(PFRTAny, PFRTAny);
#endif

#endif