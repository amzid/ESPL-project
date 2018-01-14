#include <includes.h>
#include "drawTask.h"


/*
 * @brief This task is used to draw and update all game elements on display.
 *
 */
void drawTask(DataToDraw* dataToDraw)
{
	char str[100];
	font_t font1;
	font1 = gdispOpenFont("DejaVuSans24*");

	Road* road = dataToDraw->road;
    Vehicle* ego = dataToDraw->ego;
#if (NUM_BOTS == 1)
    Vehicle* bot[NUM_BOTS] = {dataToDraw->bot1};
#endif
#if (NUM_BOTS == 2)
    Vehicle* bot[NUM_BOTS] = {dataToDraw->bot1, dataToDraw->bot2};
#endif
#if (NUM_BOTS == 3)
    Vehicle* bot[NUM_BOTS] = {dataToDraw->bot1, dataToDraw->bot2, dataToDraw->bot3};
    Vehicle* rankedVehicles[NUM_VEHICLES] = {ego, bot[0], bot[1], bot[2]};
#endif
	Map* map  = dataToDraw->map;


    Border border;
    border.sizeHigherBorder = 0;
    border.yaw_rad[LOWER_BORDER] = 0;
    border.yaw_rad[HIGHER_BORDER] = 0;

	coord joystickPosition = { 0, 0, 0 };

	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const TickType_t tickFramerate = 20;

	int fps = 50;

    ego->color = RED;

    bot[0]->a_y_0 = 1;
    bot[0]->v_y_max_straight_road = 13;
    bot[0]->absEffect = 5;
    bot[0]->color = GREEN;

    bot[1]->a_y_0 = 1;
    bot[1]->v_y_max_straight_road = 13;
    bot[1]->absEffect = 5;
    bot[1]->color = YELLOW;

    bot[2]->a_y_0 = 1;
    bot[2]->v_y_max_straight_road = 13;
    bot[2]->absEffect = 5;
    bot[2]->color = BLUE;

    while (TRUE) {

		//clear display
		gdispClear(White);

		//Read joystick values
		joystickPosition.x = (uint8_t) (ADC_GetConversionValue(ESPL_ADC_Joystick_2) >> 4) - 255 / 2;
		joystickPosition.y = (uint8_t) (ADC_GetConversionValue(ESPL_ADC_Joystick_1) >> 4) - 255 / 2;

		// Ego
		ego->v_x = joystickPosition.x * V_X_MAX / MAX_JOYSTICK_X;
		ego->a_y = joystickPosition.y * A_Y_MAX / MAX_JOYSTICK_Y;
        calculateVehicleSpeed(ego, fps);
        uint8_t changeCurrentPoint = updatePosition(ego, road);


        // Bots
        calculateBotAcceleration(rankedVehicles, road);
        for(int i=0; i<NUM_BOTS; i++) {
            calculateVehicleSpeed(bot[i], fps);
            updatePosition(bot[i], road);
        }
        updateRanking(ego, bot, road, rankedVehicles);

		// Show speed
        sprintf(str, "Ego: %d", (int)ego->v_y);
        gdispDrawString(0, 0, str, font1, Black);
        //sprintf(str, "Ego_dis_CP: %d", (int)ego->distanceFromCurrentRoadPoint);
        //gdispDrawString(0, 11, str, font1, Black);

        for(int i=0; i<NUM_BOTS; i++) {
            //sprintf(str, "Bot%d_CP: %d", i, (int) bot[i]->currentRoadPoint);
            //gdispDrawString(0, 22 + 33 * i, str, font1, Black);
            sprintf(str, "Bot%d: %d", i, (int) bot[i]->v_y);
            gdispDrawString(0, 33 + 33 * i, str, font1, Black);
        }

        // DRAW
        drawBorder(road, ego->currentRoadPoint, changeCurrentPoint, ego, &border);
        for(int i=0; i<NUM_BOTS; i++)
            drawBot(bot[i], ego, road, &border);
		gdispFillArea(ego->rel.x, ego->rel.y, VEHICLE_SIZE_X, VEHICLE_SIZE_Y, Red);
        drawMap(road, ego, bot, map);


        //TODO: Manage end of game
        if (ego->currentRoadPoint == ROAD_POINTS - 1) {
            sprintf(str, "FINISH GAME :D");
            // TODO: stop vehicle
            gdispDrawString(displaySizeX / 2 - 10, displaySizeY / 2, str, font1, Red);
        }

		// Wait for display to stop writing
		xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
		// swap buffers
		ESPL_DrawLayer();
		vTaskDelayUntil(&xLastWakeTime, tickFramerate);
	}
}

