#include "drawTask.h"


void drawTask(Road* road) {
	int v = 0;
	initializeRoad(road);
	int IdxCurrentPointInDisplay = 0;
	road->point[IdxCurrentPointInDisplay].rel.y = displaySizeY/2;
	Vehicle ego;
	ego.rel.y = displaySizeY/2;
	ego.rel.x = displaySizeX/2;

	int ref = 0;
	coord joystickPosition = {0,0,0};

	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const TickType_t tickFramerate = 20;

	int offsetX = 0,
		offsetY = 0;

	while(TRUE)
	{
		//clear display
		gdispClear(White);

		//Read joystick values
		joystickPosition.x = (uint8_t) (ADC_GetConversionValue(ESPL_ADC_Joystick_2) >> 4) - 255/2;
		joystickPosition.y = (uint8_t) (ADC_GetConversionValue(ESPL_ADC_Joystick_1) >> 4);

		//Calculate speed and update x position
		v = joystickPosition.y * CONVERT_JOY;
		ego.rel.x += joystickPosition.x  * CONVERT_JOY;

		// use rel speed and ego vehicle yaw (?) to visualize the right position in the display
		road->point[IdxCurrentPointInDisplay].rel.y += v;
		uint8_t inDisplay, firstTime = 1;
		// update relative positions
		for(int i=IdxCurrentPointInDisplay; i<IdxCurrentPointInDisplay+4; i++)
		{
			//update current index
			road->point[i].rel.y = road->point[IdxCurrentPointInDisplay].rel.y - displaySizeY * (i-IdxCurrentPointInDisplay);
			inDisplay = (road->point[i].rel.y>=-displaySizeY/2 && road->point[i].rel.y<=3*displaySizeY/2);
			if(inDisplay && firstTime)
			{
					if(i>IdxCurrentPointInDisplay)
						offsetX = 0;
					offsetY = road->point[i].rel.y - displaySizeY/2;
					drawBorder(road, IdxCurrentPointInDisplay,&offsetX,&offsetY);
					IdxCurrentPointInDisplay = i;
					firstTime = 0;
			}
		}

		//Draw Vehicles
		gdispFillArea(ego.rel.x,ego.rel.y,10,20,Blue);
		ego.rel.x += calculateReference(road,IdxCurrentPointInDisplay, v);

		// use IdxCurrentPointInDisplay to draw the right position in road map

		// Wait for display to stop writing
		xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
		// swap buffers
		ESPL_DrawLayer();
		vTaskDelayUntil(&xLastWakeTime, tickFramerate);
	}
}

void initializeRoad(Road* road)
{
	int j=0;
	for(int i=0; i<ROAD_POINTS; i++)
	{
		road->point[i].rel.x = displaySizeX/2;
		if((i/30)%2)
			j++;
		else
			j--;
		road->point[i].rel.yaw = j;
		/*
		if(i%2)
			road->point[i].rel.yaw = -5;
		else
			road->point[i].rel.yaw = 5;
		*/
	}
}

