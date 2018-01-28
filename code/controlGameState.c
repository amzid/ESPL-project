#include <includes.h>
#include "controlGameState.h"
#include "reset.h"
#include "multiplayer.h"


void controlGameState(Game* game) {
    Button okButton, exitButton, resetButton, pauseButton;
    okButton.lastState = GPIO_ReadInputDataBit(OK_BUTTON_REGISTER, OK_BUTTON_PIN);
    exitButton.lastState = GPIO_ReadInputDataBit(EXIT_BUTTON_REGISTER, EXIT_BUTTON_PIN);
    resetButton.lastState = GPIO_ReadInputDataBit(RESET_BUTTON_REGISTER, RESET_BUTTON_PIN);
    pauseButton.lastState = GPIO_ReadInputDataBit(PAUSE_BUTTON_REGISTER, PAUSE_BUTTON_PIN);

    uint8_t valuesToSend[SIZE_VALUES_TO_SEND];
    for(int i=0; i<SIZE_VALUES_TO_SEND; i++)
        valuesToSend[i] = 0;
    volatile uint8_t lastGameStateOtherPlayer = game->gameStateOtherPlayer;

    while (TRUE) {

        if(game->connectionState == NOT_CONNECTED && (game->gameState == GAME_PLAYING))// || game->gameState == GAME_PAUSED))
            game->mode = SINGLE_MODE;

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
                            if (game->mode == SINGLE_MODE) {
                                game->gameState = GAME_PLAYING;
                                xTimerStart(xTimer, 0);
                            }
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
                    initializeRoad(game->road[game->chosenMap],game->ego,game->chosenMap);
                    game->firstTime = 1;
                    game->taktGame = 0;
                    game->taktUART = 0;
                    time_s = 0;
                    break;
                }
                if(resetButton.currentState == BUTTON_PRESSED && resetButton.lastState == BUTTON_UNPRESSED) {
                    vTaskSuspend(receiveHdl);
                    initializeVehicle(game->ego);
                    initializeVehicle(game->bot1);
                    initializeVehicle(game->bot2);
                    initializeVehicle(game->bot3);
                    initializeRoad(game->road[game->chosenMap],game->ego,game->chosenMap);
                    valuesToSend[0] = 51;
                    valuesToSend[1] = BYTE_RESET;
                    sendviaUart(valuesToSend, SIZE_VALUES_TO_SEND);
                    time_s = 0;
                    xTaskNotifyGive(drawHdl);
                    xTimerStart(xTimer, 0);
                    game->taktGame = 0;
                    game->taktUART = 0;
                    game->firstTime = 1;
                    //vTaskDelay(20);
                    vTaskResume(receiveHdl);
                    break;
                }
                if(pauseButton.currentState == BUTTON_PRESSED && pauseButton.lastState == BUTTON_UNPRESSED) {
                    game->gameState = GAME_PAUSED;
                    valuesToSend[0] = 32;
                    valuesToSend[1] = GAME_PAUSED;
                    sendviaUart(valuesToSend, SIZE_VALUES_TO_SEND);
                    xTimerStop(xTimer, 0);
                    xTaskNotifyGive(drawHdl);
                    vTaskDelay(2000);
                    break;
                }
                switch(game->gameStateOtherPlayer) {

                    case START_MENU:
                        if(lastGameStateOtherPlayer == GAME_PLAYING)
                            game->mode = SINGLE_MODE;
                        break;

                    case GAME_PAUSED:
                            game->gameState = GAME_PAUSED;
                            xTaskNotifyGive(drawHdl);
                        break;
                    case BYTE_RESET:
                        initializeVehicle(game->ego);
                        initializeVehicle(game->bot1);
                        initializeVehicle(game->bot2);
                        initializeVehicle(game->bot3);
                        initializeRoad(game->road[game->chosenMap],game->ego,game->chosenMap);
                        game->firstTime = 1;
                        game->taktGame = 0;
                        game->taktUART = 0;
                        time_s = 0;
                        xTaskNotifyGive(drawHdl);
                        xTimerStart(xTimer, 0);
                        break;
                }
                break;

            case GAME_PAUSED:
                if(pauseButton.currentState == BUTTON_PRESSED && pauseButton.lastState == BUTTON_UNPRESSED) {
                    game->gameState = GAME_PLAYING;
                    xTaskNotifyGive(drawHdl);
                    xTimerStart(xTimer, 0);
                    lastGameStateOtherPlayer = game->gameStateOtherPlayer;
                    vTaskDelay(2000);
                    break;
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
                    initializeRoad(game->road[game->chosenMap],game->ego,game->chosenMap);
                    game->firstTime = 1;
                    game->taktGame = 0;
                    game->taktUART = 0;
                    time_s = 0;
                    break;
                }
                switch(game->gameStateOtherPlayer) {
                    case START_MENU:
                        if(lastGameStateOtherPlayer == GAME_PAUSED)
                            game->mode = SINGLE_MODE;
                        break;
                    case GAME_PLAYING:
                        if(lastGameStateOtherPlayer==GAME_PAUSED)
                        {
                            game->gameState = GAME_PLAYING;
                            xTaskNotifyGive(drawHdl);
                        }
                        break;
                }
                break;
        }

        // Update last state button
        okButton.lastState = okButton.currentState;
        exitButton.lastState = exitButton.currentState;
        resetButton.lastState = resetButton.currentState;
        pauseButton.lastState = pauseButton.currentState;
        lastGameStateOtherPlayer = game->gameStateOtherPlayer;

        vTaskDelay(500);
    }
}