void updateRanking(Vehicle* ego, Vehicle* bot[NUM_BOTS], Road* road, Vehicle* rankedVehicles[NUM_VEHICLES])
{
    Vehicle* vehicles[NUM_VEHICLES];
    int travDist[NUM_VEHICLES];
    vehicles[0] = ego;
    for(int i=0; i<NUM_BOTS; i++)
        vehicles[i+1] = bot[i];
    for(int i=0; i<NUM_VEHICLES; i++)
        travDist[i] = calcDist(vehicles[i]->currentRoadPoint-1, road) + vehicles[i]->distanceFromCurrentRoadPoint;
    for(int i=0; i<NUM_VEHICLES; i++)
    {
        int posMax = 0;
        for(int j=1; j<NUM_VEHICLES; j++)
        {
            if(travDist[j]>travDist[posMax])
            {
                posMax = j;
            }
        }
        vehicles[posMax]->ranking = i;
        rankedVehicles[i] = vehicles[posMax];
        travDist[posMax] = -1;
    }
}

int calcDist(int roadPoint, Road* road)
{
    if(roadPoint < 0)
        return 0;
    return road->point[roadPoint].distanceToNextRoadPoint + calcDist(roadPoint-1, road);
}

/*
 * @brief update vehicle position based on given speed
 */
uint8_t updatePosition(Vehicle* vehicle, Road* road) {
    vehicle->distanceFromCurrentRoadPoint += (int) round(vehicle->v_y);
    uint8_t currentRoadPointInDisplay = 0, changeCurrentPoint = 0;
    // update relative positions
    while(!currentRoadPointInDisplay && vehicle->currentRoadPoint<ROAD_POINTS)
    {
        //update position of next current road point
        currentRoadPointInDisplay = (vehicle->distanceFromCurrentRoadPoint>= 0) && (vehicle->distanceFromCurrentRoadPoint < road->point[vehicle->currentRoadPoint].distanceToNextRoadPoint);
        if (!currentRoadPointInDisplay)
        {
            changeCurrentPoint = 1;
            vehicle->distanceFromCurrentRoadPoint -= road->point[vehicle->currentRoadPoint].distanceToNextRoadPoint;
            vehicle->currentRoadPoint++;
        }
    }
    return changeCurrentPoint;
}


void drawBot(Vehicle* bot, Vehicle*  ego, Road* road, Border* border){
    if(bot->currentRoadPoint == ego->currentRoadPoint)
    {
        int distanceBotToEgo = bot->distanceFromCurrentRoadPoint - ego->distanceFromCurrentRoadPoint;
        bot->rel.y = displaySizeY/2 - distanceBotToEgo;
        bot->rel.x = calcX(border, bot->rel.y, road->side);

        if(bot->color == GREEN) {
            bot->rel.x += 20;
            gdispFillArea(bot->rel.x, bot->rel.y, VEHICLE_SIZE_X, VEHICLE_SIZE_Y, Green);
        }
        else if(bot->color == YELLOW) {
            bot->rel.x += 50;
            gdispFillArea(bot->rel.x, bot->rel.y, VEHICLE_SIZE_X, VEHICLE_SIZE_Y, Yellow);
        }
        else if(bot->color == BLUE) {
            bot->rel.x += 80;
            gdispFillArea(bot->rel.x, bot->rel.y, VEHICLE_SIZE_X, VEHICLE_SIZE_Y, Blue);
        }
    }
}

