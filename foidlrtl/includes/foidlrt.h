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

static const ft 	integer_type 	= 0xffffffff1000a8a9;

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
void *foidl_open_ro_mmap_file(char *);
ft 	foidl_fclose(ft);
void foidl_exit();


extern void *foidl_alloc(ft);
extern void foidl_release(void *);

// RTL Asm Error functions

extern void unknown_handler();

//	RTL Asm Math functions

#ifndef MATH_IMPL
extern PFRTAny 	foidl_add(PFRTAny, PFRTAny);
#endif

#ifndef GLOBALS_IMPL
extern void foidl_rtl_init_globals();
extern PFRTAny  nil,end,true,false;
extern struct FRTTypeG _end;
extern struct FRTTypeG _nil;
extern struct FRTTypeG _false;
extern struct FRTTypeG _one;
extern struct FRTTypeG _zero;
extern PFRTAny 	langle,rangle;
extern PFRTAny 	lbrace,rbrace;
extern PFRTAny 	lbracket,rbracket;
extern PFRTAny 	tbchr,nlchr,crchr,spchr,semichr;
extern PFRTAny  semichr,colonchr,sqchr,dqchr,eschr;
extern PFRTAny 	meta,comma;
extern PFRTAny 	zero, one,two,three,four,five,six,seven,eight;
extern PFRTAny 	nine,ten,eleven,twelve,thirteen,fourteen,fifteen,sixteen;
extern PFRTAny 	empty_string, space_string;
extern PFRTIOChannel 	cin,cout,cerr;
extern PFRTAny 	nilstr, truestr, falsestr;
#endif

#ifndef TYPE_IMPL
extern void foidl_rtl_init_types();
#endif

#ifndef ENTRYPOINT_IMPL
extern PFRTAny 	foidl_count(PFRTAny);
extern PFRTAny 	foidl_get(PFRTAny,PFRTAny);
extern PFRTAny 	foidl_getd(PFRTAny,PFRTAny,PFRTAny);
extern PFRTAny 	foidl_first(PFRTAny);
extern PFRTAny 	foidl_second(PFRTAny);
extern PFRTAny 	foidl_last(PFRTAny);
extern PFRTAny 	foidl_rest(PFRTAny);
extern PFRTAny 	foidl_remove(PFRTAny,PFRTAny);
extern PFRTAny 	foidl_dropFor(PFRTAny,PFRTAny);
extern PFRTAny 	foidl_dropLast(PFRTAny);
extern PFRTAny 	foidl_extend(PFRTAny, PFRTAny);
extern PFRTAny 	foidl_extendKV(PFRTAny, PFRTAny,PFRTAny);
extern PFRTAny 	foidl_extend_bang(PFRTAny, PFRTAny);
extern PFRTAny 	foidl_update(PFRTAny,PFRTAny,PFRTAny);
extern PFRTAny 	foidl_update_bang(PFRTAny,PFRTAny,PFRTAny);
extern PFRTAny 	foidl_pop(PFRTAny);
extern PFRTAny 	foidl_pop_bang(PFRTAny);
extern PFRTAny 	foidl_push(PFRTAny, PFRTAny);
extern PFRTAny 	foidl_push_bang(PFRTAny, PFRTAny);
extern PFRTAny  foidl_apply(PFRTAny, PFRTAny);
extern PFRTAny  foidl_map(PFRTAny, PFRTAny);
extern PFRTAny 	foidl_fold(PFRTAny, PFRTAny, PFRTAny);
extern PFRTAny 	foidl_reduce(PFRTAny, PFRTAny, PFRTAny);
extern PFRTAny  foidl_reduced(PFRTAny);
extern PFRTAny  foidl_split(PFRTAny,PFRTAny); 	//	May move to string
#endif

#ifndef F_ASPRINTF
#ifdef _MSC_VER
#include <stdarg.h>
extern int vasprintf(char **strp, const char *format, va_list ap);
extern int asprintf(char **strp, const char *format, ...);
#endif
#endif

