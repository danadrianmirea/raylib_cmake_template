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

    // Initialize volume values
    musicVolume = 0.33f;  // 33% for music
    soundVolume = 1.0f;   // 100% for sound effects

    // Load and setup background music
    backgroundMusic = LoadMusicStream("data/music.mp3");
    if (backgroundMusic.stream.buffer == NULL) {
        TraceLog(LOG_ERROR, "Failed to load music file: data/music.mp3");
    } else {
        TraceLog(LOG_INFO, "Music loaded successfully");
        SetMusicVolume(backgroundMusic, musicVolume);
        PlayMusicStream(backgroundMusic);
        musicPlaying = true;
    }

    // Load action sound
    actionSound = LoadSound("data/action.mp3");
    if (actionSound.stream.buffer == NULL) {
        TraceLog(LOG_ERROR, "Failed to load sound file: data/action.mp3");
    } else {
        TraceLog(LOG_INFO, "Action sound loaded successfully");
        SetSoundVolume(actionSound, soundVolume);
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
    UnloadSound(actionSound);
}

void Game::InitGame()
{
    isInExitMenu = false;
    lostWindowFocus = false;
    gameOver = false;
    isInMainMenu = true;  // Show main menu only on first start
    firstTimeGameStart = true;  // Set first time flag
    currentMenuSelection = 1;  // Select New Game by default
    screenScale = MIN((float)GetScreenWidth() / gameScreenWidth, (float)GetScreenHeight() / gameScreenHeight);
}