int adjustAccToRanking(Vehicle* rankedVehicles[NUM_VEHICLES], int index)
{
    if(index == 0){
        return rankedVehicles[1]->a_y;
    }
    if(index == NUM_VEHICLES-1)
    {
        return rankedVehicles[NUM_VEHICLES-2]->a_y;
    }

    return 0.5 * rankedVehicles[index-1]->a_y + 0.5 * rankedVehicles[index+1]->a_y;
}

void calculateBotAcceleration(Vehicle* rankedVehicles[NUM_VEHICLES], Road* road){
    for(int i=0; i<NUM_VEHICLES; i++) {
        if(rankedVehicles[i]->color != RED)
        {

        Vehicle* bot = rankedVehicles[i];
        if (road->point[bot->currentRoadPoint].distanceToNextRoadPoint - bot->distanceFromCurrentRoadPoint > 3 * UNIT_ROAD_DISTANCE) {
            if (bot->v_y <= bot->v_y_max_straight_road)
                bot->a_y = bot->a_y_0 * (1 - bot->v_y / bot->v_y_max_straight_road);
            else
                bot->a_y = 0;
        }
        else {
            double v_soll = road->point[bot->currentRoadPoint + 1].rel.yaw * 15.0 / 45;
            if (bot->v_y > v_soll)
                bot->a_y = -bot->absEffect;
            else
                bot->a_y = 0;
        }
        bot->a_y = 0.7 * bot->a_y + 0.3 * adjustAccToRanking(rankedVehicles,i);
        }
    }
}

void calculateVehicleSpeed(Vehicle* vehicle, int fps)
{
	if(vehicle->state == NO_COLLISION) // if collision speed is set to 0.
	{
		vehicle->v_y += vehicle->a_y / (double)fps;
		if(vehicle->v_y>=V_Y_MAX)
			vehicle->v_y=V_Y_MAX;
		else if(vehicle->v_y<=0)
			vehicle->v_y=2;
	}
}


/*
 *
 *
 */
