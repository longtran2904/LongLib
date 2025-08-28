#include "LongLib.h"
#include <windows.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <gl/gl.h>
#include <assert.h>
#include <Xinput.h>
#include "LongLib_OpenGL.c"

#include "Allocator/StackAllocator.c"
#include "Allocator/PoolAllocator.c"

#ifndef USE_OPENGL
#define USE_OPENGL 1
#endif

#ifndef ALLOW_OFFSCREEN
#define ALLOW_OFFSCREEN 1
#endif

typedef struct
{
    int width;
    int height;
} WindowDimension;

static AppDesc currentApp;
static HWND currentWindow;
static Color backgroundColor = {255, 255, 255, 255};
static uint32_t vkCode;
static bool isDown;
static bool wasDown;
static Font defaultFont;

static float fps;
static float deltaTime;
static double time;
static int64_t perfCountFrequency;
static LARGE_INTEGER lastCounter;
static int64_t lastCycleCount;

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}
static x_input_get_state* XInputGetStateFunc = XInputGetStateStub;
#define XInputGetState XInputGetStateFunc

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}
static x_input_set_state* XInputSetStateFunc = XInputSetStateStub;
#define XInputSetState XInputSetStateFunc

static void LoadXInput(void)
{
    HMODULE XInputLibrary = LoadLibrary("xinput1_4.dll");
    if (!XInputLibrary) XInputLibrary = LoadLibrary("xinput1_3.dll");
    if (XInputLibrary)
    {
        XInputGetState = (x_input_get_state*)GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (x_input_set_state*)GetProcAddress(XInputLibrary, "XInputSetState");
    }
}

void Win32InitOpenGL(HWND window);

// --------------------TODO--------------------
// Monitor's info functions
// Cursor handling
// Input functions for multiple keys
// Batch rendering
// Load PNG, JPEG sprites
// Work on Aspect ratio
// Rework on closing event
// Clean up the code (maybe finish up the not_use_opengl code?)
// Put every static variable to a Core variable
// Support vsync
// Draw and load text
// DPI-aware
// Window size vs render size (aspect ratio handling)

// --------------------WINDOW-RELATED FUNCTIONS--------------------
WindowDimension GetWindowDimension(HWND window)
{
    RECT clientRect;
    GetClientRect(window, &clientRect);
    return (WindowDimension){ clientRect.right - clientRect.left, 
        clientRect.bottom - clientRect.top };
}

LRESULT CALLBACK MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    
    switch (message)
    {
        case WM_SIZE:
        {
            WindowDimension dimension = GetWindowDimension(window);
            currentApp.windowWidth = dimension.width;
            currentApp.windowHeight = dimension.height;
        } break;
        case WM_DESTROY:
        {
            ExitWindow();
        } break;
        case WM_CLOSE:
        {
            ExitWindow();
        } break;
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            assert(!"Keyboard is handle in game loop");
        } break;
        case WM_PAINT:
        {
            PAINTSTRUCT paint;
            HDC deviceContext = BeginPaint(window, &paint);            
            WindowDimension dimension = GetWindowDimension(window);
            currentApp.windowWidth = dimension.width;
            currentApp.windowHeight = dimension.height;
#if !USE_OPENGL
            DisplayBuffer(deviceContext, &backBuffer, 0, 0, dimension.width, dimension.height);
#endif
            EndPaint(window, &paint);
        } break;
        default:
        {
            result = DefWindowProc(window, message, wParam, lParam);
        } break;
    }
    
    return result;
}

static void TranslateAndDispatchMessage()
{
    wasDown = isDown;
    MSG message;
    while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
    {
        switch (message.message)
        {
            case WM_QUIT:
            {
                ExitWindow();
            } break;
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                vkCode = message.wParam;
                wasDown = ((message.lParam & (1 << 30)) != 0);
                isDown = ((message.lParam & (1 << 31)) == 0);
            } break;
            default:
            {
                TranslateMessage(&message);
                DispatchMessage(&message);
            } break;
        }
    }
}

