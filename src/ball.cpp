
#include <raylib.h>
#include "ball.h"
#include "globals.h"

Ball::Ball()
{
    speedX = GetRandomValue(500, 700);
    speedY = GetRandomValue(500, 700);
    radius = 15;

    x = GetRandomValue(radius + 50, gameScreenWidth - radius - 50);
    y = GetRandomValue(radius + 50, gameScreenWidth - radius - 0);
}

void Ball::Update(float dt)
{
    x += speedX * dt;
    y += speedY * dt;

    if (x + radius >= gameScreenWidth - borderOffsetWidth || x - radius <= borderOffsetWidth)
    {
        speedX *= -1;
    }

    if (y + radius >= gameScreenHeight - borderOffsetHeight || y - radius <= borderOffsetHeight)
    {
        speedY *= -1;
    }
}

void Ball::Draw()
{
    DrawCircle(x, y, radius, WHITE);
}
