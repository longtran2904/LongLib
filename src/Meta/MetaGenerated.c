#define META_GENERATED
#define _CRT_SECURE_NO_WARNINGS
#define Pi 3.1415

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>

#include "LongApp.c"

#include "Meta.h"

#include <windows.h>
#include <dsound.h>
#include <gl/gl.h>

#define Unload(data) VirtualFree(*((void**)&data), 0, MEM_RELEASE)

#include "Demo.c"

static f64 deltaTime;
static f64 time;
static f32 fps;
static f64 mcpf;

#define maxNumberOfButtons 256
static bool wasKeyPressed[maxNumberOfButtons], wasKeyReleased[maxNumberOfButtons], isKeyHeldDown[maxNumberOfButtons];
static i16 mouseWheel;

static HWND hwnd;
static bool isRunning;
static AppData currentApp;
static void(*_Start)(void);
static void(*_Update)(void);
static void(*_Quit)(void);

#if 0
typedef struct Data Data;
#line 39 "LongApp_Windows.c"
struct Data
{
    int data;
    float otherData;
    bool condition;
    char* state;
};
MemberDefinition memeberOf_Data[] = 
{
    { MetaType_int, "data", 0, (size_t)&((Data*)0)->data },
    { MetaType_float, "otherData", 0, (size_t)&((Data*)0)->otherData },
    { MetaType_bool, "condition", 0, (size_t)&((Data*)0)->condition },
    { MetaType_char, "state", 1, (size_t)&((Data*)0)->state },
};
#line 46 "LongApp_Windows.c"

typedef enum A A;
#line 47 "LongApp_Windows.c"
enum A
{
    Meta,
    Programming,
    Is,
    Fun,
};

typedef struct Something Something;
#line 55 "LongApp_Windows.c"
struct Something;
typedef enum Stuff Stuff;
#line 56 "LongApp_Windows.c"
enum Stuff;

typedef struct Player Player;
#line 58 "LongApp_Windows.c"
struct Player
{
    char* name, otherName;
    int health;
    float speed;
    int atk;
    int def;
    bool isAlive;
};
MemberDefinition memeberOf_Player[] = 
{
    { MetaType_char, "name", 1, (size_t)&((Player*)0)->name },
    { MetaType_char, "otherName", 1, (size_t)&((Player*)0)->otherName },
    { MetaType_int, "health", 0, (size_t)&((Player*)0)->health },
    { MetaType_float, "speed", 0, (size_t)&((Player*)0)->speed },
    { MetaType_int, "atk", 0, (size_t)&((Player*)0)->atk },
    { MetaType_int, "def", 0, (size_t)&((Player*)0)->def },
    { MetaType_bool, "isAlive", 0, (size_t)&((Player*)0)->isAlive },
};
#line 67 "LongApp_Windows.c"

void Function(int arg1, float arg2, char* _CALLER_FILE_, int _CALLER_LINE_)
{
    arg1 += _CALLER_LINE_;
    arg2++;
    char* name = _CALLER_FILE_;
}
#define Function(arg1, arg2) Function(arg1, arg2, __FILE__, __LINE__)
#line 74 "LongApp_Windows.c"

void F(void)
{
    int b = 0;
}

int Function2(int _CALLER_LINE_)
{
    return _CALLER_LINE_;
}
#define Function2() Function2(__LINE__)
#line 84 "LongApp_Windows.c"

void Bar(bool arg, char* _CALLER_FILE_, int _CALLER_LINE_)
{
    if (arg)
        char* c = _CALLER_FILE_;
    else
        int i = _CALLER_LINE_;
}
#define Bar(arg) Bar(arg, __FILE__, __LINE__)
#line 92 "LongApp_Windows.c"

char Foo(char* _CALLER_FILE_)
{
    return _CALLER_FILE_[0];
}
#define Foo() Foo(__FILE__)
#line 97 "LongApp_Windows.c"
#endif

static void Win32InitOpenGL(HWND _window)
{
    HDC windowDC = GetDC(_window);
    
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
    ReleaseDC(_window, windowDC);
}

typedef struct
{
    int samplesPerSecond;
    int16_t volume;
    u32 runningSampleIndex;
    int bytesPerSample;
    DWORD secondaryBufferSize;
    int latencySampleCount;
} Win32SoundOutput;

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND* ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(DSC);

static LPDIRECTSOUNDBUFFER Win32InitDSound(HWND wnd, int samplesPerSecond, int bufferSize)
{
    HMODULE dSoundLib = LoadLibraryA("dsound.dll");
    
    if (dSoundLib)
    {
        DSC* directSoundCreate = (DSC*)GetProcAddress(dSoundLib, "DirectSoundCreate");
        
        LPDIRECTSOUND directSound;
        if (directSoundCreate && SUCCEEDED(directSoundCreate(0, &directSound, 0)))
        {
            WAVEFORMATEX waveFormat;
            waveFormat.wFormatTag =  WAVE_FORMAT_PCM;
            waveFormat.nChannels = 2;
            waveFormat.nSamplesPerSec = samplesPerSecond;
            waveFormat.wBitsPerSample = 16;
            waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
            waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
            waveFormat.cbSize = 0;
            
            if (SUCCEEDED(IDirectSound_SetCooperativeLevel(directSound, wnd, DSSCL_PRIORITY)))
            {
                DSBUFFERDESC bufferDesc = {
                    .dwSize = sizeof(bufferDesc),
                    .dwFlags = DSBCAPS_PRIMARYBUFFER
                };
                
                LPDIRECTSOUNDBUFFER primaryBuffer;
                if (SUCCEEDED(IDirectSound_CreateSoundBuffer(directSound, &bufferDesc, &primaryBuffer, 0)))
                {
                    if (SUCCEEDED(IDirectSoundBuffer_SetFormat(primaryBuffer, &waveFormat)))
                    {
                        OutputDebugString("Primary buffer format was set.\n");
                    }
                }
            }
            
            DSBUFFERDESC bufferDesc = {
                .dwSize = sizeof(bufferDesc),
                .dwFlags = 0,
                .dwBufferBytes = bufferSize,
                .lpwfxFormat = &waveFormat,
            };
            
            LPDIRECTSOUNDBUFFER secondaryBuffer;
            if (SUCCEEDED(IDirectSound_CreateSoundBuffer(directSound, &bufferDesc, &secondaryBuffer, 0)))
            {
                OutputDebugString("Secondary buffer format was set.\n");
                return secondaryBuffer;
            }
        }
    }
    
    return (LPDIRECTSOUNDBUFFER){0};
}

