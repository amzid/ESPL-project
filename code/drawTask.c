#include "drawTask.h"

void drawTask(DataToDraw* dataToDraw)
{
	//for debug strings
	char str[100];
	font_t font1;
	font1 = gdispOpenFont("DejaVuSans24*");

	Road* road = dataToDraw->road;
	Vehicle* ego = dataToDraw->ego;
	Map* map  = dataToDraw->map;

	road->side = (float) SIDE;

	coord joystickPosition = { 0, 0, 0 };

	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const TickType_t tickFramerate = 20;


	while (TRUE) {
		//clear display
		gdispClear(White);
		//Read joystick values
		joystickPosition.x = (uint8_t) (ADC_GetConversionValue(ESPL_ADC_Joystick_2) >> 4) - 255 / 2;
		joystickPosition.y = (uint8_t) (ADC_GetConversionValue(ESPL_ADC_Joystick_1) >> 4);

		//Calculate speed and update x position
		ego->v_x = joystickPosition.x * V_X_MAX / MAX_JOYSTICK_X;
		if(ego->state == NO_COLLISION)
		{
			ego->v_y = joystickPosition.y * V_Y_MAX / MAX_JOYSTICK_Y;
		}
		// Show speed
		sprintf(str, "v_x: %d", ego->v_x);
		gdispDrawString(0, 0, str, font1, Black);
		sprintf(str, "v_y: %d", ego->v_y);
		gdispDrawString(0, 11, str, font1, Black);

		// use rel speed to visualize the right position in the display
		road->point[ego->currentRoadPoint].rel.y += ego->v_y;
		ego->distanceFromCurrentRoadPoint += ego->v_y;

		uint8_t currentRoadPointInDisplay = 0, changeCurrentPoint = 0;

		// update relative positions
		while(!currentRoadPointInDisplay && ego->currentRoadPoint<ROAD_POINTS)
		{
			//update position of next current road point
			road->point[ego->currentRoadPoint+1].rel.y = road->point[ego->currentRoadPoint].rel.y - road->point[ego->currentRoadPoint].distanceToNextRoadPoint;

			currentRoadPointInDisplay = (road->point[ego->currentRoadPoint].rel.y >= displaySizeY / 2) && (road->point[ego->currentRoadPoint].rel.y < (displaySizeY / 2 + road->point[ego->currentRoadPoint].distanceToNextRoadPoint));
			if (!currentRoadPointInDisplay)
			{
				changeCurrentPoint = 1;
				ego->distanceFromCurrentRoadPoint -= road->point[ego->currentRoadPoint].distanceToNextRoadPoint;
				ego->currentRoadPoint++;
			}
		}

		sprintf(str, "currentRoadPoint: %d", ego->currentRoadPoint);
		gdispDrawString(0, 55, str, font1, Black);



		drawBorder(road, ego->currentRoadPoint, changeCurrentPoint, ego);

		//TODO: Manage end of game
		if (ego->currentRoadPoint == ROAD_POINTS - 1) {
			sprintf(str, "FINISH GAME :D");
			// TODO: stop vehicle
			gdispDrawString(displaySizeX / 2 - 10, displaySizeY / 2, str, font1, Red);
		}

		//Draw Vehicles
		gdispFillArea(ego->rel.x, ego->rel.y, 10, 20, Blue);

		// use IdxCurrentPointInDisplay to draw the right position in road map
		drawMap(road, ego, map);

		// Wait for display to stop writing
		xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
		// swap buffers
		ESPL_DrawLayer();
		vTaskDelayUntil(&xLastWakeTime, tickFramerate);
	}
}


