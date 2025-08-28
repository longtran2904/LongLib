/* date = November 20th 2021 3:20 pm */

#ifndef _LONG_TYPES_H
#define _LONG_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <float.h>
#include <stdbool.h>

typedef intptr_t iptr;
typedef uintptr_t uptr;
typedef size_t mptr;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

#define U8Max 255
#define U16Max 65535
#define I32Min ((i32)0x80000000)
#define I32Max ((i32)0x7fffffff)
#define U32Min 0
#define U32Max ((u32)-1)
#define U64Max ((u64)-1)
#define F32Max FLT_MAX
#define F32Min -FLT_MAX

#define OffsetOf(type, member) (uptr)&(((type*)0)->member)

typedef struct Buffer Buffer;
struct Buffer
{
    u8* data;
    uptr count;
};
typedef Buffer string;

typedef union v2 v2;
union v2
{
    struct
    {
        f32 x, y;
    };
    
    struct
    {
        f32 u, v;
    };
    
    struct
    {
        f32 width, height;
    };
};

typedef union v2u v2u;
union v2u
{
    struct
    {
        u32 x, y;
    };
    
    struct
    {
        u32 width, height;
    };
};

typedef union v2i v2i;
union v2i
{
    struct
    {
        i32 x, y;
    };
    
    struct
    {
        i32 width, height;
    };
};

typedef union v3 v3;
union v3
{
    struct
    {
        f32 x, y, z;
    };
    
    struct
    {
        f32 r, g, b;
    };
};

typedef union v4 v4;
union v4
{
    struct
    {
        union
        {
            v3 xyz;
            struct
            {
                f32 x, y, z;
            };
        };
        
        f32 w;
    };
    
    struct
    {
        union
        {
            v3 rgb;
            struct
            {
                f32 r, g, b;
            };
        };
        
        f32 a;
    };
};

typedef struct m4x4 m4x4;
struct m4x4
{
    v4 rows[4];
};

typedef union Rect Rect;
union Rect
{
    struct
    {
        v2 pos;
        v2 size;
    };
    struct
    {
        f32 x, y;
        f32 width, height;
    };
};

typedef union RectI RectI;
union RectI
{
    struct
    {
        v2i pos;
        v2u size;
    };
    struct
    {
        i32 x, y;
        u32 width, height;
    };
};

bool RectContain(Rect rect, v2 pos)
{
    bool inRangeX = (rect.pos.x < pos.x) && (rect.size.width > ((i32)pos.x + rect.pos.x));
    bool inRangeY = (rect.pos.y < pos.y) && (rect.size.width > ((i32)pos.y + rect.pos.y));
    return inRangeX && inRangeY;
}

bool RectIContain(RectI rect, v2u pos)
{
    bool inRangeX = (rect.pos.x < (int)pos.x) && (rect.size.width - pos.x> pos.x);
    bool inRangeY = (rect.pos.y < (int)pos.y) && (rect.size.width - pos.y> pos.y);
    return inRangeX && inRangeY;
}

#define KB(byte) ((byte)*1024LL)
#define MB(byte) (KB(byte)*1024LL)
#define GB(byte) (MB(byte)*1024LL)
#define TB(byte) (GB(byte)*1024LL)

#define ArrayCount(array) (sizeof(array) / sizeof((array)[0]))

#endif //_LONG_TYPES_H
