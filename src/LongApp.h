/* date = September 1st 2021 11:43 pm */

#ifndef _LONG_APP_H
#define _LONG_APP_H
#include "LongTypes.h"

typedef struct Color
{
    u8 r, g, b, a;
} Color;

typedef struct Sprite
{
    Color* data;
    v2u pivot;
    u32 width;
    u32 height;
} Sprite;

typedef struct Monitor Monitor;
struct Monitor
{
    u32 width;
    u32 height;
    u32 physicalWidth;
    u32 physicalHeight;
    u32 refreshRate;
};

typedef struct Window Window;
struct Window
{
    union
    {
        struct
        {
            v2i pos;
            v2u size;
        };
        RectI rect;
    };
    char* title;
    u32 flags;
};

typedef struct AppDesc AppDesc;
struct AppDesc
{
    void (*Start)(void);
    void (*Update)(void);
    void (*Quit)(void);
    Window window;
    Sprite drawTarget;
    f32 desiredFrameRate;
};

typedef struct AppData AppData;
struct AppData
{
    Window window;
    Sprite drawTarget;
    RectI viewTarget;
    Color backgroundColor;
};

typedef enum {
    KEY_NULL            = 0,        // Key: NULL, used for no key pressed
    
    KEY_LMB = 0x01,
    KEY_RMB = 0x02,
    KEY_MMB = 0x04,
    
    // Alphanumeric keys
    KEY_APOSTROPHE      = 39,       // Key: '
    KEY_COMMA           = 44,       // Key: ,
    KEY_MINUS           = 45,       // Key: -
    KEY_PERIOD          = 46,       // Key: .
    KEY_SLASH           = 47,       // Key: /
    KEY_ZERO            = 48,       // Key: 0
    KEY_ONE             = 49,       // Key: 1
    KEY_TWO             = 50,       // Key: 2
    KEY_THREE           = 51,       // Key: 3
    KEY_FOUR            = 52,       // Key: 4
    KEY_FIVE            = 53,       // Key: 5
    KEY_SIX             = 54,       // Key: 6
    KEY_SEVEN           = 55,       // Key: 7
    KEY_EIGHT           = 56,       // Key: 8
    KEY_NINE            = 57,       // Key: 9
    KEY_SEMICOLON       = 59,       // Key: ;
    KEY_EQUAL           = 61,       // Key: =
    
    // This is just ASCII
    KEY_A               = 65,       // Key: A | a
    KEY_B               = 66,       // Key: B | b
    KEY_C               = 67,       // Key: C | c
    KEY_D               = 68,       // Key: D | d
    KEY_E               = 69,       // Key: E | e
    KEY_F               = 70,       // Key: F | f
    KEY_G               = 71,       // Key: G | g
    KEY_H               = 72,       // Key: H | h
    KEY_I               = 73,       // Key: I | i
    KEY_J               = 74,       // Key: J | j
    KEY_K               = 75,       // Key: K | k
    KEY_L               = 76,       // Key: L | l
    KEY_M               = 77,       // Key: M | m
    KEY_N               = 78,       // Key: N | n
    KEY_O               = 79,       // Key: O | o
    KEY_P               = 80,       // Key: P | p
    KEY_Q               = 81,       // Key: Q | q
    KEY_R               = 82,       // Key: R | r
    KEY_S               = 83,       // Key: S | s
    KEY_T               = 84,       // Key: T | t
    KEY_U               = 85,       // Key: U | u
    KEY_V               = 86,       // Key: V | v
    KEY_W               = 87,       // Key: W | w
    KEY_X               = 88,       // Key: X | x
    KEY_Y               = 89,       // Key: Y | y
    KEY_Z               = 90,       // Key: Z | z
    KEY_LEFT_BRACKET    = 91,       // Key: [
    KEY_BACKSLASH       = 92,       // Key: '\'
    KEY_RIGHT_BRACKET   = 93,       // Key: ]
    KEY_GRAVE           = 96,       // Key: `
    
    // Function keys
    KEY_SPACE           = 32,       // Key: Space
    KEY_ESCAPE          = 256,      // Key: Esc
    KEY_ENTER           = 257,      // Key: Enter
    KEY_TAB             = 258,      // Key: Tab
    KEY_BACKSPACE       = 259,      // Key: Backspace
    KEY_INSERT          = 260,      // Key: Ins
    KEY_DELETE          = 261,      // Key: Del
    KEY_RIGHT           = 262,      // Key: Cursor right
    KEY_LEFT            = 263,      // Key: Cursor left
    KEY_DOWN            = 264,      // Key: Cursor down
    KEY_UP              = 265,      // Key: Cursor up
    KEY_PAGE_UP         = 266,      // Key: Page up
    KEY_PAGE_DOWN       = 267,      // Key: Page down
    KEY_HOME            = 268,      // Key: Home
    KEY_END             = 269,      // Key: End
    KEY_CAPS_LOCK       = 280,      // Key: Caps lock
    KEY_SCROLL_LOCK     = 281,      // Key: Scroll down
    KEY_NUM_LOCK        = 282,      // Key: Num lock
    KEY_PRINT_SCREEN    = 283,      // Key: Print screen
    KEY_PAUSE           = 284,      // Key: Pause
    KEY_F1              = 290,      // Key: F1
    KEY_F2              = 291,      // Key: F2
    KEY_F3              = 292,      // Key: F3
    KEY_F4              = 293,      // Key: F4
    KEY_F5              = 294,      // Key: F5
    KEY_F6              = 295,      // Key: F6
    KEY_F7              = 296,      // Key: F7
    KEY_F8              = 297,      // Key: F8
    KEY_F9              = 298,      // Key: F9
    KEY_F10             = 299,      // Key: F10
    KEY_F11             = 300,      // Key: F11
    KEY_F12             = 301,      // Key: F12
    KEY_LEFT_SHIFT      = 340,      // Key: Shift left
    KEY_LEFT_CONTROL    = 341,      // Key: Control left
    KEY_LEFT_ALT        = 342,      // Key: Alt left
    KEY_LEFT_SUPER      = 343,      // Key: Super left
    KEY_RIGHT_SHIFT     = 344,      // Key: Shift right
    KEY_RIGHT_CONTROL   = 345,      // Key: Control right
    KEY_RIGHT_ALT       = 346,      // Key: Alt right
    KEY_RIGHT_SUPER     = 347,      // Key: Super right
    KEY_KB_MENU         = 348,      // Key: KB menu
    
    // Keypad keys
    KEY_KP_0            = 320,      // Key: Keypad 0
    KEY_KP_1            = 321,      // Key: Keypad 1
    KEY_KP_2            = 322,      // Key: Keypad 2
    KEY_KP_3            = 323,      // Key: Keypad 3
    KEY_KP_4            = 324,      // Key: Keypad 4
    KEY_KP_5            = 325,      // Key: Keypad 5
    KEY_KP_6            = 326,      // Key: Keypad 6
    KEY_KP_7            = 327,      // Key: Keypad 7
    KEY_KP_8            = 328,      // Key: Keypad 8
    KEY_KP_9            = 329,      // Key: Keypad 9
    KEY_KP_DECIMAL      = 330,      // Key: Keypad .
    KEY_KP_DIVIDE       = 331,      // Key: Keypad /
    KEY_KP_MULTIPLY     = 332,      // Key: Keypad *
    KEY_KP_SUBTRACT     = 333,      // Key: Keypad -
    KEY_KP_ADD          = 334,      // Key: Keypad +
    KEY_KP_ENTER        = 335,      // Key: Keypad Enter
    KEY_KP_EQUAL        = 336,      // Key: Keypad =
} KeyboardKey;

