#include <includes.h>
#include "controlGameState.h"
#include "multiplayer.h"


void controlGameState(Game* game){
    char str[100];
    font_t font1;
    font1 = gdispOpenFont("DejaVuSans24*");

    while(TRUE) {

        sendviaUart(game);

        // Transitions of state START_MENU
        if (game->gameState == START_MENU){
           if(GPIO_ReadInputDataBit(OK_BUTTON_REGISTER, OK_BUTTON_PIN) == BUTTON_PRESSED) {
                game->menuState = MODE_CHOSEN;
               if(game->mode == SINGLE_MODE)
                   game->gameState = GAME_PLAYING;
           }
        }

    }
}