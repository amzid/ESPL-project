#include <includes.h>
#include "game.h"

#include "multiplayer.h"

/*
 * @brief This task is used to draw and update all game elements on display.
 *
 */
void drawTask(Game* game)
{
	char str[100];
	font_t font1;
	font1 = gdispOpenFont("DejaVuSans24*");

	Road* road = game->road[game->chosenMap];
    Vehicle* ego = game->ego;
    Vehicle* bot[NUM_BOTS] = {game->bot1, game->bot2, game->bot3};
    Map* map  = game->map[game->chosenMap];

    Vehicle* rankedVehicles[NUM_VEHICLES] = {ego, bot[0], bot[1], bot[2]};

    Border border;
    border.sizeHigherBorder = 0;
    border.yaw_rad[LOWER_BORDER] = 0;
    border.yaw_rad[HIGHER_BORDER] = 0;

	uint8_t joystickPositionX = 0,
            joystickPositionY = 0;

	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const TickType_t tickFramerate = configTICK_RATE_HZ/50;

	int fps = 50;

    for (int i=0; i<3; i++)
        road->highScores[i] = INITIAL_HIGH_SCORE;
    uint8_t valuesToSend[15];
    uint8_t lastCurrentPoint = 0;

    while (TRUE) {
        if(game->gameState == GAME_PLAYING) {
            game->taktGame++;

            //clear display
            gdispClear(White);

            uint8_t b_endGame = ego->currentRoadPoint == ROAD_POINTS - 1 && ego->distanceFromCurrentRoadPoint >=
                                                                            road->point[ROAD_POINTS -
                                                                                        1].distanceToNextRoadPoint -
                                                                            2 * UNIT_ROAD_DISTANCE;
            //Read joystick values
            if (!b_endGame) {
                if (time_s != 0)
                    fps = game->taktGame / (time_s / 100);
                joystickPositionX = (uint8_t) (ADC_GetConversionValue(ESPL_ADC_Joystick_2) >> 4);
                joystickPositionY = (uint8_t) (ADC_GetConversionValue(ESPL_ADC_Joystick_1) >> 4);
            }
            else {
                if (game->firstTime) {
                    xTimerStop(xTimer, 0);
                    int i = 0;

                    for (i = 0; i < 3; i++) {
                        if (time_s < (uint16_t) (road->highScores[i]))
                            break;
                    }

                    for (int j = 2; j > i; j--)
                        road->highScores[j] = road->highScores[j - 1];

                    road->highScores[i] = time_s;
                    game->firstTime = 0;
                }
                joystickPositionX = 255 / 2;
                joystickPositionY = 255 / 2;
                ego->a_y = 0;
                if (ego->v_y > 0)
                    ego->v_y--;
                else
                    ego->v_y = 0;
            }



            // Ego

            switch(game->mode) {
                case SINGLE_MODE:
                    ego->v_x = (joystickPositionX - 255 / 2) * V_X_MAX / MAX_JOYSTICK_X;
                    ego->a_y = (joystickPositionY - 255 / 2) * A_Y_MAX / MAX_JOYSTICK_Y;
                    if(!b_endGame)
                        calculateVehicleSpeed(ego, 50);
                    ego->changeCurrentPoint = updatePosition(ego, road);

                    calculateBotAcceleration(rankedVehicles, road);
                    calculateLateralSpeed(rankedVehicles, road, &border);
                    for (int i = 0; i < NUM_BOTS; i++) {
                        uint8_t b_endGameBot = bot[i]->currentRoadPoint == ROAD_POINTS - 1 &&
                                               bot[i]->distanceFromCurrentRoadPoint >=
                                               road->point[ROAD_POINTS - 1].distanceToNextRoadPoint -
                                               2 * UNIT_ROAD_DISTANCE;
                        if (!b_endGameBot)
                            calculateVehicleSpeed(bot[i], 50);
                        else {
                            bot[i]->v_x = 0;
                            if (bot[i]->v_y > 0)
                                bot[i]->v_y--;
                            else
                                bot[i]->v_y = 0;
                        }
                        updatePosition(bot[i], road);
                        updateXPosition(bot[i], &border, road);
                    }
                    break;
                case MULTIPLAYER_MODE:
                    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
                    switch (game->controlState) { // The other value is updated in the receive function
                        case SPEED_CTRL:
                            ego->a_y = (joystickPositionY - 255 / 2) * A_Y_MAX / MAX_JOYSTICK_Y;
                            if(!b_endGame)
                                calculateVehicleSpeed(ego, 50);

                            calculateBotAcceleration(rankedVehicles, road);
                            for (int i = 0; i < NUM_BOTS; i++) {
                                uint8_t b_endGameBot = bot[i]->currentRoadPoint == ROAD_POINTS - 1 &&
                                                       bot[i]->distanceFromCurrentRoadPoint >=
                                                       road->point[ROAD_POINTS - 1].distanceToNextRoadPoint -
                                                       2 * UNIT_ROAD_DISTANCE;
                                if (!b_endGameBot)
                                    calculateVehicleSpeed(bot[i], 50);
                                else {
                                    bot[i]->v_x = 0;
                                    if (bot[i]->v_y > 0)
                                        bot[i]->v_y--;
                                    else
                                        bot[i]->v_y = 0;
                                }
                                updatePosition(bot[i], road);
                            }

                            valuesToSend[0] = (int) (ego->v_y);
                            valuesToSend[1] = game->gameState;
                            valuesToSend[2] = (int) (ego->v_y * 100) % 100;

                            valuesToSend[3] = (int) (bot[0]->currentRoadPoint);
                            valuesToSend[4] = (uint8_t) (bot[0]->distanceFromCurrentRoadPoint& 0x00FF);
                            valuesToSend[5] = (uint8_t) ((bot[0]->distanceFromCurrentRoadPoint & 0xFF00) >> 8);

                            valuesToSend[6] = (int) (bot[1]->currentRoadPoint);
                            valuesToSend[7] = (uint8_t) (bot[1]->distanceFromCurrentRoadPoint& 0x00FF);
                            valuesToSend[8] = (uint8_t) ((bot[1]->distanceFromCurrentRoadPoint & 0xFF00) >> 8);

                            valuesToSend[9] = (int) (bot[2]->currentRoadPoint);
                            valuesToSend[10] = (uint8_t) (bot[2]->distanceFromCurrentRoadPoint& 0x00FF);
                            valuesToSend[11] = (uint8_t) ((bot[2]->distanceFromCurrentRoadPoint & 0xFF00) >> 8);

                            sendviaUart(valuesToSend, 12);
                            break;
                        case STEERING_CTRL:
                            ego->v_x = (joystickPositionX - 255 / 2) * V_X_MAX / MAX_JOYSTICK_X;
                            ego->changeCurrentPoint = updatePosition(ego, road);

                            calculateLateralSpeed(rankedVehicles, road, &border);
                            for (int i = 0; i < NUM_BOTS; i++)
                                updateXPosition(bot[i], &border, road);

                            valuesToSend[0] = joystickPositionX;
                            valuesToSend[1] = game->gameState;
                            valuesToSend[2] = (int) (bot[0]->rel.x);
                            valuesToSend[3] = (int) (bot[1]->rel.x);
                            valuesToSend[4] = (int) (bot[2]->rel.x);

                            valuesToSend[5] = (int) (ego->currentRoadPoint);
                            valuesToSend[6] = (uint8_t) (ego->distanceFromCurrentRoadPoint& 0x00FF);
                            valuesToSend[7] = (uint8_t) ((ego->distanceFromCurrentRoadPoint & 0xFF00) >> 8);
                            valuesToSend[8] = (uint8_t) round(road->side);

                            for(int i=9; i<12; i++)
                                valuesToSend[i] = 0;
                            sendviaUart(valuesToSend, SIZE_VALUES_TO_SEND);
                            break;
                    }
                   xTaskNotifyGive(receiveHdl);
                    break;
            }

            ego->rel.x = displaySizeX / 2 - calcX(&border, displaySizeY / 2, road->side);
            if(!b_endGame)
               updateRanking(ego, bot, road, rankedVehicles);
            checkCarCollision(rankedVehicles);

            sprintf(str, "Uart: %d", (int) game->taktUART);
            gdispDrawString(0, 11, str, font1, Black);

            sprintf(str, "Game: %d ", (int) game->taktGame);
            gdispDrawString(0, 22, str, font1, Black);

            sprintf(str, "S.Oth.P:: %d", game->gameStateOtherPlayer);
            gdispDrawString(0, 44, str, font1, Black);

            sprintf(str, "Vx: %d.%d", (int)ego->v_x, (int) (100 * ego->v_x) % 100);
            gdispDrawString(0, 55, str, font1, Black);


            // DRAW
            ego->changeCurrentPoint = (lastCurrentPoint != ego->currentRoadPoint);
            drawBorder(road, ego->currentRoadPoint, ego->changeCurrentPoint, ego, &border);
            lastCurrentPoint = ego->currentRoadPoint;
            for (int i = 0; i < NUM_BOTS; i++)
                drawBot(bot[i], ego, &border, road);
            gdispFillArea(displaySizeX / 2, ego->rel.y, VEHICLE_SIZE_X, VEHICLE_SIZE_Y, Red);

            sprintf(str, "Bot_CP: %d", (int)bot[0]->currentRoadPoint);
            gdispDrawString(0, 77, str, font1, Black);
            sprintf(str, "Bot_d: %d", (int)bot[0]->distanceFromCurrentRoadPoint);
            gdispDrawString(0, 88, str, font1, Black);
            sprintf(str, "Bot_x: %d", (int)bot[0]->rel.x);
            gdispDrawString(0, 99, str, font1, Black);

            sprintf(str, "ego current point: %d",ego->currentRoadPoint);
            gdispDrawString(0, 101, str, font1, Black);
            sprintf(str, "ego distance %d",ego->distanceFromCurrentRoadPoint);
            gdispDrawString(0, 111, str, font1, Black);
            sprintf(str, "mode %d",game->mode);
            gdispDrawString(0, 133, str, font1, Black);

            //TODO: Manage end of game
            if (ego->currentRoadPoint == ROAD_POINTS - 1 && ego->distanceFromCurrentRoadPoint >= road->point[ROAD_POINTS-1].distanceToNextRoadPoint - 3 * UNIT_ROAD_DISTANCE) {
                gdispDrawLine(road->side, displaySizeY / 2 - (road->point[ROAD_POINTS-1].distanceToNextRoadPoint - ego->distanceFromCurrentRoadPoint- 2 * UNIT_ROAD_DISTANCE),road->side + ROAD_SIZE ,displaySizeY / 2 - (road->point[ROAD_POINTS-1].distanceToNextRoadPoint - ego->distanceFromCurrentRoadPoint- 2 * UNIT_ROAD_DISTANCE), Black );
                sprintf(str, "High Scores:");
                gdispDrawString(displaySizeX/2-8, 11, str, font1, Black);
                for(int i=0; i<3 && road->highScores[i] != INITIAL_HIGH_SCORE; i++)
                {
                    sprintf(str, "%d) %d.%d%d",i+1,road->highScores[i]/100, (road->highScores[i]/10) % 10, road->highScores[i] % 10);
                    gdispDrawString(displaySizeX/2-8, 22+i*11, str, font1, Black);
                }
            }

            drawMap(road, ego, bot, map);
            drawInfo(ego,road,fps);

            // Wait for display to stop writing
            xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
            // swap buffers
            ESPL_DrawLayer();
            vTaskDelayUntil(&xLastWakeTime, tickFramerate);
        }
        else {
            if(game->gameState == GAME_PAUSED){
                //gdispClear(White);
                ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
                valuesToSend[0]++;
                valuesToSend[1] = game->gameState;
                sendviaUart(valuesToSend, SIZE_VALUES_TO_SEND);
                /*

sprintf(str, "Hallo %d",valuesToSend[0]);
gdispDrawString(0, 144, str, font1, Black);
sprintf(str, "mode %d",game->mode);
gdispDrawString(0, 133, str, font1, Black);
 */
                xTaskNotifyGive(receiveHdl);
                xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
                ESPL_DrawLayer();
            }
            else
                vTaskDelay(20);
        }
	}
}

