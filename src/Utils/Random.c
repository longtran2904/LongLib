#include <stdint.h>
#include <stdbool.h>
#include <float.h>
#include <stdio.h>
#include <assert.h>

typedef struct RNG
{
    uint32_t seed;
    uint32_t position;
} RNG;

uint32_t GetNoise1D(int position, uint32_t seed)
{
    const uint32_t BIT_NOISE1 = 0x68E31DA4;
    const uint32_t BIT_NOISE2 = 0xB5297A4D;
    const uint32_t BIT_NOISE3 = 0x1B56C4E9;

    uint32_t mangledBits = (uint32_t)position;
    mangledBits *= BIT_NOISE1;
    mangledBits += seed;
    mangledBits ^= (mangledBits >> 8);
    mangledBits += BIT_NOISE2;
    mangledBits ^= (mangledBits << 8);
    mangledBits *= BIT_NOISE3;
    mangledBits ^= (mangledBits >> 8);
    return mangledBits;
}

inline uint32_t GetNoise2D(int posX, int posY, uint32_t seed)
{
    const int PRIME_NUMBER = 198491317;
    return GetNoise1D(posX + (PRIME_NUMBER * posY), seed);
}

inline uint32_t GetNoise3D(int posX, int posY, int posZ, uint32_t seed)
{
    const int PRIME1 = 198491317;
    const int PRIME2 = 6542989;
    return GetNoise1D(posX + (PRIME1 * posY) + (PRIME2 * posZ), seed);
}

uint32_t RandomUint32(RNG* rng)
{
    return GetNoise1D(rng->position++, rng->seed);
}

int RandomRangeInt(RNG* rng, int minInclusive, int maxInclusive)
{
    return minInclusive + RandomUint32(rng) % (maxInclusive - minInclusive);
}

float RandomRangeFloat(RNG* rng, float minInclusive, float maxInclusive)
{
    return minInclusive + (maxInclusive - minInclusive) * RandomUint32(rng) / (float)UINT32_MAX;
}

float RandomValue(RNG* rng)
{
    return RandomUint32(rng) / (float)UINT32_MAX;
}

bool Random(RNG* rng, float prob)
{
    return RandomValue(rng) < prob ? true : false;
}