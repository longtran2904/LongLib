#include <stdint.h>
#include "StretchyBuffer.c"

typedef struct Hash
{
    uint32_t numOfBuckets;
    uint64_t* keys;
    uint64_t* values;
} Hash;

uint64_t HashLookup(const Hash* hash, uint64_t k, uint64_t defaultValue)
{
    for (int i = 0; i < GetSize(hash->keys); ++i)
    {
        if (hash->keys[i] == k)
            return hash->values[i];
    }
    return defaultValue;
}

void HashAdd(Hash* hash, uint64_t k, uint64_t value)
{
    PushArray(hash->keys, k);
    PushArray(hash->values, value);
    hash->numOfBuckets++;
}

void HashClear(Hash* hash)
{
    free(hash->keys);
    free(hash->values);
    *hash = (Hash){0};
}
