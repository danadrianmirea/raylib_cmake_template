#pragma once
#include <raylib.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))


extern Color darkGreen;
extern Color grey;
extern Color yellow;
extern int windowWidth;
extern int windowHeight;
extern const int gameScreenWidth;
extern const int gameScreenHeight;
extern bool exitWindow;
extern bool exitWindowRequested;
extern bool fullscreen;
extern float borderOffsetWidth;
extern float borderOffsetHeight;