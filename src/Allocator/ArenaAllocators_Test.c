#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "ArenaAllocators.c"
#include "Random.c"

#define DEBUG 1
#if DEBUG
#define LOG(x, y) printf(x, y)
#else
#define LOG(x, y) 0
#endif

void TestAlloc(ArenaAllocator* arena, int* array, int num)
{
    int size = arena->offset;
    for (int i = 0; i < num; ++i)
    {
        uintptr_t align = AlignForward((uintptr_t)arena->buffer + (uintptr_t)size, DEFAULT_ALIGNMENT) - (uintptr_t)arena->buffer;
        size = array[i] + align;
        char* str = ArenaAlloc(arena, array[i]);
        LOG("--Iternation %d--\n", i);
        LOG("Align: %u\n", align);
        LOG("Size: %d\n", array[i]);
        LOG("Previous Offset: %d\n", arena->prevOffset);
        LOG("Offset: %d\n", arena->offset);
        LOG("Pointer: %p\n", str);
        assert(arena->prevOffset == align);
        assert(arena->offset == size);
    }
}

void* TestResize(ArenaAllocator* arena, unsigned char* oldMemory, int oldSize, int newSize, int length, size_t align)
{
    void* str = ArenaAllocResizeAlign(arena, oldMemory, oldSize, newSize, align);
    LOG("Old Size: %d\n", oldSize);
    LOG("New Size: %d\n", newSize);
    LOG("Old memory: %p\n", oldMemory);
    return str;
}

int main(void)
{
    uint32_t length = 1024;
    unsigned char* buf = malloc(length);
    printf("buf: %p\n", buf);
    printf("Default Alignment: %d\n", DEFAULT_ALIGNMENT);

    printf("\n-----TEST INIT-----\n");
    ArenaAllocator arena;
    ArenaInit(&arena, buf, length);
    assert(arena.offset == 0);
    assert(arena.prevOffset == 0);
    assert(arena.length == length);
    assert(arena.buffer == buf);
    printf("-----PASS TEST-----\n");

    printf("\n-----TEST ALLOC-----\n");
    int testInt[8] = { 11, 13, 16, 4, 8, 12, 0, 3 };
    TestAlloc(&arena, testInt, 8);
    printf("-----PASS TEST-----\n");
    
    printf("\n-----TEST RESIZE-----\n");
    int oldSizes[9] = { 5, 9, 12, 11, 18, 1, 3, 9, 8};
    int newSizes[9] = { 11, 12, 5, 4, 2, 1, 6, 8, 16};

    printf("\n[Old Memory == Previous Offset]\n");
    for (int i = 0; i < 9; ++i)
    {
        unsigned char* oldMemory = (unsigned char*)AlignForward((uintptr_t)arena.buffer + (uintptr_t)arena.offset, DEFAULT_ALIGNMENT);
        void* str = TestResize(&arena, oldMemory, oldSizes[i], newSizes[i], 9, DEFAULT_ALIGNMENT);
        assert(oldMemory == str);
    }

    printf("\n[Old Memory != Previous Offset]\n");
    RNG rng;
    for (int i = 0; i < 9; ++i)
    {
        unsigned char* randomPtr = arena.buffer + RandomRangeInt(&rng, 0, arena.length);
        LOG("Random pointer: %p\n", randomPtr);
        LOG("Distance to base: %u\n", randomPtr - arena.buffer);
        TestResize(&arena, randomPtr, oldSizes[i], newSizes[i], 9, DEFAULT_ALIGNMENT);
    }
    printf("-----PASS TEST-----\n");

    printf("\n-----TEST TEMP-----\n");
    TempArena temp = TempArenaBegin(&arena);
    TestAlloc(temp.arena, testInt, 8);
    assert(temp.offset != arena.offset);
    assert(temp.prevOffset != arena.prevOffset);
    TempArenaEnd(temp);
    assert(temp.offset == arena.offset);
    assert(temp.prevOffset == arena.prevOffset);
    printf("-----PASS TEST-----\n");

    printf("\n-----TEST FREE-----\n");
    ArenaFreeAll(&arena);
    assert(arena.prevOffset == 0);
    assert(arena.offset == 0);
    printf("-----PASS TEST-----\n");

    return 0;
}