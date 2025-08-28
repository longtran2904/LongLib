#include <stdint.h>
#include <assert.h>
#include "../Utils/Math.c"
#include "Allocator.h"

typedef struct PoolFreeNode {
	struct PoolFreeNode* next;
} PoolFreeNode;

typedef struct Pool
{
    uint8_t* buffer;
	size_t length;
	size_t chunkSize;
    
	PoolFreeNode* head; // Free List Head
} Pool;

void PoolFreeAll(Pool *p) {
	size_t chunkCount = p->length / p->chunkSize;
    
	// Set all chunks to be free
	for (size_t i = 0; i < chunkCount; i++) {
		void *ptr = &p->buffer[i * p->chunkSize];
		PoolFreeNode* node = (PoolFreeNode*)ptr;
		// Push free node onto thte free list
		node->next = p->head;
		p->head = node;
	}
}

void PoolInit(Pool* p, void* buffer, size_t length,
              size_t chunkSize, size_t alignment) {
	// Align backing buffer to the specified chunk alignment
	uintptr_t initialStart = (uintptr_t)buffer;
	uintptr_t start = AlignForward(initialStart, (uintptr_t)alignment);
	length -= (size_t)(start-initialStart);
    
	// Align chunk size up to the required alignment
	chunkSize = AlignForward(chunkSize, alignment);
    
	// Assert that the parameters passed are valid
	assert(chunkSize >= sizeof(PoolFreeNode) &&
	       "Chunk size is too small");
	assert(length >= chunkSize &&
	       "Backing buffer length is smaller than the chunk size");
    
	// Store the adjusted parameters
	p->buffer = (unsigned char *)buffer;
	p->length = length;
	p->chunkSize = chunkSize;
	p->head = NULL; // Free List Head
    
	// Set up the free list for free chunks
	PoolFreeAll(p);
}

void* PoolAlloc(Pool *p) {
	// Get latest free node
    PoolFreeNode *node = p->head;
    
	if (node == NULL) {
		assert(0 && "Pool allocator has no free memory");
		return NULL;
	}
    
	// Pop free node
	p->head = p->head->next;
    
	// Zero memory by default
	return memset(node, 0, p->chunkSize);
}

void PoolFree(Pool *p, void *ptr) {
    PoolFreeNode* node;
    
	void *start = p->buffer;
	void *end = &p->buffer[p->length];
    
	if (ptr == NULL) {
		// Ignore NULL pointers
		return;
	}
    
	if (!(start <= ptr && ptr < end)) {
		assert(0 && "Memory is out of bounds of the buffer in this pool");
		return;
	}
    
	// Push free node
	node = (PoolFreeNode *)ptr;
	node->next = p->head;
	p->head = node;
}