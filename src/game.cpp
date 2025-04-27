#include <vector>
#include <utility>
#include <string>

#include "raylib.h"
#include "globals.h"
#include "game.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

bool Game::isMobile = false;

Game::Game(int width, int height)
{
    firstTimeGameStart = true;

    ballX = width / 2;
    ballY = height / 2;
    ballRadius = 50;
    ballSpeed = 300.0f;
    ballColor = RED;

#ifdef __EMSCRIPTEN__
    // Check if we're running on a mobile device
    isMobile = EM_ASM_INT({
        return /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent);
    });
#endif

    targetRenderTex = LoadRenderTexture(gameScreenWidth, gameScreenHeight);
    SetTextureFilter(targetRenderTex.texture, TEXTURE_FILTER_BILINEAR); // Texture scale filter to use

    font = LoadFontEx("Font/monogram.ttf", 64, 0, 0);

    this->width = width;
    this->height = height;
    InitGame();
}

Game::~Game()
{
    UnloadRenderTexture(targetRenderTex);
    UnloadFont(font);
}

void Game::InitGame()
{
    isInExitMenu = false;
    paused = false;
    lostWindowFocus = false;
    gameOver = false;

    screenScale = MIN((float)GetScreenWidth() / gameScreenWidth, (float)GetScreenHeight() / gameScreenHeight);
}

void Game::Reset()
{
    InitGame();
}

void Game::Update(float dt)
{
    if (dt == 0)
    {
        return;
    }

    screenScale = MIN((float)GetScreenWidth() / gameScreenWidth, (float)GetScreenHeight() / gameScreenHeight);
    UpdateUI();

    bool running = (firstTimeGameStart == false && paused == false && lostWindowFocus == false && isInExitMenu == false && gameOver == false);

    if (running)
    {
        HandleInput();
    }
}

void Game::HandleInput()
{
    float dt = GetFrameTime();
    if(IsKeyDown(KEY_W)) {
        ballY -= ballSpeed * dt;
    }
    else if(IsKeyDown(KEY_S)) {
        ballY += ballSpeed * dt;
    }

    if(IsKeyDown(KEY_A)) {
        ballX -= ballSpeed * dt;
    }
    else if(IsKeyDown(KEY_D)) {
        ballX += ballSpeed * dt;
    }

}

void Game::UpdateUI()
{
#ifndef EMSCRIPTEN_BUILD
    if (WindowShouldClose() || (IsKeyPressed(KEY_ESCAPE) && exitWindowRequested == false))
    {
        exitWindowRequested = true;
        isInExitMenu = true;
        return;
    }

    if (IsKeyPressed(KEY_ENTER) && (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)))
    {
        if (fullscreen)
        {
            fullscreen = false;
            ToggleBorderlessWindowed();
        }
        else
        {
            fullscreen = true;
            ToggleBorderlessWindowed();
        }
    }
#endif

    if(firstTimeGameStart) {
        if(IsKeyDown(KEY_ENTER)) {
            firstTimeGameStart = false;
        }
    }


    if (exitWindowRequested)
    {
        if (IsKeyPressed(KEY_Y))
        {
            exitWindow = true;
        }
        else if (IsKeyPressed(KEY_N) || IsKeyPressed(KEY_ESCAPE))
        {
            exitWindowRequested = false;
            isInExitMenu = false;
        }
    }

    if (IsWindowFocused() == false)
    {
        lostWindowFocus = true;
    }
    else
    {
        lostWindowFocus = false;
    }

#ifndef EMSCRIPTEN_BUILD
    if (exitWindowRequested == false && lostWindowFocus == false && gameOver == false && IsKeyPressed(KEY_P))
#else
    if (exitWindowRequested == false && lostWindowFocus == false && gameOver == false && (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)))
#endif
    {
        if (paused)
        {
            paused = false;
        }
        else
        {
            paused = true;
        }
    }
}

void Game::Draw()
{
    // render everything to a texture
    BeginTextureMode(targetRenderTex);
    ClearBackground(black);

    DrawCircle(ballX, ballY, ballRadius, ballColor);

    DrawUI();

    EndTextureMode();

    // render the scaled frame texture to the screen
    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexturePro(targetRenderTex.texture, (Rectangle){0.0f, 0.0f, (float)targetRenderTex.texture.width, (float)-targetRenderTex.texture.height},
                   (Rectangle){(GetScreenWidth() - ((float)gameScreenWidth * screenScale)) * 0.5f, (GetScreenHeight() - ((float)gameScreenHeight * screenScale)) * 0.5f, (float)gameScreenWidth * screenScale, (float)gameScreenHeight * screenScale},
                   (Vector2){0, 0}, 0.0f, WHITE);
    EndDrawing();
}

