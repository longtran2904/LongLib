#include <stdbool.h>
#include "../Data Structures/StretchyBuffer.c"

struct Iterator
{
    void* current;
    bool (*Next)(void);
    void (*Reset)(void);
};

bool Next(void* array, void* current, uint32_t stride)
{
    if (current >= (char*)array + GetSize(array) * stride)
    {
        return false;
    }
    current = (void*)((char*)current + stride);
    return true;
}

#define GetIterator(array) (Iterator){ array,  }

void Test(void)
{
}