#ifndef PREDICATE_IMPL
extern PFRTAny scalar_equality(PFRTAny,PFRTAny);
extern PFRTAny foidl_equal_qmark(PFRTAny,PFRTAny);
extern PFRTAny foidl_not_equal_qmark(PFRTAny,PFRTAny);
extern PFRTAny foidl_function_qmark(PFRTAny);
extern PFRTAny foidl_collection_qmark(PFRTAny);
extern PFRTAny foidl_extendable_qmark(PFRTAny);
extern PFRTAny function_strict_arg(PFRTAny, PFRTAny);
extern PFRTAny string_type_qmark(PFRTAny);
#endif

#ifndef HASH_IMPL
extern  uint32_t murmur3_32(const uint8_t*, size_t, uint32_t);
extern 	uint32_t hash(PFRTAny);
#endif

#ifndef	ALLOC_IMPL

extern const PFRTAny emtndarray[];
extern void 			foidl_gc_init();
extern void *			foidl_xall(uint32_t sz);
extern void 			foidl_xdel(void *);
//	Scalar types
extern PFRTAny 			*allocRawAnyArray(ft count);
extern PFRTAny 			allocAny(ft fclass,ft ftype,void *value);
extern PFRTAny 			allocGlobalString(char *);
extern PFRTAny 			allocGlobalStringCopy(char *);
extern PFRTAny 			allocGlobalKeywordCopy(char *);

extern PFRTAny 			allocStringWithBufferSize(uint32_t);
extern PFRTAny 			allocStringWithCopy(char *);
extern PFRTAny 			allocStringWithCopyCnt(uint32_t, char *);
extern PFRTAny 			allocAndConcatString(uint32_t,char *, uint32_t, char *, uint32_t);

extern PFRTAny 			allocGlobalCharType(int);
extern PFRTAny 			allocGlobalIntegerType(long long v);

extern PFRTAny          allocRegex(PFRTAny sbase, void* regex);

// IO Types
extern PFRTIOChannel 	allocIOChannel(ft,PFRTAny);
extern PFRTIOBuffer 	allocIOMMapBuffer(void *);
extern PFRTIOBuffer 	allocIOBlockBuffer(ft);
extern PFRTIOBuffer 	allocIONoBuffer();
extern void 			deallocChannelBuffer(PFRTIOChannel);

// Function types
extern PFRTFuncRef2 	allocFuncRef2(void *fn, ft maxarg,invoke_funcptr ifn);
extern void 			deallocFuncRef2(PFRTFuncRef2);

// Collection types
extern PFRTList   		allocList(ft, PFRTLinkNode);
extern PFRTLinkNode   	allocLinkNode();
extern PFRTLinkNode     allocLinkNodeWith(PFRTAny data, PFRTLinkNode nextNode);
extern PFRTSet 			allocSet(ft,ft,PFRTBitmapNode);
extern PFRTMap 			allocMap(ft,ft,PFRTBitmapNode);
extern PFRTVector 		allocVector(ft,ft,PFRTHamtNode,PFRTHamtNode);
extern PFRTMapEntry 	allocMapEntryWith(PFRTAny, PFRTAny);
extern PFRTSeries 		allocSeries();
extern PFRTBitmapNode 	allocNode();
extern PFRTHamtNode 	allocHamtNode();
extern PFRTBitmapNode 	allocNodeWith(uint32_t, uint32_t, ft);
extern PFRTBitmapNode 	allocNodeClone(PFRTBitmapNode, ft);
extern PFRTBitmapNode 	allocNodeWithAll(uint32_t,uint32_t,PFRTAny *);

// Iterator types
extern PFRTIterator 	allocTrieIterator(PFRTAssocType,itrNext);
extern PFRTIterator 	allocVectorIterator(PFRTVector,itrNext);
extern PFRTIterator 	allocListIterator(PFRTList,itrNext);
extern PFRTIterator 	allocSeriesIterator(PFRTSeries,itrNext);
extern PFRTIterator 	allocChannelIterator(PFRTIOBuffer,itrNext);

#endif

