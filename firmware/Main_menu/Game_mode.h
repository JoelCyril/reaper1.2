#ifndef GAME_MODE_H
#define GAME_MODE_H

#include <Arduino.h>

// Constants
#define GROUND_Y 44
#define JUMP_HEIGHT 10
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Game Variables
extern int gameDinoPosX;
extern int gameDinoPosY;
extern int gameCactusPosX;
extern int gameScore;

// Function Declarations
void gameSetup();
void gameLoop();
void exitGameMode();
void startJump();

#endif
