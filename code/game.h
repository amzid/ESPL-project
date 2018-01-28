#ifndef DRAW_TASK_H
#define DRAW_TASK_H

#include "includes.h"

extern char str[100];
extern font_t font1;

//Task
void drawTask(Game* game);

//local functions

void drawBorder(Road* road, uint16_t indexCurrentPoint, uint8_t changeCurrentPoint, Vehicle* ego, Border* border);
void drawWhiteBorder(Border* border, int side, TYPE_BORDER typeBorder);

uint8_t checkIfCollisionWithBorder(Border* border, double side);
double calcX(Border* border, int y, double side);

void drawMap(Road* road, Vehicle* ego, Vehicle* bot[NUM_BOTS], Map* map);
void drawVehiclePositionOnMap(Vehicle* vehicle, Map* map);

void drawBot(Vehicle* bot, Vehicle*  ego, Border* border, Road* road);
uint8_t updatePosition(Vehicle* vehicle, Road* road);

#endif
