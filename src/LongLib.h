#pragma once

#include <stdint.h>
#include <stdbool.h>

// --------------------TO DO--------------------
// Maybe add borderless window?
// Add controller input
// Maybe add audio-related functions?
// Maybe add threading?

typedef struct Vector2
{
    float x;
    float y;
} Vector2;

typedef enum ShapeType
{
    RECTANGLE,
    TRIANGLE,
    CIRCLE,
    POLYGON,
    LINE,
} ShapeType;

typedef struct Rect
{
    Vector2 pos;
    Vector2 size;
} Rect;

typedef struct Circle
{
    Vector2 pos;
    float radius;
} Circle;

typedef struct Triangle
{
    Vector2 a;
    Vector2 b;
    Vector2 c;
} Triangle;

typedef struct Poly
{
    uint32_t verticesCount;
    Vector2* vertices;
} Poly;

typedef struct Shape
{
    ShapeType type;
    union
    {
        Rect rect;
        Triangle triangle;
        Circle circle;
        Poly polygon;
        struct
        {
            Vector2 a;
            Vector2 b;
        } line;
    };
} Shape;

typedef struct Color
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} Color;

typedef struct Sprite
{
    uint32_t width;
    uint32_t height;
    Color* data;
} Sprite;

typedef struct Texture
{
    uint32_t width;
    uint32_t height;
    uint32_t id;
} Texture;

#define FirstChar '!'
#define GetCharIndex(c) (int)(c - FirstChar)

typedef struct Font
{
    int id;
    int lineHeight;
    Texture bitmap;
    struct
    {
        Vector2 position;
        Vector2 size;
        Vector2 offset;
    } *charsInfo;
} Font;

typedef struct AppDesc
{
    const char* windowTitle;
    uint32_t windowWidth;
    uint32_t windowHeight;
    uint32_t windowFlags;
    float frameRate;
    bool vsync;
    bool isRunning;
    bool center;
    bool fullScreen;
    bool hideCursor;
    bool blackBar; // When change aspect ration, use black bar at rear of display or stretch the image
} AppDesc;

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

// Keyboard keys
typedef enum {
    // Alphanumeric keys
    KEY_APOSTROPHE      = 39,
    KEY_COMMA           = 44,
    KEY_MINUS           = 45,
    KEY_PERIOD          = 46,
    KEY_SLASH           = 47,
    KEY_ZERO            = 48,
    KEY_ONE             = 49,
    KEY_TWO             = 50,
    KEY_THREE           = 51,
    KEY_FOUR            = 52,
    KEY_FIVE            = 53,
    KEY_SIX             = 54,
    KEY_SEVEN           = 55,
    KEY_EIGHT           = 56,
    KEY_NINE            = 57,
    KEY_SEMICOLON       = 59,
    KEY_EQUAL           = 61,
    KEY_A               = 65,
    KEY_B               = 66,
    KEY_C               = 67,
    KEY_D               = 68,
    KEY_E               = 69,
    KEY_F               = 70,
    KEY_G               = 71,
    KEY_H               = 72,
    KEY_I               = 73,
    KEY_J               = 74,
    KEY_K               = 75,
    KEY_L               = 76,
    KEY_M               = 77,
    KEY_N               = 78,
    KEY_O               = 79,
    KEY_P               = 80,
    KEY_Q               = 81,
    KEY_R               = 82,
    KEY_S               = 83,
    KEY_T               = 84,
    KEY_U               = 85,
    KEY_V               = 86,
    KEY_W               = 87,
    KEY_X               = 88,
    KEY_Y               = 89,
    KEY_Z               = 90,
    
    // Function keys
    KEY_SPACE           = 32,
    KEY_ESCAPE          = 256,
    KEY_ENTER           = 257,
    KEY_TAB             = 258,
    KEY_BACKSPACE       = 259,
    KEY_INSERT          = 260,
    KEY_DELETE          = 261,
    KEY_RIGHT           = 262,
    KEY_LEFT            = 263,
    KEY_DOWN            = 264,
    KEY_UP              = 265,
    KEY_PAGE_UP         = 266,
    KEY_PAGE_DOWN       = 267,
    KEY_HOME            = 268,
    KEY_END             = 269,
    KEY_CAPS_LOCK       = 280,
    KEY_SCROLL_LOCK     = 281,
    KEY_NUM_LOCK        = 282,
    KEY_PRINT_SCREEN    = 283,
    KEY_PAUSE           = 284,
    KEY_F1              = 290,
    KEY_F2              = 291,
    KEY_F3              = 292,
    KEY_F4              = 293,
    KEY_F5              = 294,
    KEY_F6              = 295,
    KEY_F7              = 296,
    KEY_F8              = 297,
    KEY_F9              = 298,
    KEY_F10             = 299,
    KEY_F11             = 300,
    KEY_F12             = 301,
    KEY_LEFT_SHIFT      = 340,
    KEY_LEFT_CONTROL    = 341,
    KEY_LEFT_ALT        = 342,
    KEY_LEFT_SUPER      = 343,
    KEY_RIGHT_SHIFT     = 344,
    KEY_RIGHT_CONTROL   = 345,
    KEY_RIGHT_ALT       = 346,
    KEY_RIGHT_SUPER     = 347,
    KEY_KB_MENU         = 348,
    KEY_LEFT_BRACKET    = 91,
    KEY_BACKSLASH       = 92,
    KEY_RIGHT_BRACKET   = 93,
    KEY_GRAVE           = 96,
    
    // Keypad keys
    KEY_KP_0            = 320,
    KEY_KP_1            = 321,
    KEY_KP_2            = 322,
    KEY_KP_3            = 323,
    KEY_KP_4            = 324,
    KEY_KP_5            = 325,
    KEY_KP_6            = 326,
    KEY_KP_7            = 327,
    KEY_KP_8            = 328,
    KEY_KP_9            = 329,
    KEY_KP_DECIMAL      = 330,
    KEY_KP_DIVIDE       = 331,
    KEY_KP_MULTIPLY     = 332,
    KEY_KP_SUBTRACT     = 333,
    KEY_KP_ADD          = 334,
    KEY_KP_ENTER        = 335,
    KEY_KP_EQUAL        = 336
} KeyboardKey;

