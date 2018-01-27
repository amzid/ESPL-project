#include <includes.h>
#include "multiplayer.h"
#include "reset.h"

/*
 * Functions for UART connection
 */

void sendviaUart(uint8_t* valueToSend, uint8_t nbValuesToSend) {
    UART_SendData(startByte);
    for(int i=0; i<nbValuesToSend; i++)
        UART_SendData(valueToSend[i]);
    UART_SendData(stopByte);
}




void uartReceive(Game* game) {
    char input;
    uint8_t pos = 0;
    char buffer[10]; // Start byte, 6 data bytes, End byte
    uint8_t temp;
    while (TRUE) {
        temp = xQueueReceive(ESPL_RxQueue, &input, 2000);
        game->connectionState = temp;
        // decode package by buffer position
        switch (game->gameState) {
            case START_MENU:
                receiveInStartMenu(game, input, &pos, buffer);
                break;
            case GAME_PLAYING:
            case GAME_PAUSED:
                receiveWhileGamePlaying(game, input, &pos, buffer);
                break;
        }
    }
}


void receiveInStartMenu(Game* game, uint8_t input, uint8_t* pos, char buffer[5]){
    switch(*pos) {
        // start byte
        case 0:
            if (input != startByte)
                break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            // read received data in buffer
            buffer[*pos] = input;
            (*pos)++;
            break;
        case 6:
            if (input == stopByte) {
                game->received_buffer = (uint8_t) buffer[2];
                game->gameStateOtherPlayer = (uint8_t) buffer[2];
                if (buffer[1] >= MODE_CHOSEN) {
                    if (buffer[3] == SINGLE_MODE)
                        game->connectionState = NOT_CONNECTED;
                    else {
                        if (game->menuState >= MODE_CHOSEN && game->mode == MULTIPLAYER_MODE) {
                            if (buffer[1] >= COURSE_CHOSEN) {
                                game->menuState = COURSE_CHOSEN;
                                game->chosenMap = (ChosenMap) buffer[3];
                            }
                            if (buffer[1] == CTRL_CHOSEN) {
                                game->controlState = (buffer[5] + 1) % 2;
                                game->menuState = CTRL_CHOSEN;
                                uint8_t valuesToSend[5];
                                valuesToSend[0] = (uint8_t) (game->menuState);
                                valuesToSend[1] = (uint8_t) (game->gameState);
                                valuesToSend[2] = (uint8_t) (game->mode);
                                valuesToSend[3] = (uint8_t) (game->chosenMap);
                                valuesToSend[4] = (uint8_t) (game->controlState);
                                sendviaUart(valuesToSend,5);
                                vTaskDelay(50);
                                game->gameState = GAME_PLAYING;
                                xTaskNotifyGive(drawHdl);
                                vTaskDelay(5);
                            }
                        }
                    }
                }
                *pos = 0;
            }
    }
}


void receiveWhileGamePlaying(Game* game, uint8_t input, uint8_t* pos, char buffer[5]){
    //volatile static GameState lastGameStateOtherPlayer = START_MENU;
    switch(*pos) {
        // start byte
        case 0:
            if (input != startByte)
                break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            // read received data in buffer
            buffer[*pos] = input;
            (*pos)++;
            break;
        case 6:
            if (input == stopByte) {
                game->received_buffer = (uint8_t) buffer[1];
                if (game->mode == MULTIPLAYER_MODE) {
                    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
                    // First Buffer
                    game->taktUART++;
                    if (game->controlState == SPEED_CTRL) {
                        game->ego->v_x = ((uint8_t) buffer[1] - 255 / 2) * V_X_MAX / MAX_JOYSTICK_X;
                        game->bot1->v_y = buffer[3] + (double) buffer[4] / 100.0;
                    }
                    else if (game->controlState == STEERING_CTRL) {
                        game->ego->v_y = buffer[1] + (double) buffer[3] / 100.0;
                        game->bot1->v_x = buffer[4] + (double) buffer[5] / 100.0;
                    }
                    // Second buffer: game state of other player
                    game->gameStateOtherPlayer = (uint8_t) buffer[2];
                    xTaskNotifyGive(drawHdl);
                }
                *pos = 0;
            }
            break;
    }
}