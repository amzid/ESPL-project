#ifndef RESET_H
#define RESET_H

#include "includes.h"

void initializeVehicle(Vehicle* vehicle);
void initializeRoad(Road* road, Vehicle* ego);
void fillMap(Road* road, Map* map);

#endif