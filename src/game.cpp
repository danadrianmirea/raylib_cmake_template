
#include <raylib.h>

#include "game.h"
#include "globals.h"

Game::Game()
{
    firstTimeGameStart = true;

    targetRenderTex = LoadRenderTexture(gameScreenWidth, gameScreenHeight);
    SetTextureFilter(targetRenderTex.texture, TEXTURE_FILTER_BILINEAR); // Texture scale filter to use

    InitGame();
}

Game::~Game()
{
    UnloadRenderTexture(targetRenderTex);
}

void Game::InitGame()
{
    isFirstFrameAfterReset = true;
    screenScale = MIN((float)GetScreenWidth() / gameScreenWidth, (float)GetScreenHeight() / gameScreenHeight);
}

void Game::Reset()
{
    InitGame();
}

void Game::Update(float dt)
{
    screenScale = MIN((float)GetScreenWidth() / gameScreenWidth, (float)GetScreenHeight() / gameScreenHeight);
    UpdateUI();

    bool running = (firstTimeGameStart == false && paused == false && lostWindowFocus == false && isInExitMenu == false && gameOver == false);
    if (running)
    {
        HandleInput();
        ball.Update(dt);
    }
}

void Game::HandleInput()
{
    if (isFirstFrameAfterReset)
    {
        isFirstFrameAfterReset = false;
        return;
    }

    /*
        if (IsKeyDown(KEY_LEFT))
        {

        }
        else if (IsKeyDown(KEY_RIGHT))
        {

        }
    */
}

void Game::UpdateUI()
{
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
            SetWindowSize(windowWidth, windowHeight);
        }
        else
        {
            fullscreen = true;
            SetWindowSize(windowWidth, windowHeight);
        }
        // ToggleFullscreen();
        ToggleBorderlessWindowed();
    }

    if (firstTimeGameStart && IsKeyPressed(KEY_SPACE))
    {
        firstTimeGameStart = false;
    }
    else if (gameOver && IsKeyPressed(KEY_SPACE))
    {
        Reset();
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

    if (exitWindowRequested == false && lostWindowFocus == false && gameOver == false && IsKeyPressed(KEY_P))
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
    ClearBackground(darkGreen);

    ball.Draw();
    DrawUI();

    EndTextureMode();

    // render the scaled frame texture to the screen
    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexturePro(targetRenderTex.texture, (Rectangle){0.0f, 0.0f, (float)targetRenderTex.texture.width, (float)-targetRenderTex.texture.height},
                   (Rectangle){(GetScreenWidth() - ((float)gameScreenWidth * screenScale)) * 0.5f, (GetScreenHeight() - ((float)gameScreenHeight * screenScale)) * 0.5f, (float)gameScreenWidth * screenScale, (float)gameScreenHeight * screenScale},
                   (Vector2){0, 0}, 0.0f, WHITE);

    DrawScreenSpaceUI();
    EndDrawing();
}

void Game::DrawUI()
{
    DrawRectangleRoundedLines({borderOffsetWidth, borderOffsetHeight, gameScreenWidth-borderOffsetWidth*2, gameScreenHeight-borderOffsetHeight*2}, 0.18f, 20, 2, yellow);
}

std::string Game::FormatWithLeadingZeroes(int number, int width)
{
    std::string numberText = std::to_string(number);
    int leadingZeros = width - numberText.length();
    numberText = std::string(leadingZeros, '0') + numberText;
    return numberText;
}

void Game::DrawScreenSpaceUI()
{
    if (exitWindowRequested)
    {
        DrawRectangleRounded({(float)(GetScreenWidth() / 2 - 500), (float)(GetScreenHeight() / 2 - 40), 1000, 120}, 0.76f, 20, BLACK);
        DrawText("Are you sure you want to exit? [Y/N]", GetScreenWidth() / 2 - 400, GetScreenHeight() / 2, 40, yellow);
    }
    else if (firstTimeGameStart)
    {
        DrawRectangleRounded({(float)(GetScreenWidth() / 2 - 500), (float)(GetScreenHeight() / 2 - 40), 1000, 120}, 0.76f, 20, BLACK);
        DrawText("Press SPACE to play", GetScreenWidth() / 2 - 200, GetScreenHeight() / 2, 40, yellow);
    }
    else if (paused)
    {
        DrawRectangleRounded({(float)(GetScreenWidth() / 2 - 500), (float)(GetScreenHeight() / 2 - 40), 1000, 120}, 0.76f, 20, BLACK);
        DrawText("Game paused, press P to continue", GetScreenWidth() / 2 - 400, GetScreenHeight() / 2, 40, yellow);
    }
    else if (lostWindowFocus)
    {
        DrawRectangleRounded({(float)(GetScreenWidth() / 2 - 500), (float)(GetScreenHeight() / 2 - 40), 1000, 120}, 0.76f, 20, BLACK);
        DrawText("Game paused, focus window to continue", GetScreenWidth() / 2 - 400, GetScreenHeight() / 2, 40, yellow);
    }
    else if (gameOver)
    {
        DrawRectangleRounded({(float)(GetScreenWidth() / 2 - 500), (float)(GetScreenHeight() / 2 - 40), 1000, 120}, 0.76f, 20, BLACK);
        DrawText("Game over, press SPACE to play again", GetScreenWidth() / 2 - 400, GetScreenHeight() / 2, 40, yellow);
    }
}