void Win32ClearBuffer(LPDIRECTSOUNDBUFFER buffer, Win32SoundOutput* soundOutput)
{
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;
    if(SUCCEEDED(IDirectSoundBuffer_Lock(buffer, 0, soundOutput->secondaryBufferSize,
                                         &Region1, &Region1Size,
                                         &Region2, &Region2Size,
                                         0)))
    {
        // TODO(casey): assert that Region1Size/Region2Size is valid
        uint8_t* DestSample = (uint8_t*)Region1;
        for(DWORD ByteIndex = 0;
            ByteIndex < Region1Size;
            ++ByteIndex)
        {
            *DestSample++ = 0;
        }
        
        DestSample = (uint8_t*)Region2;
        for(DWORD ByteIndex = 0;
            ByteIndex < Region2Size;
            ++ByteIndex)
        {
            *DestSample++ = 0;
        }
        
        IDirectSoundBuffer_Unlock(buffer, Region1, Region1Size, Region2, Region2Size);
    }
}

void Win32InitSoundBuffer(LPDIRECTSOUNDBUFFER buffer, Win32SoundOutput *soundOutput,int16_t* sample)
{
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;
    if(SUCCEEDED(IDirectSoundBuffer_Lock(buffer, 0, soundOutput->secondaryBufferSize,
                                         &Region1, &Region1Size,
                                         &Region2, &Region2Size,
                                         0)))
    {
        memcpy(Region1, sample, Region1Size);
        
        IDirectSoundBuffer_Unlock(buffer, Region1, Region1Size, Region2, Region2Size);
    }
}

void Win32FillSoundBuffer(LPDIRECTSOUNDBUFFER buffer, Win32SoundOutput *soundOutput, DWORD byteToLock, DWORD bytesToWrite, int16_t* sample)
{
    // TODO(casey): More strenuous test!
    void* region1;
    DWORD region1Size;
    void* region2;
    DWORD region2Size;
    if(SUCCEEDED(IDirectSoundBuffer_Lock(buffer, byteToLock, bytesToWrite,
                                         &region1, &region1Size,
                                         &region2, &region2Size,
                                         0)))
    {
        DWORD region1SampleCount = region1Size/soundOutput->bytesPerSample;
        int16_t* destSample = (int16_t*)region1;
        int16_t* sourceSample = sample;
        for(DWORD sampleIndex = 0;
            sampleIndex < region1SampleCount;
            ++sampleIndex)
        {
            *destSample++ = *sourceSample++;
            *destSample++ = *sourceSample++;
            ++soundOutput->runningSampleIndex;
        }
        
        DWORD region2SampleCount = region2Size/soundOutput->bytesPerSample;
        destSample = (int16_t*)region2;
        for(DWORD sampleIndex = 0;
            sampleIndex < region2SampleCount;
            ++sampleIndex)
        {
            *destSample++ = *sourceSample++;
            *destSample++ = *sourceSample++;
            ++soundOutput->runningSampleIndex;
        }
        
        IDirectSoundBuffer_Unlock(buffer, region1, region1Size, region2, region2Size);
    }
}

v2u GetWindowDimension(HWND wnd)
{
    RECT clientRect;
    GetClientRect(wnd, &clientRect);
    return (v2u){ clientRect.right - clientRect.left, 
        clientRect.bottom - clientRect.top };
}

void* Alloc(size_t size)
{
    return VirtualAlloc(0, size, MEM_COMMIT, PAGE_READWRITE);
}

void Free(void* data)
{
    VirtualFree(data, 0, MEM_RELEASE);
}