void drawInfo(Vehicle* ego,Road* road,int fps){
    char str[100];
    font_t font1;
    font1 = gdispOpenFont("DejaVuSans24*");

    //Draw map place holder
    gdispFillArea(0, displaySizeY - MAP_SIZE_Y,MAP_SIZE_X, MAP_SIZE_Y, White);
    gdispDrawBox(0, displaySizeY - MAP_SIZE_Y,MAP_SIZE_X, MAP_SIZE_Y, Black);

    sprintf(str, "Elapsed time: %d.%d%d", time_s/100,(time_s/10) % 10, time_s % 10);
    gdispDrawString(1, displaySizeY - MAP_SIZE_Y +10, str, font1, Black);
    sprintf(str, "Speed: %d.%d", (int) ego->v_y, (int) (ego->v_y*10 - 10 * (int)ego->v_y));
    gdispDrawString(1, displaySizeY - MAP_SIZE_Y +30, str, font1, Black);
    sprintf(str, "Rank: %d", ego->ranking+1);
    gdispDrawString(1, displaySizeY - MAP_SIZE_Y +50, str, font1, Black);

    sprintf(str, "fps %d", fps);
    gdispDrawString(0, 0, str, font1, Black);

}

void checkCarCollision(Vehicle* rankedVehicles[NUM_VEHICLES])
{
    for(int i=1; i<NUM_VEHICLES; i++) {
        for(int j=0; j<i; j++)
        {
            if(fabs(rankedVehicles[i]->rel.x - rankedVehicles[j]->rel.x) < VEHICLE_SIZE_X
               && fabs(rankedVehicles[i]->rel.y - rankedVehicles[j]->rel.y) < VEHICLE_SIZE_Y)
            {
                rankedVehicles[j]->distanceFromCurrentRoadPoint += 4;
                rankedVehicles[i]->distanceFromCurrentRoadPoint -= 4 * (rankedVehicles[i]->distanceFromCurrentRoadPoint - 4 >= 0);
            }
        }
    }
}

