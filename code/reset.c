#include <includes.h>
#include "includes.h"
#include "reset.h"

void initializeVehicle(Vehicle* vehicle)
{
    vehicle->a_x = 0;
    vehicle->a_y = 0;
    vehicle->v_x = 0;
    vehicle->v_y = 0;
    vehicle->currentRoadPoint = 0;
    vehicle->distanceFromCurrentRoadPoint = 0;
    vehicle->state = NO_COLLISION;
    switch(vehicle->color){
        case RED:
            vehicle->rel.y = displaySizeY / 2;
            vehicle->rel.x = displaySizeX / 2;
            vehicle->distanceFromCurrentRoadPoint = 30;
            break;
        case GREEN:
            vehicle->a_y_0 = 1;
            vehicle->v_y_max_straight_road = 13;
            vehicle->absEffect = 1.5;
            vehicle->rel.x = 10;
            vehicle->distanceFromCurrentRoadPoint = 90;
            break;
        case YELLOW:
            vehicle->a_y_0 = 1.2;
            vehicle->v_y_max_straight_road = 12;
            vehicle->absEffect = 1.2;
            vehicle->rel.x = 50;
            vehicle->distanceFromCurrentRoadPoint = 60;
            break;
        case BLUE:
            vehicle->a_y_0 = 1.4;
            vehicle->v_y_max_straight_road = 12;
            vehicle->absEffect = 1.5;
            vehicle->rel.x = 115;
            vehicle->distanceFromCurrentRoadPoint = 0;
            break;
    }
}

void initializeRoad(Road* road, Vehicle* ego, int index) {
    road->side = (double) SIDE;
    road->state = STRAIGHT_ROAD;
    road->point[0].rel.yaw = 45;

    for (int i = 0; i < LAP_POINTS + 1; i++) {
        road->point[i].rel.x = displaySizeX / 2;
        road->point[i].distanceToNextRoadPoint = 4 * UNIT_ROAD_DISTANCE;
    }

    switch (index){
        case 0:
            road->point[1].rel.yaw = -45;
            road->point[2].rel.yaw = 45;
            road->point[3].rel.yaw = 45;
            road->point[4].rel.yaw = -45;
            road->point[5].rel.yaw = 45;
            road->point[6].rel.yaw = 45;
            road->point[7].rel.yaw = 45;
            road->point[8].rel.yaw = 45;
            road->point[9].rel.yaw = -45;
            road->point[10].rel.yaw = 45;
            road->point[11].rel.yaw = 45;
            road->point[12].rel.yaw = -45;
            road->point[13].rel.yaw = 45;
            road->point[14].rel.yaw = 45;
            road->point[15].rel.yaw = 45;
            road->point[ROAD_POINTS / 2].rel.yaw = 45;
            break;
        case 1:
            road->point[1].rel.yaw = 45;
            road->point[2].rel.yaw = 0;
            road->point[3].rel.yaw = 45;
            road->point[4].rel.yaw = 0;
            road->point[5].rel.yaw = 45;
            road->point[6].rel.yaw = 0;
            road->point[7].rel.yaw = 45;
            road->point[8].rel.yaw = 0;
            road->point[9].rel.yaw = 45;
            road->point[10].rel.yaw = 0;
            road->point[11].rel.yaw = 45;
            road->point[12].rel.yaw = 0;
            road->point[13].rel.yaw = 45;
            road->point[14].rel.yaw = 0;
            road->point[15].rel.yaw = 45;
            road->point[ROAD_POINTS / 2].rel.yaw = 0;
            break;
        case 2:
            road->point[1].rel.yaw = 45;
            road->point[2].rel.yaw = -45;
            road->point[3].rel.yaw = -45;
            road->point[4].rel.yaw = 45;
            road->point[5].rel.yaw = 45;
            road->point[6].rel.yaw = 45;
            road->point[7].rel.yaw = 45;
            road->point[8].rel.yaw = 45;
            road->point[9].rel.yaw = 45;
            road->point[10].rel.yaw = -45;
            road->point[11].rel.yaw = -45;
            road->point[12].rel.yaw = 45;
            road->point[13].rel.yaw = 45;
            road->point[14].rel.yaw = 45;
            road->point[15].rel.yaw = 45;
            road->point[ROAD_POINTS / 2].rel.yaw = 45;
            break;
}
    //Copy Road for second lap
    for (int i = ROAD_POINTS / 2 + 1; i < ROAD_POINTS + 4; i++) {
        road->point[i] = road->point[i - ROAD_POINTS / 2];
    }
    road->point[ego->currentRoadPoint].rel.y = displaySizeY / 2;
}

void fillMap(Road* road, Map* map)
{
    double mapX = 0, mapY = 0;
    int16_t  dist = (road->point[0].distanceToNextRoadPoint * UNIT_ROAD_DISTANCE_IN_MAP / UNIT_ROAD_DISTANCE);
    map->point[0].x = (uint16_t) mapX;
    map->point[0].y = (uint16_t) mapY;
    map->orientation[0] = road->point[ROAD_POINTS/2].rel.yaw;

    // generate road map position
    for (int i = 1; i < MAP_POINTS; i++)
    {
        mapX +=  dist * cos((double) map->orientation[i-1] * 3.14 / 180);
        mapY +=  dist * sin((double) map->orientation[i-1] * 3.14 / 180);
        map->point[i].x = (int16_t) mapX;
        map->point[i].y = (int16_t) mapY;
        map->orientation[i] = map->orientation[i-1] + road->point[i].rel.yaw;
        dist = (road->point[i].distanceToNextRoadPoint * UNIT_ROAD_DISTANCE_IN_MAP / UNIT_ROAD_DISTANCE);
    }
}