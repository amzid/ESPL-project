#include "drawTask.h"

void drawTask(Road* road) {
	//for debug strings
	char str[100];
	font_t font1;
	font1 = gdispOpenFont("DejaVuSans24*");

	Vehicle ego;
	ego.rel.y = displaySizeY / 2;
	ego.rel.x = displaySizeX / 2;
	ego.v_x = 0;
	ego.v_y = 0;
	ego.currentRoadPoint = 0;
	ego.currentRelativeDistance = 0;

	initializeRoad(road);
	road->point[ego.currentRoadPoint].rel.y = displaySizeY / 2;

	coord joystickPosition = { 0, 0, 0 };

	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const TickType_t tickFramerate = 20;

	while (TRUE) {
		//clear display
		gdispClear(White);
		//Read joystick values
		joystickPosition.x = (uint8_t) (ADC_GetConversionValue(
				ESPL_ADC_Joystick_2) >> 4) - 255 / 2;
		joystickPosition.y = (uint8_t) (ADC_GetConversionValue(
				ESPL_ADC_Joystick_1) >> 4);

		//Calculate speed and update x position
		ego.v_x = joystickPosition.x * V_X_MAX / MAX_JOYSTICK_X;
		ego.v_y = joystickPosition.y * V_Y_MAX / MAX_JOYSTICK_Y;

		// Show speed
		sprintf(str, "v_x: %d", ego.v_x);
		gdispDrawString(0, 0, str, font1, Black);
		sprintf(str, "v_y: %d", ego.v_y);
		gdispDrawString(0, 11, str, font1, Black);

		// use rel speed to visualize the right position in the display
		road->point[ego.currentRoadPoint].rel.y += ego.v_y;

		uint8_t currentRoadPointInDisplay = 0, changeCurrentPoint = 0;

		// update relative positions
		while(!currentRoadPointInDisplay  && ego.currentRoadPoint<ROAD_POINTS)
		{
			//update position of next current road point
			road->point[ego.currentRoadPoint+1].rel.y = road->point[ego.currentRoadPoint].rel.y - displaySizeY;

			currentRoadPointInDisplay = (road->point[ego.currentRoadPoint].rel.y >= -displaySizeY / 2) && (road->point[ego.currentRoadPoint].rel.y <= 3 * displaySizeY / 2);
			if (!currentRoadPointInDisplay)
			{
				changeCurrentPoint = 1;
				ego.currentRoadPoint++;
			}
		}

		drawBorder(road, ego.currentRoadPoint, changeCurrentPoint, &ego);

		//TODO: Manage end of game
		if (ego.currentRoadPoint == ROAD_POINTS - 1) {
			sprintf(str, "FINISH GAME :D");
			// TODO: stop vehicle
			gdispDrawString(displaySizeX / 2 - 10, displaySizeY / 2, str, font1, Red);
		}

		//Draw Vehicles
		gdispFillArea(ego.rel.x, ego.rel.y, 10, 20, Blue);

		// use IdxCurrentPointInDisplay to draw the right position in road map
		drawMap(road, ego.currentRoadPoint);

		// Wait for display to stop writing
		xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
		// swap buffers
		ESPL_DrawLayer();
		vTaskDelayUntil(&xLastWakeTime, tickFramerate);
	}
}

void initializeRoad(Road* road) {
	int j = 0;
	road->side = SIDE;
	for (int i = 0; i < ROAD_POINTS / 2; i++) {
		road->point[i].rel.x = displaySizeX / 2;
		road->point[i].rel.yaw = 0;
	}
	for (int i = 1; i < 8; i++)
		road->point[i * ROAD_POINTS / 16].rel.yaw = 45;
	road->point[ROAD_POINTS / 2 - 1].rel.yaw = 45;
	//Copy Road for second lap
	for (int i = ROAD_POINTS / 2; i < ROAD_POINTS + 4; i++) {
		road->point[i] = road->point[i - ROAD_POINTS / 2];
	}
}

