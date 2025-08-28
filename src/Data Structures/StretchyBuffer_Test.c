#include "StretchyBuffer.c"
#include <stdio.h>
#include <assert.h>

#define PRINT_DEBUG 1
#if PRINT_DEBUG
#define LOG(x, y) printf(x, y)
#else
#define LOG(x, y) 0
#endif

#define TestReserve(array, size, testSize, testCapacity) \
    ReserveArray(array, size), LogAndAssert(array, testSize, testCapacity)

#define TestPush(array, item, testSize, testCapacity) \
    PushArray(array, item), LogAndAssert(array, testSize, testCapacity)

#define TestRange(dest, src, size, capacity) \
    PushRange(dest, src), LogAndAssert(dest, size, capacity)

#define LogAllArray(array) \
    for (int i = 0; i < GetHeader(array)->size; ++i) LOG("%d, ", array[i])

void LogAndAssert(void* array, uint32_t size, uint32_t capacity)
{
    LOG("%p\n", array);
    ArrayHeader header = GetHeaderValue(array);
    LOG("%d\n", header.size);
    LOG("%d\n", header.capacity);
    assert(header.size == size);
    assert(header.capacity == capacity);
}

int main(void)
{
    int* array = NULL;

    TestReserve(array, 2, 0, 2);
    int* debug = array;
    assert(debug != NULL);

    TestPush(array, 2, 1, 2);
    assert(array == debug);
    assert(array[0] == 2);

    TestPush(array, 4, 2, 2);
    assert(array == debug);
    assert(array[1] == 4);
    assert(IsFull(array));

    TestPush(array, 3, 3, 4);
    assert(array[2] == 3);

    TestReserve(array, 10, 3, 10);
    assert(!IsFull(array));

    debug = array;
    TestReserve(array, 2, 3, 10);
    assert(array == debug);
    LogAllArray(array);

    int* array1 = NULL;

    PushArray(array1, 5);
    PushArray(array1, 2);
    PushArray(array1, 7);
    LogAllArray(array1);

    printf("Testing\n");
    printf("%p\n", array);
    printf("%d\n", GetHeader(array)->size);
    printf("%p\n", array1);
    printf("%d\n", GetHeader(array1)->size);
    TestRange(array, array1, 6, 10);
    LogAllArray(array);
    assert(array[1] == 4);
    assert(array[4] == 2);

    LOG("%s", "\n");
    TestRange(array1, array, 9, 9);
    assert(array1[0] == 5);
    assert(array1[7] == 2);

    char* array2 = NULL;
    TestPush(array2, 'a', 1, 1);
    assert(array2[0] == 'a');

    bool* array3 = NULL;
    TestReserve(array3, 0, 0, 0);
    TestPush(array3, false, 1, 1);
    assert(array3[0] == false);

    bool* array4 = NULL;
    TestRange(array3, array4, 1, 1);
    assert(array4 == NULL);

    TestRange(array4, array3, 1, 1);

    TestReserve(array, 20, GetSize(array), 20);
    CompressArray(array);
    LOG("\n%d\n", GetHeader(array)->size);
    LOG("%d\n", GetHeader(array)->capacity);
    assert(IsFull(array));

    return 0;
}