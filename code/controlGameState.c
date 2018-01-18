#include <includes.h>
#include "controlGameState.h"


void controlGameState(Game* game) {
    char str[100];
    font_t font1;
    font1 = gdispOpenFont("DejaVuSans24*");

    Button okButton;
    okButton.lastState = GPIO_ReadInputDataBit(OK_BUTTON_REGISTER, OK_BUTTON_PIN);

    while (TRUE) {



        // update buttons
        okButton.currentState = GPIO_ReadInputDataBit(OK_BUTTON_REGISTER, OK_BUTTON_PIN);

        // Transitions of state START_MENU

        switch(game->gameState){
            case START_MENU:
                //
                switch(game->menuState){
                    case NOT_CHOSEN:
                        if(okButton.currentState == BUTTON_PRESSED &&
                            okButton.lastState == BUTTON_UNPRESSED) {
                            game->menuState = MODE_CHOSEN;
                            if (game->mode == SINGLE_MODE)
                                game->gameState = GAME_PLAYING;
                            vTaskDelay(1000);
                        }
                        break;
                    case MODE_CHOSEN:
                        if(okButton.currentState == BUTTON_PRESSED &&
                        okButton.lastState == BUTTON_UNPRESSED) {
                            game->menuState = CTRL_CHOSEN;
                        }
                        break;
                }
                break;
        }

        // Update last state button
        okButton.lastState = okButton.currentState;

    }
}