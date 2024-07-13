#include <raylib.h>
#include "game.h"
#include "globals.h"

int main()
{
    InitWindow(gameScreenWidth, gameScreenHeight, "Adrian's raylib template");


    InitAudioDevice();
    SetMasterVolume(0.22f);
    SetExitKey(KEY_NULL); // Disable KEY_ESCAPE to close window, X-button still works

    windowWidth = GetScreenWidth();
    windowHeight = GetScreenHeight();
    SetWindowSize(windowWidth, windowHeight);
    
    SetWindowPosition(50, 50);
    SetTargetFPS(144);

    Game game;
    ToggleBorderlessWindowed();
    float dt = 0.0f;

    while (!exitWindow)
    {
        dt = GetFrameTime();
        game.Update(dt);
        game.Draw();
    }

    CloseAudioDevice();
    CloseWindow();

    return 0;
}