// Errors
#ifndef ERRORS_IMPL
extern PFRTAny foidl_fail();
extern PFRTAny writeCerr(PFRTAny);
extern PFRTAny writeCerrNl(PFRTAny);
extern void    foidl_ep_excp(PFRTAny);
extern void    foidl_ep_excp2(PFRTAny,PFRTAny);
extern PFRTAny const unsupported;
extern PFRTAny const index_out_of_bounds;
extern PFRTAny const update_not_integral;
extern PFRTAny const extend_map_two_arg;
extern PFRTAny const fold_requires_function;
extern PFRTAny const fold_requires_collection;
extern PFRTAny const reduce_requires_function;
extern PFRTAny const reduce_requires_collection;
extern PFRTAny const not_read_channel;
extern PFRTAny const not_write_channel;
extern PFRTAny const already_closed_channel;
extern PFRTAny const requires_map;
extern PFRTAny const missing_channel;
extern PFRTAny const missing_open;
extern PFRTAny const unknown_open_flag;
extern PFRTAny const unknown_buffer_flag;
extern PFRTAny const unknown_reader_flag;
extern PFRTAny const unknown_writer_flag;
extern PFRTAny const file_is_directory;
#endif

// Characters
#ifndef CHARACTER_IMPL
extern void foidl_rtl_init_chars();
extern PFRTAny 	allocCharWithValue(ft);
extern PFRTAny 	allocECharWithValue(ft, ft);
#endif

// Integers
#ifndef INTEGER_IMPL
extern void 	foidl_rtl_init_ints();
extern PFRTAny 	allocIntegerWithValue(long long val);
extern PFRTAny 	foidl_add_ints(PFRTAny, PFRTAny);
extern PFRTAny 	foidl_sub_ints(PFRTAny, PFRTAny);
extern PFRTAny 	foidl_div_ints(PFRTAny, PFRTAny);
extern PFRTAny 	foidl_mul_ints(PFRTAny, PFRTAny);
extern PFRTAny 	foidl_mod_ints(PFRTAny, PFRTAny);
#endif

// String
#ifndef STRING_IMPL
extern 	void foildl_rtl_init_strings();
extern 	PFRTAny string_get(PFRTAny,PFRTAny);
extern 	PFRTAny string_get_default(PFRTAny, PFRTAny,PFRTAny);
extern  PFRTAny string_first(PFRTAny);
extern  PFRTAny string_second(PFRTAny);
extern  PFRTAny string_rest(PFRTAny);
extern  PFRTAny string_last(PFRTAny);
extern 	PFRTAny string_update(PFRTAny, PFRTAny, PFRTAny);
extern 	PFRTAny string_update_bang(PFRTAny, PFRTAny, PFRTAny);
extern  PFRTAny string_extend(PFRTAny, PFRTAny);
extern  PFRTAny string_extend_bang(PFRTAny, PFRTAny);
extern  PFRTAny string_droplast(PFRTAny);
extern  PFRTAny string_droplast_bang(PFRTAny);
extern  PFRTAny release_string(PFRTAny s);
//extern 	PFRTAny charPtrToString(char *);
#endif

#ifndef INVOKE_IMPL
extern PFRTAny 	foidl_fref_instance(PFRTAny);
extern PFRTAny 	foidl_imbue(PFRTAny,PFRTAny);
extern PFRTAny 	dispatch0(PFRTAny fn);
extern PFRTAny 	dispatch1(PFRTAny fn, PFRTAny arg1);
extern PFRTAny 	dispatch2(PFRTAny fn, PFRTAny arg1, PFRTAny arg2);
extern PFRTAny 	dispatch0i(PFRTAny fn);
extern PFRTAny 	dispatch1i(PFRTAny fn, PFRTAny arg1);
extern PFRTAny 	dispatch2i(PFRTAny fn, PFRTAny arg1, PFRTAny arg2);
#endif

#ifndef ITERATORS_IMPL
extern PFRTIterator iteratorFor(PFRTAny);
extern PFRTAny 		iteratorNext(PFRTIterator);
#endif

//	Node functions
#ifndef NODE_IMPL
extern const struct FRTBitmapNode _empty_champ_node;
extern PFRTBitmapNode   const empty_champ_node;
extern uint32_t 		bit_count(uint32_t);
extern uint32_t 		bit_pos(uint32_t);
extern uint32_t 		data_count(PFRTBitmapNode);
extern uint32_t 		node_count(PFRTBitmapNode);
extern uint32_t 		nodelength(PFRTBitmapNode);
extern uint32_t 		dataIndex(uint32_t, uint32_t);
extern uint32_t 		nodeIndex(uint32_t, uint32_t);
extern uint32_t 		payloadArity(PFRTBitmapNode);
extern uint32_t 		nodeArity(PFRTBitmapNode);
extern uint32_t 		mask(uint32_t, ft);
extern uint32_t 		sizePredicate(PFRTBitmapNode);
extern PFRTBitmapNode 	getNode(PFRTBitmapNode, uint32_t);
extern void 			arraycopy(PFRTAny *, PFRTAny *,uint32_t);
#endif

