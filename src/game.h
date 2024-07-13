#pragma once

#include <string>

#include "ball.h"

class Game
{
public:
    Game();
    ~Game();
    void InitGame();
    void Reset();
    Game(const Game &) = delete;
    const Game &operator=(const Game &g) = delete;
    Game(Game &&) = delete;
    Game &&operator=(Game &&g) = delete;

    void Update(float dt);
    void HandleInput();
    void UpdateUI();
    void Draw();
    void DrawUI();
    void DrawScreenSpaceUI();
    std::string FormatWithLeadingZeroes(int number, int width);


    bool firstTimeGameStart;
    bool isFirstFrameAfterReset;
    bool isInExitMenu;
    bool paused;
    bool lostWindowFocus;
    bool gameOver;


private:
    Ball ball;

    float screenScale;
    RenderTexture2D targetRenderTex;
    Font font;
};