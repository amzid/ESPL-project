#include <includes.h>
#include "controlGameState.h"
#include "reset.h"

void controlGameState(Game* game) {
    char str[100];
    font_t font1;
    font1 = gdispOpenFont("DejaVuSans24*");

    Button okButton, exitButton, resetButton, pauseButton;
    okButton.lastState = GPIO_ReadInputDataBit(OK_BUTTON_REGISTER, OK_BUTTON_PIN);
    exitButton.lastState = GPIO_ReadInputDataBit(EXIT_BUTTON_REGISTER, EXIT_BUTTON_PIN);
    resetButton.lastState = GPIO_ReadInputDataBit(RESET_BUTTON_REGISTER, RESET_BUTTON_PIN);
    pauseButton.lastState = GPIO_ReadInputDataBit(PAUSE_BUTTON_REGISTER, PAUSE_BUTTON_PIN);

    while (TRUE) {
        // update buttons
        okButton.currentState = GPIO_ReadInputDataBit(OK_BUTTON_REGISTER, OK_BUTTON_PIN);
        exitButton.currentState = GPIO_ReadInputDataBit(EXIT_BUTTON_REGISTER, EXIT_BUTTON_PIN);
        resetButton.currentState = GPIO_ReadInputDataBit(RESET_BUTTON_REGISTER, RESET_BUTTON_PIN);
        pauseButton.currentState = GPIO_ReadInputDataBit(PAUSE_BUTTON_REGISTER, PAUSE_BUTTON_PIN);

        // Transitions of state START_MENU
        switch(game->gameState){
            case START_MENU:
                switch(game->menuState){
                    case NOT_CHOSEN:
                        if(okButton.currentState == BUTTON_PRESSED && okButton.lastState == BUTTON_UNPRESSED) {
                            game->menuState = MODE_CHOSEN;
                            vTaskDelay(1000);
                        }
                        break;
                    case MODE_CHOSEN:
                        if(okButton.currentState == BUTTON_PRESSED && okButton.lastState == BUTTON_UNPRESSED) {
                            game->menuState = COURSE_CHOSEN;
                            if (game->mode == SINGLE_MODE)
                                game->gameState = GAME_PLAYING;
                            vTaskDelay(1000);
                        }
                        break;
                    case COURSE_CHOSEN:
                        if(okButton.currentState == BUTTON_PRESSED && okButton.lastState == BUTTON_UNPRESSED) {
                            game->menuState = CTRL_CHOSEN;
                            vTaskDelay(1000);
                        }
                        break;
                }
                break;

            case GAME_PLAYING:
                if(exitButton.currentState == BUTTON_PRESSED && exitButton.lastState == BUTTON_UNPRESSED) {
                    game->gameState = START_MENU;
                    game->menuState = NOT_CHOSEN;
                    game->controlState = SPEED_CTRL;
                    game->mode = SINGLE_MODE;

                    initializeVehicle(game->ego);
                    initializeVehicle(game->bot1);
                    initializeVehicle(game->bot2);
                    initializeVehicle(game->bot3);
                    initializeRoad(game->road,game->ego);
                    fillMap(game->road,game->map);
                }
                if(resetButton.currentState == BUTTON_PRESSED && resetButton.lastState == BUTTON_UNPRESSED) {
                    initializeVehicle(game->ego);
                    initializeVehicle(game->bot1);
                    initializeVehicle(game->bot2);
                    initializeVehicle(game->bot3);
                    initializeRoad(game->road,game->ego);
                    fillMap(game->road,game->map);
                }
                if(pauseButton.currentState == BUTTON_PRESSED && pauseButton.lastState == BUTTON_UNPRESSED) {
                    game->gameState = GAME_PAUSED;
                    vTaskDelay(500);
                }
                break;

            case GAME_PAUSED:
                if(pauseButton.currentState == BUTTON_PRESSED && pauseButton.lastState == BUTTON_UNPRESSED) {
                    game->gameState = GAME_PLAYING;
                    vTaskDelay(500);
                }
                if(exitButton.currentState == BUTTON_PRESSED && exitButton.lastState == BUTTON_UNPRESSED) {
                    game->gameState = START_MENU;
                    game->menuState = NOT_CHOSEN;
                    game->controlState = SPEED_CTRL;
                    game->mode = SINGLE_MODE;

                    initializeVehicle(game->ego);
                    initializeVehicle(game->bot1);
                    initializeVehicle(game->bot2);
                    initializeVehicle(game->bot3);
                    initializeRoad(game->road,game->ego);
                    fillMap(game->road,game->map);
                }
                if(resetButton.currentState == BUTTON_PRESSED && resetButton.lastState == BUTTON_UNPRESSED) {
                    game->gameState = GAME_PLAYING;
                    initializeVehicle(game->ego);
                    initializeVehicle(game->bot1);
                    initializeVehicle(game->bot2);
                    initializeVehicle(game->bot3);
                    initializeRoad(game->road,game->ego);
                    fillMap(game->road,game->map);
                }
                break;
        }

        // Update last state button
        okButton.lastState = okButton.currentState;
        exitButton.lastState = exitButton.currentState;
        resetButton.lastState = resetButton.currentState;
        pauseButton.lastState = pauseButton.currentState;
    }
}