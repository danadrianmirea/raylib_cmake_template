#include "game.h"
#include <raylib.h>

Game::Game()
{
    x = 100;
    y = 100;
    speedX = 5;
    speedY = 5;
    radius = 15;
}

void Game::Update()
{
    x += speedX;
    y += speedY;

    if (x + radius >= GetScreenWidth() || x - radius <= 0)
        speedX *= -1;

    if (y + radius >= GetScreenHeight() || y - radius <= 0)
        speedY *= -1;
}

void Game::Draw()
{
    DrawCircle(x, y, radius, WHITE);
}