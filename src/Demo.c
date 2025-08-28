void TestKeyBoard(u32 key, char* keyname)
{
    if (IsPressed(key) || IsDown(key) || IsUp(key))
        OutputDebugString(keyname);
    if (IsPressed(key))
        OutputDebugString(" is pressed\n");
    if (IsDown(key))
        OutputDebugString(" is down\n");
    if (IsUp(key))
        OutputDebugString(" is Up\n");
}

void Update(void)
{
    static f32 rotation = 0;
    rotation -= .05f;
    
#if 1
    RectI rect = { 300, 300, 100, 100 };
    DrawRectEx(rect, rotation, 1, WHITE);
    
    RectI rect2 = { 120, 120, 150, 150 };
    DrawRectEx(rect2, rotation, 1, GREEN);
#endif
    
    DrawCircle(30, 600, 100);
#if 1
    DrawCircleFast((v2i){ 500, 500 }, 100);
    DrawCircleFast((v2i){ 700, 500 }, 100);
    DrawCircleFast((v2i){ 900, 600 }, 200);
    DrawCircleFast((v2i){ 1200, 600 }, 200);
#else
    DrawCircleV((v2i){ 500, 500 }, 100);
    DrawCircleV((v2i){ 700, 500 }, 100);
    DrawCircleV((v2i){ 900, 600 }, 200);
    DrawCircleV((v2i){ 1200, 600 }, 200);
#endif
    
#if 1
    DrawLine(10, 20, 10, 200);
    DrawLine(10, 10, 100, 10);
    DrawLine(100, 600, 250, 650);
    DrawLine(150, 500, 200, 800);
    DrawLine(500, 300, 700, 500);
#endif
    
#if 1
    defaultColor = RED;
    DrawLine(200, 700, 300, 200);
    DrawLine(300, 900, 700, 200);
    DrawLine(200, 700, 400, 500);
    defaultColor = WHITE;
#endif
    
#if 1
    defaultColor = YELLOW;
    DrawLine(800, 700, 500, 200);
    DrawLine(900, 900, 400, 500);
    DrawLine(500, 700, 300, 500);
    defaultColor = WHITE;
#endif
    
#if 1
    defaultColor = BLUE;
    DrawLine(1000, 500, 500, 700);
    DrawLine(900, 300, 700, 800);
    DrawLine(700, 700, 500, 900);
    defaultColor = WHITE;
#endif
    
    if (IsPressed('F'))
        //currentWindow.flags ^= FLAG_FULLSCREEN_MODE;
        OutputDebugString("Toggle full screen\n");
    
#if 1
    TestKeyBoard('A', "A");
    TestKeyBoard('S', "S");
    TestKeyBoard(KEY_LMB, "LMB");
    TestKeyBoard(KEY_RMB, "RMB");
    TestKeyBoard(KEY_MMB, "MMB");
#endif
    
    v2i pos = GetMousePos();
    static char buffer[256];
    snprintf(buffer, 256, "Mouse: (%u, %u), Scroll: %d\n", pos.x, pos.y, GetScrollWheel());
    OutputDebugString(buffer);
}

AppDesc InitApp(i32 argc, char** argv)
{
    return (AppDesc){
        .Update = Update
    };
}