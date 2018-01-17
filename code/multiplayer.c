#include <includes.h>
#include "multiplayer.h"

/*
 * Functions for UART connection
 */

void sendviaUart(Game* game) {
    UART_SendData(startByte);


    if(game->menuState == MODE_CHOSEN)
    {
   UART_SendData( (uint8_t) game->mode);

    }
    else
    {
        UART_SendData(BYTE_NOT_CHOSEN);
    }

    UART_SendData(stopByte);
}



void uartReceive(Game* game) {
    char input;
    uint8_t pos = 0;
    char buffer[8]; // Start byte, 6 data bytes, End byte
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
                //game->received_input = input;
                buffer[pos] = input;
                pos++;
                break;
            case 2:
                if(input == stopByte)
                {
                   // game->received_input=buffer[1] ;
                    if(buffer[1] != BYTE_NOT_CHOSEN)
                    {

                        if(buffer[1] == MULTIPLAYER_MODE && game->menuState == MODE_CHOSEN && game->mode == MULTIPLAYER_MODE)
                        {
                            game->gameState= GAME_PLAYING;
                        }
                    }
                }
                pos = 0;
        }
    }
}