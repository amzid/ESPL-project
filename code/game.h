#ifndef DRAW_TASK_H
#define DRAW_TASK_H

#include "includes.h"


// Task
void drawTask(Game* game);

/************************************************************************************************************************/
/*
 * Calculations
 */
/************************************************************************************************************************/
/*
 * @brief update vehicle y rel position
 */
void calculateVehicleSpeed(Vehicle* vehicle, int fps);
/*
 * @brief Main road state machine, detect curves and compute border position accordingly
 */
void updateRoadStatesAndCalculateBorderPosition(Road* road, uint16_t indexCurrentPoint, uint8_t changeCurrentPoint, Vehicle* ego, Border* border);
/*
 * @brief This function calculates the x position of left road border knowing the y position
 * If the desired value is x position of right road border, please call this function and add
 * ROAD_SIDE to the returned value.
 */
double calcX(Border* border, int y, double side);
/*
 * @brief Update vehicle relative positions according to given CP and distance
 */
void updateRelY(Vehicle* bot, Vehicle* ego, Road* road );
/*
 * @brief Update vehicle rankings
 */
void updateRanking(Vehicle* ego, Vehicle* bot[NUM_BOTS], Road* road, Vehicle* rankedVehicles[NUM_VEHICLES]);
/*
 * @brief update vehicle position (CP and dist) based on given speed
 */
uint8_t updatePosition(Vehicle* vehicle, Road* road);
/*
 * @brief calculate total distance when given a CP
 */
int calcDist(int roadPoint, Road* road);

/************************************************************************************************************************/
/*
 * Draw functions
 */
/************************************************************************************************************************/
void drawVehicle(uint16_t x, uint16_t y, int color);
void drawFinishGameAndHighScores(Road* road, Vehicle* ego);
void drawGrassAndRoad(Road* road, Border* border, Vehicle* ego);
void drawInfo(Vehicle* ego,Road* road,int fps);
void drawMap(Road* road, Vehicle* ego, Vehicle* bot[NUM_BOTS], Map* map);
void drawVehiclePositionOnMap(Vehicle* vehicle, Map* map);
void drawBot(Vehicle* bot, Vehicle*  ego, Border* border, Road* road);
void drawBoxGame(Box* box, double yaw_rad);

/************************************************************************************************************************/
/*
 * Artificial intelligence
 */
/************************************************************************************************************************/
/*
 * @brief acceleration is combination of self-computed acceleration based on initial parameters
 * and influence of next vehicle's acceleration
 */
void calculateBotAcceleration(Vehicle* rankedVehicles[NUM_VEHICLES], Road* road);
/*
 * @brief adjust acceleration to ranking
 */
int adjustAccToRanking(Vehicle* rankedVehicles[NUM_VEHICLES], int index);
/*
 * @brief Calculate speed in x direction
 */
void calculateLateralSpeed(Vehicle* rankedVehicles[NUM_VEHICLES], Road* road, Border* border);
/*
 * @brief update x position based on computed v_x
 */
void updateXPosition(Vehicle* bot, Border* border, Road* road);

/************************************************************************************************************************/
/*
 * Collision
 */
/************************************************************************************************************************/
void checkCarCollision(Vehicle* rankedVehicles[NUM_VEHICLES]);
uint8_t checkIfCollisionWithBorder(Border* border, double side);


#endif
