#ifndef DRAW_TASK_H
#define DRAW_TASK_H

#include "includes.h"

//Task
void drawTask(DataToDraw* dataToDraw);

//local functions
void drawBorder(Road* road, uint16_t indexCurrentPoint, uint8_t changeCurrentPoint, Vehicle* ego);
int calculateReference(Road* road, int index, int v_y);
void drawMap(Road* road, Vehicle* ego, Map* map);

#endif