LRESULT CALLBACK MainWindowCallback(HWND wnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    
    switch (message)
    {
        case WM_SIZE:
        {
            currentApp.window.size = GetWindowDimension(wnd);
            
            // TODO: change the glViewport depend on whether USE_BLACK_BAR is enable
        } break;
        case WM_DESTROY:
        case WM_CLOSE:
        case WM_QUIT:
        {
            isRunning = false;
        } break;
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            u32 vkCode = (u32)wParam;
            bool wasDown = ((lParam & (1 << 30)) != 0);
            bool isDown = ((lParam & (1 << 31)) == 0);
            
            wasKeyPressed[vkCode] = isDown && !wasDown;
            wasKeyReleased[vkCode] = !isDown && wasDown;
            isKeyHeldDown[vkCode] = isDown;
        } break;
        case WM_MOUSEWHEEL:
        {
            mouseWheel = GET_WHEEL_DELTA_WPARAM(wParam);;
        } break;
        default:
        {
            result = DefWindowProc(wnd, message, wParam, lParam);
        } break;
    }
    
    return result;
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
    WNDCLASS windowClass = {
        .style = CS_HREDRAW|CS_VREDRAW,
        .lpfnWndProc = MainWindowCallback,
        .hInstance = hInstance,
        .lpszClassName = "LongAppClass"
    };
    
    if (!RegisterClass(&windowClass)) return -1;
    
    // TODO: Configurate app's frameRate and flags properly
    AppDesc desc = InitApp(0, 0);
    
    _Start = desc.Start;
    _Update = desc.Update;
    _Quit = desc.Quit;
    
    currentApp = (AppData)
    {
        .window = desc.window,
        .drawTarget = desc.drawTarget,
        .viewTarget = desc.window.rect
    }
    
    if (!desc.window.flags)
        currentApp.window.flags = WS_OVERLAPPEDWINDOW|WS_VISIBLE;
    
    // TODO: CreateWindowEx needs both width and height to be CW_USEDEFAULT
    // Handle the case when only either app.windowWidth or app.windowHeight
    bool useDefault = true;
    RECT rect = {
        .left = 0,
        .right =  currentApp.window.size.width,
        .top = 0,
        .bottom = currentApp.window.size.height
    };
    
    if (rect.right || rect.bottom)
    {
        AdjustWindowRect(&rect, currentApp.window.flags, 0);
        useDefault = false;
    }
    
    hwnd= CreateWindowEx(
                         0,
                         windowClass.lpszClassName,
                         currentApp.window.title,
                         currentApp.window.flags,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         useDefault ? CW_USEDEFAULT : rect.right - rect.left,
                         useDefault ? CW_USEDEFAULT : rect.bottom - rect.top,
                         0,
                         0,
                         hInstance,
                         0);
    
    if (!hwnd) return -1;
    
    if (!CheckSprite(currentApp.drawTarget))
    {
        drawTarget = (Sprite){ .data = 0, .width = currentApp.window.size.width, .height = currentApp.window.size.height };
        drawTarget.data = (Color*)VirtualAlloc(0, DataSize(drawTarget), MEM_COMMIT, PAGE_READWRITE);
        if (!drawTarget.data) return -1;
    }
    else
        drawTarget = currentApp.drawTarget;
    
    if ((currentApp.viewTarget.width == 0) || (currentApp.viewTarget.height == 0))
        currentApp.viewTarget = (RectI){ 0, 0, drawTarget.width, drawTarget.height };
    
    // TODO: Init OpenGL and draw target
    // TODO: Load and init other libraries
    
    Win32SoundOutput soundOutput = {0};
    soundOutput.samplesPerSecond = 48000;
    soundOutput.volume = (int16_t)(6000/* / Function2()*/);
    soundOutput.bytesPerSample = sizeof(int16_t)*2;
    soundOutput.secondaryBufferSize = soundOutput.samplesPerSecond * soundOutput.bytesPerSample;
    soundOutput.latencySampleCount = soundOutput.samplesPerSecond / 15;
    
    soundSample = VirtualAlloc(0, soundOutput.secondaryBufferSize,MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    sampleSize = soundOutput.samplesPerSecond;
    
    isRunning = true;
    
    Sound sound = LoadSound("D:/Documents/Projects/LongLib/src/Data/music_test.wav");
    
    soundOutput.secondaryBufferSize = soundOutput.bytesPerSample * sound.sampleCount;
    LPDIRECTSOUNDBUFFER secondaryBuffer = Win32InitDSound(hwnd, soundOutput.samplesPerSecond, soundOutput.secondaryBufferSize);
    Win32ClearBuffer(secondaryBuffer, &soundOutput);
    Win32InitSoundBuffer(secondaryBuffer, &soundOutput, sound.sample);
    //Win32FillSoundBuffer(secondaryBuffer, &soundOutput, 0, soundOutput.secondaryBufferSize, sound.samples[0]);
    IDirectSoundBuffer_Play(secondaryBuffer, 0, 0, DSBPLAY_LOOPING);
    
    LARGE_INTEGER perfCountFrequencyResult;
    QueryPerformanceFrequency(&perfCountFrequencyResult);
    int64_t perfCountFrequency = perfCountFrequencyResult.QuadPart;
    
    LARGE_INTEGER lastCounter;
    QueryPerformanceCounter(&lastCounter);
    uint64_t lastCycleCount = __rdtsc();
    
    uint64_t endCycleCount;
    LARGE_INTEGER endCounter;
    
    uint64_t cyclesElapsed;
    int64_t counterElapsed;
    
    if (_Start)
        _Start();
    
    MSG message;
    while (isRunning)
    {
        memset(wasKeyPressed, 0, maxNumberOfButtons);
        memset(wasKeyReleased, 0, maxNumberOfButtons);
        
        while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
        
        // NOTE(casey): DirectSound output test
        DWORD playCursor;
        DWORD writeCursor;
        if(SUCCEEDED(IDirectSoundBuffer_GetCurrentPosition(secondaryBuffer, &playCursor, &writeCursor)))
        {
            DWORD byteToLock = ((soundOutput.runningSampleIndex*soundOutput.bytesPerSample) %
                                soundOutput.secondaryBufferSize);
            
            DWORD targetCursor =
                ((playCursor +
                  (soundOutput.latencySampleCount*soundOutput.bytesPerSample)) %
                 soundOutput.secondaryBufferSize);
            DWORD bytesToWrite;
            // TODO(casey): Change this to using a lower latency offset from the playcursor
            // when we actually start having sound effects.
            if(byteToLock > targetCursor)
            {
                bytesToWrite = (soundOutput.secondaryBufferSize - byteToLock);
                bytesToWrite += targetCursor;
            }
            else
            {
                bytesToWrite = targetCursor - byteToLock;
            }
            
            //Win32FillSoundBuffer(secondaryBuffer, &soundOutput, byteToLock, bytesToWrite, soundSample);
        }
        
        if (_Update)
            _Update();
        
        // TODO: Swap buffer, batch rendering, run at fixed frame rate, use vsync, etc
        HDC deviceContext = GetDC(hwnd);
        
        if (false)
            SetWindowPos(hwnd, 0, // TODO: replace 0 with window flags
                         currentApp.window.pos.x, currentApp.window.pos.y,
                         currentApp.window.size.width, currentApp.window.size.height,
                         SWP_NOOWNERZORDER|SWP_NOZORDER); // TODO: replace these temp flags with window flags
        
        StretchDIBits(deviceContext,
                      0, 0, currentApp.window.size.width, currentApp.window.size.height,
                      currentApp.viewTarget.x, currentApp.viewTarget.y, currentApp.viewTarget.width, currentApp.viewTarget.height,
                      drawTarget.data, &(BITMAPINFO){
                          .bmiHeader = {
                              .biSize = sizeof(BITMAPINFOHEADER),
                              .biWidth = drawTarget.width,
                              .biHeight = drawTarget.height,
                              .biPlanes = 1,
                              .biBitCount = 32,
                              .biCompression = BI_RGB
                          }
                      },
                      DIB_RGB_COLORS, SRCCOPY
                      );
        memset(drawTarget.data, 0, DataSize(drawTarget));
        
        ReleaseDC(hwnd, deviceContext);
        
        endCycleCount = __rdtsc();
        QueryPerformanceCounter(&endCounter);
        
        cyclesElapsed = endCycleCount - lastCycleCount;
        counterElapsed = endCounter.QuadPart - lastCounter.QuadPart;
        deltaTime = (((1000.0f*(f64)counterElapsed) / (f64)perfCountFrequency));
        time += deltaTime;
        fps = (int)(perfCountFrequency / counterElapsed);
        mcpf = ((f64)cyclesElapsed / (1000.0f * 1000.0f));
        
        static char buffer[256];
        snprintf(buffer, 256, "Delta Time: %f, Time: %f, Fps: %d, MCPF: %f\n", deltaTime, time, fps, mcpf);
        OutputDebugString(buffer);
        
        lastCounter = endCounter;
        lastCycleCount = endCycleCount;
    }
    
    if (_Quit)
        _Quit();
    // Clean up other stuff
    
    return 0;
}