void drawWhiteBorder(double yaw_rad[2], int side, TYPE_BORDER typeBorder,int sizeHigherBorder)
{
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

double calcX(int y, double yaw_rad[2], double side, int sizeHigherBorder) {
	double a = 0, b = 0;
	if (sizeHigherBorder >= displaySizeY / 2) // Consider only left higher border
	{
		if (yaw_rad[HIGHER_BORDER] == 0)
			return side;
		a = -1 / tan(yaw_rad[HIGHER_BORDER]);
		b = -a * (sizeHigherBorder * tan(yaw_rad[HIGHER_BORDER]) + side);
		return ((y - b) / a);
	} else {
		//TODO Dedicace Halim
	}
}

void drawBorder(Road* road, uint16_t indexCurrentPoint, uint8_t changeCurrentPoint, Vehicle* ego) {
	//States
	static double yaw_rad[2] = { 0, 0 };

	//Output of state machine
	int sizeHigherBorder;

	// For interpolation
	double weightIndex, weightNxtIdx;
	double yaw_rad_computed = 0, yaw_rad_joystick = 0;
	static double yaw_rad_screen = 0, distanceEgoToborder = 0;
	double weightYaw;

	uint16_t distanceToNextRoadPoint = 0, offsetY = 0;
	distanceToNextRoadPoint = (uint16_t)(road->point[indexCurrentPoint].distanceToNextRoadPoint - ego->distanceFromCurrentRoadPoint);
	offsetY = (uint16_t)((ego->distanceFromCurrentRoadPoint)%UNIT_ROAD_DISTANCE);

	yaw_rad_joystick = (double) (ego->v_x * 22 / MAX_JOYSTICK_X) * 3.14 / 180.0;

	/*
	 * DEBUG
	 */
	char str[100];
	font_t font1;
	font1 = gdispOpenFont("DejaVuSans24*");
	sprintf(str, "offsetY: %d", offsetY);
	gdispDrawString(0, 22, str, font1, Black);
	sprintf(str, "Road state: %d", road->state);
	gdispDrawString(0, 33, str, font1, Black);

	/*
	 * DEBUG
	 */


	// control state changes
	if (road->state == STRAIGHT_ROAD && distanceToNextRoadPoint <= UNIT_ROAD_DISTANCE)
	{
		yaw_rad[HIGHER_BORDER] += road->point[indexCurrentPoint + 1].rel.yaw * 3.14 / 180;
		road->state = START_CURVE;
	}
	else if (road->state == START_CURVE && changeCurrentPoint)
	{
		yaw_rad_screen = road->point[indexCurrentPoint].rel.yaw * 3.14 / 180;
		road->state = MIDDLE_CURVE;
	}
	else if(road->state == END_CURVE && ego->distanceFromCurrentRoadPoint >= UNIT_ROAD_DISTANCE)
	{
		yaw_rad[HIGHER_BORDER] = 0;
		road->state = STRAIGHT_ROAD;
	}



	//Define states
	if (road->state == START_CURVE)
	{
		if (offsetY > displaySizeY / 2)
			sizeHigherBorder = offsetY - displaySizeY / 2;
		else
			sizeHigherBorder = 0;
	}
	else if (road->state == MIDDLE_CURVE) //TODO: improve implementation and minimize conditional structures
	{
		//weightNxtIdx = (double)(road->point[indexCurrentPoint].rel.y-displaySizeY/2) / (double)(displaySizeY);
		//weightIndex = -(double)(road->point[indexCurrentPoint+1].rel.y-displaySizeY/2) / (double)(displaySizeY);
		//yaw_rad_computed = (road->point[indexCurrentPoint].rel.yaw * weightIndex + road->point[indexCurrentPoint+1].rel.yaw * weightNxtIdx) * 3.14 / 180;
		if (ego->distanceFromCurrentRoadPoint <= UNIT_ROAD_DISTANCE/2 && fabs(yaw_rad[HIGHER_BORDER]) <= fabs(yaw_rad_joystick)) // Interpolation successful: angle successfully fixed through joystick
		{
			sizeHigherBorder = (displaySizeY / 2) + offsetY;
			yaw_rad[HIGHER_BORDER] = 0;
			ego->state = NO_COLLISION;
			road->state = END_CURVE;
		}
		else // angle not fixed yet
		{
				if(fabs(yaw_rad[HIGHER_BORDER]) <= fabs(yaw_rad_joystick))
				{
					road->state = STRAIGHT_ROAD;
					ego->state = NO_COLLISION;
				}

				yaw_rad_screen -= yaw_rad_joystick;
				yaw_rad[LOWER_BORDER] = yaw_rad_screen - road->point[indexCurrentPoint].rel.yaw * 3.14 / 180;
				yaw_rad[HIGHER_BORDER] = yaw_rad_screen;

				if(ego->distanceFromCurrentRoadPoint <= UNIT_ROAD_DISTANCE/2)
					sizeHigherBorder = displaySizeY / 2 + offsetY;
				else
				{
					yaw_rad[LOWER_BORDER] = 0;
					sizeHigherBorder = displaySizeY;
					road->side += ego->v_y * tan(yaw_rad[HIGHER_BORDER]);
				}

				distanceEgoToborder = (displaySizeX / 2 - calcX(displaySizeY / 2, yaw_rad, road->side, sizeHigherBorder) - SIZE_BORDER) * cos(yaw_rad[HIGHER_BORDER]);
				if(distanceEgoToborder <= 3)
				{
					gdispFillCircle((int) round( calcX(displaySizeY / 2, yaw_rad, road->side, sizeHigherBorder)) + SIZE_BORDER, displaySizeY/2, 2, Red);
					ego->state = COLLISION_WITH_BORDER;
					ego->v_y = 0;
				}
				else
				{
					ego->state = NO_COLLISION;
				}
		}
	}
	else if (road->state == END_CURVE) // Now not used
	{
		sizeHigherBorder = (displaySizeY / 2) + offsetY;
		if (offsetY >= displaySizeY / 2)
		{
			road->state = STRAIGHT_ROAD;
		}
	}
	else // road->state == STRAIGHT_ROAD
	{
		yaw_rad[HIGHER_BORDER] = 0;
		yaw_rad[LOWER_BORDER] = 0;
		sizeHigherBorder = offsetY;
	}



	road->side -= ego->v_y * tan(yaw_rad_joystick);

	drawWhiteBorder(yaw_rad, (int) road->side, LOWER_BORDER, sizeHigherBorder);
	drawWhiteBorder(yaw_rad, (int) road->side, HIGHER_BORDER, sizeHigherBorder);

	// draw the left white border
	//drawBlackBox(calcX(offsetY,yaw_rad,road->side),offsetY,yaw_rad);
	//drawBlackBox(calcX(displaySizeY/2+offsetY,yaw_rad,road->side),displaySizeY/2+offsetY,yaw_rad);
	//drawBlackBox(calcX(displaySizeY+offsetY,yaw_rad,road->side),displaySizeY+offsetY,yaw_rad);
}








/*
 * Functions related to map
 */

//plot road map and vehicle position
void drawMap(Road* road, Vehicle* ego, Map* map)
{
	uint16_t offsetX = displaySizeX - MAP_SIZE_X + 60,
			offsetY = displaySizeY - MAP_SIZE_Y + 15;

	//Draw map place holder
	gdispFillArea(displaySizeX - MAP_SIZE_X, displaySizeY - MAP_SIZE_Y,MAP_SIZE_X, MAP_SIZE_Y, White);
	gdispDrawBox(displaySizeX - MAP_SIZE_X, displaySizeY - MAP_SIZE_Y,MAP_SIZE_X, MAP_SIZE_Y, Black);

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

	//draw map
	gdispDrawPoly(0, 0, map, MAP_POINTS, Black);

	//Draw ego vehicle position
	dist = ego->distanceFromCurrentRoadPoint  * UNIT_ROAD_DISTANCE_IN_MAP / UNIT_ROAD_DISTANCE;
	uint16_t egoMapX = map->point[(ego->currentRoadPoint % (ROAD_POINTS / 2))].x + dist * cos((double) map->orientation[(ego->currentRoadPoint % (ROAD_POINTS / 2))] * 3.14 / 180),
			 egoMapY = map->point[(ego->currentRoadPoint % (ROAD_POINTS / 2))].y + dist * sin((double) map->orientation[(ego->currentRoadPoint % (ROAD_POINTS / 2))] * 3.14 / 180);
	gdispFillCircle(egoMapX, egoMapY, 2, Red);
}
