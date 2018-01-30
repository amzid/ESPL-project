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
    char buffer[15]; // Start byte, 6 data bytes, End byte
    uint8_t temp;
    while (TRUE) {
        temp = xQueueReceive(ESPL_RxQueue, &input, 5000);
        game->connectionState = temp;
        // decode package by buffer position
        switch (game->gameState) {
            case START_MENU:
                receiveInStartMenu(game, input, &pos, buffer);
                break;
            case GAME_PLAYING:
            case GAME_PAUSED:
            case START_GAME:
                receiveWhileGamePlaying(game, input, &pos, buffer);
                break;
        }
    }
}


void receiveInStartMenu(Game* game, uint8_t input, uint8_t* pos, char buffer[15]){
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
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
            // read received data in buffer
            buffer[*pos] = input;
            (*pos)++;
            break;
        case 13:
            if (input == stopByte) {
                game->received_buffer = (uint8_t) buffer[1];
                game->gameStateOtherPlayer = (uint8_t) buffer[2];
                if (game->gameStateOtherPlayer == START_MENU && buffer[1] >= MODE_CHOSEN) {
                    game->modeOtherPlayer = buffer[3];
                    if(game->modeOtherPlayer == MULTIPLAYER_MODE) {
                        if (game->menuState >= MODE_CHOSEN && game->mode == MULTIPLAYER_MODE) {
                            if (buffer[1] >= COURSE_CHOSEN) {
                                if(game->menuState < COURSE_CHOSEN)
                                    game->menuState = COURSE_CHOSEN;
                                game->chosenMap = (ChosenMap) buffer[4];
                            }
                            if (buffer[1] == CTRL_CHOSEN) {
                                game->controlState = (buffer[5] + 1) % 2;
                                game->menuState = CTRL_CHOSEN;
                                if(buffer[12]!=1) {
                                    uint8_t valuesToSend[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
                                    valuesToSend[0] = (uint8_t) (game->menuState);
                                    valuesToSend[1] = (uint8_t) (game->gameState);
                                    valuesToSend[2] = (uint8_t) (game->mode);
                                    valuesToSend[3] = (uint8_t) (game->chosenMap);
                                    valuesToSend[4] = (uint8_t) (game->controlState);
                                    sendviaUart(valuesToSend, 12);
                                    vTaskDelay(20);
                                }
                                game->gameState = START_GAME;
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


void receiveWhileGamePlaying(Game* game, uint8_t input, uint8_t* pos, char buffer[15]){
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
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
            // read received data in buffer
            buffer[*pos] = input;
            (*pos)++;
            break;
        case 13:
            if (input == stopByte) {
                game->received_buffer = (uint8_t) buffer[1];
                if (game->mode == MULTIPLAYER_MODE) {
                    // Second buffer: game state of other player
                    game->gameStateOtherPlayer = (uint8_t) buffer[2];
                    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
                    if (game->gameStateOtherPlayer == GAME_PLAYING) {
                        // First Buffer
                        game->taktUART++;
                        if (game->controlState == SPEED_CTRL) {
                            game->ego->v_x = ((uint8_t) buffer[1] - 255 / 2) * V_X_MAX / MAX_JOYSTICK_X;
                            game->bot1->rel.x = buffer[3];
                            game->bot2->rel.x = buffer[4];
                            game->bot3->rel.x = buffer[5];
                            game->ego->currentRoadPoint = buffer[6];
                            game->ego->distanceFromCurrentRoadPoint = *((uint16_t *) (buffer + 7));
                            game->road[game->chosenMap]->side = (double) buffer[9];
                        } else if (game->controlState == STEERING_CTRL) {
                            game->ego->v_y = buffer[1] + (double) buffer[3] / 100.0;
                            game->bot1->currentRoadPoint = buffer[4];
                            game->bot1->distanceFromCurrentRoadPoint = *((uint16_t *) (buffer + 5));
                            game->bot2->currentRoadPoint = buffer[7];
                            game->bot2->distanceFromCurrentRoadPoint = *((uint16_t *) (buffer + 8));
                            game->bot3->currentRoadPoint = buffer[10];
                            game->bot3->distanceFromCurrentRoadPoint = *((uint16_t *) (buffer + 11));
                        }
                    } else if (game->gameStateOtherPlayer == START_GAME) {
                        game->controlState = (buffer[5] + 1) % 2;
                    }
                    if (game->gameState == GAME_PLAYING){
                        if(game->gameStateOtherPlayer == GAME_PLAYING)
                            xTaskNotifyGive(drawHdl);
                        else{
                            uint8_t valuesToSend[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
                            valuesToSend[1] = (uint8_t) (game->gameState);
                            sendviaUart(valuesToSend, 12);
                            if (game->gameStateOtherPlayer == START_GAME)
                                time_s = 0;
                            xTaskNotifyGive(receiveHdl);
                        }
                    }
                    else
                        xTaskNotifyGive(drawHdl);
                    }
                *pos = 0;
            }
            break;
    }
}