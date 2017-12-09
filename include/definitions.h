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
#define ROAD_POINTS 1024
#define ROAD_SIZE 200
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
	coord abs;
	coord rel;
}RoadPoint;
typedef struct
{
	RoadPoint point[ROAD_POINTS];
} Road;

/*
 * Vehicle definitions
 */
#define CONVERT_JOY 1/10
typedef struct
{
	coord rel;
} Vehicle;

#endif