void drawWhiteBorder(double yaw_rad[2], int side, TYPE_BORDER typeBorder,
		int sizeHigherBorder) {
	uint8_t outSizeX = 10, inSizeX = 6, outSizeY = displaySizeY / 2, inSizeY =
			displaySizeY / 4;
	int size[2];

	size[HIGHER_BORDER] = sizeHigherBorder; // TODO
	size[LOWER_BORDER] = displaySizeY - size[HIGHER_BORDER];

	struct point corners[NB_CORNERS_BORDER];
	corners[CORNER_D_L].x = side
			- (typeBorder == LOWER_BORDER)
					* round(size[LOWER_BORDER] * tan(yaw_rad[LOWER_BORDER]));
	corners[CORNER_D_L].y = size[HIGHER_BORDER]
			+ size[LOWER_BORDER] * (typeBorder == LOWER_BORDER);

	corners[CORNER_H_L].x = round(
			corners[CORNER_D_L].x
					+ size[typeBorder] * tan(yaw_rad[typeBorder]));
	corners[CORNER_H_L].y = size[HIGHER_BORDER] * (typeBorder == LOWER_BORDER);

	corners[CORNER_H_R].x = corners[CORNER_H_L].x + outSizeX;
	corners[CORNER_H_R].y = corners[CORNER_H_L].y;

	corners[CORNER_D_R].x = corners[CORNER_D_L].x + outSizeX;
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

int calcX(int y, double yaw_rad[2], double side, int sizeHigherBorder) {
	double a = 0, b = 0;
	if (sizeHigherBorder >= displaySizeY / 2) // Consider only higher border
			{
		if (yaw_rad[HIGHER_BORDER] == 0)
			return side;
		a = -1 / tan(yaw_rad[HIGHER_BORDER]);
		b = -a * (sizeHigherBorder * tan(yaw_rad[HIGHER_BORDER]) + side);
		return round((y - b) / a);
	} else {
		//TODO Dedicace Halim
	}
}

void drawBorder(Road* road, uint16_t indexCurrentPoint, uint8_t changeCurrentPoint, Vehicle* ego) {
	//States
	static State state = IDLE;
	static double yaw_rad[2] = { 0, 0 };

	//Output of state machine
	int sizeHigherBorder;

	// For interpolation
	double weightIndex, weightNxtIdx;
	double yaw_rad_computed = 0, yaw_rad_joystick = 0;
	static double yaw_rad_screen = 0, distanceEgoToborder = 0;
	double weightYaw;

	// control state changes

	if (state == IDLE
			&& (road->point[indexCurrentPoint].rel.yaw
					- road->point[indexCurrentPoint + 1].rel.yaw) != 0) {
		yaw_rad[HIGHER_BORDER] += road->point[indexCurrentPoint + 1].rel.yaw
				* 3.14 / 180;
		state = START;
	}

	else if (state == START && changeCurrentPoint) {
		yaw_rad_screen = road->point[indexCurrentPoint].rel.yaw * 3.14 / 180;
		state = INTERPOLATE;
	} else if ((state == INTERPOLATE || state == END) && changeCurrentPoint) {
		yaw_rad[HIGHER_BORDER] = 0;
		state = IDLE;
	}

	int offsetY = road->point[indexCurrentPoint].rel.y - displaySizeY / 2;
	yaw_rad_joystick = (double) (ego->v_x * 90 / MAX_JOYSTICK_X) * 3.14 / 180.0;

	//Define states
	if (state == START)
	{
		if (offsetY > displaySizeY / 2)
			sizeHigherBorder = offsetY - displaySizeY / 2;
		else
			sizeHigherBorder = 0;
	}
	else if (state == INTERPOLATE)
	{
		//weightNxtIdx = (double)(road->point[indexCurrentPoint].rel.y-displaySizeY/2) / (double)(displaySizeY);
		//weightIndex = -(double)(road->point[indexCurrentPoint+1].rel.y-displaySizeY/2) / (double)(displaySizeY);
		//yaw_rad_computed = (road->point[indexCurrentPoint].rel.yaw * weightIndex + road->point[indexCurrentPoint+1].rel.yaw * weightNxtIdx) * 3.14 / 180;
		if (fabs(yaw_rad[HIGHER_BORDER]) <= fabs(yaw_rad_joystick)) // Interpolation successful: angle successfully fixed through joystick
		{
			yaw_rad[HIGHER_BORDER] = 0;
			state = END;
		}
		else // angle not fixed yet
		{
			if (offsetY < displaySizeY / 2) // User fixing alone perspective
			{
				yaw_rad_screen -= yaw_rad_joystick;
				yaw_rad[LOWER_BORDER] = yaw_rad_screen - road->point[indexCurrentPoint].rel.yaw * 3.14 / 180;
				yaw_rad[HIGHER_BORDER] = yaw_rad_screen;
				sizeHigherBorder = displaySizeY / 2 + offsetY;
				distanceEgoToborder = fabs(calcX(displaySizeY / 2, yaw_rad, road->side, sizeHigherBorder) - displaySizeY / 2) * cos(yaw_rad[HIGHER_BORDER]);
			}
			else // In addition to user (joystick) we are fixing road perspective and adapting ego vehicle position
			{
				sizeHigherBorder = displaySizeY;
				yaw_rad[LOWER_BORDER] = 0;
				//Change road perspective
				weightYaw = (double) (displaySizeY - offsetY);
				weightYaw /= 120.0;
				road->side -= ((weightYaw * yaw_rad_screen - yaw_rad[HIGHER_BORDER]) / yaw_rad_screen) * tan(yaw_rad_screen) * displaySizeY / 2;
				yaw_rad[HIGHER_BORDER] = weightYaw * yaw_rad_screen - yaw_rad_joystick;
				//road->side += cos(yaw_rad[HIGHER_BORDER]) * (fabs(calcX(displaySizeY/2,yaw_rad,road->side,sizeHigherBorder) - displaySizeY/2) * cos(yaw_rad[HIGHER_BORDER]) - distanceEgoToborder);
				//distanceEgoToborder = fabs(calcX(displaySizeY/2,yaw_rad,road->side,sizeHigherBorder) - displaySizeY/2) * cos(yaw_rad[HIGHER_BORDER]);
			}
		}
	}
	else if (state == END) // Now not used
	{
		sizeHigherBorder = (displaySizeY / 2) + offsetY;
		if (offsetY >= displaySizeY / 2)
		{
			yaw_rad[LOWER_BORDER] = 0;
			state = IDLE;
		}
	}
	else // state == IDLE
	{
		//yaw_rad[HIGHER_BORDER] = -yaw_rad_joystick;
		//yaw_rad[LOWER_BORDER] = -yaw_rad_joystick;
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

//uses info about road points (number and yaw angle) to draw road map
coord roadPointToRelPositionInMap(RoadPoint* lastRoadPoint) {
	uint8_t dist = 10;
	coord mapPos = { 0, 0, 0 };
	mapPos.yaw = lastRoadPoint->rel.yaw + lastRoadPoint->mapPosition.yaw;
	mapPos.x = lastRoadPoint->mapPosition.x + dist * cos((double) mapPos.yaw * 3.14 / 180);
	mapPos.y = lastRoadPoint->mapPosition.y + dist * sin((double) mapPos.yaw * 3.14 / 180);

	return mapPos;
}

//plot road map and vehicle position
void drawMap(Road* road, uint16_t currIdx) {
	gdispFillArea(displaySizeX - MAP_SIZE_X, displaySizeY - MAP_SIZE_Y,
			MAP_SIZE_X, MAP_SIZE_Y, White);
	gdispDrawBox(displaySizeX - MAP_SIZE_X, displaySizeY - MAP_SIZE_Y,
			MAP_SIZE_X, MAP_SIZE_Y, Black);
	uint16_t offsetX = displaySizeX - MAP_SIZE_X + 40, offsetY = displaySizeY
			- MAP_SIZE_Y + 15, normX = 2, normY = 2;
	point map[MAP_POINTS];
	// generate road map position
	road->point[0].mapPosition.x = 0;
	road->point[0].mapPosition.y = 0;
	road->point[0].mapPosition.yaw = 0;
	for (int i = 1; i < ROAD_POINTS / 2; i++)
	{
		road->point[i].mapPosition = roadPointToRelPositionInMap(road->point + i - 1);

	}
	for (int i = 0; i < MAP_POINTS; i++)
	{
		map[i].x = offsetX + road->point[i * RESOLUTION_MAP].mapPosition.x / normX;
		map[i].y = offsetY + road->point[i * RESOLUTION_MAP].mapPosition.y / normY;
	}
	//map[MAP_POINTS-1] = map[0];
	//draw map
	gdispDrawPoly(0, 0, map, MAP_POINTS, Black);
	gdispFillCircle(map[(currIdx % (ROAD_POINTS / 2)) / RESOLUTION_MAP].x, map[(currIdx % (ROAD_POINTS / 2)) / RESOLUTION_MAP].y, 2, Red);
}
