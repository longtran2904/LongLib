#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "StretchyBuffer.c"

typedef struct
{
    char* block;
    uint32_t blockUsed;
    char** strings;
} StringsArray;

void Clear(StringsArray* array)
{
    free(array->block);
    free(array->strings);
    *array = (StringsArray){0};
}

void Push(StringsArray *array, char* s)
{
    char* item = array->block + array->blockUsed;
    PushArray(array->strings, item);
    uint32_t length = strlen(s) + 1;
    memcpy(item, s, length);
    array->blockUsed += length;
}

char* Printf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    uint32_t size = vsnprintf(NULL, 0, format, args) + 1;
    char* buffer = malloc(size);
    vsnprintf(buffer, size, format, args);
    buffer[size-1] = 0;
    va_end(args);
    return buffer;
}

void ArrayPrintf(char **buffer, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    
    va_list args2;
    va_copy(args2, args);
    
    int32_t newSize = vsnprintf(NULL, 0, format, args2);
    va_end(args2);
    
    uint32_t currentSize = GetHeaderValue(*buffer).size;
    ReserveArray(*buffer, currentSize + newSize + 1);
    vsnprintf(*buffer + currentSize, newSize + 1, format, args);
    GetHeader(*buffer)->size += newSize;
    
    va_end(args);
}

char* StrDup(const char* text, uint32_t num)
{
    char* buffer = NULL;
    int length = strlen(text);
    ReserveArray(buffer, length *  num + 1);
    for (int i = 0; i < num; ++i)
    {
        memcpy(buffer + length * i, text, length);
    }
    printf("\n");
    buffer[length*num] = 0;
    return buffer;
}