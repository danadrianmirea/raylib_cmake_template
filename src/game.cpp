#include <vector>
#include <utility>
#include <string>
#include <cmath>
#include "raylib.h"
#include "globals.h"
#include "game.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

bool Game::isMobile = false;

Game::Game(int width, int height)
{
    isInitialLaunch = true;
    ballX = width / 2;
    ballY = height / 2;
    ballRadius = 50;
    ballSpeed = 300.0f;
    ballColor = RED;

#ifdef __EMSCRIPTEN__
    isMobile = EM_ASM_INT({
        return /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent);
    });
#endif

    targetRenderTex = LoadRenderTexture(gameScreenWidth, gameScreenHeight);
    SetTextureFilter(targetRenderTex.texture, TEXTURE_FILTER_BILINEAR);
    font = LoadFontEx("data/PressStart2P-Regular.ttf", 64, 0, 0);
    musicVolume = 0.10f;
    soundVolume = 0.5f;

    backgroundMusic = LoadMusicStream("data/music.mp3");
    if (backgroundMusic.stream.buffer == NULL) {
        TraceLog(LOG_ERROR, "Failed to load music file: data/music.mp3");
    } else {
        TraceLog(LOG_INFO, "Music loaded successfully");
        SetMusicVolume(backgroundMusic, musicVolume);
        isMusicPlaying = false;
    }

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
    isInMainMenu = true;
    isInitialLaunch = true;
    currentMenuSelection = 1; // on first game start, select new game, continue is not available
    screenScale = MIN((float)GetScreenWidth() / gameScreenWidth, (float)GetScreenHeight() / gameScreenHeight);

    PlayMusicStream(backgroundMusic);
}

