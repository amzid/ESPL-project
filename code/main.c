/**
 * This is the main file of the ESPLaboratory project.
 *
 * @author: Ahmed Mzid
 * 			Helmi Abidi
 *
 */

#include "includes.h"

#define ROAD_POINTS 1024
#define STACK_SIZE 1000

#define ROAD_SIZE 150

QueueHandle_t ESPL_RxQueue; // Already defined in ESPL_Functions.h
SemaphoreHandle_t ESPL_DisplayReady;

static const uint16_t displaySizeX = 320,
					  displaySizeY = 240;

typedef struct {
	int x;
	int y;
	short int yaw;
} coord;

typedef struct
{
	coord abs;
	coord rel;
}RoadPoint;

typedef struct
{
	RoadPoint point[ROAD_POINTS];
} Road;

#define NB_CORNERS_BORDER 4
typedef enum {CORNER_D_L,CORNER_H_L,CORNER_H_R,CORNER_D_R} Corner;

void mainTask(Road* road);

int main()
{
	// Initialize Board functions and graphics
	ESPL_SystemInit();

	Road road;

	// Initializes Tasks with their respective priority
	xTaskCreate(mainTask, "mainTask", STACK_SIZE, &road, 4, NULL);

	// Start FreeRTOS Scheduler
	vTaskStartScheduler();

}

void initializeRoad(Road* road)
{
	for(int i=0; i<ROAD_POINTS; i++)
	{
		road->point[i].abs.x = displaySizeX/2;
		road->point[i].abs.y = (i+1) * displaySizeY/2;
		road->point[i].rel.x = displaySizeX/2;
		road->point[i].rel.yaw = (2*i) % 360;
	}
}

void drawBorder(RoadPoint point)
{
	uint8_t side = 60;
	uint8_t outSizeX = 10,
			inSizeX = 6,
			outSizeY = displaySizeY/2,
			inSizeY = displaySizeY/4,
			offsetX = displaySizeX/2 - 20;
	double yaw_rad = 3.14 * point.rel.yaw / 180;
	struct point corners[2][NB_CORNERS_BORDER];

	corners[0][CORNER_D_L].x = side+1;
	corners[0][CORNER_D_L].y = displaySizeY/2;
	corners[0][CORNER_H_L].x = round(corners[0][CORNER_D_L].x + displaySizeY/2 * sin(yaw_rad));
	corners[0][CORNER_H_L].y = 0;
	corners[0][CORNER_H_R].x = corners[0][CORNER_H_L].x + outSizeX;
	corners[0][CORNER_H_R].y = corners[0][CORNER_H_L].y;
	corners[0][CORNER_D_R].x = corners[0][CORNER_D_L].x + outSizeX;
	corners[0][CORNER_D_R].y = corners[0][CORNER_D_L].y;

	corners[1][CORNER_D_L].x = (corners[0][CORNER_H_L].x + corners[0][CORNER_D_L].x)/2 +2;
	corners[1][CORNER_D_L].y = (corners[0][CORNER_H_L].y + corners[0][CORNER_D_L].y)/2;
	corners[1][CORNER_H_L].x = corners[0][CORNER_H_L].x + 2;
	corners[1][CORNER_H_L].y = corners[0][CORNER_H_L].y + 2;
	corners[1][CORNER_H_R].x = corners[1][CORNER_H_L].x + inSizeX;
	corners[1][CORNER_H_R].y = corners[1][CORNER_H_L].y;
	corners[1][CORNER_D_R].x = corners[1][CORNER_D_L].x + inSizeX;
	corners[1][CORNER_D_R].y = corners[1][CORNER_D_L].y;

	gdispGFillConvexPoly(GDISP,0,0,corners[0],4,Black);
	gdispGFillConvexPoly(GDISP,0,0,corners[1],4,White);

	gdispGFillConvexPoly(GDISP,corners[0][CORNER_D_L].x-corners[0][CORNER_H_L].x,displaySizeY/2,corners[0],4,Black);
	gdispGFillConvexPoly(GDISP,corners[0][CORNER_D_L].x-corners[0][CORNER_H_L].x,displaySizeY/2,corners[1],4,White);

	gdispGFillConvexPoly(GDISP,ROAD_SIZE,0,corners[0],4,Black);
	gdispGFillConvexPoly(GDISP,ROAD_SIZE,0,corners[1],4,White);

	gdispGFillConvexPoly(GDISP,ROAD_SIZE+corners[0][CORNER_D_L].x-corners[0][CORNER_H_L].x,displaySizeY/2,corners[0],4,Black);
	gdispGFillConvexPoly(GDISP,ROAD_SIZE+corners[0][CORNER_D_L].x-corners[0][CORNER_H_L].x,displaySizeY/2,corners[1],4,White);

}

void mainTask(Road* road) {
	int v = 20; // in m*s^-1
	initializeRoad(road);
	int IdxCurrentPointInDisplay = 0;
	for(int i=0; i<4; i++)
	{
		road->point[i].rel.y = (1-i) * displaySizeY/2;
	}
	while(TRUE)
	{
		//clear display
		gdispClear(White);

		//transform joystick values to rel speed and ego vehicle yaw

		// use rel speed and ego vehicle yaw (?) to visualize the right position in the display
		road->point[IdxCurrentPointInDisplay].rel.y += v * 1;
		uint8_t inDisplay, firstTime = 1;
		// update relative positions
		for(int i=IdxCurrentPointInDisplay; i<IdxCurrentPointInDisplay+4; i++)
		{
			//update current index
			road->point[i].rel.y = road->point[IdxCurrentPointInDisplay].rel.y - displaySizeY * (i-IdxCurrentPointInDisplay);
			inDisplay = (road->point[i].rel.y>=-displaySizeY/2 && road->point[i].rel.y<=3*displaySizeY/2);
			if(inDisplay)
			{
				drawBorder(road->point[i]);
				if(firstTime)
				{
					IdxCurrentPointInDisplay = i;
					firstTime = 0;
				}
			}
		}

		// use IdxCurrentPointInDisplay to draw the right position in road map

		// Wait for display to stop writing
		xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
		// swap buffers
		ESPL_DrawLayer();
		vTaskDelay(20);
	}
}

// interpret joystick values as relative speed and vehicle yaw
void joystickToRelSpeed()
{
}

//uses info about road points (number and yaw angle) to draw road map
coord roadPointToRelPositionInMap(RoadPoint* lastRoadPoint)
{
}

//plot road map and vehicle position
void drawMap(Road* road)
{
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
