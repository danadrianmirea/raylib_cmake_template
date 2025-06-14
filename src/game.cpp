#include <vector>
#include <utility>
#include <string>
#include <cmath>  // For sqrtf

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

    font = LoadFontEx("data/PressStart2P-Regular.ttf", 64, 0, 0);

    // Load and setup background music
    backgroundMusic = LoadMusicStream("data/music.mp3");
    if (backgroundMusic.stream.buffer == NULL) {
        TraceLog(LOG_ERROR, "Failed to load music file: data/music.mp3");
    } else {
        TraceLog(LOG_INFO, "Music loaded successfully");
        SetMusicVolume(backgroundMusic, 0.5f);  // Set volume to 50%
        musicPlaying = false;
    }

    this->width = width;
    this->height = height;
    InitGame();
}

Game::~Game()
{
    UnloadRenderTexture(targetRenderTex);
    UnloadFont(font);
    UnloadMusicStream(backgroundMusic);
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
    ballX = width / 2;
    ballY = height / 2;
}

void Game::Update(float dt)
{
    if (dt == 0)
    {
        return;
    }

    screenScale = MIN((float)GetScreenWidth() / gameScreenWidth, (float)GetScreenHeight() / gameScreenHeight);
    UpdateUI();

    // Update music
    if (musicPlaying) {
        UpdateMusicStream(backgroundMusic);
        if (IsMusicStreamPlaying(backgroundMusic) == false) {
            TraceLog(LOG_WARNING, "Music stopped playing unexpectedly");
            musicPlaying = false;
        }
    }

    bool running = (firstTimeGameStart == false && paused == false && lostWindowFocus == false && isInExitMenu == false && gameOver == false);

    if (running)
    {
        HandleInput();
    }
}

void Game::HandleInput()
{
    float dt = GetFrameTime();

    if(!isMobile) { // desktop and web controls
        if(IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
            ballY -= ballSpeed * dt;
        }
    else if(IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
        ballY += ballSpeed * dt;
    }

    if(IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
        ballX -= ballSpeed * dt;
    }
        else if(IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
            ballX += ballSpeed * dt;
        }
    } 
    else // mobile controls
    {
        if(IsGestureDetected(GESTURE_DRAG) || IsGestureDetected(GESTURE_HOLD)) {
            // Get touch position in screen coordinates
            Vector2 touchPosition = GetTouchPosition(0);
            
            // Convert screen coordinates to game coordinates
            float gameX = (touchPosition.x - (GetScreenWidth() - (gameScreenWidth * screenScale)) * 0.5f) / screenScale;
            float gameY = (touchPosition.y - (GetScreenHeight() - (gameScreenHeight * screenScale)) * 0.5f) / screenScale;
            
            Vector2 ballCenter = { ballX, ballY };
            Vector2 direction = { gameX - ballCenter.x, gameY - ballCenter.y };
            
            // Normalize the direction vector
            float length = sqrtf(direction.x * direction.x + direction.y * direction.y);
            if(length > 0) {
                direction.x /= length;
                direction.y /= length;
                
                ballX += direction.x * ballSpeed * dt;
                ballY += direction.y * ballSpeed * dt;
            }
        }
    }
}

