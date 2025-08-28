#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include "Random.c"

#define DEBUG 0

#if DEBUG
#define LOG(x, y) printf(x, y)
#else
#define LOG(x, y) x, y
#endif

int main(void)
{
    RNG rng = {0};
    const int numberOfTests = 10;

    printf("-----Test RandomUint32-----\n");
    for (int i = 0; i < numberOfTests; ++i)
    {
        LOG("%u\n", RandomUint32(&rng));
    }
    printf("-----Done Test!-----\n");

    printf("\n\n-----Test RandomBool-----\n");
    int trueCounter = 0;
    int falseCounter = 0;
    float prob = .75;
    for (int i = 0; i < numberOfTests; ++i)
    {
        LOG("%s\n", Random(&rng, prob) ? (trueCounter++, "true") : (falseCounter++, "false"));
    }
    printf("%d\n", trueCounter);
    printf("%d\n", falseCounter);
    printf("-----Done Test!-----\n");

    printf("\n\n-----Test RandomRangeInt-----\n");
    for (int i = 0; i < numberOfTests; ++i)
    {
        int min = -20;
        int max = 78;
        int value = RandomRangeInt(&rng, min, max);
        LOG("%d\n", value);
        assert((value <= max) && (value >= min));
    }
    printf("-----Done Test!-----\n");

    printf("\n\n-----Test RandomRangeFloat-----\n");
    for (int i = 0; i < numberOfTests; ++i)
    {
        float min = 20.5374;
        float max = 120.6351;
        float value = RandomRangeFloat(&rng, min, max);
        LOG("%f\n", value);
        assert((value <= max) && (value >= min));
    }
    printf("-----Done Test!-----\n");

    return 0;
}