#pragma once

class Ball
{
public:
    Ball();
    void Update(float dt);
    void Draw();

private:
    float x;
    float y;
    int speedX;
    int speedY;
    int radius;
};
