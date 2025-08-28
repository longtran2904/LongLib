#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "../Utils/Math.c"
#include "Allocator.h"

typedef struct ArenaAllocator
{
    unsigned char* buffer;
    size_t length;
    size_t offset;
    size_t prevOffset;
} ArenaAllocator;

void ArenaInit(ArenaAllocator* a, void *buffer, size_t length) {
	a->buffer = (unsigned char *)buffer;
	a->length = length;
	a->offset = 0;
    a->prevOffset = 0;
}

void* ArenaAllocAlign(ArenaAllocator* arena, size_t size, size_t align)
{
	// Align 'a->offset' forward to the specified alignment
	uintptr_t currentPtr = (uintptr_t)arena->buffer + (uintptr_t)arena->offset;
	uintptr_t offset = AlignForward(currentPtr, align);
    offset -= (uintptr_t)arena->buffer; // Change to relative offset
    
    if (offset + size <= arena->length)
    {
        void* ptr = &arena->buffer[offset];
        arena->prevOffset = offset;
        arena->offset = offset + size;
        memset(ptr, 0, size);
        return ptr;
    }
    return NULL;
}

// Because C doesn't have default parameter
void* ArenaAlloc(ArenaAllocator* arena, size_t size)
{
    return ArenaAllocAlign(arena, size, DEFAULT_ALIGNMENT);
}

void* ArenaAllocResizeAlign(ArenaAllocator* a, void* oldMemory, size_t oldSize, size_t newSize, size_t align)
{
	unsigned char *oldMem = (unsigned char *)oldMemory;
    
	assert(IsPowerOfTwo(align));
    if (oldMem == NULL || oldSize == 0)
        return ArenaAllocAlign(a, newSize, align);
    else if (a->buffer <= oldMem && oldMem < a->buffer + a->length)
    {
        if (a->buffer + a->prevOffset == oldMem)
        {
            if (a->prevOffset + newSize > a->length)
            {
                assert(0 && "The new memory (newSize + prevOffset) is out of bounds of the buffer in this arena!");
                return NULL;
            }
            a->offset = a->prevOffset + newSize;
            if (newSize > oldSize)
                memset(&a->buffer[a->offset], 0, newSize - oldSize); // zero the new memory by default
            return oldMemory;
        }
        else
        {
            void* newMemory = ArenaAllocAlign(a, newSize, align);
            if (!newMemory)
            {
                assert(0 && "The new memory (newSize + offset) is out of bounds of the buffer in this arena!");
                return NULL;
            }
            size_t copySize = oldSize < newSize ? oldSize : newSize;
            memmove(newMemory, oldMemory, copySize);
            return newMemory;
        }
    }
    else
    {
        assert(0 && "Memory is out of bounds of the buffer in this arena");
        return NULL;
    }
}

// Because C doesn't have default parameter
void* ArenaAllocResize(ArenaAllocator* a, void* oldMemory, size_t oldSize, size_t newSize)
{
    return ArenaAllocResizeAlign(a, oldMemory, oldSize, newSize, DEFAULT_ALIGNMENT);
}

void ArenaFreeAll(ArenaAllocator* arena)
{
    arena->offset = 0;
    arena->prevOffset = 0;
}

typedef struct TempArena
{
    ArenaAllocator* arena;
    size_t prevOffset;
    size_t offset;
} TempArena;

TempArena TempArenaBegin(ArenaAllocator* arena)
{
    return (TempArena){arena, arena->prevOffset, arena->offset};
}

void TempArenaEnd(TempArena temp)
{
    temp.arena->prevOffset = temp.prevOffset;
    temp.arena->offset = temp.offset;
}