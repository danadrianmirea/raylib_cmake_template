#pragma once

#include <string>
#include "raylib.h"
#include "globals.h"

class Game
{
public:
    Game(int width, int height);
    ~Game();
    void InitGame();
    void Reset();
    void Update(float dt);
    void HandleInput();
    void UpdateUI();

    void Draw();
    void DrawUI();
    void DrawMainMenu();
    void DrawOptionsMenu();
    std::string FormatWithLeadingZeroes(int number, int width);
    void Randomize();

    static bool isMobile;

private:
    bool isInitialLaunch = true;
    bool isInExitMenu;
    bool lostWindowFocus = false;
    bool gameOver;
    bool isInMainMenu = false;
    bool isInOptionsMenu = false;
    int currentMenuSelection = 0;
    int optionsMenuSelection = 0;
    float soundVolume = 1.0f;
    float musicVolume = 1.0f;
    bool isDraggingSoundSlider = false;
    bool isDraggingMusicSlider = false;
    bool isInExitConfirmation = false;
    bool isMusicPlaying = false;

    float screenScale;
    RenderTexture2D targetRenderTex;
    Font font;

    int width;
    int height;

    float ballX;
    float ballY;
    int ballRadius;
    float ballSpeed;
    Color ballColor;

    Music backgroundMusic;
    Sound actionSound;
};