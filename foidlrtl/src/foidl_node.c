/*
	foidl_node.c
	Library support for node (CHAMP)
	
	Copyright Frank V. Castellucci
	All Rights Reserved
*/

#define  NODE_IMPL

#include <foidlrt.h>
#include <stdio.h>

const struct FRTBitmapNodeG _empty_champ_node = {
	global_signature,
	bitmapnode_class,
	0,
	0,
	(PFRTAny *) emtndarray};

PFRTBitmapNode const empty_champ_node = (PFRTBitmapNode) &_empty_champ_node.fclass;

//	Bit twiddling


uint32_t bit_count(uint32_t bmap) {	
	return __builtin_popcount(bmap);
}

uint32_t bit_pos(uint32_t mask) {
	return TUPLELENSFT << mask;
}

uint32_t node_count(PFRTBitmapNode node) {
	if(node == empty_champ_node) return 0;
	return bit_count(node->nodemap);
}

uint32_t data_count(PFRTBitmapNode node) {
	if(node == empty_champ_node) return 0;
	return bit_count(node->datamap);	
}

uint32_t nodelength(PFRTBitmapNode node) {
	return ((data_count(node) << TUPLELENSFT) + node_count(node)) ;
}

uint32_t dataIndex(uint32_t datamap, uint32_t bitpos) {
	return bit_count(datamap & (bitpos - TUPLELENSFT));
}

uint32_t nodeIndex(uint32_t nodemap, uint32_t bitpos) {
      return bit_count(nodemap & (bitpos - TUPLELENSFT));
}

uint32_t payloadArity(PFRTBitmapNode node) {
	return data_count(node);
}

uint32_t nodeArity(PFRTBitmapNode node) {
	return node_count(node);
}

uint32_t sizePredicate(PFRTBitmapNode node) {
	if(nodeArity(node) == 0) {
		return payloadArity(node);
	}
	else 
		return TUPLELEN;
}


PFRTBitmapNode getNode(PFRTBitmapNode node, uint32_t index) {
	return (PFRTBitmapNode) node->slots[nodelength(node) - 1 - index];
}

uint32_t mask(uint32_t keyHash, ft shift) {
	return (keyHash >> shift) & MASK;
}

//	Common functions 

void arraycopy(PFRTAny *src, PFRTAny *dst,uint32_t len) {
	for(uint32_t i = 0; i < len; i++) {
		PFRTAny  srcVal = src[i];
		dst[i] = srcVal;
	}
}
