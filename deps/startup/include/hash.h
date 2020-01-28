#ifndef HASH_H
#define HASH_H

#include <stdint.h>



// djb2 hash for short values
static unsigned long hashStringDjb2(unsigned char *str){
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}


#define hash_string(x,len,seed) XXH32(x,len,seed)
#define hash_string16(x,len,seed) hashStringDjb2(x)

#define hash_add(lookupAddr, str, arrSize)
#define hash_add16(lookupAddr, str, arrSize)


typedef struct hash_t {
  uint32_t num_buckets;
  uint32_t *keys;
  uint32_t *values;
} sHash_t;


uint32_t hash_lookup(const sHash_t *hash, uint32_t k, uint32_t default_value);
//hash_add(&info_lookup, hash_string("my name"), array_size(info) - 1);

typedef struct hash16_t {
  uint16_t num_buckets;
  uint16_t *keys;
  uint16_t *values;
} sHash16_t;

uint16_t hash16_lookup(const sHash16_t *hash, uint16_t k, uint16_t default_value);
//hash_add16(&info_lookup, hash_string("my name"), array_size(info) - 1);


#endif