void Game::Reset()
{
    isInExitMenu = false;
    lostWindowFocus = false;
    gameOver = false;
    isInMainMenu = false;  // Don't show menu on reset
    firstTimeGameStart = false;  // Not first time anymore

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
    }

    // Only run game logic if no menus are open and game is not paused
    bool running = (firstTimeGameStart == false && 
                   lostWindowFocus == false && 
                   !isInMainMenu && 
                   !isInOptionsMenu && 
                   gameOver == false);

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

        // Play sound when spacebar is pressed
        if (IsKeyPressed(KEY_SPACE)) {
            if (actionSound.stream.buffer != NULL) {
                StopSound(actionSound);  // Stop any previous playback
                PlaySound(actionSound);
            } 
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
    // Handle ESC key for menu toggling
    if (IsKeyPressed(KEY_ESCAPE))
    {
        if (isInOptionsMenu)
        {
            // Close all menus and return to gameplay
            isInOptionsMenu = false;
            isInMainMenu = false;
        }
        else if (!firstTimeGameStart)  // Only allow ESC to toggle menu if not first time
        {
            // Toggle main menu
            isInMainMenu = !isInMainMenu;
        }
    }

    // Handle exit confirmation dialog
    if (isInExitConfirmation)
    {
        if (IsKeyPressed(KEY_Y))
        {
            exitWindow = true;
        }
        else if (IsKeyPressed(KEY_N))
        {
            isInExitConfirmation = false;
        }
        return;  // Skip other UI updates while in exit confirmation
    }

    // Handle main menu
    if (isInMainMenu)
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            if (currentMenuSelection == 0) // Continue
            {
                isInMainMenu = false;
                firstTimeGameStart = false;
            }
            else if (currentMenuSelection == 1) // New Game
            {
                Reset();  // This will set isInMainMenu to false
            }
            else if (currentMenuSelection == 2) // Options
            {
                isInMainMenu = false;
                isInOptionsMenu = true;
                // No need to read volumes as we maintain them ourselves
            }
            else if (currentMenuSelection == 3) // Quit
            {
                isInExitConfirmation = true;
                isInMainMenu = false;
            }
        }
        else if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
        {
            if (firstTimeGameStart && currentMenuSelection == 1)  // If on New Game and first time
            {
                currentMenuSelection = 3;  // Skip Continue, go to Quit
            }
            else
            {
                currentMenuSelection = (currentMenuSelection - 1 + 4) % 4;
            }
        }
        else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
        {
            if (firstTimeGameStart && currentMenuSelection == 3)  // If on Quit and first time
            {
                currentMenuSelection = 1;  // Skip Continue, go to New Game
            }
            else
            {
                currentMenuSelection = (currentMenuSelection + 1) % 4;
            }
        }

        // Handle mouse interaction for main menu
        Vector2 mousePos = GetMousePosition();
        float gameX = (mousePos.x - (GetScreenWidth() - (gameScreenWidth * screenScale)) * 0.5f) / screenScale;
        float gameY = (mousePos.y - (GetScreenHeight() - (gameScreenHeight * screenScale)) * 0.5f) / screenScale;

        const int menuStartY = gameScreenHeight / 2 - 100;
        const int menuStartX = gameScreenWidth / 2 - 150;
        const int menuItemHeight = 50;

        // Check menu item hover and click
        for (int i = 0; i < 4; i++)
        {
            if (firstTimeGameStart && i == 0) continue;  // Skip Continue option on first time

            Rectangle menuItemRect = {
                (float)menuStartX,
                (float)(menuStartY + i * menuItemHeight),
                (float)300,
                (float)menuItemHeight
            };

            if (CheckCollisionPointRec({gameX, gameY}, menuItemRect))
            {
                currentMenuSelection = i;
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                {
                    if (i == 0) // Continue
                    {
                        isInMainMenu = false;
                    }
                    else if (i == 1) // New Game
                    {
                        isInMainMenu = false;
                        Reset();
                    }
                    else if (i == 2) // Options
                    {
                        isInMainMenu = false;
                        isInOptionsMenu = true;
                    }
                    else if (i == 3) // Quit
                    {
                        isInExitConfirmation = true;
                        isInMainMenu = false;
                    }
                }
            }
        }
    }
    // Handle options menu
    else if (isInOptionsMenu)
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            if (optionsMenuSelection == 2) // Back option
            {
                isInOptionsMenu = false;
                isInMainMenu = true;
            }
        }
        else if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
        {
            optionsMenuSelection = (optionsMenuSelection - 1 + 3) % 3;
        }
        else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
        {
            optionsMenuSelection = (optionsMenuSelection + 1) % 3;
        }
        else if (optionsMenuSelection == 0) // Sound Volume
        {
            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
            {
                soundVolume = fmaxf(0.0f, soundVolume - 0.05f);
                SetSoundVolume(actionSound, soundVolume);
            }
            else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
            {
                soundVolume = fminf(1.0f, soundVolume + 0.05f);
                SetSoundVolume(actionSound, soundVolume);
            }
        }
        else if (optionsMenuSelection == 1) // Music Volume
        {
            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
            {
                musicVolume = fmaxf(0.0f, musicVolume - 0.05f);
                SetMusicVolume(backgroundMusic, musicVolume);
            }
            else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
            {
                musicVolume = fminf(1.0f, musicVolume + 0.05f);
                SetMusicVolume(backgroundMusic, musicVolume);
            }
        }

        // Handle mouse interaction for options menu
        Vector2 mousePos = GetMousePosition();
        float gameX = (mousePos.x - (GetScreenWidth() - (gameScreenWidth * screenScale)) * 0.5f) / screenScale;
        float gameY = (mousePos.y - (GetScreenHeight() - (gameScreenHeight * screenScale)) * 0.5f) / screenScale;

        // Slider dimensions
        const int sliderWidth = 200;
        const int sliderHeight = 20;
        const int menuStartY = gameScreenHeight / 2 - 100;
        const int menuStartX = gameScreenWidth / 2 - 150;
        const int menuItemHeight = 50;

        // Sound volume slider
        Rectangle soundSliderRect = {
            (float)menuStartX + 150,
            (float)(menuStartY + menuItemHeight),
            (float)sliderWidth,
            (float)sliderHeight
        };

        // Music volume slider
        Rectangle musicSliderRect = {
            (float)menuStartX + 150,
            (float)(menuStartY + menuItemHeight * 2),
            (float)sliderWidth,
            (float)sliderHeight
        };

        // Back button
        Rectangle backButtonRect = {
            (float)menuStartX,
            (float)(menuStartY + menuItemHeight * 3),
            (float)sliderWidth + 150,
            (float)menuItemHeight
        };

        // Handle slider dragging
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec({gameX, gameY}, soundSliderRect)) {
                isDraggingSoundSlider = true;
                optionsMenuSelection = 0;
            }
            else if (CheckCollisionPointRec({gameX, gameY}, musicSliderRect)) {
                isDraggingMusicSlider = true;
                optionsMenuSelection = 1;
            }
        }
        else {
            isDraggingSoundSlider = false;
            isDraggingMusicSlider = false;
        }

        // Handle back button click
        if (CheckCollisionPointRec({gameX, gameY}, backButtonRect)) {
            optionsMenuSelection = 2;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                isInOptionsMenu = false;
                isInMainMenu = true;
            }
        }

        if (isDraggingSoundSlider) {
            soundVolume = (gameX - soundSliderRect.x) / soundSliderRect.width;
            soundVolume = fmaxf(0.0f, fminf(1.0f, soundVolume));
            SetSoundVolume(actionSound, soundVolume);
        }
        else if (isDraggingMusicSlider) {
            musicVolume = (gameX - musicSliderRect.x) / musicSliderRect.width;
            musicVolume = fmaxf(0.0f, fminf(1.0f, musicVolume));
            SetMusicVolume(backgroundMusic, musicVolume);
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
}

