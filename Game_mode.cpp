#include "Game_mode.h"
#include "globals.h" 


int gameDinoPosX = 10;
int gameDinoPosY = GROUND_Y;
int gameCactusPosX = SCREEN_WIDTH;
int gameScore = 0;
bool isJumping = false;
bool isGameOver = false;
int jumpVelocity = 0;  

void gameSetup() {

    gameDinoPosY = GROUND_Y;
    gameCactusPosX = SCREEN_WIDTH;
    gameScore = 0;
    isJumping = false;
    isGameOver = false;
    jumpVelocity = 0;
}


void gameLoop() {

    if (readButton(BUTTON_BACK) == LOW) {
        Serial.println("Exiting to main menu...");
        exitMode();
        return;  

    if (isGameOver) {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr); 
        u8g2.setCursor(30, 35);
        u8g2.print("GAME OVER");
        u8g2.sendBuffer();
        delay(2000);

     
        while (readButton(BUTTON_UP) == HIGH);  

        gameSetup();
        return;
    }


    if (readButton(BUTTON_UP) == LOW) {
        startJump();
    }


    if (isJumping) {
        gameDinoPosY += jumpVelocity; 
        jumpVelocity += 1; 
        
        if (gameDinoPosY >= GROUND_Y) {  
            gameDinoPosY = GROUND_Y;
            isJumping = false;
            jumpVelocity = 0;
        }
    }

    gameCactusPosX -= 2;
    if (gameCactusPosX < -5) { 
        gameCactusPosX = SCREEN_WIDTH;
        gameScore++;
    }

  
    if (gameCactusPosX < gameDinoPosX + 5 && gameCactusPosX > gameDinoPosX - 5 &&
        gameDinoPosY + 5 >= GROUND_Y - 2) {
        isGameOver = true;
    }

    
    u8g2.clearBuffer();  

    u8g2.drawLine(0, GROUND_Y + 1, SCREEN_WIDTH, GROUND_Y + 1);

    u8g2.drawBox(gameDinoPosX, gameDinoPosY, 5, 5);

    u8g2.drawBox(gameCactusPosX, GROUND_Y - 5, 5, 5);

    u8g2.setFont(u8g2_font_ncenB08_tr);  
    u8g2.setCursor(10, 10);  
    u8g2.print("Score: ");
    u8g2.print(gameScore);

    u8g2.sendBuffer(); 

}


void startJump() {
    if (!isJumping) {
        isJumping = true;
        jumpVelocity = -7;  
    }
}