bool UpdateFrame(void);

// Window-related functions
void CreateWindow(AppDesc* app);
void ExitWindow(void);
bool IsFullScreen(void);
void ToggleFullScreen(void);
uint32_t GetWidth(void);
uint32_t GetHeight(void);
void ResizeWindow(uint32_t width, uint32_t height, bool center);
void ToggleBlackBar();
uint32_t GetMonitorWidth(void);
uint32_t GetMonitorHeight(void);
uint32_t GetMonitorRefreshRate(void);
void ShowMouseCursor(void);
void HideMouseCursor(void);
void SetMouseCursor(int cursor);
void LoadMouseCursor(const char* fileName);

// Input-related functions
bool IsKeyPressed(int key);                       // The key is currently being held
bool IsKeyDown(int key);                          // last frame up, this frame down, the player need to release and press the key again in order to make this return true again
bool IsKeyUp(int key);                            // last frame down, this frame up, the player need to press and release the key again in order to make this return true again
int GetKeyPressed(void);                          // Get key pressed (keycode), call it multiple times for keys queued
bool IsMouseButtonPressed(int button);            // Detect if a mouse button has been pressed once (last frame down and this frame up)
bool IsMouseButtonDown(int button);               // Detect if a mouse button is being pressed
bool IsMouseButtonUp(int button);                 // Detect if a mouse button is NOT being pressed
Vector2 GetMousePos(void);

// Timing functions
float GetFPS(void);
double GetTime(void);
float GetDeltaTime(void);

// Sprite functions
Sprite CreateSprite(uint32_t width, uint32_t height, Color backgroundColor);
Sprite LoadSprite(const char* fileName);                                                             // Load image from file into CPU memory (RAM)
void UnloadSprite(Sprite* sprite);                                                                   // Unload image from CPU memory (RAM)
bool ExportSprite(Sprite sprite, const char* fileName);                                              // Export image data to file, returns true on success
Sprite GetPartialSprite(uint32_t x, uint32_t y, uint32_t width, uint32_t height, Sprite sprite);
void DrawSpriteToSprite(uint32_t destX, uint32_t destY, Sprite destSprite, Sprite srcSprite);
Sprite ScaleSprite(Sprite sprite, float scale);

// Texture functions
Texture CreateTexture(Sprite sprite);
Texture LoadTexture(const char* fileName);
void UnloadTexture(Texture* texture);

// Font loading/unloading functions
Font LoadFont(const char* fileName); // ttf otf fnt
void UnloadFont(Font font);
extern Font defaultFont;
//void SetDefaultFont(Font font);
//Font GetDefaultFont(void);

// Text drawing functions
void DrawFPS(int posX, int posY);                                                               // Draw current FPS
void PrintText(const char* text, int posX, int posY, int fontSize, Color color);
void PrintTextEx(const char* text, Font font, Vector2 position, float spacing, float fontSize, Color color);

// Drawing functions
extern Color backgroundColor;
//void SetBackgroundColor(Color color);
//Color GetBackgroundColor(Color color);
void DrawSprite(Sprite sprite, float x, float y);
void DrawSpriteEx(Sprite sprite, Vector2 pos, Color tint);
void DrawTexture(Texture texture, float x, float y);
void DrawTextureV(Texture texture, Vector2 pos);
void DrawTextureQuad(Texture texture, Vector2 pos, Vector2 size);
void DrawTextureEx(Texture texture, Vector2 pos, Vector2 size, float rotation, Color tint);
void DrawPartialTexture(Texture texture, Vector2 pos, Vector2 size, Vector2 uvPos, Vector2 uvSize);
void DrawPartialTextureEx(Texture texture, Rect rect, Rect uvRect, float rotation, Color tint);
void DrawShape(Shape* shape);
void DrawShapeEx(Shape* shape, bool fill, Color color);

// Files management functions
uint8_t* LoadFileData(const char *fileName, uint32_t *bytesRead);     // Load file data as byte array (read)
void UnloadFileData(uint8_t* data);                                       // Unload file data allocated by LoadFileData()
bool SaveFileData(const char* fileName, void* data, uint32_t bytesToWrite); // Save data to file from byte array (write), returns true on success
char* LoadFileText(const char* fileName);                                       // Load text data from file (read), returns a '\0' terminated string
void UnloadFileText(char* text);                                       // Unload file text data allocated by LoadFileText()
bool SaveFileText(const char* fileName, char* text);                            // Save text data to file (write), string must be '\0' terminated, returns true on success
bool FileExists(const char* fileName);                                          // Check if file or directory exists
bool IsFileExtension(const char* fileName, const char* ext);                    // Check file extension (not including point: png, wav)
const char* GetFileExtension(const char* fileName);                             // Get pointer to extension for a filename string (includes dot: ".png")
const char* GetFileName(const char* filePath);                                  // Get pointer to filename for a path string
const char* GetFileNameWithoutExt(const char* filePath);                        // Get filename string without extension (uses static string)