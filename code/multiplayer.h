#ifndef MULTIPLAYER_H
#define MULTIPLAYER_H

#include "includes.h"

static const uint8_t    startByte = 0xAA,
                        stopByte  = 0x55;

void sendviaUart(uint8_t* valueToSend, uint8_t nbValuesToSend);
void uartReceive(Game* game);

#endif