void drawBorder(Road* road, uint16_t indexCurrentPoint, uint8_t changeCurrentPoint, Vehicle* ego, Border* border)
{
	//States: border->yaw_rad[2]


	//Output of state machine: border->sizeHigherBorder

	// For interpolation
	static double yaw_rad_screen = 0;
	double weightYaw;

	//For input data
	double yaw_rad_joystick = 0;
	uint16_t distanceToNextRoadPoint = 0, offsetY = 0;

	//Calculate input y comp
	distanceToNextRoadPoint = (uint16_t)(road->point[indexCurrentPoint].distanceToNextRoadPoint - ego->distanceFromCurrentRoadPoint);
	offsetY = (uint16_t)((ego->distanceFromCurrentRoadPoint)%UNIT_ROAD_DISTANCE);

	//calculate input x comp
	yaw_rad_joystick = (double) (ego->v_x * MAX_JOYSTICK_ANGLE_X / MAX_JOYSTICK_X) * 3.14 / 180.0;

	// control state changes
	if (road->state == STRAIGHT_ROAD && distanceToNextRoadPoint <= UNIT_ROAD_DISTANCE)
	{
        border->yaw_rad[HIGHER_BORDER] += road->point[indexCurrentPoint + 1].rel.yaw * 3.14 / 180;
		road->state = START_CURVE;
	}
	else if (road->state == START_CURVE && changeCurrentPoint)
	{
		yaw_rad_screen = road->point[indexCurrentPoint].rel.yaw * 3.14 / 180;
		road->state = MIDDLE_CURVE;
	}
	else if(road->state == END_CURVE && ego->distanceFromCurrentRoadPoint >= UNIT_ROAD_DISTANCE)
	{
        border->yaw_rad[HIGHER_BORDER] = 0;
		road->state = STRAIGHT_ROAD;
	}

	//Define states
	if (road->state == START_CURVE)
	{
		if (offsetY > displaySizeY / 2)
            border->sizeHigherBorder = offsetY - displaySizeY / 2;
		else
            border->sizeHigherBorder = 0;
	}
	else if (road->state == MIDDLE_CURVE) //TODO: improve implementation and minimize conditional structures
	{
		// Here many cases are possible:
		// 1 case: Angle successfully fixed through joystick (user applies enough x speed during enough time
		// While curve can still be seen in lower part of screen -> state END_CURVE is necessary
		if (ego->distanceFromCurrentRoadPoint <= UNIT_ROAD_DISTANCE/2 && fabs(border->yaw_rad[HIGHER_BORDER]) <= fabs(yaw_rad_joystick))
		{
            border->sizeHigherBorder = (displaySizeY / 2) + offsetY; // offsetY < (displaySizeY / 2)
            border->yaw_rad[HIGHER_BORDER] = 0; // ego vehicle is again parallel to road
			ego->state = NO_COLLISION;
			road->state = END_CURVE;
		}
		else // case 2: angle not fixed yet in the first UNIT_ROAD_DISTANCE/2 OR case 3: curve cannot be seen in lower part of screen (ego->distanceFromCurrentRoadPoint > UNIT_ROAD_DISTANCE/2)
		{
			// Applicable for case 3 -> No need for END_CURVE; road is already straight
			if(fabs(border->yaw_rad[HIGHER_BORDER]) <= fabs(yaw_rad_joystick))
			{
				road->state = STRAIGHT_ROAD;
				ego->state = NO_COLLISION;
			}
			else
			{
				yaw_rad_screen -= yaw_rad_joystick;
				if(ego->state == COLLISION_WITH_BORDER && fabs(yaw_rad_screen+yaw_rad_joystick) < fabs(yaw_rad_screen))
					yaw_rad_screen += yaw_rad_joystick; // If collision user cannot make yaw angle bigger
                border->yaw_rad[LOWER_BORDER] = yaw_rad_screen - road->point[indexCurrentPoint].rel.yaw * 3.14 / 180;
                border->yaw_rad[HIGHER_BORDER] = yaw_rad_screen;

				if(ego->distanceFromCurrentRoadPoint <= UNIT_ROAD_DISTANCE/2)
                    border->sizeHigherBorder = displaySizeY / 2 + offsetY;
				else
				{
                    border->yaw_rad[LOWER_BORDER] = 0;
                    border->sizeHigherBorder = displaySizeY;
					road->side += (double) ego->v_y * tan(border->yaw_rad[HIGHER_BORDER]);
				}

				// Check for collision
				if(checkIfCollisionWithBorder(border,road->side)) // if between ego vehicle and border less than 3 px -> collision
				{
					ego->state = COLLISION_WITH_BORDER;
					ego->v_y = 0;
				}
				else
				{
					ego->state = NO_COLLISION;
				}
			}
		}
	}
	else if (road->state == END_CURVE)
	{
        border->sizeHigherBorder = (displaySizeY / 2) + offsetY;
		if (offsetY >= displaySizeY / 2)
		{
			road->state = STRAIGHT_ROAD;
		}
	}
	else // road->state == STRAIGHT_ROAD
	{
        border->yaw_rad[HIGHER_BORDER] = 0;
        border->yaw_rad[LOWER_BORDER] = 0;
        border->sizeHigherBorder = offsetY;
	}

	road->side -= (double) ego->v_y * tan(yaw_rad_joystick);

	//Check collision
	if(road->state != MIDDLE_CURVE)
	{
		if(ego->rel.x - road->side - SIZE_BORDER <= 0)
		{
			road->side = ego->rel.x - SIZE_BORDER - 3;
			gdispFillArea(ego->rel.x - 3, ego->rel.y, 3, VEHICLE_SIZE_Y, Red);
			ego->state = COLLISION_WITH_BORDER;
			if(ego->v_y > 1)
				ego->v_y --;
		}
		else if (road->side + ROAD_SIZE - ego->rel.x - VEHICLE_SIZE_X <= 0)
		{
			road->side = ego->rel.x + VEHICLE_SIZE_X - ROAD_SIZE + 3;
			gdispFillArea(ego->rel.x + VEHICLE_SIZE_X, ego->rel.y, 3, VEHICLE_SIZE_Y, Red);
			ego->state = COLLISION_WITH_BORDER;
			if(ego->v_y > 1)
				ego->v_y --;
		}
		else
		{
			ego->state = NO_COLLISION;
		}
	}

	drawWhiteBorder(border, (int) road->side, LOWER_BORDER);
	drawWhiteBorder(border, (int) road->side, HIGHER_BORDER);

	// This is part of improving game design. It is postponed to the end of project.
	//drawBlackBox(calcX(offsetY,yaw_rad,road->side),offsetY,yaw_rad);
	//drawBlackBox(calcX(displaySizeY/2+offsetY,yaw_rad,road->side),displaySizeY/2+offsetY,yaw_rad);
	//drawBlackBox(calcX(displaySizeY+offsetY,yaw_rad,road->side),displaySizeY+offsetY,yaw_rad);
}