void Game::Reset()
{
    isInExitMenu = false;
    lostWindowFocus = false;
    gameOver = false;
    isInMainMenu = false;
    isInitialLaunch = false;
    isMusicPlaying = true;
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

    if (isMusicPlaying) {
        UpdateMusicStream(backgroundMusic);
    }

    // Only run game logic if no menus are open and game is not paused
    bool running = (!lostWindowFocus && 
                   !isInMainMenu && 
                   !isInOptionsMenu && 
                   !isInExitConfirmation && 
                   !gameOver);

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

        if (IsKeyPressed(KEY_SPACE)) {
            if (actionSound.stream.buffer != NULL) {
                StopSound(actionSound);
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

void Game::UpdateMenu()
{
    if (isInMainMenu)
    {
        isMusicPlaying = false;
        // Track mouse movement for hover only
        static Vector2 lastMousePos = GetMousePosition();
        Vector2 currentMousePos = GetMousePosition();
        bool mouseMoved = (currentMousePos.x != lastMousePos.x || currentMousePos.y != lastMousePos.y);
        lastMousePos = currentMousePos;

        // Handle keyboard navigation
        if (IsKeyPressed(KEY_ENTER))
        {
            if (currentMenuSelection == 0) // Continue
            {
                isInMainMenu = false;
                isInitialLaunch = false;
                isMusicPlaying = true;
            }
            else if (currentMenuSelection == 1) // New Game
            {
                Reset();
            }
            else if (currentMenuSelection == 2) // Options
            {
                isInMainMenu = false;
                isInOptionsMenu = true;
            }
            else if (currentMenuSelection == 3) // Quit
            {
                isInExitConfirmation = true;
                isInMainMenu = false;
            }
        }
        else if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
        {
            currentMenuSelection = (currentMenuSelection - 1 + 4) % 4;
            if(isInitialLaunch && currentMenuSelection == 0) {
                currentMenuSelection = 3;
            }
        }
        else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
        {
            currentMenuSelection = (currentMenuSelection + 1) % 4;
            if(isInitialLaunch && currentMenuSelection == 0) {
                currentMenuSelection = 1;
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
            if (isInitialLaunch && i == 0) continue;  // Skip Continue option on first time

            Rectangle menuItemRect = {
                (float)menuStartX,
                (float)(menuStartY + i * menuItemHeight),
                (float)300,
                (float)menuItemHeight
            };

            // Handle hover only if mouse has moved
            if (mouseMoved && CheckCollisionPointRec({gameX, gameY}, menuItemRect))
            {
                currentMenuSelection = i;
            }

            // Handle clicks regardless of mouse movement
            if (CheckCollisionPointRec({gameX, gameY}, menuItemRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                currentMenuSelection = i;
                if (i == 0) // Continue
                {
                    isInMainMenu = false;
                    isInitialLaunch = false;
                    isMusicPlaying = true;
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
    // Handle options menu
    else if (isInOptionsMenu)
    {
        // Track mouse movement for hover only
        static Vector2 lastMousePos = GetMousePosition();
        Vector2 currentMousePos = GetMousePosition();
        bool mouseMoved = (currentMousePos.x != lastMousePos.x || currentMousePos.y != lastMousePos.y);
        lastMousePos = currentMousePos;

        if (IsKeyPressed(KEY_ENTER))
        {
            if (optionsMenuSelection == 2) // Back option
            {
                isInOptionsMenu = false;
                isInMainMenu = true;
                // Reset key repeat timers when leaving options menu
                leftKeyPressed = false;
                rightKeyPressed = false;
                upKeyPressed = false;
                downKeyPressed = false;
                leftKeyTimer = 0.0f;
                rightKeyTimer = 0.0f;
                upKeyTimer = 0.0f;
                downKeyTimer = 0.0f;
            }
        }
        // Handle key repeat for options menu navigation and volume adjustment
        float dt = GetFrameTime();
        
        // Handle UP/DOWN navigation with auto-repeat
        bool upPressed = IsKeyDown(KEY_UP) || IsKeyDown(KEY_W);
        bool downPressed = IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S);
        
        if (upPressed) {
            if (!upKeyPressed) {
                // First press
                optionsMenuSelection = (optionsMenuSelection - 1 + 3) % 3;
                upKeyPressed = true;
                upKeyTimer = 0.0f;
            } else {
                // Auto-repeat
                upKeyTimer += dt;
                if (upKeyTimer >= keyRepeatDelay) {
                    upKeyTimer = keyRepeatDelay - keyRepeatInterval;
                    optionsMenuSelection = (optionsMenuSelection - 1 + 3) % 3;
                }
            }
        } else {
            upKeyPressed = false;
            upKeyTimer = 0.0f;
        }
        
        if (downPressed) {
            if (!downKeyPressed) {
                // First press
                optionsMenuSelection = (optionsMenuSelection + 1) % 3;
                downKeyPressed = true;
                downKeyTimer = 0.0f;
            } else {
                // Auto-repeat
                downKeyTimer += dt;
                if (downKeyTimer >= keyRepeatDelay) {
                    downKeyTimer = keyRepeatDelay - keyRepeatInterval;
                    optionsMenuSelection = (optionsMenuSelection + 1) % 3;
                }
            }
        } else {
            downKeyPressed = false;
            downKeyTimer = 0.0f;
        }
        
        // Handle LEFT/RIGHT volume adjustment with auto-repeat
        bool leftPressed = IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A);
        bool rightPressed = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D);
        
        if (optionsMenuSelection == 0) // Sound Volume
        {
            if (leftPressed) {
                if (!leftKeyPressed) {
                    // First press
                    soundVolume = fmaxf(0.0f, soundVolume - 0.05f);
                    SetSoundVolume(actionSound, soundVolume);
                    if (actionSound.stream.buffer != NULL) {
                        StopSound(actionSound);
                        PlaySound(actionSound);
                    }
                    leftKeyPressed = true;
                    leftKeyTimer = 0.0f;
                } else {
                    // Auto-repeat
                    leftKeyTimer += dt;
                    if (leftKeyTimer >= keyRepeatDelay) {
                        leftKeyTimer = keyRepeatDelay - keyRepeatInterval;
                        soundVolume = fmaxf(0.0f, soundVolume - 0.05f);
                        SetSoundVolume(actionSound, soundVolume);
                        if (actionSound.stream.buffer != NULL) {
                            StopSound(actionSound);
                            PlaySound(actionSound);
                        }
                    }
                }
            } else {
                leftKeyPressed = false;
                leftKeyTimer = 0.0f;
            }
            
            if (rightPressed) {
                if (!rightKeyPressed) {
                    // First press
                    soundVolume = fminf(1.0f, soundVolume + 0.05f);
                    SetSoundVolume(actionSound, soundVolume);
                    if (actionSound.stream.buffer != NULL) {
                        StopSound(actionSound);
                        PlaySound(actionSound);
                    }
                    rightKeyPressed = true;
                    rightKeyTimer = 0.0f;
                } else {
                    // Auto-repeat
                    rightKeyTimer += dt;
                    if (rightKeyTimer >= keyRepeatDelay) {
                        rightKeyTimer = keyRepeatDelay - keyRepeatInterval;
                        soundVolume = fminf(1.0f, soundVolume + 0.05f);
                        SetSoundVolume(actionSound, soundVolume);
                        if (actionSound.stream.buffer != NULL) {
                            StopSound(actionSound);
                            PlaySound(actionSound);
                        }
                    }
                }
            } else {
                rightKeyPressed = false;
                rightKeyTimer = 0.0f;
            }
        }
        else if (optionsMenuSelection == 1) // Music Volume
        {
            if (leftPressed) {
                if (!leftKeyPressed) {
                    // First press
                    musicVolume = fmaxf(0.0f, musicVolume - 0.05f);
                    SetMusicVolume(backgroundMusic, musicVolume);
                    leftKeyPressed = true;
                    leftKeyTimer = 0.0f;
                } else {
                    // Auto-repeat
                    leftKeyTimer += dt;
                    if (leftKeyTimer >= keyRepeatDelay) {
                        leftKeyTimer = keyRepeatDelay - keyRepeatInterval;
                        musicVolume = fmaxf(0.0f, musicVolume - 0.05f);
                        SetMusicVolume(backgroundMusic, musicVolume);
                    }
                }
            } else {
                leftKeyPressed = false;
                leftKeyTimer = 0.0f;
            }
            
            if (rightPressed) {
                if (!rightKeyPressed) {
                    // First press
                    musicVolume = fminf(1.0f, musicVolume + 0.05f);
                    SetMusicVolume(backgroundMusic, musicVolume);
                    rightKeyPressed = true;
                    rightKeyTimer = 0.0f;
                } else {
                    // Auto-repeat
                    rightKeyTimer += dt;
                    if (rightKeyTimer >= keyRepeatDelay) {
                        rightKeyTimer = keyRepeatDelay - keyRepeatInterval;
                        musicVolume = fminf(1.0f, musicVolume + 0.05f);
                        SetMusicVolume(backgroundMusic, musicVolume);
                    }
                }
            } else {
                rightKeyPressed = false;
                rightKeyTimer = 0.0f;
            }
        }

        // Handle mouse interaction for options menu
        Vector2 mousePos = GetMousePosition();
        float gameX = (mousePos.x - (GetScreenWidth() - (gameScreenWidth * screenScale)) * 0.5f) / screenScale;
        float gameY = (mousePos.y - (GetScreenHeight() - (gameScreenHeight * screenScale)) * 0.5f) / screenScale;

        // Slider dimensions
        const int menuStartY = gameScreenHeight / 2 - 120;
        const int menuStartX = gameScreenWidth / 2 - 200;
        const int menuItemHeight = 60;
        const int sliderWidth = 250;
        const int sliderHeight = 20;
        const int menuWidth = 500;
        const int menuHeight = 280;

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

        // Handle hover only if mouse has moved
        if (mouseMoved)
        {
            if (CheckCollisionPointRec({gameX, gameY}, soundSliderRect)) {
                optionsMenuSelection = 0;
            }
            else if (CheckCollisionPointRec({gameX, gameY}, musicSliderRect)) {
                optionsMenuSelection = 1;
            }
            else if (CheckCollisionPointRec({gameX, gameY}, backButtonRect)) {
                optionsMenuSelection = 2;
            }
        }

        // Handle slider dragging and clicks regardless of mouse movement
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec({gameX, gameY}, soundSliderRect)) {
                isDraggingSoundSlider = true;
                optionsMenuSelection = 0;
                // Play sound when starting to drag the sound slider
                if (actionSound.stream.buffer != NULL) {
                    StopSound(actionSound);
                    PlaySound(actionSound);
                }
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
        if (CheckCollisionPointRec({gameX, gameY}, backButtonRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            isInOptionsMenu = false;
            isInMainMenu = true;
            // Reset key repeat timers when leaving options menu
            leftKeyPressed = false;
            rightKeyPressed = false;
            upKeyPressed = false;
            downKeyPressed = false;
            leftKeyTimer = 0.0f;
            rightKeyTimer = 0.0f;
            upKeyTimer = 0.0f;
            downKeyTimer = 0.0f;
        }

        if (isDraggingSoundSlider) {
            float newVolume = (gameX - soundSliderRect.x) / soundSliderRect.width;
            newVolume = fmaxf(0.0f, fminf(1.0f, newVolume));
            if (newVolume != soundVolume) {  // Only play sound if volume actually changed
                soundVolume = newVolume;
                SetSoundVolume(actionSound, soundVolume);
                if (actionSound.stream.buffer != NULL) {
                    StopSound(actionSound);
                    PlaySound(actionSound);
                }
            }
        }
        else if (isDraggingMusicSlider) {
            musicVolume = (gameX - musicSliderRect.x) / musicSliderRect.width;
            musicVolume = fmaxf(0.0f, fminf(1.0f, musicVolume));
            SetMusicVolume(backgroundMusic, musicVolume);
        }
    }
}

void Game::UpdateUI()
{
    // Handle window close button (X)
    if (WindowShouldClose() && !isInExitConfirmation)
    {
        isInExitConfirmation = true;
        isInMainMenu = false;
        isInOptionsMenu = false;
        isMusicPlaying = false;
        return;
    }

    // Handle window focus first
    if (IsWindowFocused() == false)
    {
        lostWindowFocus = true;
        isMusicPlaying = false;
        return;
    }
    else
    {
        lostWindowFocus = false;
        isMusicPlaying = true;
    }

    // Handle exit confirmation dialog first
    if (isInExitConfirmation)
    {
        if(isMusicPlaying) 
        {
            isMusicPlaying = false;
        }

        if(isInitialLaunch)
        {
            isInitialLaunch = false;
        }

        if (IsKeyPressed(KEY_Y))
        {
            exitWindow = true;
        }
        else if (IsKeyPressed(KEY_N))
        {
            isInExitConfirmation = false;
            isInMainMenu = false;
            isInOptionsMenu = false;
            isMusicPlaying = true;
        }
        return;  // Skip other UI updates while in exit confirmation
    }

    // Handle ESC key for menu toggling
    if (IsKeyPressed(KEY_ESCAPE))
    {
        if (isInOptionsMenu)
        {
            // Close options menu and return to main menu
            isInOptionsMenu = false;
            isInMainMenu = true;
            isMusicPlaying = false;
            // Reset key repeat timers when leaving options menu
            leftKeyPressed = false;
            rightKeyPressed = false;
            upKeyPressed = false;
            downKeyPressed = false;
            leftKeyTimer = 0.0f;
            rightKeyTimer = 0.0f;
            upKeyTimer = 0.0f;
            downKeyTimer = 0.0f;
        }
        else if (!isInitialLaunch && !isInMainMenu)  // Only allow ESC to open menu if not first time and not already in menu
        {
            isInMainMenu = true;
            isMusicPlaying = false;
        }
        else if (isInMainMenu && !isInitialLaunch)  // Allow ESC to close menu if not first time
        {
            // Close main menu and return to gameplay
            isInMainMenu = false;
            isMusicPlaying = true;
            isInitialLaunch = false;
        }
    }
    UpdateMenu();
}

void Game::DrawUI()
{
    if (isInMainMenu)
    {
        DrawMainMenu();
    }
    else if (isInOptionsMenu)
    {
        DrawOptionsMenu();
    }
    else if (isInExitConfirmation)
    {
        DrawRectangleRounded({(float)(gameScreenWidth / 2 - 250), (float)(gameScreenHeight / 2 - 30), 500.0f, 60.0f}, 0.76f, 20, BLACK);
        DrawText("Are you sure you want to exit? (Y/N)", gameScreenWidth / 2 - 200, gameScreenHeight / 2 - 10, 20, WHITE);
    }
    else if (lostWindowFocus)
    {
        DrawRectangleRounded({(float)(gameScreenWidth / 2 - 250), (float)(gameScreenHeight / 2 - 30), 500.0f, 60.0f}, 0.76f, 20, BLACK);
        DrawText("Game paused, focus window to continue", gameScreenWidth / 2 - 200, gameScreenHeight / 2 - 10, 20, WHITE);
    }
    else if (gameOver)
    {
        DrawRectangleRounded({(float)(gameScreenWidth / 2 - 250), (float)(gameScreenHeight / 2 - 30), 500.0f, 60.0f}, 0.76f, 20, BLACK);
        DrawText("Game over, press Enter to play again", gameScreenWidth / 2 - 200, gameScreenHeight / 2, 20, YELLOW);
    }
}

void Game::DrawMainMenu()
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
        else if (i == 0 && isInitialLaunch)
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

void Game::DrawOptionsMenu()
{
    const int menuStartY = gameScreenHeight / 2 - 120;
    const int menuStartX = gameScreenWidth / 2 - 200;
    const int menuItemHeight = 60;
    const int sliderWidth = 250;
    const int sliderHeight = 20;
    const int menuWidth = 500;
    const int menuHeight = 280;

    // Draw menu background
    DrawRectangle(menuStartX - 10, menuStartY - 10, menuWidth, menuHeight, {0, 0, 0, 200});

    // Draw menu title
    DrawText("Options", menuStartX, menuStartY, 20, WHITE);

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
    DrawText(soundVolText, menuStartX + 150 + sliderWidth + 20, menuStartY + menuItemHeight, 20, WHITE);

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
    DrawText(musicVolText, menuStartX + 150 + sliderWidth + 20, menuStartY + menuItemHeight * 2, 20, WHITE);

    // Draw back button
    DrawText("Back", menuStartX, menuStartY + menuItemHeight * 3, 20, 
            (optionsMenuSelection == 2) ? YELLOW : WHITE);
}

void Game::Draw()
{
    // Render everything to the texture
    BeginTextureMode(targetRenderTex);
    ClearBackground(GRAY);
    DrawCircle(ballX, ballY, ballRadius, ballColor);
    DrawFPS(10, 10);
    DrawUI();
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