Monitor GetMonitorData(void)
{
    HDC dc = GetDC(hwnd);
    Monitor result = {
        GetDeviceCaps(dc, HORZRES),
        GetDeviceCaps(dc, VERTRES),
        GetDeviceCaps(dc, HORZSIZE),
        GetDeviceCaps(dc, VERTSIZE),
        GetDeviceCaps(dc, VREFRESH)
    };
    ReleaseDC(hwnd, dc);
    
    return result;
}


bool IsDown(u32 key)
{
    assert(key < maxNumberOfButtons);
    return isKeyHeldDown[key];
}

bool IsPressed(u32 key)
{
    assert(key < maxNumberOfButtons);
    return wasKeyPressed[key];
}

bool IsUp(u32 key)
{
    assert(key < maxNumberOfButtons);
    return wasKeyReleased[key];
}


v2u GetMousePos(void)
{
    POINT point;
    if (GetCursorPos(&point))
        return (v2u){ point.x, point.y };
    else
        return (v2u){0};
}

i16 GetScrollWheel(void)
{
    return mouseWheel;
}


f32 GetFPS(void)
{
    return fps;
}

f64 GetDeltaTime(void)
{
    return deltaTime;
}

f64 GetTime(void)
{
    return time;
}

f64 GetMCPF(void)
{
    return mcpf;
}


Buffer LoadData(char* fileName)
{
    Buffer result = {0};
    HANDLE fileHandle = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER fileSize;
        if (GetFileSizeEx(fileHandle, &fileSize))
        {
            assert(fileSize.QuadPart <= UINT32_MAX); // less than 4GB
            result.data = VirtualAlloc(0, fileSize.QuadPart, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if (result.data)
            {
                u32 fileSize32 = (u32)fileSize.QuadPart;
                if (ReadFile(fileHandle, result.data, fileSize32, (LPDWORD)&result.count, 0))
                {
                    assert(fileSize32 == result.count);
                }
                else
                {
                    Unload(result);
                }
            }
        }
        CloseHandle(fileHandle);
    }
    return result;
}

char* LoadText(char* fileName)
{
    char* result = 0;
    HANDLE fileHandle = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER fileSize;
        if (GetFileSizeEx(fileHandle, &fileSize))
        {
            assert(fileSize.QuadPart <= UINT32_MAX); // less than 4GB
            result = (char*)VirtualAlloc(0, fileSize.QuadPart + 1, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if (result)
            {
                u32 fileSize32 = (u32)fileSize.QuadPart;
                u32 bytesRead;
                if (ReadFile(fileHandle, result, fileSize32, (LPDWORD)&bytesRead, 0))
                {
                    assert(fileSize32 == bytesRead);
                    result[bytesRead] = 0;
                }
                else
                {
                    Unload(result);
                }
            }
        }
        CloseHandle(fileHandle);
    }
    return result;
}

bool SaveData(char* fileName, Buffer buffer)
{
    bool result = false;
    HANDLE fileHandle = CreateFile(fileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD bytesRead;
        BOOL canWrite = WriteFile(fileHandle, buffer.data, (DWORD)buffer.count, &bytesRead, 0);
        if (canWrite)
        {
            result = buffer.count == bytesRead;
        }
        CloseHandle(fileHandle);
    }
    return result;
}

bool SaveText(char* fileName, char* text)
{
    bool result = false;
    HANDLE fileHandle = CreateFile(fileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD bytesRead;
        DWORD bytesToWrite = (DWORD)strlen(text);
        BOOL canWrite = WriteFile(fileHandle, text, bytesToWrite, &bytesRead, 0);
        if (canWrite)
            result = bytesToWrite == bytesRead;
        CloseHandle(fileHandle);
    }
    return result;
}

bool FileExists(char* fileName)
{
    return GetFileAttributes(fileName) != INVALID_FILE_ATTRIBUTES;
}

bool IsFileExtension(char* fileName, char* ext)
{
    const char* fileExt = GetFileExt(fileName);
    return strcmp(fileExt, ext) == 0; 
}

