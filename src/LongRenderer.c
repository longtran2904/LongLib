#include "LongRenderer.h"

static bool IsRectValid(RectI rect, Sprite sprite)
{
    if ((rect.width == 0) || (rect.height == 0))
        return false;
    else if ((rect.x + rect.width > drawTarget.width) || (rect.y + rect.height > drawTarget.height))
        return false;
    
    return true;
}

static bool MakeRectValid(RectI* rect, Sprite sprite)
{
    if ((rect->x >= (i32)sprite.width) || (rect->y >= (i32)sprite.height))
        return false;
    if (((rect->x + (i32)rect->width) <= 0) || ((rect->y + (i32)rect->height) <= 0))
        return false;
    if (rect->x + rect->width > sprite.width)
        rect->width = (i32)sprite.width - rect->x;
    if (rect->y + rect->height > sprite.height)
        rect->height = (i32)sprite.height - rect->y;
    if (rect->x < 0)
    {
        rect->width = rect->x + (i32)rect->height;
        rect->x = 0;
    }
    if (rect->y < 0)
    {
        rect->height = rect->y + (i32)rect->height;
        rect->y = 0;
    }
    return true;
}

void DrawPixel(Sprite sprite, u32 x, u32 y, Color color)
{
    if ((x >= sprite.width) && (y >= sprite.height) && sprite.data)
        return;
    sprite.data[x + sprite.width * y] = color;
}

void DrawRect(i32 x, i32 y, u32 width, u32 height)
{
    DrawRectColorR((RectI){ x, y, width, height }, defaultColor);
}

void DrawRectV(v2i pos, v2u size)
{
    DrawRectColorR((RectI){ pos, size }, defaultColor);
}

void DrawRectR(RectI rect)
{
    DrawRectColorR(rect, defaultColor);
}

void DrawRectColor(i32 x, i32 y, u32 width, u32 height, Color color)
{
    DrawRectColorR((RectI){ x, y, width, height }, color);
}

void DrawRectColorV(v2i pos, v2u size, Color color)
{
    DrawRectColorR((RectI){ pos, size }, color);
}

void DrawRectColorR(RectI rect, Color color)
{
    if (!MakeRectValid(&rect, drawTarget))
        return;
    
    for (u32 y = rect.y; y < rect.y + rect.height; ++y)
    {
        for (u32 x = rect.x; x < rect.x + rect.width; ++x)
        {
            DrawPixel(drawTarget, x, y, color);
        }
    }
}

v2 AddV2(v2 a, v2 b)
{
    return (v2){ a.x + b.x, a.y + b.y };
}

v2 SubtractV2(v2 a, v2 b)
{
    return (v2){ a.x - b.x, a.y - b.y };
}

v2 ScaleV2(v2 a, f32 b)
{
    return (v2){ a.x * b, a.y * b };
}

f32 Dot(v2 a, v2 b)
{
    return (a.x * b.x + a.y * b.y);
}

#define Degree2Rad (Pi / (f64)180)
#define Rad2Degree ((f64)180 / PI)

void DrawRectEx(RectI rect, f32 rotation, f32 scale, Color color)
{
#if 0
    if (!MakeRectValid(&rect, drawTarget))
        return;
#endif
    
    rotation = (f32)((f64)rotation * Degree2Rad);
    
    v2 rotatedX = { cosf(rotation), sinf(rotation) };
    v2 rotatedY = { -rotatedX.y, rotatedX.x };
    
    rotatedX = ScaleV2(rotatedX, scale * rect.width);
    rotatedY = ScaleV2(rotatedY, scale * rect.height);
    
    v2 size = { scale * rect.width, scale * rect.height };
    
    v2 bottomLeft = { (f32)rect.x, (f32)rect.y };
    v2 topLeft = AddV2(bottomLeft, rotatedY);
    v2 bottomRight = AddV2(bottomLeft, rotatedX);
    v2 topRight = AddV2(topLeft, rotatedX);
    
    f32 minX = fminf(fminf(bottomLeft.x, bottomRight.x), fminf(topLeft.x, topRight.x));
    f32 minY = fminf(fminf(bottomLeft.y, bottomRight.y), fminf(topLeft.y, topRight.y));
    f32 maxX = fmaxf(fmaxf(bottomLeft.x, bottomRight.x), fmaxf(topLeft.x, topRight.x));
    f32 maxY = fmaxf(fmaxf(bottomLeft.y, bottomRight.y), fmaxf(topLeft.y, topRight.y));
    
#if 1
    RectI bounds = { (i32)minX, (i32)minY, (u32)(maxX - minX), (u32)(maxY - minY) };
    if (!MakeRectValid(&bounds, drawTarget))
        return;
    
    DrawRectColorR(bounds, (Color){ 230,  41,  55, 255 });
    for (u32 y = (u32)bounds.y; y < (u32)bounds.y + bounds.height; ++y)
    {
        for (u32 x = (u32)bounds.x; x < (u32)bounds.x + bounds.width; ++x)
        {
            v2 dir = SubtractV2((v2){ (f32)x, (f32)y }, bottomLeft);
            f32 dirX = Dot(dir, rotatedX);
            if ((dirX < 0) || (dirX > size.x * size.x))
                continue;
            f32 dirY = Dot(dir, rotatedY);
            if ((dirY < 0) || (dirY > size.y * size.y))
                continue;
            DrawPixel(drawTarget, x, y, color);
        }
    }
#endif
    
    Color defColor = defaultColor;
    defaultColor = ORANGE;
    DrawRect((i32)bottomLeft.x, (i32)bottomLeft.y, 5, 5);
    DrawRect((i32)bottomRight.x, (i32)bottomRight.y, 5, 5);
    DrawRect((i32)topLeft.x, (i32)topLeft.y, 5, 5);
    DrawRect((i32)topLeft.x, (i32)topLeft.y, 5, 5);
    DrawRect((i32)topRight.x, (i32)topRight.y, 5, 5);
    defaultColor = defColor;
    
}