//  Vector functions
#ifndef VECTOR_IMPL
extern  PFRTHamtNode const empty_node;
extern 	PFRTVector 	 const empty_vector;
extern 	PFRTAny foidl_vector_inst_bang();
extern  PFRTAny foidl_vector_extend_bang(PFRTAny,PFRTAny);
extern  PFRTAny vector_from_argv(int, char**);
extern  PFRTAny vector_nth(PFRTVector, ft);
extern 	PFRTAny vector_first(PFRTAny);
extern 	PFRTAny vector_second(PFRTAny);
extern 	PFRTAny vector_rest(PFRTAny);
extern 	PFRTAny vector_extend(PFRTAny,PFRTAny);
extern 	PFRTAny vector_update(PFRTAny,PFRTAny,PFRTAny);
extern 	PFRTAny vector_update_bang(PFRTAny,PFRTAny,PFRTAny);
extern 	PFRTAny vector_pop(PFRTAny);
extern 	PFRTAny vector_pop_bang(PFRTAny);
extern 	PFRTAny vector_get(PFRTAny,PFRTAny);
extern 	PFRTAny vector_get_default(PFRTAny, PFRTAny,PFRTAny);
extern  PFRTAny	vector_drop_bang(PFRTAny,PFRTAny);
extern  PFRTAny	vector_dropLast_bang(PFRTAny);
extern 	PFRTAny vectorGetDefault(PFRTAny v, uint32_t index);
extern 	PFRTAny vector_print(PFRTIOChannel,PFRTAny);
#endif

//	Set function
#ifndef SET_IMPL
extern PFRTAny  const empty_set;
extern PFRTAny 	foidl_set_inst_bang();
extern PFRTAny 	foidl_set_extend_bang(PFRTAny s, PFRTAny k);
extern PFRTAny 	set_extend(PFRTAny,PFRTAny);
extern PFRTAny 	set_remove(PFRTAny,PFRTAny);
extern PFRTAny 	set_first(PFRTAny);
extern PFRTAny 	set_second(PFRTAny);
extern PFRTAny 	set_rest(PFRTAny);
extern PFRTAny 	set_get(PFRTAny,PFRTAny);
extern PFRTBitmapNode set_getNode(PFRTBitmapNode, uint32_t);
extern PFRTAny 	set_get_default(PFRTAny, PFRTAny,PFRTAny);
extern PFRTAny 	setGetDefault(PFRTAny node, uint32_t index);
extern PFRTAny 	set_print(PFRTIOChannel,PFRTAny);
#endif

//	Map functions
#ifndef MAP_IMPL
extern PFRTAny const empty_map;
extern PFRTAny 	foidl_map_inst_bang();
extern PFRTAny 	foidl_map_extend_bang(PFRTAny,PFRTAny,PFRTAny);
extern PFRTAny 	map_first(PFRTAny);
extern PFRTAny 	map_second(PFRTAny);
extern PFRTAny 	map_rest(PFRTAny);
extern PFRTAny  map_get(PFRTAny, PFRTAny);
extern PFRTAny  map_get_default(PFRTAny, PFRTAny,PFRTAny);
extern PFRTAny 	map_extend(PFRTAny,PFRTAny,PFRTAny);
extern PFRTAny 	map_update(PFRTAny,PFRTAny,PFRTAny);
extern PFRTAny  map_remove(PFRTAny, PFRTAny);
extern PFRTAny  map_contains_qmark(PFRTAny, PFRTAny);
extern PFRTAny 	mapGetDefault(PFRTAny node, uint32_t index);
extern PFRTAny 	map_print(PFRTIOChannel,PFRTAny);
#endif

