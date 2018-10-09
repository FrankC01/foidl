/*
	foidl_hash.c
	Library support for HAMT map
	
	Copyright Frank V. Castellucci
	All Rights Reserved
*/

#define  HASH_IMPL

#include <foidlrt.h>
#include <stdio.h>

uint32_t murmur3_32(const uint8_t* key, size_t len, uint32_t seed) {
  uint32_t h = seed;
  if (len > 3) {
    const uint32_t* key_x4 = (const uint32_t*) key;
    size_t i = len >> 2;
    do {
      uint32_t k = *key_x4++;
      k *= 0xcc9e2d51;
      k = (k << 15) | (k >> 17);
      k *= 0x1b873593;
      h ^= k;
      h = (h << 13) | (h >> 19);
      h = (h * 5) + 0xe6546b64;
    } while (--i);
    key = (const uint8_t*) key_x4;
  }
  if (len & 3) {
    size_t i = len & 3;
    uint32_t k = 0;
    key = &key[i - 1];
    do {
      k <<= 8;
      k |= *key--;
    } while (--i);
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    h ^= k;
  }
  h ^= len;
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;
  return h;
}

uint32_t hash(PFRTAny);

static uint32_t nodeHashCode(PFRTBitmapNode node) {
 	uint32_t prime = 31;
 	uint32_t result = 0;
	result = prime * result + (node->nodemap);
	result = prime * result + (node->datamap);
	uint32_t nlen = nodelength(node);
	uint32_t nhsh = 0;
	for(uint32_t i=0;i<nlen;i++)
		nhsh += hash(node->slots[i]);
	result = prime * result + nhsh;
	return result;
}

uint32_t hash(PFRTAny p) {
	if(p->fclass == scalar_class) {
		if(p->ftype == string_type || p->ftype == keyword_type)
			return murmur3_32((uint8_t *)p->value,p->count,0);			
		else if (p->ftype == integer_type)
			return murmur3_32((uint8_t*)&p->value,8,0);	
    else if (p->ftype == character_type)
      return murmur3_32((uint8_t*)&p->value,8,0); 		
	}
	else if(p->fclass == bitmapnode_class) {
		return nodeHashCode((PFRTBitmapNode) p);
	}
  else if(p->fclass == collection_class) {
    return murmur3_32((uint8_t*)p,8,0);//p->hash;
  }
  else if(p->fclass == function_class) {
    return murmur3_32((uint8_t*)p,8,0);
  }
  else if(p->fclass == io_class) {
    return murmur3_32((uint8_t*)p,8,0);
  }
	else {
		printf("HASH: Can't handle class of 0x%08llX\n",p->fclass);
		//exit(-1);
		unknown_handler();
	}
	return 0;
}