void drawBorder(Road* road, uint16_t indexCurrentPoint, int* pOffsetX, int* pOffsetY)
{
	uint8_t side = 60;
	uint8_t outSizeX = 10,
			inSizeX = 6,
			outSizeY = displaySizeY/2,
			inSizeY = displaySizeY/4;

	double weightIndex,weightNxtIdx;
	weightIndex = (double)(road->point[indexCurrentPoint].rel.y-displaySizeY/2) / (double)(displaySizeY);
	weightNxtIdx = -(double)(road->point[indexCurrentPoint+1].rel.y-displaySizeY/2) / (double)(displaySizeY);
	//calculate yaw in the middle point
	double yaw_rad = (road->point[indexCurrentPoint].rel.yaw * weightIndex + road->point[indexCurrentPoint+1].rel.yaw * weightNxtIdx) * 3.14 / 180;

	*pOffsetX = -round(*pOffsetY * tan(yaw_rad));

	int offsetX = *pOffsetX,
		   offsetY = *pOffsetY;
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


	gdispGFillConvexPoly(GDISP,offsetX-2*(corners[0][CORNER_D_L].x-corners[0][CORNER_H_L].x),offsetY-displaySizeY,corners[0],4,Black);
	gdispGFillConvexPoly(GDISP,offsetX-2*(corners[0][CORNER_D_L].x-corners[0][CORNER_H_L].x),offsetY-displaySizeY,corners[1],4,White);

	gdispGFillConvexPoly(GDISP,offsetX-(corners[0][CORNER_D_L].x-corners[0][CORNER_H_L].x),offsetY-displaySizeY/2,corners[0],4,Black);
	gdispGFillConvexPoly(GDISP,offsetX-(corners[0][CORNER_D_L].x-corners[0][CORNER_H_L].x),offsetY-displaySizeY/2,corners[1],4,White);

	gdispGFillConvexPoly(GDISP,offsetX,offsetY,corners[0],4,Black);
	gdispGFillConvexPoly(GDISP,offsetX,offsetY,corners[1],4,White);

	gdispGFillConvexPoly(GDISP,offsetX+corners[0][CORNER_D_L].x-corners[0][CORNER_H_L].x,offsetY+displaySizeY/2,corners[0],4,Black);
	gdispGFillConvexPoly(GDISP,offsetX+corners[0][CORNER_D_L].x-corners[0][CORNER_H_L].x,offsetY+displaySizeY/2,corners[1],4,White);

	gdispGFillConvexPoly(GDISP,offsetX+ROAD_SIZE-2*(corners[0][CORNER_D_L].x-corners[0][CORNER_H_L].x),offsetY-displaySizeY,corners[0],4,Black);
	gdispGFillConvexPoly(GDISP,offsetX+ROAD_SIZE-2*(corners[0][CORNER_D_L].x-corners[0][CORNER_H_L].x),offsetY-displaySizeY,corners[1],4,White);

	gdispGFillConvexPoly(GDISP,offsetX+ROAD_SIZE-(corners[0][CORNER_D_L].x-corners[0][CORNER_H_L].x),offsetY-displaySizeY/2,corners[0],4,Black);
	gdispGFillConvexPoly(GDISP,offsetX+ROAD_SIZE-(corners[0][CORNER_D_L].x-corners[0][CORNER_H_L].x),offsetY-displaySizeY/2,corners[1],4,White);

	gdispGFillConvexPoly(GDISP,offsetX+ROAD_SIZE,offsetY,corners[0],4,Black);
	gdispGFillConvexPoly(GDISP,offsetX+ROAD_SIZE,offsetY,corners[1],4,White);

	gdispGFillConvexPoly(GDISP,offsetX+ROAD_SIZE+corners[0][CORNER_D_L].x-corners[0][CORNER_H_L].x,offsetY+displaySizeY/2,corners[0],4,Black);
	gdispGFillConvexPoly(GDISP,offsetX+ROAD_SIZE+corners[0][CORNER_D_L].x-corners[0][CORNER_H_L].x,offsetY+displaySizeY/2,corners[1],4,White);

}

int calculateReference(Road* road, int index, int v_y)
{
	int ref;
	double weightIndex = 0,
			weightNxtIdx = 0;
	weightIndex = (double)(road->point[index].rel.y-displaySizeY/2) / (double)(displaySizeY);
	weightNxtIdx = -(double)(road->point[index+1].rel.y-displaySizeY/2) / (double)(displaySizeY);
	//calculate yaw in the middle point
	double yawMiddle = (road->point[index].rel.yaw * weightIndex + road->point[index+1].rel.yaw * weightNxtIdx) * 3.14 / 180;
	ref = round(v_y * tan(yawMiddle));
	return ref;
}


//uses info about road points (number and yaw angle) to draw road map
coord roadPointToRelPositionInMap(RoadPoint* lastRoadPoint)
{
}

//plot road map and vehicle position
void drawMap(Road* road)
{
}
