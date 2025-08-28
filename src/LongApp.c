#include "LongApp.h"
#include "LongRenderer.c"

Sprite drawTarget;
Color backgroundColor;

int16_t* soundSample;
uint32_t sampleSize;

void* Alloc(size_t size);
void Free(void* data);

#define SpriteSize(sprite) sprite.width * sprite.height
#define DataSize(sprite) sprite.width * sprite.height * sizeof(*sprite.data)
#define RowSize(sprite) sprite.width * sizeof(*sprite.data)
#define CheckSprite(sprite) (((sprite).width > 0) && ((sprite).height > 0) && ((sprite).data))

#if 0
Sprite CreateSprite(uint32_t width, uint32_t height, Color fillColor)
{
    Sprite result = { width, height, NULL };
    Color* data = (Color*)Alloc(DataSize(result));
    
    if (data)
    {
        result.data = data;
        for (uint32_t i = 0; i < SpriteSize(result); ++i)
        {
            data[i] = fillColor;
        }
    }
    
    return result;
}

void UnloadSprite(Sprite sprite)
{
    Free(sprite.data);
}

Sprite GetPartialSprite(Sprite sprite, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    if ((width == 0) || (height == 0) || (x >= width) || (y >= height)) return (Sprite){ 0, 0, 0 };
    
    if (sprite.width < x + width) width = sprite.width - x;
    if (sprite.height < y + height) height = sprite.height - y;
    
    Sprite partialSprite = { width, height, NULL };
    Color* data = (Color*)Alloc(DataSize(sprite));
    
    if (data)
    {
        partialSprite.data = data;
        for (uint32_t row = 0; row < height; ++row)
        {
            assert((partialSprite.data + row * width + width) <= (partialSprite.data + width * height));
            assert((sprite.data + (row + y) * sprite.width + x + width) <= (sprite.data + sprite.width * sprite.height));
            
            memcpy(partialSprite.data + row * width, sprite.data + (row + y) * sprite.width + x, width * sizeof(*sprite.data));
        }
    }
    return partialSprite;
}

void DrawSpriteToSprite(Sprite destSprite, uint32_t destX, uint32_t destY, Sprite srcSprite, uint32_t srcX, uint32_t srcY)
{
    uint32_t width = srcSprite.width;
    uint32_t height = srcSprite.height;
    
    if ((destX >= destSprite.width) || (destY >= destSprite.height)) return;
    
    if (destSprite.width < destX + width) width = destSprite.width - destX;
    if (destSprite.height < destY + height) height = destSprite.height - destY;
    
    for (uint32_t row = 0; row < height; ++row)
    {
        assert((destSprite.data + (row + destY) * destSprite.width + destX + width) <= (destSprite.data + destSprite.width * destSprite.height));
        memcpy(destSprite.data + (row + destY) * destSprite.width + destX, srcSprite.data + row * width, width * sizeof(*srcSprite.data));
    }
}

Sprite CloneSprite(Sprite sprite)
{
    Sprite result = { sprite.width, sprite.height, NULL };
    Color* data = (Color*)Alloc(DataSize(result));
    
    if (data)
    {
        result.data = data;
        memcpy(result.data, sprite.data, DataSize(result));
    }
    
    return result;
}

void DrawSprite(Sprite sprite, int x, int y)
{
    DrawSpriteToSprite(drawTarget, x, y, sprite, 0, 0);
}

extern Sprite drawTarget;

void DrawRect(int x, int y, int width, int height)
{
    DrawRectEx(V2(x, y), V2(width, height), WHITE, true);
}

void DrawRectV(Vector2 pos, Vector2 size)
{
    DrawRectEx(pos, size, WHITE, true);
}