enum WindowFlag
{
    FLAG_VSYNC_HINT         = 0x00000040,   // Set to try enabling V-Sync on GPU
    FLAG_FULLSCREEN_MODE    = 0x00000002,   // Set to run program in fullscreen
    FLAG_WINDOW_UNDECORATED = 0x00000008,   // Set to disable window decoration (frame and buttons)
    FLAG_WINDOW_HIDDEN      = 0x00000080,   // Set to hide window
    FLAG_WINDOW_MINIMIZED   = 0x00000200,   // Set to minimize window (iconify)
    FLAG_WINDOW_RESIZABLE   = 0x00000004,   // Set to allow resizable window
    FLAG_WINDOW_MAXIMIZED   = 0x00000400,   // Set to maximize window (expanded to monitor)
    FLAG_WINDOW_UNFOCUSED   = 0x00000800,   // Set to window non focused
    FLAG_WINDOW_TOPMOST     = 0x00001000,   // Set to window always on top
    FLAG_WINDOW_ALWAYS_RUN  = 0x00000100,   // Set to allow windows running while minimized
    FLAG_WINDOW_TRANSPARENT = 0x00000010,   // Set to allow transparent framebuffer
    FLAG_WINDOW_HIGHDPI     = 0x00002000,   // Set to support HighDPI
    FLAG_MSAA_4X_HINT       = 0x00000020,   // Set to try enabling MSAA 4X
    
