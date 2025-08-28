#include <stdint.h>
#include <assert.h>
#include "../Utils/Math.c"
#include "Allocator.h"

typedef struct Stack
{
    Allocator base;
    uint8_t* buffer;
    size_t length;
    size_t prevOffset;
    size_t offset;
} Stack;

typedef struct StackHeader
{
    size_t prevOffset;
    size_t padding;
} StackHeader;

void StackInit(Stack* s, void* buffer, size_t length)
{
	s->buffer = (uint8_t*)buffer;
	s->length = length;
	s->offset = 0;
}

void* StackAllocAlign(Stack* s, size_t size, size_t alignment)
{
	uintptr_t currentAddr, nextAddr ;
	size_t padding;
    StackHeader *header;
    
	assert(IsPowerOfTwo(alignment));
    
	if (alignment > 128) {
		// As the padding is 8 bits (1 byte), the largest alignment that can
		// be used is 128 bytes
		alignment = 128;
	}
    
    currentAddr = (uintptr_t)s->buffer + (uintptr_t)s->offset;
	padding = CalcPaddingWithHeader(currentAddr, (uintptr_t)alignment, sizeof(StackHeader));
	if (s->offset + padding + size > s->length) {
		// Stack allocator is out of memory
		return NULL;
	}
    
    s->prevOffset = s->offset;
	s->offset += padding;
    
    nextAddr = currentAddr + (uintptr_t)padding;
	header = (StackHeader*)(nextAddr - sizeof(StackHeader));
	header->padding = padding;
    header->prevOffset = s->prevOffset;
    
	s->offset += size;
    
	return memset((void *)nextAddr, 0, size);
}

// Because C does not have default parameters
void* StackAlloc(Stack* s, size_t size) {
	return StackAllocAlign(s, size, DEFAULT_ALIGNMENT);
}

void StackFree(Stack* s, void* ptr) {
	if (ptr != NULL) {
		uintptr_t start, end, currentAddr;
        StackHeader* header;
		size_t prevOffset;
        
		start = (uintptr_t)s->buffer;
		end = start + (uintptr_t)s->length;
		currentAddr = (uintptr_t)ptr;
        
		if (!(start <= currentAddr && currentAddr < end)) {
			assert(0 && "Out of bounds memory address passed to stack allocator (free)");
			return;
		}
        
		if (currentAddr >= start+(uintptr_t)s->offset)
        {
            // Allow double frees
            return;
        }
        
        header = (StackHeader*)(currentAddr - sizeof(StackHeader));
        
        // Calculate previous offset from the header and its address
        prevOffset = (size_t)(currentAddr - (uintptr_t)header->padding - start);
        
        if (prevOffset != header->prevOffset)
        {
            assert(0 && "Out of order stack allocator free");
            return;
        }
        
        s->offset = prevOffset;
        s->prevOffset = header->prevOffset;
	}
}

void StackFreeAll(Stack *s) {
	s->offset = 0;
}

void* StackResizeAlign(Stack* s, void* ptr, size_t oldSize, size_t newSize, size_t alignment) {
	if (ptr == NULL) {
		return StackAllocAlign(s, newSize, alignment);
	} else if (newSize == 0) {
		StackFree(s, ptr);
		return NULL;
	} else {
		uintptr_t start, end, currentAddr;
		void *newPtr;
        
		start = (uintptr_t)s->buffer;
		end = start + (uintptr_t)s->length;
		currentAddr = (uintptr_t)ptr;
		if (!(start <= currentAddr && currentAddr < end)) {
			assert(0 && "Out of bounds memory address passed to stack allocator (resize)");
			return NULL;
		}
        
		if (currentAddr >= start + (uintptr_t)s->offset) {
			// Treat as a double free
			return NULL;
		}
        
		if (oldSize == newSize)
        {
			return ptr;
		}
        
        if (ptr == s->buffer + s->prevOffset)
        {
            if (newSize > oldSize)
            {
                memset(s->buffer + s->offset, 0, newSize - oldSize);
            }
            s->offset = s->prevOffset + newSize;
        }
        
		newPtr = StackAllocAlign(s, newSize, alignment);
		memmove(newPtr, ptr, oldSize < newSize ? oldSize : newSize);
		return newPtr;
	}
}

void* StackResize(Stack *s, void *ptr, size_t oldSize, size_t newSize) {
	return StackResizeAlign(s, ptr, oldSize, newSize, DEFAULT_ALIGNMENT);
}