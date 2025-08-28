#include "LongLib.h"
#include <windows.h>
#include <gl/gl.h>
#include <math.h>

extern AppDesc currentApp;

static void Win32InitOpenGL(HWND window)
{
    HDC windowDC = GetDC(window);
    
    PIXELFORMATDESCRIPTOR desiredPixelFormat = {
        .nSize = sizeof(desiredPixelFormat),
        .nVersion = 1,
        .iPixelType = PFD_TYPE_RGBA,
        .dwFlags = PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER,
        .cColorBits = 32,
        .cAlphaBits = 8,
        .iLayerType = PFD_MAIN_PLANE
    };
    int suggestedPixelFormatIndex = ChoosePixelFormat(windowDC, &desiredPixelFormat);
    PIXELFORMATDESCRIPTOR suggestedPixelFormat;
    DescribePixelFormat(windowDC, suggestedPixelFormatIndex,
                        sizeof(suggestedPixelFormat), &suggestedPixelFormat);
    SetPixelFormat(windowDC, suggestedPixelFormatIndex, &suggestedPixelFormat);
    
    HGLRC openGLRC = wglCreateContext(windowDC);
    if (wglMakeCurrent(windowDC, openGLRC))
    {
        
    }
    ReleaseDC(window, windowDC);
}

static float ScreenXToGL(float x)
{
    return x / currentApp.windowWidth * 2 - 1;
}

static float ScreenYToGL(float y)
{
    return y / currentApp.windowHeight * 2 - 1;
}

static Vector2 ScreenToGL(Vector2 v)
{
    return (Vector2){ v.x / currentApp.windowWidth * 2. - 1., v.y / currentApp.windowHeight * 2. - 1. };
}

static Vector2 AddVector2(Vector2 a, Vector2 b)
{
    return (Vector2){ a.x + b.x, a.y + b.y };
}

static Vector2 MulVector2(Vector2 a, float b)
{
    return (Vector2){ a.x * b, a.y * b};
}

static void PushVertex(Vector2 vertex)
{
    vertex = ScreenToGL(vertex);
    glVertex2f(vertex.x, vertex.y);
}


void SetBackgroundColor(Color color)
{
    glClearColor(color.r / 255.0, color.g / 255.0, color.b / 255.0, color.a / 255.0);
    glClear(GL_COLOR_BUFFER_BIT);
}

Texture CreateTexture(Sprite sprite)
{
    Texture texture = { .width = sprite.width, .height = sprite.height };
    
    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, sprite.width, sprite.height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, sprite.data);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    return texture;
}

void UnloadTexture(Texture* texture)
{
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &texture->id);
    *texture = (Texture){ 0, 0, 0 };
}

void DrawSprite(Sprite sprite, float x, float y)
{
    Texture texture = CreateTexture(sprite);
    DrawTexture(texture, x, y);
    UnloadTexture(&texture);
}

void DrawTexture(Texture texture, float x, float y)
{
    DrawTextureEx(texture, (Vector2){ x, y }, (Vector2){ 1, 1 }, 0, WHITE);
}

void DrawTextureV(Texture texture, Vector2 pos)
{
    DrawTextureEx(texture, pos, (Vector2){ 1, 1 }, 0, WHITE);
}

void DrawTextureQuad(Texture texture, Vector2 pos, Vector2 scale)
{
    DrawTextureEx(texture, pos, scale, 0, WHITE);
}

void DrawTextureEx(Texture texture, Vector2 pos, Vector2 scale, float rotation, Color tint)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glBegin(GL_QUADS);
    glColor4ub(tint.r, tint.g, tint.b, tint.a);
    
    float x = ScreenXToGL(pos.x);
    float y = ScreenYToGL(pos.y);
    float width = texture.width * scale.x / currentApp.windowWidth * 2;
    float height = texture.height * scale.y / currentApp.windowHeight * 2;
    
    glTexCoord2i(0, 0);
    glVertex2f(x, y);
    
    glTexCoord2i(1, 0);
    glVertex2f(x + width, y);
    
    glTexCoord2i(1, 1);
    glVertex2f(x + width, y + height);
    
    glTexCoord2i(0, 1);
    glVertex2f(x, y + height);
    
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void DrawShape(Shape* shape)
{
    DrawShapeEx(shape, true, WHITE);
}

#define PI 3.141592654

void DrawShapeEx(Shape* shape, bool fill, Color color)
{
    glColor4ub(color.r, color.g, color.b, color.a);
    switch (shape->type)
    {
        case RECTANGLE:
        {
            glBegin(GL_QUADS);
            Vector2 startPos = ScreenToGL(shape->rect.pos);
            Vector2 endPos = ScreenToGL(AddVector2(shape->rect.pos, shape->rect.size));
            glVertex2f(startPos.x, startPos.y);
            glVertex2f(endPos.x, startPos.y);
            glVertex2f(endPos.x, endPos.y);
            glVertex2f(startPos.x, endPos.y);
            glEnd();
        } break;
        case TRIANGLE:
        {
            glBegin(GL_TRIANGLES);
            PushVertex(shape->triangle.a);
            PushVertex(shape->triangle.b);
            PushVertex(shape->triangle.c);
            glEnd();
        } break;
        case LINE:
        {
            glBegin(GL_LINE);
            PushVertex(shape->line.a);
            PushVertex(shape->line.b);
            glEnd();
        } break;
        case CIRCLE:
        {
            static const int MAX_POINT = 180;
            glBegin(GL_LINE_LOOP);
            double delta = 360.0 / (double)MAX_POINT;
            delta *= PI / 180; // Converting to radian
            for (int i = 0; i < MAX_POINT; ++i)
            {
                Vector2 dir = { cos(delta * i), sin(delta * i) };
                Vector2 vertex = AddVector2(MulVector2(dir, shape->circle.radius), shape->circle.pos);
                PushVertex(vertex);
            }
            glEnd();
        } break;
        case POLYGON:
        {
            glBegin(GL_POLYGON);
            for (int i = 0; i < shape->polygon.verticesCount; ++i)
            {
                PushVertex(shape->polygon.vertices[i]);
            }
            glEnd();
        } break;
    }
}