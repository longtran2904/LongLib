#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef struct ArrayHeader
{
    uint32_t size;
    uint32_t capacity;
} ArrayHeader;

#define GetHeader(array) ((ArrayHeader*)((char*)array - sizeof(ArrayHeader)))
#define GetHeaderValue(array) (array ? *GetHeader(array) : (ArrayHeader){0, 0})
#define GetSize(array) (array ? GetHeader(array)->size : 0)
#define GetCapacity(array) (array ? GetHeader(array)->capacity : 0)
#define GetArray(header) ((void*)((char*)header + sizeof(ArrayHeader)))
#define IsFull(array) (GetSize(array) == GetCapacity(array))

static void* ResizeArray(ArrayHeader* header /* Can be NULL */, uint32_t capacity, uint32_t stride)
{
    ArrayHeader* newHeader = (ArrayHeader*)realloc(header, capacity * stride + sizeof(ArrayHeader));
    newHeader->capacity = capacity;
    if (!header) newHeader->size = 0;
    return GetArray(newHeader);
}

static void* CopyArray(void* dest, void* src, uint32_t stride)
{
    uint32_t srcSize = GetSize(src);
    uint32_t offset = 0;
    ArrayHeader* header = NULL;
    uint32_t allocSize = srcSize;
    bool canResize = true;
    if (dest)
    {
        header = GetHeader(dest);
        offset = header->size * stride;
        canResize = header->capacity - header->size < srcSize;
        allocSize += header->size;
    }
    if (canResize) dest = ResizeArray(header, allocSize, stride);
    GetHeader(dest)->size = allocSize;
    memcpy((char*)dest + offset, src, srcSize * stride);
    return dest;
}

#define ReserveArray(array, size) \
    GetCapacity(array) - GetSize(array) < size ? array = ResizeArray(array ? GetHeader(array) : NULL, size + GetSize(array), sizeof(*array)) : 0

#define PushArray(array, item) \
    IsFull(array) ? array = (array ? ResizeArray(GetHeader(array), GetSize(array) * 2, sizeof(*array)) : ResizeArray(NULL, 1, sizeof(*array))) : 0, \
    array[GetHeader(array)->size++] = item

#define PushRange(dest, src) \
    src ? dest = CopyArray(dest, src, sizeof(*dest)) : 0

#define CompressArray(array) \
    !IsFull(array) ? ResizeArray(GetHeader(array), GetSize(array), sizeof(*array)) : 0

#define FreeArray(array) \
    free(GetHeader(array)), array = NULL
