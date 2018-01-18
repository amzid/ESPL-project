#ifndef CONTROL_H
#define CONTROL_H

#include "includes.h"


void controlGameState(Game* game);

typedef struct{
    uint8_t currentState;
    uint8_t lastState;
} Button;

#endif