    //FLAG_APP_STRETCH = , // Uses black bar or stretch when has a different aspect ratio
    //FLAG_APP_HIDE_CURSOR = ,
};

#define IsWindowFlag(flag) (currentWindow.flags & (flag))
#define SetWindowFlags(flags) currentWindow.flags |= (flags)
#define ClearWindowFlags(flags) currentWindow.flags &=  ~(flags)

#define LIGHTGRAY  (Color){ 200, 200, 200, 255 }
#define GRAY       (Color){ 130, 130, 130, 255 }
#define DARKGRAY   (Color){  80,  80,  80, 255 }
#define YELLOW     (Color){ 253, 249,   0, 255 }
#define GOLD       (Color){ 255, 203,   0, 255 }
#define ORANGE     (Color){ 255, 161,   0, 255 }
#define PINK       (Color){ 255, 109, 194, 255 }
#define RED        (Color){ 230,  41,  55, 255 }
#define MAROON     (Color){ 190,  33,  55, 255 }
#define GREEN      (Color){   0, 228,  48, 255 }
#define LIME       (Color){   0, 158,  47, 255 }
#define DARKGREEN  (Color){   0, 117,  44, 255 }
#define SKYBLUE    (Color){ 102, 191, 255, 255 }
#define BLUE       (Color){   0, 121, 241, 255 }
#define DARKBLUE   (Color){   0,  82, 172, 255 }
#define PURPLE     (Color){ 200, 122, 255, 255 }
#define VIOLET     (Color){ 135,  60, 190, 255 }
#define DARKPURPLE (Color){ 112,  31, 126, 255 }
#define BEIGE      (Color){ 211, 176, 131, 255 }
#define BROWN      (Color){ 127, 106,  79, 255 }
#define DARKBROWN  (Color){  76,  63,  47, 255 }
#define WHITE      (Color){ 255, 255, 255, 255 }
#define BLACK      (Color){   0,   0,   0, 255 }
#define BLANK      (Color){   0,   0,   0,   0 }
#define MAGENTA    (Color){ 255,   0, 255, 255 }
#define RAYWHITE   (Color){ 245, 245, 245, 255 }

AppDesc InitApp(i32 argc, char** argv);
Monitor GetMonitor(void);
AppData GetAppData(void);
void SetAppData(AppData* data);

// Input
bool IsDown(u32 key);
bool IsPressed(u32 key);
bool IsUp(u32 key);
v2i GetMousePos(void);
i16 GetScrollWheel(void);

// Timing
void ChangeDesiredFPS(f32 fps);
f32 GetFPS(void);
f64 GetDeltaTime(void);
f64 GetTime(void);
f64 GetMCPF(void); // Get mega cycle counts per frame

#if 0
// AUDIO
Buffer LoadSound(char* fileName);
u32 PlaySound(Buffer sound);
u32 PlaySoundEx(Buffer sound, f32 volume, f32 pitch, bool looping);
void StopSound(u32 sound);
void StopAllSounds(void);
bool IsPaused(u32 sound);
void SetPaused(u32 sound, bool paused);
#endif

// File management
Buffer LoadData(char* fileName);
char* LoadText(char* fileName);
bool SaveData(char* fileName, Buffer buffer);
bool SaveText(char* fileName, char* text);

bool FileExists(char* fileName);
bool IsFileExtension(char* fileName, char* ext);
char* GetFileExt(char* fileName);
char* GetFileName(char* filePath);
char* GetFileNameWithoutExt(char* filePath);

#endif //_LONG_APP_H