/*
 * @brief This functions draws the higher/lower part of the border (left and right).
 *
 */
void drawWhiteBorder(Border* border, int side, TYPE_BORDER typeBorder)
{
    int sizeHigherBorder = border->sizeHigherBorder;
    double yaw_rad[2] = {border->yaw_rad[LOWER_BORDER], border->yaw_rad[HIGHER_BORDER]};
	uint8_t sizeX = SIZE_BORDER;
	int size[2];

	size[HIGHER_BORDER] = sizeHigherBorder; // TODO
	size[LOWER_BORDER] = displaySizeY - size[HIGHER_BORDER];

	struct point corners[NB_CORNERS_BORDER];
	corners[CORNER_D_L].x = side
			- (typeBorder == LOWER_BORDER)
			* round(size[LOWER_BORDER] * tan(yaw_rad[LOWER_BORDER]));
	corners[CORNER_D_L].y = size[HIGHER_BORDER] + size[LOWER_BORDER] * (typeBorder == LOWER_BORDER);

	corners[CORNER_H_L].x = round(corners[CORNER_D_L].x + size[typeBorder] * tan(yaw_rad[typeBorder]));
	corners[CORNER_H_L].y = size[HIGHER_BORDER] * (typeBorder == LOWER_BORDER);

	corners[CORNER_H_R].x = corners[CORNER_H_L].x + sizeX;
	corners[CORNER_H_R].y = corners[CORNER_H_L].y;

	corners[CORNER_D_R].x = corners[CORNER_D_L].x + sizeX;
	corners[CORNER_D_R].y = corners[CORNER_D_L].y;

	gdispDrawPoly(0, 0, corners, NB_CORNERS_BORDER, Black);
	gdispDrawPoly(1, 0, corners, NB_CORNERS_BORDER, Black);

	gdispDrawPoly(ROAD_SIZE, 0, corners, NB_CORNERS_BORDER, Black);
	gdispDrawPoly(1 + ROAD_SIZE, 0, corners, NB_CORNERS_BORDER, Black);
}

uint8_t checkIfCollisionWithBorder(Border* border, double side)
{
    int sizeHigherBorder = border->sizeHigherBorder;
    double yaw_rad[2] = {border->yaw_rad[LOWER_BORDER], border->yaw_rad[HIGHER_BORDER]};
    double distanceEgoToLeftborder = (displaySizeX / 2 - calcX(border, displaySizeY / 2, side) - SIZE_BORDER) * cos(yaw_rad[HIGHER_BORDER]),
		distanceEgoToRightborder = ( calcX(border, displaySizeY / 2, side) + ROAD_SIZE - VEHICLE_SIZE_X - displaySizeX / 2) * cos(yaw_rad[HIGHER_BORDER]);
	if(distanceEgoToLeftborder <= 2)
	{
		gdispFillCircle((int) round( calcX(border, displaySizeY / 2, side)) + SIZE_BORDER, displaySizeY/2, 2, Red);
		return TRUE;
	}
	if(distanceEgoToRightborder <= 2)
	{
		gdispFillCircle((int) round( calcX(border, displaySizeY / 2, side)) + ROAD_SIZE , displaySizeY/2, 2, Red);
		return TRUE;
	}
	return FALSE;
}

/*
 * @brief This function calculates the x position of left road border knowing the y position
 * If the desired value is x position of right road border, please call this function and add
 * ROAD_SIDE to the returned value.
 */
