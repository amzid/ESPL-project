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
#define STACK_SIZE 800

/*
 * Road definitions
 */
#define LAP_POINTS              16
#define ROAD_POINTS 			(2 * LAP_POINTS)
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
    int highScores[3];
} Road;

/*
 * Map definitions
 */
#define NUM_MAPS                        3

#define MAP_SIZE_X 						displaySizeX/3
#define MAP_SIZE_Y 						displaySizeY/3
#define MAP_POINTS 						(ROAD_POINTS/2)
#define UNIT_ROAD_DISTANCE_IN_MAP 		3

typedef struct
{
	point point[MAP_POINTS];
	int orientation[MAP_POINTS];
    int color;
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

#define VEHICLE_SIZE_X          30
#define VEHICLE_SIZE_Y          30

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
    uint8_t changeCurrentPoint;
} Vehicle;

/*
 * Game struct and game control
 */
typedef enum {SINGLE_MODE, MULTIPLAYER_MODE} Mode;
typedef enum {START_MENU, GAME_PLAYING, GAME_PAUSED} GameState;
typedef enum {NOT_CONNECTED, CONNECTED} ConnectionState;
typedef enum { NOT_CHOSEN, MODE_CHOSEN, COURSE_CHOSEN, CTRL_CHOSEN } MenuState;
typedef enum { SPEED_CTRL, STEERING_CTRL } ControlState;
typedef enum { INDEX_MAP_1, INDEX_MAP_2, INDEX_MAP_3} ChosenMap;
typedef struct
{
    // Game control
    volatile ConnectionState connectionState;
    GameState gameState;
    Mode mode;
	MenuState menuState;
    ControlState controlState;
    ChosenMap chosenMap;
    // Game parts
	Road* road[3];
	Vehicle* ego;
    Vehicle* bot1;
    Vehicle* bot2;
    Vehicle* bot3;
	Map* map[3];

    uint8_t gameStateOtherPlayer;
    uint8_t received_buffer;
    uint32_t taktUART;
    uint32_t taktGame;
    uint8_t elapsed_s;
    uint16_t elapsed_ms;
    uint8_t firstTime;
} Game;

#define INITIAL_HIGH_SCORE 10000

/*
 * Menu definition
 */
typedef struct {
    uint16_t x;
    uint16_t y;
    uint8_t sizeX;
    uint8_t sizeY;
    char text[50];
    int  color;
    int colorText;
} Box;

/*
 * Button definitions
 */
#define BUTTON_PRESSED         0
#define BUTTON_UNPRESSED       1

#define OK_BUTTON_REGISTER     ESPL_Register_Button_A
#define OK_BUTTON_PIN          ESPL_Pin_Button_A

#define EXIT_BUTTON_REGISTER     ESPL_Register_Button_E
#define EXIT_BUTTON_PIN          ESPL_Pin_Button_E

#define RESET_BUTTON_REGISTER     ESPL_Register_Button_C
#define RESET_BUTTON_PIN          ESPL_Pin_Button_C

#define PAUSE_BUTTON_REGISTER     ESPL_Register_Button_B
#define PAUSE_BUTTON_PIN          ESPL_Pin_Button_B

extern TimerHandle_t xTimer;
extern uint16_t time_s;
extern TaskHandle_t drawHdl, receiveHdl;

#define BYTE_RESET                0xFF
#define SIZE_VALUES_TO_SEND       12
extern GameState lastGameStateOtherPlayer;
extern char str[100];
extern font_t font1;

#endif