void Game::Draw()
{
    // Render everything to the texture
    BeginTextureMode(targetRenderTex);
    ClearBackground(GRAY);
    DrawCircle(ballX, ballY, ballRadius, ballColor);
    DrawFPS(10, 10);

    if (isInMainMenu)
    {
        const int menuStartY = gameScreenHeight / 2 - 100;
        const int menuStartX = gameScreenWidth / 2 - 150;
        const int menuItemHeight = 50;

        // Draw menu background
        DrawRectangle(menuStartX - 10, menuStartY - 10, 320, 220, {0, 0, 0, 200});

        // Draw menu items
        const char* menuItems[] = {"Continue", "New Game", "Options", "Quit Game"};
        for (int i = 0; i < 4; i++)
        {
            Color textColor;
            if (i == currentMenuSelection)
            {
                textColor = YELLOW;
            }
            else if (i == 0 && firstTimeGameStart)  // Gray out Continue on first start
            {
                textColor = DARKGRAY;
            }
            else
            {
                textColor = WHITE;
            }
            DrawText(menuItems[i], menuStartX, menuStartY + i * menuItemHeight, 20, textColor);
        }
    }
    else if (isInOptionsMenu)
    {
        const int menuStartY = gameScreenHeight / 2 - 100;
        const int menuStartX = gameScreenWidth / 2 - 150;
        const int menuItemHeight = 50;
        const int sliderWidth = 200;
        const int sliderHeight = 20;

        // Draw menu background
        DrawRectangle(menuStartX - 10, menuStartY - 10, 320, 220, {0, 0, 0, 200});

        // Draw menu title
        DrawText("Options", menuStartX, menuStartY - 40, 20, WHITE);

        // Draw sound volume slider
        DrawText("Sound Volume", menuStartX, menuStartY + menuItemHeight, 20, 
                (optionsMenuSelection == 0) ? YELLOW : WHITE);
        DrawRectangle(menuStartX + 150, menuStartY + menuItemHeight, sliderWidth, sliderHeight, GRAY);
        DrawRectangle(menuStartX + 150, menuStartY + menuItemHeight, 
                    sliderWidth * soundVolume, sliderHeight, 
                    (optionsMenuSelection == 0) ? YELLOW : WHITE);
        // Draw sound volume percentage
        char soundVolText[32];
        sprintf(soundVolText, "%d%%", (int)(soundVolume * 100));
        DrawText(soundVolText, menuStartX + 150 + sliderWidth + 10, menuStartY + menuItemHeight, 20, WHITE);

        // Draw music volume slider
        DrawText("Music Volume", menuStartX, menuStartY + menuItemHeight * 2, 20, 
                (optionsMenuSelection == 1) ? YELLOW : WHITE);
        DrawRectangle(menuStartX + 150, menuStartY + menuItemHeight * 2, sliderWidth, sliderHeight, GRAY);
        DrawRectangle(menuStartX + 150, menuStartY + menuItemHeight * 2, 
                    sliderWidth * musicVolume, sliderHeight, 
                    (optionsMenuSelection == 1) ? YELLOW : WHITE);
        // Draw music volume percentage
        char musicVolText[32];
        sprintf(musicVolText, "%d%%", (int)(musicVolume * 100));
        DrawText(musicVolText, menuStartX + 150 + sliderWidth + 10, menuStartY + menuItemHeight * 2, 20, WHITE);

        // Draw back button
        DrawText("Back", menuStartX, menuStartY + menuItemHeight * 3, 20, 
                (optionsMenuSelection == 2) ? YELLOW : WHITE);
    }
    else if (isInExitConfirmation)
    {
        DrawRectangleRounded({(float)(gameScreenWidth / 2 - 250), (float)(gameScreenHeight / 2 - 30), 500.0f, 60.0f}, 0.76f, 20, BLACK);
        DrawText("Are you sure you want to exit? (Y/N)", gameScreenWidth / 2 - 200, gameScreenHeight / 2 - 10, 20, WHITE);
    }
    else if (lostWindowFocus)
    {
        DrawRectangleRounded({(float)(gameScreenWidth / 2 - 250), (float)(gameScreenHeight / 2 - 30), 500.0f, 60.0f}, 0.76f, 20, BLACK);
        DrawText("Game paused, focus window to continue", gameScreenWidth / 2 - 200, gameScreenHeight / 2, 20, YELLOW);
    }
    else if (gameOver)
    {
        DrawRectangleRounded({(float)(gameScreenWidth / 2 - 250), (float)(gameScreenHeight / 2 - 30), 500.0f, 60.0f}, 0.76f, 20, BLACK);
        DrawText("Game over, press Enter to play again", gameScreenWidth / 2 - 200, gameScreenHeight / 2, 20, YELLOW);
    }
    EndTextureMode();

    // Draw the texture to the screen
    BeginDrawing();    
    ClearBackground(BLACK);
    DrawTexturePro(
        targetRenderTex.texture,
        {0, 0, (float)targetRenderTex.texture.width, (float)-targetRenderTex.texture.height},
        {(GetScreenWidth() - ((float)gameScreenWidth * screenScale)) * 0.5f,
            (GetScreenHeight() - ((float)gameScreenHeight * screenScale)) * 0.5f,
            (float)gameScreenWidth * screenScale,
            (float)gameScreenHeight * screenScale},
        {0, 0},
        0,
        WHITE);
    EndDrawing();
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