void Game::DrawUI()
{
    float screenX = 0.0f;
    float screenY = 0.0f;

    // DrawRectangleRoundedLines({borderOffsetWidth, borderOffsetHeight, gameScreenWidth - borderOffsetWidth * 2, gameScreenHeight - borderOffsetHeight * 2}, 0.18f, 20, 2, yellow);
    DrawTextEx(font, "Adrian's Raylib Template", {300, 10}, 34, 2, yellow);

    if (exitWindowRequested)
    {
        DrawRectangleRounded({screenX + (float)(gameScreenWidth / 2 - 250), screenY + (float)(gameScreenHeight / 2 - 20), 500, 60}, 0.76f, 20, BLACK);
        DrawText("Are you sure you want to exit? [Y/N]", screenX + (gameScreenWidth / 2 - 200), screenY + gameScreenHeight / 2, 20, yellow);
    }
    else if (firstTimeGameStart)
    {
        DrawRectangleRounded({screenX + (float)(gameScreenWidth / 2 - 250), screenY + (float)(gameScreenHeight / 2 - 20), 500, 80}, 0.76f, 20, BLACK);
        if (isMobile) {
            DrawText("Tap to play", screenX + (gameScreenWidth / 2 - 60), screenY + gameScreenHeight / 2 + 10, 20, yellow);
        } else {
#ifndef EMSCRIPTEN_BUILD            
            DrawText("Press Enter to play", screenX + (gameScreenWidth / 2 - 100), screenY + gameScreenHeight / 2 - 10, 20, yellow);
            DrawText("Alt+Enter: toggle fullscreen", screenX + (gameScreenWidth / 2 - 120), screenY + gameScreenHeight / 2 + 30, 20, yellow);
#else
            DrawText("Press Enter to play", screenX + (gameScreenWidth / 2 - 100), screenY + gameScreenHeight / 2 + 10, 20, yellow);
#endif
        }
    }
    else if (paused)
    {
        DrawRectangleRounded({screenX + (float)(gameScreenWidth / 2 - 250), screenY + (float)(gameScreenHeight / 2 - 20), 500, 60}, 0.76f, 20, BLACK);
#ifndef EMSCRIPTEN_BUILD
        DrawText("Game paused, press P to continue", screenX + (gameScreenWidth / 2 - 200), screenY + gameScreenHeight / 2, 20, yellow);
#else
        if (isMobile) {
            DrawText("Game paused, tap to continue", screenX + (gameScreenWidth / 2 - 200), screenY + gameScreenHeight / 2, 20, yellow);
        } else {
            DrawText("Game paused, press P or ESC to continue", screenX + (gameScreenWidth / 2 - 200), screenY + gameScreenHeight / 2, 20, yellow);
        }
#endif
    }
    else if (lostWindowFocus)
    {
        DrawRectangleRounded({screenX + (float)(gameScreenWidth / 2 - 250), screenY + (float)(gameScreenHeight / 2 - 20), 500, 60}, 0.76f, 20, BLACK);
        DrawText("Game paused, focus window to continue", screenX + (gameScreenWidth / 2 - 200), screenY + gameScreenHeight / 2, 20, yellow);
    }
    else if (gameOver)
    {
        DrawRectangleRounded({screenX + (float)(gameScreenWidth / 2 - 250), screenY + (float)(gameScreenHeight / 2 - 20), 500, 60}, 0.76f, 20, BLACK);
        if (isMobile) {
            DrawText("Game over, tap to play again", screenX + (gameScreenWidth / 2 - 200), screenY + gameScreenHeight / 2, 20, yellow);
        } else {
            DrawText("Game over, press Enter to play again", screenX + (gameScreenWidth / 2 - 200), screenY + gameScreenHeight / 2, 20, yellow);
        }
    }
}

std::string Game::FormatWithLeadingZeroes(int number, int width)
{
    std::string numberText = std::to_string(number);
    int leadingZeros = width - numberText.length();
    numberText = std::string(leadingZeros, '0') + numberText;
    return numberText;
}

void Game::Randomize()
{
}