void Game::UpdateUI()
{
#ifndef EMSCRIPTEN_BUILD
    if (WindowShouldClose() || (IsKeyPressed(KEY_ESCAPE) && optionWindowRequested == false))
    {
        optionWindowRequested = true;
        isInExitMenu = true;
        return;
    }
#endif

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

    if(firstTimeGameStart ) {
        if(isMobile) {
            if(IsGestureDetected(GESTURE_TAP)) {
                firstTimeGameStart = false;
                if (!musicPlaying) {
                    PlayMusicStream(backgroundMusic);
                    if (IsMusicStreamPlaying(backgroundMusic)) {
                        TraceLog(LOG_INFO, "Music started playing");
                        musicPlaying = true;
                    } else {
                        TraceLog(LOG_ERROR, "Failed to start music playback");
                    }
                }
            }
        }
        else if(IsKeyDown(KEY_ENTER)) {
            firstTimeGameStart = false;
            if (!musicPlaying) {
                PlayMusicStream(backgroundMusic);
                if (IsMusicStreamPlaying(backgroundMusic)) {
                    TraceLog(LOG_INFO, "Music started playing");
                    musicPlaying = true;
                } else {
                    TraceLog(LOG_ERROR, "Failed to start music playback");
                }
            }
        }
    }

    if (optionWindowRequested)
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            // Handle menu selection based on current selection
            switch (currentMenuSelection) {
                case 0: // Continue
                    optionWindowRequested = false;
                    isInExitMenu = false;
                    if (!musicPlaying) {
                        PlayMusicStream(backgroundMusic);
                        if (IsMusicStreamPlaying(backgroundMusic)) {
                            TraceLog(LOG_INFO, "Music started playing");
                            musicPlaying = true;
                        } else {
                            TraceLog(LOG_ERROR, "Failed to start music playback");
                        }
                    }
                    break;
                case 1: // New game
                    Reset();
                    optionWindowRequested = false;
                    isInExitMenu = false;
                    if (!musicPlaying) {
                        PlayMusicStream(backgroundMusic);
                        if (IsMusicStreamPlaying(backgroundMusic)) {
                            TraceLog(LOG_INFO, "Music started playing");
                            musicPlaying = true;
                        } else {
                            TraceLog(LOG_ERROR, "Failed to start music playback");
                        }
                    }
                    break;
                case 2: // Options
                    // TODO: Implement options menu
                    break;
                case 3: // Quit game
                    exitWindow = true;
                    break;
            }
        }
        else if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
        {
            currentMenuSelection = (currentMenuSelection - 1 + 4) % 4;
        }
        else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
        {
            currentMenuSelection = (currentMenuSelection + 1) % 4;
        }
        else if (IsKeyPressed(KEY_ESCAPE))
        {
            optionWindowRequested = false;
            isInExitMenu = false;
        }

        // Handle mouse interaction
        Vector2 mousePos = GetMousePosition();
        // Convert screen coordinates to game coordinates
        float gameX = (mousePos.x - (GetScreenWidth() - (gameScreenWidth * screenScale)) * 0.5f) / screenScale;
        float gameY = (mousePos.y - (GetScreenHeight() - (gameScreenHeight * screenScale)) * 0.5f) / screenScale;

        // Menu item dimensions
        const int menuItemHeight = 40;
        const int menuStartY = gameScreenHeight / 2 - 80;
        const int menuStartX = gameScreenWidth / 2 - 100;
        const int menuItemWidth = 200;

        // Check if mouse is over any menu item
        for (int i = 0; i < 4; i++) {
            Rectangle menuItemRect = {
                (float)menuStartX,
                (float)(menuStartY + (i * menuItemHeight)),
                (float)menuItemWidth,
                (float)menuItemHeight
            };

            if (CheckCollisionPointRec({gameX, gameY}, menuItemRect)) {
                currentMenuSelection = i;
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    // Handle menu selection based on current selection
                    switch (currentMenuSelection) {
                        case 0: // Continue
                            optionWindowRequested = false;
                            isInExitMenu = false;
                            if (!musicPlaying) {
                                PlayMusicStream(backgroundMusic);
                                if (IsMusicStreamPlaying(backgroundMusic)) {
                                    TraceLog(LOG_INFO, "Music started playing");
                                    musicPlaying = true;
                                } else {
                                    TraceLog(LOG_ERROR, "Failed to start music playback");
                                }
                            }
                            break;
                        case 1: // New game
                            Reset();
                            optionWindowRequested = false;
                            isInExitMenu = false;
                            if (!musicPlaying) {
                                PlayMusicStream(backgroundMusic);
                                if (IsMusicStreamPlaying(backgroundMusic)) {
                                    TraceLog(LOG_INFO, "Music started playing");
                                    musicPlaying = true;
                                } else {
                                    TraceLog(LOG_ERROR, "Failed to start music playback");
                                }
                            }
                            break;
                        case 2: // Options
                            // TODO: Implement options menu
                            break;
                        case 3: // Quit game
                            exitWindow = true;
                            break;
                    }
                }
                break;
            }
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

    if (optionWindowRequested == false && lostWindowFocus == false && gameOver == false && IsKeyPressed(KEY_P))
    {
        paused = !paused;
    }
}

void Game::Draw()
{
    // render everything to a texture
    BeginTextureMode(targetRenderTex);
    ClearBackground(GRAY);

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
    DrawTextEx(font, "Adrian's Raylib Template", {300, 10}, 18, 2, yellow);

    if (optionWindowRequested)
    {
        DrawRectangleRounded({screenX + (float)(gameScreenWidth / 2 - 250), screenY + (float)(gameScreenHeight / 2 - 100), 500, 200}, 0.76f, 20, BLACK);
        
        const char* menuItems[] = {"Continue", "New game", "Options", "Quit game"};
        int yOffset = gameScreenHeight / 2 - 80;
        
        for (int i = 0; i < 4; i++) {
            Color textColor = (i == currentMenuSelection) ? YELLOW : WHITE;
            DrawText(menuItems[i], screenX + (gameScreenWidth / 2 - 100), yOffset + (i * 40), 20, textColor);
        }
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