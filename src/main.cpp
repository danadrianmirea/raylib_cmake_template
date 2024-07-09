#include <raylib.h>
#include "game.h"
#include "globals.h"

int main()
{

    Game game = Game();

    InitWindow(screenWidth, screenHeight, "Adrian's raylib template");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        // TODO add render to texture, minimal starter UI
        BeginDrawing();
        ClearBackground(darkGreen);
        game.Update();
        game.Draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}