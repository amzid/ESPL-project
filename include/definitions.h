#ifndef TYPES_H
#define TYPES_H

#include "includes.h"

/*
 * Display definitions
 */
static const uint16_t displaySizeX = 320,
					  displaySizeY = 240;

/*
 * FreeRTOS definitions
 */
#define STACK_SIZE 1000

/*
 * Road definitions
 */
#define ROAD_POINTS 64
#define ROAD_SIZE 150
#define SIDE (displaySizeX/2 - ROAD_SIZE/2)
#define NB_CORNERS_BORDER 4
#define CORNER_D_L 0
#define CORNER_H_L 1
#define CORNER_H_R 2
#define CORNER_D_R 3

typedef struct {
	int x;
	int y;
	double yaw;
} coord;
typedef struct
{
	coord rel;
	coord mapPosition;
}RoadPoint;
typedef struct
{
	RoadPoint point[ROAD_POINTS+4];
	double side;
} Road;

typedef enum {LOWER_BORDER,HIGHER_BORDER} TYPE_BORDER;
typedef enum {START , INTERPOLATE , END , IDLE} State;
/*
 * Map definitions
 */
#define MAP_SIZE_X displaySizeX/3
#define MAP_SIZE_Y displaySizeY/3
#define RESOLUTION_MAP 2
#define MAP_POINTS (ROAD_POINTS/2)/RESOLUTION_MAP


/*
 * Vehicle definitions
 */

#define MAX_JOYSTICK_X (255/2)
#define MAX_JOYSTICK_Y 255
#define V_X_MAX 10
#define V_Y_MAX 10

typedef struct
{
	coord rel;
	int v_x;
	int v_y;
	uint16_t currentRoadPoint;
	int currentRelativeDistance; //Distance to the current road point
} Vehicle;

#endif