void DrawCircle(i32 x, i32 y, u32 radius)
{
    DrawCircleColorV((v2i){ x, y }, radius, defaultColor);
}

void DrawCircleColor(i32 x, i32 y, u32 radius, Color color)
{
    DrawCircleColorV((v2i){ x, y }, radius, color);
}

void DrawCircleV(v2i pos, u32 radius)
{
    DrawCircleColorV(pos, radius, defaultColor);
}

void DrawCircleColorV(v2i pos, u32 radius, Color color)
{
    RectI bounds = { pos.x - radius, pos.y - radius, radius*2, radius*2 };
    if (!MakeRectValid(&bounds, drawTarget))
        return;
    for (u32 y = (u32)bounds.y; y < (u32)bounds.y + bounds.height; ++y)
    {
        for (u32 x = (u32)bounds.x; x < (u32)bounds.x + bounds.width; ++x)
        {
            v2u p = { x - pos.x, y - pos.y };
            if ((p.x*p.x + p.y*p.y) <= (radius * radius))
                DrawPixel(drawTarget, x, y, color);
        }
    }
}

void DrawCircleFast(v2i pos, u32 radius)
{
    u32 x = radius;
    u32 y = 0;
    u32 r2 = radius * radius;
    while (y <= x)
    {
        // Draw 8 pixels
        u32 temp = x;
        
        while (x > 0)
        {
            DrawPixel(drawTarget, pos.x + x, pos.y + y, ORANGE);
            DrawPixel(drawTarget, pos.x + x, pos.y - y, ORANGE);
            DrawPixel(drawTarget, pos.x - x, pos.y + y, ORANGE);
            DrawPixel(drawTarget, pos.x - x, pos.y - y, ORANGE);
            DrawPixel(drawTarget, pos.x + y, pos.y + x, ORANGE);
            DrawPixel(drawTarget, pos.x - y, pos.y + x, ORANGE);
            DrawPixel(drawTarget, pos.x + y, pos.y - x, ORANGE);
            DrawPixel(drawTarget, pos.x - y, pos.y - x, ORANGE);
            
            --x;
        }
        
        x = temp;
        
        if ((x*x + y*y) > r2)
            --x;
        ++y;
    }
    DrawPixel(drawTarget, pos.x, pos.y, ORANGE);
}

void DrawTriangle(v2u a, v2u b, v2u c)
{
    
}

void DrawTriangleEx(v2u a, v2u b, v2u c, Color color)
{
    
}

void Swap(int* a, int* b)
{
    *a += *b;
    *b = *a - *b;
    *a -= *b;
}

void DrawLine(i32 x1, i32 y1, i32 x2, i32 y2)
{
    DrawLineColor(x1, y1, x2, y2, defaultColor);
}

void DrawLineV(v2i a, v2i b)
{
    DrawLineColor(a.x, a.y, b.x, b.y, defaultColor);
}

void DrawLineColor(i32 x1, i32 y1, i32 x2, i32 y2, Color color)
{
    bool flip = y1 > y2;
    if ((x1 > x2) && flip)
    {
        Swap(&x1, &x2);
        Swap(&y1, &y2);
        flip = false;
    }
    else if (x1 > x2)
    {
        Swap(&x1, &x2);
        Swap(&y1, &y2);
        flip = true;
    }
    // calculate dx and dy
    i32 dx = x2 - x1;
    i32 dy = flip ? (y1 - y2) : (y2 - y1);
    
    if(dy <= dx)
    {
        // initial value of decision parameter d
        int d = dy - (dx/2);
        int x = x1, y = y1;
        
        DrawPixel(drawTarget, x, y, color);
        
        // iterate through value of X
        while (x < x2)
        {
            x++;
            
            // East is chosen
            if (d < 0)
                d += dy;
            else // North East is chosen
            {
                d += (dy - dx);
                if (flip)
                    y--;
                else
                    y++;
            }
            
            DrawPixel(drawTarget, x, y, color);
        }
    }
    else
    {
        // initial value of decision parameter d
        int d = dx - (dy/2);
        int x = x1, y = y1;
        
        DrawPixel(drawTarget, x, y, color);
        
        // iterate through value of X
        while (flip ? (y > y2) : (y < y2))
        {
            if (flip)
                y--;
            else
                y++;
            
            // East is chosen
            if (d < 0)
                d = d + dx;
            else // North East is chosen
            {
                d += (dx - dy);
                x++;
            }
            
            DrawPixel(drawTarget, x, y, color);
        }
    }
}

void DrawLineColorV(v2i a, v2i b, Color color)
{
    DrawLineColor(a.x, a.y, b.x, b.y, color);
}