double calcX(Border* border, int y, double side)
{
    int sizeHigherBorder = border->sizeHigherBorder;
    double yaw_rad[2] = {border->yaw_rad[LOWER_BORDER], border->yaw_rad[HIGHER_BORDER]};
	double a = 0, b = 0;
	if (sizeHigherBorder >= y) // Consider only left higher border
	{
		if (yaw_rad[HIGHER_BORDER] == 0)
			return side;
		a = -1 / tan(yaw_rad[HIGHER_BORDER]);
		b = -a * (sizeHigherBorder * tan(yaw_rad[HIGHER_BORDER]) + side);
		return ((y - b) / a);
	}
	else
	{
        if (yaw_rad[LOWER_BORDER] == 0)
            return side;
        a = 1 / tan(yaw_rad[LOWER_BORDER]);
        b = sizeHigherBorder - side * a;
        return ((y - b) / a);
    }
}

/*
 * @brief This function draws road map and vehicle position
 */

void drawMap(Road* road, Vehicle* ego, Vehicle* bot[NUM_BOTS], Map* map)
{
	//Draw map place holder
	gdispFillArea(displaySizeX - MAP_SIZE_X, displaySizeY - MAP_SIZE_Y,MAP_SIZE_X, MAP_SIZE_Y, White);
	gdispDrawBox(displaySizeX - MAP_SIZE_X, displaySizeY - MAP_SIZE_Y,MAP_SIZE_X, MAP_SIZE_Y, Black);

	//draw map
	gdispDrawPoly(0, 0, map, MAP_POINTS, Black);

	//Draw ego vehicle position
    drawVehiclePositionOnMap(ego, map);
    for(int i=0; i<NUM_BOTS; i++)
        drawVehiclePositionOnMap(bot[i], map);
}

void drawVehiclePositionOnMap(Vehicle* vehicle, Map* map)
{
    static uint8_t colorIndex = 0;

    uint16_t dist = vehicle->distanceFromCurrentRoadPoint  * UNIT_ROAD_DISTANCE_IN_MAP / UNIT_ROAD_DISTANCE;
    uint16_t vehicleMapX = map->point[(vehicle->currentRoadPoint % (ROAD_POINTS / 2))].x + dist * cos((double) map->orientation[(vehicle->currentRoadPoint % (ROAD_POINTS / 2))] * 3.14 / 180),
            vehicleMapY = map->point[(vehicle->currentRoadPoint % (ROAD_POINTS / 2))].y + dist * sin((double) map->orientation[(vehicle->currentRoadPoint % (ROAD_POINTS / 2))] * 3.14 / 180);
    switch(colorIndex % NUM_VEHICLES){
        case 0:
            gdispFillCircle(vehicleMapX, vehicleMapY, 2, Red);
            break;
        case 1:
            gdispFillCircle(vehicleMapX, vehicleMapY, 2, Green);
            break;
        case 2:
            gdispFillCircle(vehicleMapX, vehicleMapY, 2, Yellow);
            break;
        case 3:
            gdispFillCircle(vehicleMapX, vehicleMapY, 2, Blue);
            break;
    }
    colorIndex++;
}

/*
void drawBlackBox(int x, int y, double yaw_rad) {
	uint8_t outSizeX = 10, inSizeX = 10, outSizeY = displaySizeY / 2, inSizeY =
			displaySizeY / 4;

	struct point corners[NB_CORNERS_BORDER];
	corners[CORNER_D_L].x = x + 1;
	corners[CORNER_D_L].y = y;

	corners[CORNER_H_L].x = round(
			corners[CORNER_D_L].x + displaySizeY / 4 * tan(yaw_rad));
	corners[CORNER_H_L].y = y - displaySizeY / 4;

	corners[CORNER_H_R].x = corners[CORNER_H_L].x + inSizeX;
	corners[CORNER_H_R].y = corners[CORNER_H_L].y;

	corners[CORNER_D_R].x = corners[CORNER_D_L].x + inSizeX;
	corners[CORNER_D_R].y = corners[CORNER_D_L].y;

	gdispGFillConvexPoly(GDISP, 0, 0, corners, NB_CORNERS_BORDER, Black);
	gdispGFillConvexPoly(GDISP, ROAD_SIZE, 0, corners, NB_CORNERS_BORDER,
			Black);
}
*/
