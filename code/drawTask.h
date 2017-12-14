#ifndef DRAW_TASK_H
#define DRAW_TASK_H

#include "includes.h"

//Task
void drawTask(Road* road);

//local functions
void initializeRoad(Road* road);
void drawBorder(Road* road, uint16_t indexCurrentPoint, uint8_t changeCurrentPoint, Vehicle* ego);
int calculateReference(Road* road, int index, int v_y);
void drawMap(Road* road, uint16_t currIdx);

#endif
