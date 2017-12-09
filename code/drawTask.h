#ifndef DRAW_TASK_H
#define DRAW_TASK_H

#include "includes.h"

//Task
void drawTask(Road* road);

//local functions
void initializeRoad(Road* road);
void drawBorder(Road* road, uint16_t indexCurrentPoint, int* pOffsetX, int* pOffsetY);
int calculateReference(Road* road, int index, int v_y);


#endif
