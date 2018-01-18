#include <includes.h>
#include "multiplayer.h"

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
    char buffer[21]; // Start byte, 6 data bytes, End byte
    uint8_t temp;
    while (TRUE)
    {
        temp = xQueueReceive(ESPL_RxQueue, &input, 2000);
        game->connectionState = temp;
        // decode package by buffer position
        switch(pos) {
            // start byte
            case 0:
                if(input != startByte)
                    break;
            case 1:
                // read received data in buffer
                buffer[pos] = input;
                pos++;
            case 2:
                if(input == stopByte)
                {
                    game->received_buffer = buffer[1];
                    if(game->gameState == START_MENU) {
                        if (buffer[1] == CTRL_CHOSEN && game->menuState == CTRL_CHOSEN &&
                            game->mode == MULTIPLAYER_MODE) {
                            sendviaUart( (uint8_t*) &(game->menuState), 1);
                            game->gameState = GAME_PLAYING;
                            xSemaphoreGive(game2rcv);
                            vTaskDelay(5);
                        }
                    }
                    else if(game->gameState == GAME_PLAYING && game->mode == MULTIPLAYER_MODE)
                    {
                        if(xSemaphoreTake(game2rcv, portMAX_DELAY) == pdTRUE) {
                            game->taktUART++;
                            if (game->controlState == SPEED_CTRL)
                                game->ego->v_x = ((uint8_t) buffer[1] - 255 / 2) * V_X_MAX / MAX_JOYSTICK_X;
                            else if (game->controlState == STEERING_CTRL) {
                                game->ego->a_y = ((uint8_t) buffer[1] - 255 / 2) * A_Y_MAX / MAX_JOYSTICK_Y;
                            }
                            xSemaphoreGive(game2rcv);
                            vTaskDelay(5);
                        }
                        else{
                            vTaskDelay(5);
                        }
                    }
                    pos = 0;
                }

        }
    }
}
