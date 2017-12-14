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

int main()
{
	// Initialize Board functions and graphics
	ESPL_SystemInit();

	//Create and initialize road and vehicles
	Vehicle ego;
	initializeVehicle(&ego);
	Road road;
	initializeRoad(&road,&ego);
	Map map;

	DataToDraw dataToDraw = {&road, &ego, &map};

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
	road->side = SIDE;
	road->state = STRAIGHT_ROAD;
	for (int i = 0; i < LAP_POINTS; i++) {
		road->point[i].rel.x = displaySizeX / 2;
		road->point[i].rel.yaw = 45;
		road->point[i].distanceToNextRoadPoint = 8 * UNIT_ROAD_DISTANCE;
	}
	//Copy Road for second lap
	for (int i = ROAD_POINTS / 2; i < ROAD_POINTS + 4; i++) {
		road->point[i] = road->point[i - ROAD_POINTS / 2];
	}
	road->point[ego->currentRoadPoint].rel.y = displaySizeY / 2;
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