char* GetFileExt(char* fileName)
{
    char* dot = strchr(fileName, '.');
    if (!dot || dot == fileName) return NULL;
    return (dot + 1);
}


static char *strprbrk(char *s, char *charset)
{
    char *latestMatch = NULL;
    for (; s = strpbrk(s, charset), s != NULL; latestMatch = s++) { }
    return latestMatch;
}

char* GetFileName(char* filePath)
{
    char* fileName = NULL;
    if (filePath != NULL) fileName = strprbrk(filePath, "\\/");
    if (!fileName || (fileName == filePath)) return filePath;
    return fileName + 1;
}

char* GetFileNameWithoutExt(char* filePath)
{
#define MAX_FILENAMEWITHOUTEXT_LENGTH 128
    
    static char fileName[MAX_FILENAMEWITHOUTEXT_LENGTH];
    memset(fileName, 0, MAX_FILENAMEWITHOUTEXT_LENGTH);
    
    if (filePath != NULL) strcpy(fileName, GetFileName(filePath)); // Get filename with extension
    
    size_t len = strlen(fileName);
    
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
}#define META_GENERATED
#define _CRT_SECURE_NO_WARNINGS
#define Pi 3.1415

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>

#include "LongApp.c"
#include "LongAudio.c"

#include <windows.h>
#include <dsound.h>
#include <gl/gl.h>

#define Unload(data) VirtualFree(*((void**)&data), 0, MEM_RELEASE)

#include "Demo.c"

static f64 deltaTime;
static f64 time;
static f32 fps;
static f64 mcpf;

#define maxNumberOfButtons 256
static bool wasKeyPressed[maxNumberOfButtons], wasKeyReleased[maxNumberOfButtons], isKeyHeldDown[maxNumberOfButtons];
static i16 mouseWheel;

static HWND hwnd;
static bool isRunning;
static AppData currentApp;
static void(*_Start)(void);
static void(*_Update)(void);
static void(*_Quit)(void);

static void Win32InitOpenGL(HWND _window)
{
    HDC windowDC = GetDC(_window);
    
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
    ReleaseDC(_window, windowDC);
}

typedef struct
{
    int samplesPerSecond;
    int16_t volume;
    u32 runningSampleIndex;
    int bytesPerSample;
    DWORD secondaryBufferSize;
    int latencySampleCount;
} Win32SoundOutput;

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND* ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(DSC);

static LPDIRECTSOUNDBUFFER Win32InitDSound(HWND wnd, int samplesPerSecond, int bufferSize)
{
    HMODULE dSoundLib = LoadLibraryA("dsound.dll");
    
    if (dSoundLib)
    {
        DSC* directSoundCreate = (DSC*)GetProcAddress(dSoundLib, "DirectSoundCreate");
        
        LPDIRECTSOUND directSound;
        if (directSoundCreate && SUCCEEDED(directSoundCreate(0, &directSound, 0)))
        {
            WAVEFORMATEX waveFormat;
            waveFormat.wFormatTag =  WAVE_FORMAT_PCM;
            waveFormat.nChannels = 2;
            waveFormat.nSamplesPerSec = samplesPerSecond;
            waveFormat.wBitsPerSample = 16;
            waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
            waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
            waveFormat.cbSize = 0;
            
            if (SUCCEEDED(IDirectSound_SetCooperativeLevel(directSound, wnd, DSSCL_PRIORITY)))
            {
                DSBUFFERDESC bufferDesc = {
                    .dwSize = sizeof(bufferDesc),
                    .dwFlags = DSBCAPS_PRIMARYBUFFER
                };
                
                LPDIRECTSOUNDBUFFER primaryBuffer;
                if (SUCCEEDED(IDirectSound_CreateSoundBuffer(directSound, &bufferDesc, &primaryBuffer, 0)))
                {
                    if (SUCCEEDED(IDirectSoundBuffer_SetFormat(primaryBuffer, &waveFormat)))
                    {
                        OutputDebugString("Primary buffer format was set.\n");
                    }
                }
            }
            
            DSBUFFERDESC bufferDesc = {
                .dwSize = sizeof(bufferDesc),
                .dwFlags = 0,
                .dwBufferBytes = bufferSize,
                .lpwfxFormat = &waveFormat,
            };
            
            LPDIRECTSOUNDBUFFER secondaryBuffer;
            if (SUCCEEDED(IDirectSound_CreateSoundBuffer(directSound, &bufferDesc, &secondaryBuffer, 0)))
            {
                OutputDebugString("Secondary buffer format was set.\n");
                return secondaryBuffer;
            }
        }
    }
    
    return (LPDIRECTSOUNDBUFFER){0};
}

void Win32ClearBuffer(LPDIRECTSOUNDBUFFER buffer, Win32SoundOutput* soundOutput)
{
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;
    if(SUCCEEDED(IDirectSoundBuffer_Lock(buffer, 0, soundOutput->secondaryBufferSize,
                                         &Region1, &Region1Size,
                                         &Region2, &Region2Size,
                                         0)))
    {
        // TODO(casey): assert that Region1Size/Region2Size is valid
        uint8_t* DestSample = (uint8_t*)Region1;
        for(DWORD ByteIndex = 0;
            ByteIndex < Region1Size;
            ++ByteIndex)
        {
            *DestSample++ = 0;
        }
        
        DestSample = (uint8_t*)Region2;
        for(DWORD ByteIndex = 0;
            ByteIndex < Region2Size;
            ++ByteIndex)
        {
            *DestSample++ = 0;
        }
        
        IDirectSoundBuffer_Unlock(buffer, Region1, Region1Size, Region2, Region2Size);
    }
}

