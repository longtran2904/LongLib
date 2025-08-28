#include "LongPlatform.h"
#include <windows.h>

static f64 deltaTime;
static f64 time;
static f32 fps;
static f64 mcpf;

static Sprite drawTarget;

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
    Platform platform = InitPlaform(0, 0);
    
    if (!platform.window.flags)
        platform.window.flags = WS_OVERLAPPEDWINDOW|WS_VISIBLE;
    
    // TODO: CreateWindowEx needs both width and height to be CW_USEDEFAULT
    // Handle the case when only either app.windowWidth or app.windowHeight
    bool useDefault = true;
    RECT rect = {
        .left = 0,
        .right =  platform.window.size.width,
        .top = 0,
        .bottom = platform.window.size.height
    };
    
    if (rect.right || rect.bottom)
    {
        AdjustWindowRect(&rect, platform.window.flags, 0);
        useDefault = false;
    }
    
    hwnd = CreateWindowEx(
                          0,
                          windowClass.lpszClassName,
                          platform.window.title,
                          platform.window.flags,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          useDefault ? CW_USEDEFAULT : rect.right - rect.left,
                          useDefault ? CW_USEDEFAULT : rect.bottom - rect.top,
                          0,
                          0,
                          hInstance,
                          0);
    
    if (!hwnd) return -1;
    
    if (!CheckSprite(platform.drawTarget))
    {
        drawTarget = (Sprite){ .data = 0, .width = platform.window.size.width, .height = platform.window.size.height };
        drawTarget.data = (Color*)VirtualAlloc(0, DataSize(drawTarget), MEM_COMMIT, PAGE_READWRITE);
        if (!drawTarget.data) return -1;
    }
    else
        drawTarget = platform.drawTarget;
    
    if ((platform.viewTarget.width == 0) || (platform.viewTarget.height == 0))
        platform.viewTarget = (RectI){ 0, 0, drawTarget.width, drawTarget.height };
    
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
    
    if (platform.Start)
        platform.Start();
    
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
        
        if (platform.Update)
            platform.Update();
        
        // TODO: Swap buffer, batch rendering, run at fixed frame rate, use vsync, etc
        HDC deviceContext = GetDC(hwnd);
        
        if (false)
            SetWindowPos(hwnd, 0, // TODO: replace 0 with window flags
                         platform.window.pos.x, platform.window.pos.y,
                         platform.window.size.width, platform.window.size.height,
                         SWP_NOOWNERZORDER|SWP_NOZORDER); // TODO: replace these temp flags with window flags
        
        StretchDIBits(deviceContext,
                      0, 0, platform.window.size.width, platform.window.size.height,
                      platform.viewTarget.x, platform.viewTarget.y, platform.viewTarget.width, platform.viewTarget.height,
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
    
    if (platform.Quit)
        platform.Quit();
    // Clean up other stuff
    
    return 0;
}