void calculateLateralSpeed(Vehicle* rankedVehicles[NUM_VEHICLES], Road* road, Border* border){
    char str[100];
    font_t font1;
    font1 = gdispOpenFont("DejaVuSans24*");

    double side = 0, x_soll = 0;
    for(int i=0; i<NUM_VEHICLES; i++) {
        if(rankedVehicles[i]->color != RED) {
            Vehicle *bot = rankedVehicles[i];
            side = calcX(border,bot->rel.y,road->side);
            if(bot->ranking == 0) {
                x_soll = ROAD_SIZE / 2;
            }
            else{
                //if(fabs(rankedVehicles[i]->rel.y - rankedVehicles[i-1]->rel.y) <= UNIT_ROAD_DISTANCE) {
                    if (rankedVehicles[i]->rel.x <= rankedVehicles[i - 1]->rel.x + VEHICLE_SIZE_X / 2) // überholen in the left direction of next vehicle
                    {
                        x_soll = (rankedVehicles[i - 1]->rel.x + (SIZE_BORDER)) / 2;
                    } else {
                        x_soll = (rankedVehicles[i - 1]->rel.x + VEHICLE_SIZE_X + (ROAD_SIZE)) / 2;
                    }
                //}
//                else
//                {
//                    x_soll = rankedVehicles[i]->rel.x;
//                }
            }

            rankedVehicles[i]->v_x = round(x_soll - rankedVehicles[i]->rel.x) / 50.0;
            /*
            if(rankedVehicles[i]->color == BLUE){
                sprintf(str, "Xsoll: %d", (int)round(x_soll));
                gdispDrawString(0, 144, str, font1, Black);
                sprintf(str, "Vx: %d", (int)round(rankedVehicles[i]->v_x * 100));
                gdispDrawString(0, 155, str, font1, Black);
            }
             */
        }
    }
}

