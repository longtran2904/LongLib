/* date = November 21st 2021 2:55 pm */

#ifndef _LONG_RENDERER_H
#define _LONG_RENDERER_H

Sprite drawTarget;
Color defaultColor = { 255, 255, 255, 255 };
Color backgroundColor;

// Shapes
void DrawRect(i32 x, i32 y, u32 width, u32 height);
void DrawRectV(v2i pos, v2u size);
void DrawRectR(RectI rect);
void DrawRectColor(i32 x, i32 y, u32 width, u32 height, Color color);
void DrawRectColorV(v2i pos, v2u size, Color color);
void DrawRectColorR(RectI rect, Color color);
void DrawRectEx(RectI rect, f32 rotation, f32 scale, Color color);

void DrawCircle(i32 x, i32 y, u32 radius);
void DrawCircleV(v2i pos, u32 radius);
void DrawCircleColor(i32 x, i32 y, u32 radius, Color color);
void DrawCircleColorV(v2i pos, u32 radius, Color color);

void DrawTriangle(v2u a, v2u b, v2u c);
void DrawTriangleEx(v2u a, v2u b, v2u c, Color color);

void DrawLine(i32 x1, i32 y1, i32 x2, i32 y2);
void DrawLineV(v2i a, v2i b);
void DrawLineColor(i32 x1, i32 y1, i32 x2, i32 y2, Color color);
void DrawLineColorV(v2i a, v2i b, Color color);

void DrawPixel(Sprite sprite, u32 x, u32 y, Color color);

// Sprites
typedef enum PivotMode PivotMode;
enum PivotMode
{
    BottomLeft,
    BottomRight,
    UpRight,
    UpLeft,
};

Sprite CreateSprite(u32 width, u32 height, Color fillColor);
Sprite LoadSprite(char* fileName);
bool ExportSprite(Sprite sprite, char* fileName);
Sprite GetPartialSprite(Sprite sprite, i32 x, i32 y, u32 width, u32 height);
Sprite GetPartialSpriteV(Sprite sprite, v2i pos, v2u size);
Sprite GetPartialSpriteR(Sprite sprite, RectI rect);
Sprite CloneSprite(Sprite sprite, f32 scale);

void SetSpritePivot(Sprite* sprite, PivotMode mode);

void DrawSprite(Sprite sprite, i32 x, i32 y);
void DrawSpriteV(Sprite sprite, v2i pos);
void DrawPartialSprite(Sprite sprite, i32 x, i32 y, u32 width, u32 height);
void DrawPartialSpriteV(Sprite sprite, v2i pos, v2u size);
void DrawPartialSpriteR(Sprite sprite, RectI rect);
void DrawSpriteEx(Sprite sprite, i32 x, i32 y, f32 rotation, f32 scale, Color tint);

#endif //_LONG_RENDERER_H
