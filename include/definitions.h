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
#define STACK_SIZE 2000

/*
 * Road definitions
 */
#define LAP_POINTS              8
#define ROAD_POINTS 			2 * LAP_POINTS
#define ROAD_SIZE 				150
#define SIDE 					(displaySizeX/2 - ROAD_SIZE/2)
#define UNIT_ROAD_DISTANCE  	displaySizeY

//Border
#define SIZE_BORDER             10
#define NB_CORNERS_BORDER 		4
#define CORNER_D_L 				0
#define CORNER_H_L 				1
#define CORNER_H_R 				2
#define CORNER_D_R 				3

typedef struct {
    double yaw_rad[2];
    int sizeHigherBorder;
} Border;

typedef enum {STRAIGHT_ROAD, START_CURVE , MIDDLE_CURVE , END_CURVE} StateRoad;
typedef enum {LOWER_BORDER,HIGHER_BORDER} TYPE_BORDER;

typedef struct {
	double x;
	int y;
	double yaw;
} coord;
typedef struct
{
	coord rel;
	coord mapPosition;
	uint16_t distanceToNextRoadPoint;
}RoadPoint;
typedef struct
{
	double side;
	RoadPoint point[ROAD_POINTS+4]; // 4 additional points to make sure of a right finish
	StateRoad state;
} Road;

/*
 * Map definitions
 */
#define MAP_SIZE_X 						displaySizeX/3
#define MAP_SIZE_Y 						displaySizeY/3
#define MAP_POINTS 						ROAD_POINTS/2
#define UNIT_ROAD_DISTANCE_IN_MAP 		3

typedef struct
{
	point point[MAP_POINTS];
	int orientation[MAP_POINTS];
} Map;

/*
 * Vehicle definitions
 */
#define NUM_BOTS                3
#define NUM_VEHICLES            4

#define MAX_JOYSTICK_X 			(255/2)
#define MAX_JOYSTICK_Y 			(255)
#define V_Y_MAX                 30
#define V_X_MAX 				20
#define A_Y_MAX 				7
#define MAX_JOYSTICK_ANGLE_X 	22

#define VEHICLE_SIZE_X          10
#define VEHICLE_SIZE_Y          20

typedef enum {NO_COLLISION, COLLISION_WITH_BORDER} StateVehicle;
typedef enum {RED, GREEN, YELLOW, BLUE} Color;

typedef struct
{
	// Bot specific definitions
	double a_y_0;
    double v_y_max_straight_road;
    double absEffect;
    Color color;
	// General definitions
	coord rel;
	double v_x;
	double v_y;
	double a_x;
	double a_y;
	uint16_t currentRoadPoint;
	uint16_t distanceFromCurrentRoadPoint; //Distance to the current road point
	StateVehicle state;
    uint8_t ranking;
} Vehicle;

/*
 * Data to draw
 */
typedef struct
{
	Road* road;
	Vehicle* ego;
    Vehicle* bot1;
#if (NUM_BOTS>1)
    Vehicle* bot2;
#endif
#if (NUM_BOTS>2)
    Vehicle* bot3;
#endif
	Map* map;
} DataToDraw;

#endif