void updateXPosition(Vehicle* bot, Border* border, Road* road){
    if(bot->rel.x <= SIZE_BORDER)
    {
        bot->state = COLLISION_WITH_BORDER;
        bot->v_y = 1;
    }
    else {
        bot->state = NO_COLLISION;
    }
    bot->rel.x += bot->v_x * (bot->rel.x + bot->v_x >= SIZE_BORDER && bot->rel.x + bot->v_x + VEHICLE_SIZE_X < ROAD_SIZE);
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


void drawBot(Vehicle* bot, Vehicle*  ego, Border* border, Road* road){
    char str[100];
    font_t font1;
    font1 = gdispOpenFont("DejaVuSans24*");

    int distanceBotToEgo = 0xFFFF;
    if(bot->currentRoadPoint % LAP_POINTS == ego->currentRoadPoint  % LAP_POINTS) {
        distanceBotToEgo = bot->distanceFromCurrentRoadPoint - ego->distanceFromCurrentRoadPoint;
    }
    else if((bot->currentRoadPoint-1) % LAP_POINTS == ego->currentRoadPoint  % LAP_POINTS) {
        distanceBotToEgo = (bot->distanceFromCurrentRoadPoint + (road->point[ego->currentRoadPoint].distanceToNextRoadPoint - ego->distanceFromCurrentRoadPoint));
    }
    else if(bot->currentRoadPoint  % LAP_POINTS == (ego->currentRoadPoint-1) % LAP_POINTS) {
        distanceBotToEgo = -(ego->distanceFromCurrentRoadPoint + (road->point[bot->currentRoadPoint].distanceToNextRoadPoint - bot->distanceFromCurrentRoadPoint));
    }
    if(fabs(distanceBotToEgo) <= displaySizeY/2 ){
        bot->rel.y = displaySizeY/2 - distanceBotToEgo;

        if(bot->color == GREEN) {
            /*
            sprintf(str, "side: %d", (int)round(calcX(border, bot->rel.y, road->side)));
            gdispDrawString(0, 166, str, font1, Black);
            sprintf(str, "relX: %d", (int)round(bot->rel.x));
            gdispDrawString(0, 177, str, font1, Black);
             */
            gdispFillArea(bot->rel.x + calcX(border, bot->rel.y, road->side), bot->rel.y, VEHICLE_SIZE_X, VEHICLE_SIZE_Y, Green);
        }
        else if(bot->color == YELLOW) {
            gdispFillArea(bot->rel.x + calcX(border, bot->rel.y, road->side), bot->rel.y, VEHICLE_SIZE_X, VEHICLE_SIZE_Y, Yellow);
        }
        else if(bot->color == BLUE) {
            gdispFillArea(bot->rel.x + calcX(border, bot->rel.y, road->side), bot->rel.y, VEHICLE_SIZE_X, VEHICLE_SIZE_Y, Blue);
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
            double v_soll = road->point[bot->currentRoadPoint + 1].rel.yaw * 10.0 / 45;
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
        if(fps != 0) {
            vehicle->v_y += vehicle->a_y / (double) fps;
        }
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
    char str[100];
    font_t font1;
    font1 = gdispOpenFont("DejaVuSans24*");
	//States: border->yaw_rad[2]


	//Output of state machine: border->sizeHigherBorder

	// For interpolation
	static double yaw_rad_screen = 0;

	//For input data
	double yaw_rad_joystick = 0;
	uint16_t distanceToNextRoadPoint = 0, offsetY = 0;

	//Calculate input y comp
	distanceToNextRoadPoint = (uint16_t)(road->point[indexCurrentPoint].distanceToNextRoadPoint - ego->distanceFromCurrentRoadPoint);
	offsetY = (uint16_t)((ego->distanceFromCurrentRoadPoint)%UNIT_ROAD_DISTANCE);

	//calculate input x comp
	yaw_rad_joystick = (ego->v_x * MAX_JOYSTICK_ANGLE_X / MAX_JOYSTICK_X) * 3.14 / 180.0;

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

    sprintf(str, "RS: %d", road->state);
    gdispDrawString(0, 122, str, font1, Black);


    if(fabs(yaw_rad_screen+yaw_rad_joystick) < fabs(yaw_rad_screen))
        yaw_rad_screen += yaw_rad_joystick;


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
                border->yaw_rad[LOWER_BORDER] = yaw_rad_screen - road->point[indexCurrentPoint].rel.yaw * 3.14 / 180;
                border->yaw_rad[HIGHER_BORDER] = yaw_rad_screen;

				if(ego->distanceFromCurrentRoadPoint <= UNIT_ROAD_DISTANCE/2)
                    border->sizeHigherBorder = displaySizeY / 2 + offsetY;
				else
				{
                    border->yaw_rad[LOWER_BORDER] = 0;
                    border->sizeHigherBorder = displaySizeY;
					road->side += ego->v_y * tan(border->yaw_rad[HIGHER_BORDER]);
				}

				// Check for collision
				if(checkIfCollisionWithBorder(border,road->side)) // if between ego vehicle and border less than 3 px -> collision
				{
					ego->state = COLLISION_WITH_BORDER;
					ego->v_y = 0;
                    // yaw_rad_screen -= 0.01;
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

	road->side -= ego->v_y * tan(yaw_rad_joystick);

	//Check collision
	if(road->state != MIDDLE_CURVE)
	{
		if(displaySizeX/2 - road->side - SIZE_BORDER <= 0) // Collision with left side
		{
			road->side = displaySizeX/2 - SIZE_BORDER - 3;
			gdispFillArea(displaySizeX/2 - 3, ego->rel.y, 3, VEHICLE_SIZE_Y, Red);
			ego->state = COLLISION_WITH_BORDER;
			if(ego->v_y > 1)
				ego->v_y --;
		}
		else if (road->side + ROAD_SIZE - displaySizeX/2 - VEHICLE_SIZE_X <= 0) // Collision with right side
		{
			road->side = displaySizeX/2 + VEHICLE_SIZE_X - ROAD_SIZE + 3;
			gdispFillArea(displaySizeX/2 + VEHICLE_SIZE_X, ego->rel.y, 3, VEHICLE_SIZE_Y, Red);
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
        a = -1 / tan(yaw_rad[LOWER_BORDER]);
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
	gdispDrawPoly(displaySizeX - MAP_SIZE_X + 60, displaySizeY - MAP_SIZE_Y + 15, map, MAP_POINTS, Black);

	//Draw ego vehicle position
    drawVehiclePositionOnMap(ego, map);
    for(int i=0; i<NUM_BOTS; i++)
        drawVehiclePositionOnMap(bot[i], map);
}

void drawVehiclePositionOnMap(Vehicle* vehicle, Map* map)
{
    static uint8_t colorIndex = 0;

    int16_t dist = vehicle->distanceFromCurrentRoadPoint  * UNIT_ROAD_DISTANCE_IN_MAP / UNIT_ROAD_DISTANCE;
    int16_t vehicleMapX = displaySizeX - MAP_SIZE_X + 60 + map->point[(vehicle->currentRoadPoint % (ROAD_POINTS / 2))].x + dist * cos((double) map->orientation[(vehicle->currentRoadPoint % (ROAD_POINTS / 2))] * 3.14 / 180),
            vehicleMapY = displaySizeY - MAP_SIZE_Y + 15 + map->point[(vehicle->currentRoadPoint % (ROAD_POINTS / 2))].y + dist * sin((double) map->orientation[(vehicle->currentRoadPoint % (ROAD_POINTS / 2))] * 3.14 / 180);
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
