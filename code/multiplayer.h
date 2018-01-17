#ifndef MULTIPLAYER_H
#define MULTIPLAYER_H

#include "includes.h"

#define BYTE_NOT_CHOSEN      0xFF

static const uint8_t    startByte = 0xAA,
                        stopByte  = 0x55;

void sendviaUart(Game* game);
void uartReceive(Game* game);

#endif