static void HandleControllerInput()
{
    for (DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; ++controllerIndex)
    {
        XINPUT_STATE controllerState;
        if (XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS)
        {
            XINPUT_GAMEPAD* pad = &controllerState.Gamepad;
            bool up = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
            bool down = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
            bool left = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
            bool right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
            bool start = (pad->wButtons & XINPUT_GAMEPAD_START);
            bool back = (pad->wButtons & XINPUT_GAMEPAD_BACK);
            bool leftShoulder = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
            bool rightShoulder = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
            bool aButton = (pad->wButtons & XINPUT_GAMEPAD_A);
            bool bButton = (pad->wButtons & XINPUT_GAMEPAD_B);
            bool xButton = (pad->wButtons & XINPUT_GAMEPAD_X);
            bool yButton = (pad->wButtons & XINPUT_GAMEPAD_Y);
            
            int16_t stickX = pad->sThumbLX;
            int16_t stickY = pad->sThumbLY;
        }
    }
    
    XINPUT_VIBRATION vibration = { .wLeftMotorSpeed = 60000, .wRightMotorSpeed = 60000 };
    XInputSetState(0, &vibration);
}

bool UpdateFrame(void)
{
    // TODO: Draw batch buffer here
    
    HDC deviceContext = GetDC(currentWindow);
    
    glViewport(0, 0, currentApp.windowWidth, currentApp.windowHeight);
    SwapBuffers(deviceContext);
    
    glClearColor(backgroundColor.r / 255.0, backgroundColor.g / 255.0, backgroundColor.b / 255.0, backgroundColor.a / 255.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    ReleaseDC(currentWindow, deviceContext);
    
    TranslateAndDispatchMessage();
    HandleControllerInput();
    
    int64_t endCycleCount = __rdtsc();
    LARGE_INTEGER endCounter;
    QueryPerformanceCounter(&endCounter);
    
    int64_t cycleElapsed = endCycleCount - lastCycleCount;
    int64_t counterElapsed = endCounter.QuadPart - lastCounter.QuadPart;
    deltaTime = (double)counterElapsed / (double)perfCountFrequency;
    time += deltaTime;
    fps = (double)perfCountFrequency / (double)counterElapsed;
    float mcpf = ((double)cycleElapsed / 1000000.0);
    
    printf("%fs/f / %fFPS / %fMC/F / %lfs\n", deltaTime, fps, mcpf, time);
    lastCounter = endCounter;
    lastCycleCount = endCycleCount;
    
    // TODO: Run at a fixed framerate
    // TODO: implement vsync
    
    return currentApp.isRunning;
}

void OpenWindow(AppDesc* app)
{
    LoadXInput();
    
    LARGE_INTEGER perfCountFrequencyResult;
    QueryPerformanceFrequency(&perfCountFrequencyResult);
    perfCountFrequency = perfCountFrequencyResult.QuadPart;
    QueryPerformanceCounter(&lastCounter);
    lastCycleCount = __rdtsc();
    
    WNDCLASS windowClass = {
        .style = CS_HREDRAW|CS_VREDRAW,
        .lpfnWndProc = MainWindowCallback,
        .hInstance = NULL,
        .lpszClassName = "LongLibClass"
    };
    
    if (RegisterClass(&windowClass))
    {
        RECT rect;
        rect.left = 0;
        rect.right = app->windowWidth ? app->windowWidth : CW_USEDEFAULT;
        rect.top = 0;
        rect.bottom = app->windowHeight ? app->windowHeight : CW_USEDEFAULT;
        printf("%d\n", AdjustWindowRect(&rect, app->windowFlags ? app->windowFlags : WS_OVERLAPPEDWINDOW|WS_VISIBLE, 0));
        printf("App Size: (%u, %u)\n", app->windowWidth, app->windowHeight);
        printf("(%d, %d)\n", rect.right, rect.bottom);
        
        HWND window = CreateWindowEx(
                                     0,
                                     windowClass.lpszClassName,
                                     app->windowTitle,
                                     app->windowFlags ? app->windowFlags : WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                     CW_USEDEFAULT,
                                     CW_USEDEFAULT,
                                     rect.right - rect.left,
                                     rect.bottom - rect.top,
                                     0,
                                     0,
                                     NULL,
                                     0);
        printf("Rect: (%d, %d, %d, %d)\n", rect.left, rect.top, rect.right, rect.bottom);
        
        if (window)
        {
            WindowDimension dimension = GetWindowDimension(window);
            printf("(%d, %d)\n", dimension.width, dimension.height);
            app->isRunning = true;
            if (!app->windowWidth) app->windowWidth = dimension.width;
            if (!app->windowHeight) app->windowHeight = dimension.height;
            currentApp = *app;
            currentWindow = window;
            Win32InitOpenGL(window);
        }
    }
}

void ExitWindow(void)
{
    currentApp.isRunning = false;
}

bool IsFullScreen(void)
{
    DWORD style = GetWindowLong(currentWindow, GWL_STYLE);
    return !(style & WS_OVERLAPPEDWINDOW);
}

void ToggleFullScreen(void)
{
    static WINDOWPLACEMENT windowPos = { sizeof(windowPos) };
    DWORD style = GetWindowLong(currentWindow, GWL_STYLE);
    if (style & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO monitorInfo = { sizeof(monitorInfo) };
        if (GetWindowPlacement(currentWindow, &windowPos) &&
            GetMonitorInfo(MonitorFromWindow(currentWindow, MONITOR_DEFAULTTOPRIMARY), &monitorInfo))
        {
            SetWindowLong(currentWindow, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(currentWindow, HWND_TOP,
                         monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
                         monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                         monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLong(currentWindow, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(currentWindow, &windowPos);
        SetWindowPos(currentWindow, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

uint32_t GetWindowWidth(void)
{
    return currentApp.windowWidth;
}

uint32_t GetWindowHeight(void)
{
    return currentApp.windowHeight;
}

// --------------------INPUT-RELATED FUNCTIONS--------------------
bool IsKeyPressed(int key)
{
    return (vkCode == key) && isDown && wasDown;
}

bool IsKeyDown(int key)
{
    return (vkCode == key) && isDown && !wasDown;
}

bool IsKeyUp(int key)
{
    return (vkCode == key) && !isDown && wasDown;
}

int GetKeyPressed(void);
bool IsMouseButtonPressed(int button);
bool IsMouseButtonDown(int button);
bool IsMouseButtonUp(int button);
Vector2 GetMousePos(void);

// --------------------TIMING FUNCTIONS--------------------
float GetFPS(void)
{
    return fps;
}

double GetTime(void)
{
    return time;
}

float GetDeltaTime(void)
{
    return deltaTime;
}

// --------------------SPRITE FUNCTIONS--------------------
#pragma pack(push, 1)
typedef struct BmpHeader
{
    // File Header
    uint16_t fileType;
    uint32_t fileSize;
    uint16_t reserve1;
    uint16_t reserve2;
    uint32_t bitmapOffset;
    
    // DIB Header
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bitsPerPixel;
} BmpHeader;
#pragma pack(pop)

static void SwitchRedAndBlue(Sprite sprite)
{
    Color* pixel = sprite.data;
    for (int32_t y = 0; y < sprite.height; ++y)
    {
        for (int32_t x = 0; x < sprite.width; ++x)
        {
            Color temp = *pixel;
            pixel->b = temp.r;
            pixel->r = temp.b;
            pixel++;
        }
    }
}

Sprite CreateSprite(uint32_t width, uint32_t height, Color backgroundColor)
{
    Color* data = VirtualAlloc(0, width * height * sizeof(Color), MEM_COMMIT, PAGE_READWRITE);
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            data[x + y * width] = backgroundColor;
        }
    }
    return (Sprite){ width, height, data };
}

static Color* ConvertToRBGA(void* data, uint32_t size)
{
    Color* newData = VirtualAlloc(0, size * sizeof(Color), MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    Color* pixel = (Color*)data;
    for (int i = 0; i < size; ++i)
    {
        newData[i] = (Color){ pixel->r, pixel->g, pixel->b, 255 };
        pixel = (Color*)((char*)pixel + 3);
    }
    return newData;
}

// NOTE: this function can only load bmp file for now, and the bmp file need to be 32bit without compression
Sprite LoadSprite(const char* fileName)
{
    uint32_t fileSize = 0;
    uint8_t* readResult = LoadFileData(fileName, &fileSize);
    if (readResult)
    {
        if (IsFileExtension(fileName, "bmp"))
        {
            BmpHeader* header = (BmpHeader*)readResult;
            uint32_t size = header->width * header->height * sizeof(Color);
            Color* pixels = VirtualAlloc(0, size, MEM_COMMIT, PAGE_READWRITE);
            if (header->bitsPerPixel == 32)
            {
                memcpy(pixels, readResult + header->bitmapOffset, size);
            }
            if (header->bitsPerPixel == 24)
            {
                pixels = ConvertToRBGA(readResult + header->bitmapOffset, size / sizeof(Color));
            }
            printf("Pixel: %p\n", pixels);
            Sprite sprite = { header->width, header->height, pixels };
            SwitchRedAndBlue(sprite);
            UnloadFileData(readResult);
            return sprite;
        }
    }
    
    return (Sprite){0};
}

bool ExportSprite(Sprite sprite, const char* fileName)
{
    bool result = false;
    if (IsFileExtension(fileName, "bmp"))
    {
        uint32_t size = 40; // DIB's type is BITMAPINFOHEADER
        uint32_t bitmapOffset = size + 14; // bitmapOffset == size + sizeof(bitmap file header), bitmap file header always == 14 
        uint32_t fileSize = bitmapOffset + sprite.width * sprite.height * sizeof(*sprite.data);
        BmpHeader* header = VirtualAlloc(0, fileSize, MEM_COMMIT, PAGE_READWRITE);
        if (header)
        {
            *header = (BmpHeader){
                .fileType = (((uint16_t)'M') << 8) | (0x00ff & 'B'), // Convert "BM" to a uint16
                .fileSize = fileSize,
                .bitmapOffset = bitmapOffset,
                .size = size,
                .width = sprite.width,
                .height = sprite.height,
                .planes = 1,
                .bitsPerPixel = sizeof(Color) * 8
            };
            void* dest = (char*)header + bitmapOffset;
            memcpy(dest, sprite.data, fileSize - bitmapOffset);
            SwitchRedAndBlue((Sprite){ header->width, header->height, dest });
            result = SaveFileData(fileName, header, fileSize);
            result &= VirtualFree(header, 0, MEM_RELEASE);
        }
    }
    return result;
}

void UnloadSprite(Sprite* sprite)
{
    sprite->width = 0;
    sprite->height = 0;
    VirtualFree(sprite->data, 0, MEM_RELEASE);
    sprite->data = NULL;
}

Sprite GetPartialSprite(uint32_t x, uint32_t y, uint32_t width, uint32_t height, Sprite sprite)
{
    if (!width || !height) return (Sprite){ 0, 0, 0 };
#if ALLOW_OFFSCREEN
    assert(x < sprite.width);
    assert(y < sprite.height);
    if (sprite.width < x + width) width = sprite.width - x;
    if (sprite.height < y + height) height = sprite.height - y;
#else
    assert(sprite.width >= x + width);
    assert(sprite.height >= y + height);
#endif
    Sprite partialSprite = { width, height, VirtualAlloc(0, width * height * sizeof(*sprite.data), MEM_COMMIT, PAGE_READWRITE) };
    for (uint32_t row = 0; row < height; ++row)
    {
        assert((partialSprite.data + row * width + width) <= (partialSprite.data + width * height));
        assert((sprite.data + (row + y) * sprite.width + x + width) <= (sprite.data + sprite.width * sprite.height));
        memcpy(partialSprite.data + row * width, sprite.data + (row + y) * sprite.width + x, width * sizeof(*sprite.data));
    }
    return partialSprite;
}

void DrawSpriteToSprite(uint32_t x, uint32_t y, Sprite destSprite, Sprite srcSprite)
{
    uint32_t width = srcSprite.width;
    uint32_t height = srcSprite.height;
#if ALLOW_OFFSCREEN
    assert(x < destSprite.width);
    assert(y < destSprite.height);
    if (destSprite.width < x + width) width = destSprite.width - x;
    if (destSprite.height < y + height) height = destSprite.height - y;
#else
    assert(destSprite.width >= x + srcSprite.width);
    assert(destSprite.height >= y + srcSprite.height);
#endif
    for (uint32_t row = 0; row < height; ++row)
    {
        assert((destSprite.data + (row + y) * destSprite.width + x + width) <= (destSprite.data + destSprite.width * destSprite.height));
        memcpy(destSprite.data + (row + y) * destSprite.width + x, srcSprite.data + row * width, width * sizeof(*srcSprite.data));
    }
}

// --------------------FONT LOADING FUNCTIONS--------------------
Font LoadFont(const char* fileName)
{
    //Load font: AddFontResourceEx + CreateFont
    //Create Bitmap & HDC - CreateDIBSection, CreateCompatibleDC, SelectObject.
    //Get font metrics: GetTextMetrics.
    //Get codepoint size: GetTextExtentPoint32
    // Assign ID
    // Assign charsCount
    // Load all the bitmaps
    // Assign charsInfo
    Font result = {0};
    
    if (AddFontResourceEx(fileName, FR_PRIVATE, 0))
    {
        HFONT font = CreateFont(0, 0, 0, 0,
                                FW_NORMAL, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                ANTIALIASED_QUALITY,
                                DEFAULT_PITCH|FF_DONTCARE,
                                "");
        if (font)
        {
            HDC deviceContext = CreateCompatibleDC(GetDC(0));
            SelectObject(deviceContext, font);
            
            TEXTMETRIC textMetric;
            GetTextMetrics(deviceContext, &textMetric);
            
            printf("Height: %d\n", textMetric.tmHeight);
            printf("Ascent: %d\n", textMetric.tmAscent);
            printf("Descent: %d\n", textMetric.tmDescent);
            result.lineHeight = textMetric.tmHeight;
            
            int count = GetFontUnicodeRanges(deviceContext, 0);
            GLYPHSET* glyphSet = VirtualAlloc(0, count, MEM_COMMIT, PAGE_READWRITE);
            printf("Bytes write to glyphs: %u\n", GetFontUnicodeRanges(deviceContext, glyphSet));
            printf("Glyphs Supported: %d\n", glyphSet->cGlyphsSupported);
            /*for (int i = 0; i < glyphSet->cRanges; ++i)
            {
            printf("Unicode range #%d:, %u-%d\n", i, glyphSet->ranges[i].wcLow, glyphSet->ranges[i].cGlyphs);
            }*/
            
            uint32_t maxWidth = 0;
            for (char i = '!'; i < '~'; ++i)
            {
                wchar_t cheesePoint = (wchar_t)i;
                
                SIZE size;
                GetTextExtentPoint32W(deviceContext, &cheesePoint, 1, &size);
                printf("Character %c: (%d, %d)\n", i, size.cx, size.cy);
                maxWidth += size.cx;
                //result.charsInfo[i - '!'] = { .position = (Vector2){ maxWidth, textMetric.tmHeight }, .size = (Vector2){ size.cx, size.cy } };
            }
            
            printf("Max Size: (%u, %u)\n", maxWidth, textMetric.tmHeight);
            
            void* bits;
            BITMAPINFO info = {
                .bmiHeader = {
                    .biSize = sizeof(info.bmiHeader),
                    .biWidth = maxWidth,
                    .biHeight = textMetric.tmHeight,
                    .biPlanes = 1,
                    .biBitCount = 32,
                    .biCompression = BI_RGB
                }
            };
            HBITMAP bitmap = CreateDIBSection(deviceContext, &info, DIB_RGB_COLORS, &bits, 0, 0);
            SelectObject(deviceContext, bitmap);
            SetBkColor(deviceContext, RGB(0, 0, 0));
            
            SetTextColor(deviceContext, RGB(255, 255, 255));
            TextOut(deviceContext, 0, 0, "ABCDEFG", 7);
            result.bitmap = CreateTexture((Sprite){ maxWidth, textMetric.tmHeight, bits });
            printf("Bitmap: (%u, %u)\n", result.bitmap.width, result.bitmap.height);
        }
    }
    
    return result;
}

void SetDefaultFont(Font font)
{
    defaultFont = font;
}

Font GetDefaultFont(void)
{
    return defaultFont;
}

// --------------------TEXT DRAWING FUNCTIONS--------------------
static Sprite GetCharTexture(const char* fileName, const char* fontName, uint32_t codePoint)
{
    Sprite texture;
    
    int maxWidth = 1024;
    int maxHeight = 1024;
    void* bits = 0;
    AddFontResourceEx(fileName, FR_PRIVATE, 0);
    HFONT font = CreateFont(128, 0, 0, 0,
                            FW_NORMAL, // Weight
                            FALSE, // Italic
                            FALSE, // Underline
                            FALSE, // Strikeout
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            ANTIALIASED_QUALITY,
                            DEFAULT_PITCH|FF_DONTCARE,
                            fontName);
    
    HDC deviceContext = CreateCompatibleDC(GetDC(0));
    
    BITMAPINFO info = {
        .bmiHeader = {
            .biSize = sizeof(info.bmiHeader),
            .biWidth = maxWidth,
            .biHeight = maxHeight,
            .biPlanes = 1,
            .biBitCount = 32,
            .biCompression = BI_RGB
        }
    };
    HBITMAP bitmap = CreateDIBSection(deviceContext, &info, DIB_RGB_COLORS, &bits, 0, 0);
    SelectObject(deviceContext, bitmap);
    SelectObject(deviceContext, font);
    SetBkColor(deviceContext, RGB(0, 0, 0));
    
    TEXTMETRIC textMetric;
    GetTextMetrics(deviceContext, &textMetric);
    
    wchar_t cheesePoint = (wchar_t)codePoint;
    
    SIZE size;
    GetTextExtentPoint32W(deviceContext, &cheesePoint, 1, &size);
    
    int width = size.cx;
    int height = size.cy;
    printf("Size: (%d, %d)\n", width, height);
    
    SetTextColor(deviceContext, RGB(255, 255, 255));
    TextOutW(deviceContext, 0, 0, &cheesePoint, 1);
    
    int minX =  10000;
    int minY =  10000;
    int maxX = -10000;
    int maxY = -10000;
    
    uint32_t* row = (uint32_t*)bits + (maxHeight - 1) * maxWidth;
    for (int y = 0; y < height; ++y)
    {
        uint32_t* pixel = row;
        for (int x = 0; x < width; ++x)
        {
            if (*pixel != 0)
            {
                if (minX > x)
                {
                    minX = x;
                }
                if (minY > y)
                {
                    minY = y;
                }
                if (maxX < x)
                {
                    maxX = x;
                }
                if (maxY < y)
                {
                    maxY = y;
                }
            }
            ++pixel;
        }
        row -= maxWidth;
    }
    
    
    if (minX <= maxX)
    {
#if 0
        --minX;
        --minY;
        ++maxX;
        ++maxY;
#endif
        
        width = maxX - minX + 1;
        height = maxY - minY + 1;
        
        texture = (Sprite){ width, height, VirtualAlloc(0, width * height * sizeof(*texture.data), MEM_COMMIT, PAGE_READWRITE) };
        uint8_t* destRow = (uint8_t*)texture.data + (height - 1)*texture.width*sizeof(*texture.data);
        uint32_t* srcRow = (uint32_t*)bits + (maxHeight - 1 - minY) * maxWidth;
        
        for (int y = minY; y <= maxY; ++y)
        {
            uint32_t* source = (uint32_t*)srcRow + minX;
            uint32_t* dest = (uint32_t*)destRow;
            for (int x = minX; x <= maxX; ++x)
            {
                uint32_t pixel = *source;
                uint8_t gray = (uint8_t)(pixel & 0xFF);;
                *dest++ = ((gray << 24) | (gray << 16) | (gray <<  8) | (gray <<  0));
                ++source;
            }
            destRow -= texture.width*sizeof(*texture.data);
            srcRow -= maxWidth;
        }
    }
    return texture;
}

static Sprite* LoadAllTextBitmap(const char* fileName, const char* fontName)
{
    static Sprite allBitmaps['~' - '!' + 1];
    for (int8_t c = '!'; c < '~' + 1; ++c)
    {
        allBitmaps[c - '!'] = GetCharTexture(fileName, fontName, c);
    }
    return allBitmaps;
}

// --------------------FILES MANAGEMENT FUNCTIONS--------------------
// NOTE: These functions can only load files that are less than 4GB, if the file has exactly 4GB, it will also fails
uint8_t* LoadFileData(const char* fileName, uint32_t* bytesRead)
{
    void* result = 0;
    HANDLE fileHandle = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    printf("Is Handle valid: %s\n", fileHandle != INVALID_HANDLE_VALUE ? "true" : "false");
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER fileSize;
        if (GetFileSizeEx(fileHandle, &fileSize))
        {
            assert(fileSize.QuadPart <= UINT32_MAX); // less than 4GB
            result = VirtualAlloc(0, fileSize.QuadPart, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            printf("Result: %p\n", result);
            if (result)
            {
                uint32_t fileSize32 = (uint32_t)fileSize.QuadPart;
                if (ReadFile(fileHandle, result, fileSize32, bytesRead, 0))
                {
                    assert(fileSize32 == *bytesRead);
                }
                else
                {
                    UnloadFileData(result);
                }
            }
        }
        CloseHandle(fileHandle);
    }
    return result;
}

void UnloadFileData(uint8_t* data)
{
    if (data)
    {
        VirtualFree(data, 0, MEM_RELEASE);
    }
}

bool SaveFileData(const char* fileName, void* data, uint32_t bytesToWrite)
{
    bool result = false;
    HANDLE fileHandle = CreateFile(fileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD bytesRead;
        BOOL canWrite = WriteFile(fileHandle, data, bytesToWrite, &bytesRead, 0);
        if (canWrite)
        {
            result = bytesToWrite == bytesRead;
        }
        CloseHandle(fileHandle);
    }
    return result;
}

char* LoadFileText(const char* fileName)
{
    char* result = 0;
    HANDLE fileHandle = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER fileSize;
        if (GetFileSizeEx(fileHandle, &fileSize))
        {
            assert(fileSize.QuadPart + 1 <= UINT32_MAX); // less than 4GB
            result = VirtualAlloc(0, fileSize.QuadPart + 1, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if (result)
            {
                uint32_t fileSize32 = (uint32_t)(fileSize.QuadPart + 1);
                DWORD bytesRead = 0;
                if (ReadFile(fileHandle, result, fileSize32, &bytesRead, 0))
                {
                    assert(fileSize32 == bytesRead + 1);
                    // WARNING: I heard that \r\n is converted to \n on reading, so read bytes count gets reduced by the number of lines, but I didn't know if it is still true.
                    //if (bytesRead < fileSize32) result = ReallocFunction(result, bytesRead + 1);
                    result[bytesRead] = '\0';
                }
                else
                {
                    UnloadFileText(result);
                }
            }
        }
        CloseHandle(fileHandle);
    }
    return result;
}

void UnloadFileText(char* text)
{
    if (text)
    {
        VirtualFree(text, 0, MEM_RELEASE);
    }
}

bool SaveFileText(const char* fileName, char* text)
{
    bool result = false;
    HANDLE fileHandle = CreateFile(fileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    int textLength = lstrlen(text);
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD bytesRead;
        if (WriteFile(fileHandle, text, textLength, &bytesRead, 0))
        {
            printf("Text length: %d\n", textLength);
            printf("Bytes Read: %lu\n", bytesRead);
            result = textLength == bytesRead;
        }
        CloseHandle(fileHandle);
    }
    return result;
}

bool FileExists(const char* fileName)
{
    return GetFileAttributes(fileName) != INVALID_FILE_ATTRIBUTES;
}

bool IsFileExtension(const char* fileName, const char* ext)
{
    const char* fileExt = GetFileExtension(fileName);
    return strcmp(fileExt, ext) == 0; 
}

const char* GetFileExtension(const char* fileName)
{
    const char* dot = strchr(fileName, '.');
    if (!dot || dot == fileName) return NULL;
    return (dot + 1);
}

// String pointer reverse break: returns right-most occurrence of charset in s
static const char *strprbrk(const char *s, const char *charset)
{
    const char *latestMatch = NULL;
    for (; s = strpbrk(s, charset), s != NULL; latestMatch = s++) { }
    return latestMatch;
}

const char* GetFileName(const char* filePath)
{
    const char* fileName = NULL;
    if (filePath != NULL) fileName = strprbrk(filePath, "\\/");
    printf("File Name: %s\n", fileName);
    if (!fileName || (fileName == filePath)) return filePath;
    return fileName + 1;
}

const char* GetFileNameWithoutExt(const char* filePath)
{
#define MAX_FILENAMEWITHOUTEXT_LENGTH 128
    
    static char fileName[MAX_FILENAMEWITHOUTEXT_LENGTH];
    memset(fileName, 0, MAX_FILENAMEWITHOUTEXT_LENGTH);
    
    if (filePath != NULL) strcpy(fileName, GetFileName(filePath)); // Get filename with extension
    
    int len = strlen(fileName);
    
    for (int i = 0; (i < len) && (i < MAX_FILENAMEWITHOUTEXT_LENGTH); i++)
    {
        if (fileName[i] == '.')
        {
            // NOTE: We break on first '.' found
            fileName[i] = '\0';
            break;
        }
    }
    
    return fileName;
}

//INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
int main(void)
{
    uint32_t windowWidth = 1280;
    uint32_t windowHeight = 720;
    
    OpenWindow(&(AppDesc){
                   .windowTitle = "LongLib",
                   .windowWidth = windowWidth,
                   .windowHeight = windowHeight
               });
    
    const char* currentDir = "D:/Documents/Projects/LongLib/src";
    const char* spriteName = "D:/Documents/Projects/LongLib/src/Data/sample_640×426.bmp";
    const char* sprite32Name = "D:/Documents/Projects/LongLib/src/Data/spaceman.bmp";
    Sprite sprite = LoadSprite(spriteName);
    printf("Sprite 24bit: (%u, %u, %p)\n", sprite.width, sprite.height, sprite.data);
    
    //Font arial = LoadFont("C:/Windows/Fonts/arial.ttf");
    //Font cour = LoadFont("C:/Windows/Fonts/cour.ttf");
    Font comic = LoadFont("C:/Windows/Fonts/comic.ttf");
    
    Sprite sprite32 = LoadSprite(sprite32Name);
    for (int i = 0; i < sprite32.width * sprite32.height; ++i)
        if (sprite32.data[i].r == 0 && sprite32.data[i].g == 0 && sprite32.data[i].b == 0)
            sprite32.data[i].a = 0;
    
    Texture texture = CreateTexture(sprite32);
    //Sprite* allBitmaps = LoadAllTextBitmap("C:/Windows/Fonts/cour.ttf", "Courier New");
    Sprite* allBitmaps = LoadAllTextBitmap("C:/Windows/Fonts/arial.ttf", "Arial");
    
    int x = 0, y = 0;
    float tX = 128, tY = 128;
    Vector2 dir = { 1, -1 };
    
    while (UpdateFrame())
    {
        if (IsKeyDown(KEY_F))
        {
            ToggleFullScreen();
        }
        if (IsKeyPressed(KEY_LEFT_ALT) && IsKeyPressed(KEY_F4))
        {
            ExitWindow();
        }
        
        backgroundColor = BLACK;
        
        DrawShapeEx(&(Shape){
                        .type = RECTANGLE,
                        .rect.pos  = (Vector2){ currentApp.windowWidth/2, currentApp.windowHeight/2 },
                        .rect.size = (Vector2){ currentApp.windowWidth/2, currentApp.windowHeight/2 },
                    }, true, SKYBLUE);
        
        //DrawTexture(comic.bitmap, 256, 256);
        int posX = 0, posY= 0, lineHeight = 0;
        for (int i = 0; i < '~' - '!'; ++i)
        {
            Sprite bitmap = allBitmaps[i];
            
            if (posX + bitmap.width> currentApp.windowWidth)
            {
                posY += lineHeight;
                posX = 0;
                lineHeight = 0;
            }
            
            DrawSprite(bitmap, posX, posY);
            posX += bitmap.width + 4;
            
            if (bitmap.height > lineHeight)
                lineHeight = bitmap.height;
        }
        
        Shape rect = {
            .type = RECTANGLE,
            .rect.size = (Vector2){ 120, 120 },
        };
        rect.rect.pos.y = currentApp.windowHeight - rect.rect.size.y;
        DrawShapeEx(&rect, true, RED);
        
        int radius = 200;
        DrawShapeEx(&(Shape){
                        .type = CIRCLE,
                        .circle.radius = radius,
                        .circle.pos = (Vector2){ x++ * 2 % (int)(currentApp.windowWidth + radius * 2) - radius,
                            y++ * 2 % (int)(currentApp.windowHeight + radius * 2) - radius }
                    }, true, GOLD);
        
        DrawTexture(texture, tX, tY);
        if (tX > currentApp. windowWidth - texture.width || tX < 0)
            dir.x = -dir.x;
        if (tY > currentApp.windowHeight - texture.height || tY < 0)
            dir.y = -dir.y;
        tX += dir.x * 256 * deltaTime;
        tY += dir.y * 256 * deltaTime;
    }
    
    ExitWindow();
    
    return 0;
}