void Win32InitSoundBuffer(LPDIRECTSOUNDBUFFER buffer, Win32SoundOutput *soundOutput,int16_t* sample)
{
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;
    if(SUCCEEDED(IDirectSoundBuffer_Lock(buffer, 0, soundOutput->secondaryBufferSize,
                                         &Region1, &Region1Size,
                                         &Region2, &Region2Size,
                                         0)))
    {
        memcpy(Region1, sample, Region1Size);
        
        IDirectSoundBuffer_Unlock(buffer, Region1, Region1Size, Region2, Region2Size);
    }
}

void Win32FillSoundBuffer(LPDIRECTSOUNDBUFFER buffer, Win32SoundOutput *soundOutput, DWORD byteToLock, DWORD bytesToWrite, int16_t* sample)
{
    // TODO(casey): More strenuous test!
    void* region1;
    DWORD region1Size;
    void* region2;
    DWORD region2Size;
    if(SUCCEEDED(IDirectSoundBuffer_Lock(buffer, byteToLock, bytesToWrite,
                                         &region1, &region1Size,
                                         &region2, &region2Size,
                                         0)))
    {
        DWORD region1SampleCount = region1Size/soundOutput->bytesPerSample;
        int16_t* destSample = (int16_t*)region1;
        int16_t* sourceSample = sample;
        for(DWORD sampleIndex = 0;
            sampleIndex < region1SampleCount;
            ++sampleIndex)
        {
            *destSample++ = *sourceSample++;
            *destSample++ = *sourceSample++;
            ++soundOutput->runningSampleIndex;
        }
        
        DWORD region2SampleCount = region2Size/soundOutput->bytesPerSample;
        destSample = (int16_t*)region2;
        for(DWORD sampleIndex = 0;
            sampleIndex < region2SampleCount;
            ++sampleIndex)
        {
            *destSample++ = *sourceSample++;
            *destSample++ = *sourceSample++;
            ++soundOutput->runningSampleIndex;
        }
        
        IDirectSoundBuffer_Unlock(buffer, region1, region1Size, region2, region2Size);
    }
}

v2u GetWindowDimension(HWND wnd)
{
    RECT clientRect;
    GetClientRect(wnd, &clientRect);
    return (v2u){ clientRect.right - clientRect.left, 
        clientRect.bottom - clientRect.top };
}

void* Alloc(size_t size)
{
    return VirtualAlloc(0, size, MEM_COMMIT, PAGE_READWRITE);
}

void Free(void* data)
{
    VirtualFree(data, 0, MEM_RELEASE);
}

static void UpdateKeyCode(u32 code, bool isDown, bool wasDown)
{
    wasKeyPressed[code] = isDown && !wasDown;
    wasKeyReleased[code] = !isDown && wasDown;
    isKeyHeldDown[code] = isDown;
}

