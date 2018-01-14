/**
 * This is the main file of the ESPLaboratory project.
 *
 * @author: Ahmed Mzid
 * 			Helmi Abidi
 *
 */

#include "includes.h"

#include "drawTask.h"


QueueHandle_t ESPL_RxQueue; // Already defined in ESPL_Functions.h
SemaphoreHandle_t ESPL_DisplayReady;

void initializeVehicle(Vehicle* vehicle);
void initializeRoad(Road* road, Vehicle* ego);
void fillMap(Road* road, Map* map);

int main()
{
	// Initialize Board functions and graphics
	ESPL_SystemInit();

	//Create and initialize road and vehicles
	Vehicle ego;
	Vehicle bot;
	initializeVehicle(&ego);
	initializeVehicle(&bot);
	Road road;
	initializeRoad(&road,&ego);
	Map map;
	fillMap(&road,&map);

	DataToDraw dataToDraw = {&road, &ego, &bot, &map};

	// Initializes Tasks with their respective priority
	xTaskCreate(drawTask, "drawTask", STACK_SIZE, &dataToDraw, 4, NULL);

	// Start FreeRTOS Scheduler
	vTaskStartScheduler();

}

void initializeVehicle(Vehicle* vehicle)
{
	vehicle->rel.y = displaySizeY / 2;
	vehicle->rel.x = displaySizeX / 2;
	vehicle->v_x = 0;
	vehicle->v_y = 0;
	vehicle->currentRoadPoint = 0;
	vehicle->distanceFromCurrentRoadPoint = 0;
	vehicle->state = NO_COLLISION;
}

void initializeRoad(Road* road, Vehicle* ego)
{
	road->side = (double) SIDE;
	road->state = STRAIGHT_ROAD;
	for (int i = 0; i < LAP_POINTS; i++) {
		road->point[i].rel.x = displaySizeX / 2;
		road->point[i].rel.yaw = -45;
		road->point[i].distanceToNextRoadPoint = 8 * UNIT_ROAD_DISTANCE;
	}
	//Copy Road for second lap
	for (int i = ROAD_POINTS / 2; i < ROAD_POINTS + 4; i++) {
		road->point[i] = road->point[i - ROAD_POINTS / 2];
	}
	road->point[ego->currentRoadPoint].rel.y = displaySizeY / 2;
}

void fillMap(Road* road, Map* map)
{
	uint16_t offsetX = displaySizeX - MAP_SIZE_X + 60,
			offsetY = displaySizeY - MAP_SIZE_Y + 15;

	double mapX = offsetX, mapY = offsetY;
	uint16_t  dist = (road->point[0].distanceToNextRoadPoint * UNIT_ROAD_DISTANCE_IN_MAP / UNIT_ROAD_DISTANCE);
	map->point[0].x = (uint16_t) mapX;
	map->point[0].y = (uint16_t) mapY;
	map->orientation[0] = road->point[0].rel.yaw;

	// generate road map position
	for (int i = 1; i < MAP_POINTS; i++)
	{
		mapX +=  dist * cos((double) map->orientation[i-1] * 3.14 / 180);
		mapY +=  dist * sin((double) map->orientation[i-1] * 3.14 / 180);
		map->point[i].x = (uint16_t) mapX;
		map->point[i].y = (uint16_t) mapY;
		map->orientation[i] = map->orientation[i-1] + road->point[i].rel.yaw;
		dist = (road->point[i].distanceToNextRoadPoint * UNIT_ROAD_DISTANCE_IN_MAP / UNIT_ROAD_DISTANCE);
	}
}

/*
 *  Hook definitions needed for FreeRTOS to function.
 */
void vApplicationIdleHook() {
	while (TRUE) {
	};
}

void vApplicationMallocFailedHook() {
	while(TRUE) {
	};
}
