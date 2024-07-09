#pragma once

class Game
{
public:
    Game();
    void Update();
    void Draw();

private:
    int x;
    int y;
    int speedX;
    int speedY;
    int radius;
};