LRESULT CALLBACK MainWindowCallback(HWND wnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    switch (message)
    {
        case WM_SIZE:
        {
            currentApp.window.size = GetWindowDimension(wnd);
            
            // TODO: change the glViewport depend on whether USE_BLACK_BAR is enable
        } break;
        case WM_DESTROY:
        case WM_CLOSE:
        case WM_QUIT:
        {
            isRunning = false;
        } break;
        
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        {
            u32 buttons[] = { MK_LBUTTON, MK_RBUTTON, MK_MBUTTON };
            u32 vkCodes[] = { KEY_LMB, KEY_RMB, KEY_MMB };
            assert(sizeof(vkCodes) == sizeof(buttons));
            for (u32 i = 0; i < sizeof(buttons)/sizeof(buttons[0]); ++i)
                UpdateKeyCode(vkCodes[i], wParam & buttons[i], isKeyHeldDown[vkCodes[i]]);
        } break;
        
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            u32 vkCode = (u32)wParam;
            bool wasDown = ((lParam & (1 << 30)) != 0);
            bool isDown = ((lParam & (1 << 31)) == 0);
            UpdateKeyCode(vkCode, isDown, wasDown );
        } break;
        case WM_MOUSEWHEEL:
        {
            mouseWheel = GET_WHEEL_DELTA_WPARAM(wParam);;
        } break;
        default:
        {
            result = DefWindowProc(wnd, message, wParam, lParam);
        } break;
    }
    
    return result;
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
    WNDCLASS windowClass = {
        .style = CS_HREDRAW|CS_VREDRAW,
        .lpfnWndProc = MainWindowCallback,
        .hInstance = hInstance,
        .lpszClassName = "LongAppClass"
    };
    
    if (!RegisterClass(&windowClass)) return -1;
    
    // TODO: Configurate app's frameRate and flags properly
    AppDesc desc = InitApp(0, 0);
    
    _Start = desc.Start;
    _Update = desc.Update;
    _Quit = desc.Quit;
    
    currentApp = (AppData)
    {
        .window = desc.window,
        .drawTarget = desc.drawTarget,
        .viewTarget = desc.window.rect
    };
    
    if (!desc.window.flags)
        currentApp.window.flags = WS_OVERLAPPEDWINDOW|WS_VISIBLE;
    
    // TODO: CreateWindowEx needs both width and height to be CW_USEDEFAULT
    // Handle the case when only either app.windowWidth or app.windowHeight
    bool useDefault = true;
    RECT rect = {
        .left = 0,
        .right =  currentApp.window.size.width,
        .top = 0,
        .bottom = currentApp.window.size.height
    };
    
    if (rect.right || rect.bottom)
    {
        AdjustWindowRect(&rect, currentApp.window.flags, 0);
        useDefault = false;
    }
    
    hwnd= CreateWindowEx(
                         0,
                         windowClass.lpszClassName,
                         currentApp.window.title,
                         currentApp.window.flags,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         useDefault ? CW_USEDEFAULT : rect.right - rect.left,
                         useDefault ? CW_USEDEFAULT : rect.bottom - rect.top,
                         0,
                         0,
                         hInstance,
                         0);
    
    if (!hwnd) return -1;
    
    if (!CheckSprite(currentApp.drawTarget))
    {
        drawTarget = (Sprite){ .data = 0, .width = currentApp.window.size.width, .height = currentApp.window.size.height };
        drawTarget.data = (Color*)VirtualAlloc(0, DataSize(drawTarget), MEM_COMMIT, PAGE_READWRITE);
        if (!drawTarget.data) return -1;
    }
    else
        drawTarget = currentApp.drawTarget;
    
    if ((currentApp.viewTarget.width == 0) || (currentApp.viewTarget.height == 0))
        currentApp.viewTarget = (RectI){ 0, 0, drawTarget.width, drawTarget.height };
    
    // TODO: Init OpenGL and draw target
    // TODO: Load and init other libraries
    
    Win32SoundOutput soundOutput = {0};
    soundOutput.samplesPerSecond = 48000;
    soundOutput.volume = (int16_t)(6000/* / Function2()*/);
    soundOutput.bytesPerSample = sizeof(int16_t)*2;
    soundOutput.secondaryBufferSize = soundOutput.samplesPerSecond * soundOutput.bytesPerSample;
    soundOutput.latencySampleCount = soundOutput.samplesPerSecond / 15;
    
    soundSample = VirtualAlloc(0, soundOutput.secondaryBufferSize,MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    sampleSize = soundOutput.samplesPerSecond;
    
    isRunning = true;
    
    Sound sound = LoadSound("D:/Documents/Projects/LongLib/src/Data/music_test.wav");
    
    soundOutput.secondaryBufferSize = soundOutput.bytesPerSample * sound.sampleCount;
    LPDIRECTSOUNDBUFFER secondaryBuffer = Win32InitDSound(hwnd, soundOutput.samplesPerSecond, soundOutput.secondaryBufferSize);
    Win32ClearBuffer(secondaryBuffer, &soundOutput);
    Win32InitSoundBuffer(secondaryBuffer, &soundOutput, sound.sample);
    //Win32FillSoundBuffer(secondaryBuffer, &soundOutput, 0, soundOutput.secondaryBufferSize, sound.samples[0]);
    IDirectSoundBuffer_Play(secondaryBuffer, 0, 0, DSBPLAY_LOOPING);
    
    LARGE_INTEGER perfCountFrequencyResult;
    QueryPerformanceFrequency(&perfCountFrequencyResult);
    int64_t perfCountFrequency = perfCountFrequencyResult.QuadPart;
    
    LARGE_INTEGER lastCounter;
    QueryPerformanceCounter(&lastCounter);
    uint64_t lastCycleCount = __rdtsc();
    
    uint64_t endCycleCount;
    LARGE_INTEGER endCounter;
    
    uint64_t cyclesElapsed;
    int64_t counterElapsed;
    
    if (_Start)
        _Start();
    
    MSG message;
    while (isRunning)
    {
        memset(wasKeyPressed, 0, maxNumberOfButtons);
        memset(wasKeyReleased, 0, maxNumberOfButtons);
        
        while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
        
        // NOTE(casey): DirectSound output test
        DWORD playCursor;
        DWORD writeCursor;
        if(SUCCEEDED(IDirectSoundBuffer_GetCurrentPosition(secondaryBuffer, &playCursor, &writeCursor)))
        {
            DWORD byteToLock = ((soundOutput.runningSampleIndex*soundOutput.bytesPerSample) %
                                soundOutput.secondaryBufferSize);
            
            DWORD targetCursor =
            ((playCursor +
              (soundOutput.latencySampleCount*soundOutput.bytesPerSample)) %
             soundOutput.secondaryBufferSize);
            DWORD bytesToWrite;
            // TODO(casey): Change this to using a lower latency offset from the playcursor
            // when we actually start having sound effects.
            if(byteToLock > targetCursor)
            {
                bytesToWrite = (soundOutput.secondaryBufferSize - byteToLock);
                bytesToWrite += targetCursor;
            }
            else
            {
                bytesToWrite = targetCursor - byteToLock;
            }
            
            //Win32FillSoundBuffer(secondaryBuffer, &soundOutput, byteToLock, bytesToWrite, soundSample);
        }
        
        if (_Update)
            _Update();
        
        // TODO: Swap buffer, batch rendering, run at fixed frame rate, use vsync, etc
        HDC deviceContext = GetDC(hwnd);
        
        if (false)
            SetWindowPos(hwnd, 0, // TODO: replace 0 with window flags
                         currentApp.window.pos.x, currentApp.window.pos.y,
                         currentApp.window.size.width, currentApp.window.size.height,
                         SWP_NOOWNERZORDER|SWP_NOZORDER); // TODO: replace these temp flags with window flags
        
        StretchDIBits(deviceContext,
                      0, 0, currentApp.window.size.width, currentApp.window.size.height,
                      currentApp.viewTarget.x, currentApp.viewTarget.y, currentApp.viewTarget.width, currentApp.viewTarget.height,
                      drawTarget.data, &(BITMAPINFO){
                          .bmiHeader = {
                              .biSize = sizeof(BITMAPINFOHEADER),
                              .biWidth = drawTarget.width,
                              .biHeight = drawTarget.height,
                              .biPlanes = 1,
                              .biBitCount = 32,
                              .biCompression = BI_RGB
                          }
                      },
                      DIB_RGB_COLORS, SRCCOPY
                      );
        memset(drawTarget.data, 0, DataSize(drawTarget));
        
        ReleaseDC(hwnd, deviceContext);
        
        endCycleCount = __rdtsc();
        QueryPerformanceCounter(&endCounter);
        
        cyclesElapsed = endCycleCount - lastCycleCount;
        counterElapsed = endCounter.QuadPart - lastCounter.QuadPart;
        deltaTime = (((1000.0f*(f64)counterElapsed) / (f64)perfCountFrequency));
        time += deltaTime;
        fps = (f32)perfCountFrequency / (f32)counterElapsed;
        mcpf = ((f64)cyclesElapsed / (1000.0f * 1000.0f));
        
        static char buffer[256];
        snprintf(buffer, 256, "Delta Time: %f, Time: %f, Fps: %f, MCPF: %f\n", deltaTime, time, fps, mcpf);
        OutputDebugString(buffer);
        
        lastCounter = endCounter;
        lastCycleCount = endCycleCount;
    }
    
    if (_Quit)
        _Quit();
    // Clean up other stuff
    
    return 0;
}