void DrawRectEx(Vector2 pos, Vector2 size, Color color, bool fill)
{
    int startX = (int)pos.x, startY = (int)pos.y;
    int endX = (int)(pos.x + size.x), endY = (int)(pos.y + size.y);
    
    if (pos.x >= drawTarget.width || pos.y >= drawTarget.height) return;
    if (pos.x + size.x < 0 || pos.y + size.y < 0) return;
    
    if (pos.x < 0) startX = 0;
    if (pos.y < 0) startX = 0;
    
    if (pos.x + size.x > drawTarget.width) endX = drawTarget.width;
    if (pos.y + size.y > drawTarget.height) endY = drawTarget.height;
    
    if (fill)
    {
        for (int y = startY; y < endY; ++y)
        {
            for (int x = startX; x < endX; ++x)
            {
                drawTarget.data[x + drawTarget.width * y] = color;
            }
        }
    }
    else
    {
        for (int x = startX; x < endX; ++x)
        {
            drawTarget.data[x + drawTarget.width * startY] = color;
            drawTarget.data[x + drawTarget.width * endY] = color;
        }
        for (int y = startY - 1; y < endY - 1; ++y)
        {
            drawTarget.data[startX + drawTarget.width * y] = color;
            drawTarget.data[endX + drawTarget.width * y] = color;
        }
    }
}

void DrawCircle(int x, int y, int radius)
{
    DrawCircleEx((Vector2){ (float)x, (float)y }, radius, WHITE, true);
}

void DrawCircleEx(Vector2 pos, int radius, Color color, bool fill)
{
    if (pos.x > drawTarget.width || pos.y > drawTarget.height) return;
    if (pos.x < 0 || pos.y < 0) return;
    
    // TODO: Check for out of drawTarget's bounds
    
    int x = radius;
    int y = 0;
    int r2 = radius * radius;
    int posX = (int)pos.x;
    int posY = (int)pos.y;
    
    while (y <= x)
    {
        drawTarget.data[posX + x + drawTarget.width * (posY + y)] = color;
        drawTarget.data[posX + x + drawTarget.width * (posY - y)] = color;
        drawTarget.data[posX - x + drawTarget.width * (posY + y)] = color;
        drawTarget.data[posX - x + drawTarget.width * (posY - y)] = color;
        drawTarget.data[posX + y + drawTarget.width * (posY + x)] = color;
        drawTarget.data[posX - y + drawTarget.width * (posY + x)] = color;
        drawTarget.data[posX + y + drawTarget.width * (posY - x)] = color;
        drawTarget.data[posX - y + drawTarget.width * (posY - x)] = color;
        
        if (x*x + y*y > r2)
            x -= 1;
        y += 1;
    }
}

void DrawLine(int startX, int startY, int endX, int endY)
{
    DrawLineEx((Vector2){ (float)startX, (float)startY }, (Vector2){ (float)endX, (float)endY }, WHITE);
}

void DrawLineEx(Vector2 start, Vector2 end, Color color)
{
    int startX = (int)start.x, startY = (int)start.y, endX = (int)end.x, endY = (int)end.y;
    
    if (startX > endX)
    {
        startX = (int)end.x;
        startY = (int)end.y;
        endX = (int)start.x;
        endY = (int)start.y;
    }
    
    if (startY > endY)
    {
        // TODO: Handle the case when startY > endY
    }
    
    // TODO: Check for out of drawTarget's bounding box
    
    int dx = endX - startX, dy = endY - startY;
    bool swap = dx < dy;
    
    int x,y;
    if (swap)
    {
        int p0 = 2*dx - dy;
        
        for (x = startX, y = startY; y < endY; ++y)
        {
            drawTarget.data[x + drawTarget.width * y] = color;
            if (p0 < 0)
            {
                p0 += 2 * dx;
            }
            else
            {
                x++;
                p0 += 2*dx - 2*dy;
            }
        }
    }
    else
    {
        int p0 = 2*dy - dx;
        
        for (x = startX, y = startY; x < endX; ++x)
        {
            drawTarget.data[x + drawTarget.width * y] = color;
            if (p0 < 0)
            {
                p0 += 2 * dy;
            }
            else
            {
                y++;
                p0 += 2*dy-2*dx;
            }
        }
    }
}

void PlaySound(Sound* sound)
{
    if (sound)
    {
        int16_t* dest = soundSample;
        int16_t* src = &(sound->samples[0][sound->playIndex]);
        uint32_t sampleCount = sound->sampleCount - sound->playIndex;
        if (sampleCount > sampleSize)
            sampleCount = sampleSize;
        for (uint32_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex)
        {
            *dest++ = *src;
            *dest++ = *src;
            ++src;
            ++sound->playIndex;
        }
    }
}
#endif