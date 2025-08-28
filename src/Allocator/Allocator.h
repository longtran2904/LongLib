/* date = August 22nd 2021 9:08 pm */

#ifndef _ALLOCATOR_H
#define _ALLOCATOR_H

#ifndef DEFAULT_ALIGNMENT
#define DEFAULT_ALIGNMENT (2*sizeof(void*))
#endif

typedef struct Allocator
{
    void* (*Alloc)(struct Allocator* allocator, size_t length);
    void (*Free)(struct Allocator* allocator, void* ptr);
    void (*FreeAll)(struct Allocator* allocator);
    void* (*Resize)(struct Allocator* allocator);
} Allocator;

#endif //_ALLOCATOR_H