Monitor GetMonitorData(void)
{
    HDC dc = GetDC(hwnd);
    Monitor result = {
        GetDeviceCaps(dc, HORZRES),
        GetDeviceCaps(dc, VERTRES),
        GetDeviceCaps(dc, HORZSIZE),
        GetDeviceCaps(dc, VERTSIZE),
        GetDeviceCaps(dc, VREFRESH)
    };
    ReleaseDC(hwnd, dc);
    
    return result;
}


bool IsDown(u32 key)
{
    assert(key < maxNumberOfButtons);
    return isKeyHeldDown[key];
}

bool IsPressed(u32 key)
{
    assert(key < maxNumberOfButtons);
    return wasKeyPressed[key];
}

bool IsUp(u32 key)
{
    assert(key < maxNumberOfButtons);
    return wasKeyReleased[key];
}


v2i GetMousePos(void)
{
    POINT mouseP;
    GetCursorPos(&mouseP);
    ScreenToClient(hwnd, &mouseP);
    return (v2i){ mouseP.x, mouseP.y };
}

i16 GetScrollWheel(void)
{
    return mouseWheel;
}


f32 GetFPS(void)
{
    return fps;
}

f64 GetDeltaTime(void)
{
    return deltaTime;
}

f64 GetTime(void)
{
    return time;
}

f64 GetMCPF(void)
{
    return mcpf;
}


Buffer LoadData(char* fileName)
{
    Buffer result = {0};
    HANDLE fileHandle = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER fileSize;
        if (GetFileSizeEx(fileHandle, &fileSize))
        {
            assert(fileSize.QuadPart <= UINT32_MAX); // less than 4GB
            result.data = VirtualAlloc(0, fileSize.QuadPart, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if (result.data)
            {
                u32 fileSize32 = (u32)fileSize.QuadPart;
                if (ReadFile(fileHandle, result.data, fileSize32, (LPDWORD)&result.count, 0))
                {
                    assert(fileSize32 == result.count);
                }
                else
                {
                    Unload(result);
                }
            }
        }
        CloseHandle(fileHandle);
    }
    return result;
}

char* LoadText(char* fileName)
{
    char* result = 0;
    HANDLE fileHandle = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER fileSize;
        if (GetFileSizeEx(fileHandle, &fileSize))
        {
            assert(fileSize.QuadPart <= UINT32_MAX); // less than 4GB
            result = (char*)VirtualAlloc(0, fileSize.QuadPart + 1, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if (result)
            {
                u32 fileSize32 = (u32)fileSize.QuadPart;
                u32 bytesRead;
                if (ReadFile(fileHandle, result, fileSize32, (LPDWORD)&bytesRead, 0))
                {
                    assert(fileSize32 == bytesRead);
                    result[bytesRead] = 0;
                }
                else
                {
                    Unload(result);
                }
            }
        }
        CloseHandle(fileHandle);
    }
    return result;
}

bool SaveData(char* fileName, Buffer buffer)
{
    bool result = false;
    HANDLE fileHandle = CreateFile(fileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD bytesRead;
        BOOL canWrite = WriteFile(fileHandle, buffer.data, (DWORD)buffer.count, &bytesRead, 0);
        if (canWrite)
        {
            result = buffer.count == bytesRead;
        }
        CloseHandle(fileHandle);
    }
    return result;
}

bool SaveText(char* fileName, char* text)
{
    bool result = false;
    HANDLE fileHandle = CreateFile(fileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD bytesRead;
        DWORD bytesToWrite = (DWORD)strlen(text);
        BOOL canWrite = WriteFile(fileHandle, text, bytesToWrite, &bytesRead, 0);
        if (canWrite)
            result = bytesToWrite == bytesRead;
        CloseHandle(fileHandle);
    }
    return result;
}

bool FileExists(char* fileName)
{
    return GetFileAttributes(fileName) != INVALID_FILE_ATTRIBUTES;
}

bool IsFileExtension(char* fileName, char* ext)
{
    const char* fileExt = GetFileExt(fileName);
    return strcmp(fileExt, ext) == 0; 
}

char* GetFileExt(char* fileName)
{
    char* dot = strchr(fileName, '.');
    if (!dot || dot == fileName) return NULL;
    return (dot + 1);
}


static char *strprbrk(char *s, char *charset)
{
    char *latestMatch = NULL;
    for (; s = strpbrk(s, charset), s != NULL; latestMatch = s++) { }
    return latestMatch;
}

char* GetFileName(char* filePath)
{
    char* fileName = NULL;
    if (filePath != NULL) fileName = strprbrk(filePath, "\\/");
    if (!fileName || (fileName == filePath)) return filePath;
    return fileName + 1;
}

char* GetFileNameWithoutExt(char* filePath)
{
#define MAX_FILENAMEWITHOUTEXT_LENGTH 128
    
    static char fileName[MAX_FILENAMEWITHOUTEXT_LENGTH];
    memset(fileName, 0, MAX_FILENAMEWITHOUTEXT_LENGTH);
    
    if (filePath != NULL) strcpy(fileName, GetFileName(filePath)); // Get filename with extension
    
    size_t len = strlen(fileName);
    
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