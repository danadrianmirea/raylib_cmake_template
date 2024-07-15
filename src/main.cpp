#include <raylib.h>
#include "game.h"
#include "globals.h"

int main()
{
    InitWindow(gameScreenWidth, gameScreenHeight, "Adrian's raylib template");
    //windowWidth = GetScreenWidth();
    //windowHeight = GetScreenHeight();
    //SetWindowSize(windowWidth, windowHeight);
    SetWindowPosition(50, 50);

    InitAudioDevice();
    SetMasterVolume(0.22f);
    SetExitKey(KEY_NULL);

    Game game;
    ToggleBorderlessWindowed();
    SetTargetFPS(144);

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