// List
#ifndef LIST_IMPL
extern  PFRTAny  const empty_list;
extern  PFRTLinkNode empty_link;
extern 	PFRTAny  foidl_list_inst_bang();
extern 	PFRTAny  foidl_list_extend_bang(PFRTAny,PFRTAny);
extern 	PFRTAny  list_extend(PFRTAny,PFRTAny);
extern  PFRTAny  list_prepend_bang(PFRTAny,PFRTAny);
extern 	PFRTAny  list_get(PFRTAny,PFRTAny);
extern  PFRTAny  list_get_default(PFRTAny, PFRTAny, PFRTAny);
extern  PFRTAny  list_index_of(PFRTAny,PFRTAny);
extern 	PFRTAny  list_update(PFRTAny,PFRTAny,PFRTAny);
extern 	PFRTAny  list_update_bang(PFRTAny,PFRTAny,PFRTAny);
extern 	PFRTAny  list_pop(PFRTAny);
extern 	PFRTAny  list_pop_bang(PFRTAny);
extern  PFRTAny  list_drop_bang(PFRTAny,PFRTAny);
extern 	PFRTAny  list_push(PFRTAny,PFRTAny);
extern 	PFRTAny  list_push_bang(PFRTAny,PFRTAny);
extern 	PFRTAny  list_first(PFRTAny);
extern 	PFRTAny  list_second(PFRTAny);
extern 	PFRTAny  list_rest(PFRTAny);
extern 	PFRTAny  list_print(PFRTIOChannel,PFRTAny);
extern  PFRTAny  release_list_bang(PFRTAny s);
#endif

// IO
#ifndef IO_IMPL
extern 	PFRTAny  writeCout(PFRTAny);
extern 	PFRTAny  writeCoutNl(PFRTAny);
extern 	PFRTAny  foidl_open_bang(PFRTAny);
extern 	PFRTAny  foidl_write_bang(PFRTIOChannel,PFRTAny);
extern 	PFRTAny  io_get(PFRTAny,PFRTAny);
extern 	PFRTAny  foidl_io_count(PFRTAny);
extern  PFRTAny  foidl_io_first(PFRTAny);
extern  PFRTAny  foidl_io_second(PFRTAny);
extern  PFRTAny  foidl_io_countto(PFRTAny,PFRTAny);
extern  PFRTAny  foidl_io_countnotto(PFRTAny,PFRTAny);
#endif

// IO Handlers
#ifndef IOHANDLERS_IMPL
extern PFRTAny 	io_nextByteReader(PFRTIterator);
extern PFRTAny 	io_nextCharReader(PFRTIterator);
extern PFRTAny 	io_nextStringReader(PFRTIterator);
extern PFRTFuncRef const byte_reader;
extern PFRTFuncRef const char_reader;
extern PFRTFuncRef const string_reader;
extern PFRTFuncRef const char_writer;
extern PFRTAny 	io_skipto(PFRTIOChannel, PFRTAny);
extern PFRTAny 	io_skipwhile(PFRTIOChannel, PFRTAny);
extern PFRTAny 	io_countto(PFRTIOChannel, PFRTAny);
extern PFRTAny 	io_take_until(PFRTIOChannel, PFRTAny);
extern PFRTAny 	io_take_string(PFRTIOChannel);
extern PFRTAny 	io_take_qchar(PFRTIOChannel);
extern PFRTAny 	io_unread(PFRTIOChannel);
extern PFRTAny 	io_count(PFRTIOChannel);
extern PFRTAny 	io_first(PFRTIOChannel);
extern PFRTAny 	io_second(PFRTIOChannel);
extern PFRTAny 	io_peek_match(PFRTIOChannel,PFRTAny);
extern void 	io_blockBufferClose(PFRTIOChannel);
#endif

// Series

#ifndef SERIES_IMPL
extern  const PFRTSeries infinite;
extern 	void foidl_rtl_init_series();
extern 	PFRTAny foidl_series(PFRTAny,PFRTAny,PFRTAny);
extern  PFRTAny ss_defstep;
extern 	PFRTAny ss_defend;
#endif

//	Regex
#ifndef REGEX_IMPL
extern void foidl_rt_init_regex();
extern PFRTAny foidl_regex_cmp_qmark(PFRTAny, PFRTAny);
#endif

#endif