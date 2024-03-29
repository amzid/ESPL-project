#include <includes.h>
#include "game.h"

#include "multiplayer.h"
#include "reset.h"

/*
 * @brief This task is used to draw and update all game elements on display.
 *
 */
void drawTask(Game* game) {

    // Get parameters from menu
    Road *road = game->road[game->chosenMap];
    Vehicle *ego = game->ego;
    Vehicle *bot[NUM_BOTS] = {game->bot1, game->bot2, game->bot3};
    Map *map = game->map[game->chosenMap];

    // Initialize local variables
    Vehicle *rankedVehicles[NUM_VEHICLES] = {ego, bot[0], bot[1], bot[2]};

    Border border;
    border.sizeHigherBorder = 0;
    border.yaw_rad[LOWER_BORDER] = 0;
    border.yaw_rad[HIGHER_BORDER] = 0;

    uint8_t joystickPositionX = 0,
            joystickPositionY = 0;

    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    const TickType_t tickFramerate = configTICK_RATE_HZ / 50;

    for (int i = 0; i < 3; i++)
        road->highScores[i] = INITIAL_HIGH_SCORE;
    uint8_t valuesToSend[SIZE_VALUES_TO_SEND];
    uint8_t lastCurrentPoint = 0;
    volatile uint8_t b_endGame = 0, b_endGameBot = 0;

    game->firstTimeStartGame = 1;

    while (TRUE) {
        if (game->gameState == GAME_PLAYING) {
            tactDrawTask++;
            if ((game->firstTimeStartGame && game->gameStateOtherPlayer == GAME_PLAYING) ||
                (game->firstTimeStartGame && game->mode == SINGLE_MODE)) {
                //tactDrawTask = xTaskGetTickCount();
                road = game->road[game->chosenMap];
                map = game->map[game->chosenMap];
                game->firstTimeStartGame = 0;
                game->firstTimeEndGame = 1;
                game->taktGame = 0;
                game->taktUART = 0;
                b_endGame = 0;
                b_endGameBot = 0;
                time_s = 100;
                if (xTimerIsTimerActive(xTimer) == pdFALSE)
                    xTimerStart(xTimer, 0);
            }

            game->taktGame++;

            //clear display
            gdispClear(White);

            if (time_s >= 200)
                b_endGame = b_endGame || (ego->currentRoadPoint == ROAD_POINTS - 1
                                          && ego->distanceFromCurrentRoadPoint >=
                                             road->point[ROAD_POINTS - 1].distanceToNextRoadPoint -
                                             2 * UNIT_ROAD_DISTANCE) || (ego->currentRoadPoint > ROAD_POINTS - 1);
            else
                b_endGame = 0;
            // change state to end of game
            if (b_endGame) //ego->state != END_GAME &&
                ego->state = END_GAME;

            for (int i = 0; i < NUM_BOTS; i++) {
                b_endGameBot = (bot[i]->currentRoadPoint == ROAD_POINTS - 1 &&
                                bot[i]->distanceFromCurrentRoadPoint >=
                                road->point[ROAD_POINTS - 1].distanceToNextRoadPoint -
                                2 * UNIT_ROAD_DISTANCE) || (bot[i]->currentRoadPoint > ROAD_POINTS - 1);
                if (b_endGameBot) // bot[i]->state != END_GAME &&
                    bot[i]->state = END_GAME;
            }
            if (ego->state != END_GAME) {
                //Read joystick values
                joystickPositionX = (uint8_t) (ADC_GetConversionValue(ESPL_ADC_Joystick_2) >> 4);
                joystickPositionY = (uint8_t) (ADC_GetConversionValue(ESPL_ADC_Joystick_1) >> 4);
            } else {
                if (game->firstTimeEndGame) {
                    xTimerStop(xTimer, 0);
                    int i = 0;

                    for (i = 0; i < 3; i++) {
                        if (time_s < (uint16_t) (road->highScores[i]))
                            break;
                    }

                    for (int j = 2; j > i; j--)
                        road->highScores[j] = road->highScores[j - 1];

                    road->highScores[i] = time_s;
                    game->firstTimeEndGame = 0;
                }
                joystickPositionX = 255 / 2;
                joystickPositionY = 255 / 2;
                ego->a_y = 0;
                ego->v_y = 0;
            }

            if (game->mode == MULTIPLAYER_MODE)
                ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

            ego->rel.x = displaySizeX / 2 - calcX(&border, displaySizeY / 2, road->side);
            if (!b_endGame)
                updateRanking(ego, bot, road, rankedVehicles);

            // Update positions based on collision detection
            for (int i = 0; i < NUM_BOTS; i++)
                updateRelY(bot[i], ego, road);
            checkCarCollision(rankedVehicles);
            for (int i = 0; i < NUM_BOTS; i++)
                updateRelY(bot[i], ego, road);
            // DRAW
            ego->changeCurrentPoint = (lastCurrentPoint != ego->currentRoadPoint);
            if (ego->state != END_GAME)
                updateRoadStatesAndCalculateBorderPosition(road, ego->currentRoadPoint, ego->changeCurrentPoint, ego, &border);
            lastCurrentPoint = ego->currentRoadPoint;
            // Gras and road
            drawGrassAndRoad(road, &border, ego);

            // Manage game end and draw game end line
            if (ego->currentRoadPoint == ROAD_POINTS - 1 && ego->distanceFromCurrentRoadPoint >=
                                                            road->point[ROAD_POINTS - 1].distanceToNextRoadPoint -
                                                            3 * UNIT_ROAD_DISTANCE) {
                drawFinishGameAndHighScores(road, ego);
            }
            if (ego->currentRoadPoint == (LAP_POINTS - 1) && ego->distanceFromCurrentRoadPoint >=
                                                             road->point[LAP_POINTS - 1].distanceToNextRoadPoint -
                                                             3 * UNIT_ROAD_DISTANCE) {
                Box box = {road->side + SIZE_BORDER, displaySizeY / 2 -
                                                     (road->point[ROAD_POINTS - 1].distanceToNextRoadPoint -
                                                      ego->distanceFromCurrentRoadPoint - 2 * UNIT_ROAD_DISTANCE), 20,
                           10};
                for (int i = 0; i < 7; i++) {
                    box.color = White * !(i % 2) + Black * ((i % 2));
                    drawBoxGame(&box, 0);
                    box.x += 20;
                }
            }

            //Draw vehicles
            for (int i = 0; i < NUM_BOTS; i++)
                drawBot(bot[i], ego, &border, road);
            drawVehicle(displaySizeX / 2, ego->rel.y, Red);

            drawMap(road, ego, bot, map);
            drawInfo(ego, road, fps);

            // Calculation and send process based on game mode and control choice
            switch (game->mode) {
                case SINGLE_MODE:
                    ego->v_x = (joystickPositionX - 255 / 2) * V_X_MAX / MAX_JOYSTICK_X;
                    ego->a_y = (joystickPositionY - 255 / 2) * A_Y_MAX / MAX_JOYSTICK_Y;
                    if (ego->state != END_GAME)
                        calculateVehicleSpeed(ego, 50);
                    ego->changeCurrentPoint = updatePosition(ego, road);

                    calculateBotAcceleration(rankedVehicles, road);
                    calculateLateralSpeed(rankedVehicles, road, &border);
                    for (int i = 0; i < NUM_BOTS; i++) {
                        if (bot[i]->state != END_GAME)
                            calculateVehicleSpeed(bot[i], 50);
                        else {
                            bot[i]->v_x = 0;
                            bot[i]->v_y = 0;
                            bot[i]->a_y = 0;
                        }
                        updatePosition(bot[i], road);
                        updateXPosition(bot[i], &border, road);
                    }
                    break;
                    case MULTIPLAYER_MODE:
                        // In case of Multiplayer mode:
                        // - When controling speed, ego y speed and and bots y position are calculated and sent
                        // - When controling steering, ego position and and bots x position are calculated and sent
                        switch (game->controlState) { // The other value is updated in the receive function
                        case SPEED_CTRL:
                            // Compute new values
                            //Ego
                            ego->a_y = (joystickPositionY - 255 / 2) * A_Y_MAX / MAX_JOYSTICK_Y;
                            if (ego->state != END_GAME)
                                calculateVehicleSpeed(ego, 50);
                            //Bots
                            calculateBotAcceleration(rankedVehicles, road);
                            for (int i = 0; i < NUM_BOTS; i++) {
                                if (bot[i]->state != END_GAME)
                                    calculateVehicleSpeed(bot[i], 50);
                                else {
                                    bot[i]->v_x = 0;
                                    bot[i]->v_y = 0;
                                    bot[i]->a_y = 0;
                                }
                                updatePosition(bot[i], road);
                            }
                            // Send computed values
                            valuesToSend[0] = (int) (ego->v_y);
                            valuesToSend[1] = game->gameState;
                            valuesToSend[2] = (int) (ego->v_y * 100) % 100;
                            valuesToSend[3] = (int) (bot[0]->currentRoadPoint);
                            valuesToSend[4] = (uint8_t) (bot[0]->distanceFromCurrentRoadPoint & 0x00FF);
                            valuesToSend[5] = (uint8_t) ((bot[0]->distanceFromCurrentRoadPoint & 0xFF00) >> 8);
                            valuesToSend[6] = (int) (bot[1]->currentRoadPoint);
                            valuesToSend[7] = (uint8_t) (bot[1]->distanceFromCurrentRoadPoint & 0x00FF);
                            valuesToSend[8] = (uint8_t) ((bot[1]->distanceFromCurrentRoadPoint & 0xFF00) >> 8);
                            valuesToSend[9] = (int) (bot[2]->currentRoadPoint);
                            valuesToSend[10] = (uint8_t) (bot[2]->distanceFromCurrentRoadPoint & 0x00FF);
                            valuesToSend[11] = (uint8_t) ((bot[2]->distanceFromCurrentRoadPoint & 0xFF00) >> 8);
                            sendviaUart(valuesToSend, 12);
                            break;
                        case STEERING_CTRL:
                            // Compute new values
                            // Ego
                            ego->v_x = (joystickPositionX - 255 / 2) * V_X_MAX / MAX_JOYSTICK_X;
                            ego->changeCurrentPoint = updatePosition(ego, road);
                            // Bots
                            calculateLateralSpeed(rankedVehicles, road, &border);
                            for (int i = 0; i < NUM_BOTS; i++)
                                updateXPosition(bot[i], &border, road);
                            // Send computed values
                            valuesToSend[0] = joystickPositionX;
                            valuesToSend[1] = game->gameState;
                            valuesToSend[2] = (int) (bot[0]->rel.x);
                            valuesToSend[3] = (int) (bot[1]->rel.x);
                            valuesToSend[4] = (int) (bot[2]->rel.x);
                            valuesToSend[5] = (int) (ego->currentRoadPoint);
                            valuesToSend[6] = (uint8_t) (ego->distanceFromCurrentRoadPoint & 0x00FF);
                            valuesToSend[7] = (uint8_t) ((ego->distanceFromCurrentRoadPoint & 0xFF00) >> 8);
                            valuesToSend[8] = (uint8_t) round(road->side);
                            for (int i = 9; i < 12; i++)
                                valuesToSend[i] = 0;
                            sendviaUart(valuesToSend, SIZE_VALUES_TO_SEND);
                            break;
                    }
                    xTaskNotifyGive(receiveHdl);
                    break;
            }

            // Wait for display to stop writing
            xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
            // swap buffers
            ESPL_DrawLayer();
            // Maintain a fixed fps of 50Hz
            vTaskDelayUntil(&xLastWakeTime, tickFramerate);
        }
        else if (game->gameState == GAME_PAUSED) {
            gdispClear(Black);
            font1 = gdispOpenFont("UI1 Narrow");
            sprintf(str, "TAKE A DEEP BREATH !!");
            gdispDrawString(displaySizeX / 2 - 50, displaySizeY / 2 - 30, str, font1, White);
            sprintf(str, "TO RESUME PRESS B");
            gdispDrawString(displaySizeX / 2 - 50, displaySizeY / 2 + 30, str, font1, White);

            if (game->mode == MULTIPLAYER_MODE)
                ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            valuesToSend[0]++;
            valuesToSend[1] = game->gameState;
            sendviaUart(valuesToSend, SIZE_VALUES_TO_SEND);
            if (game->mode == MULTIPLAYER_MODE)
                xTaskNotifyGive(receiveHdl);
            xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
            // swap buffers
            ESPL_DrawLayer();
            vTaskDelayUntil(&xLastWakeTime, tickFramerate);
        }
        else if (game->gameState == START_GAME) {
            if (xTimerIsTimerActive(xTimer) == pdFALSE)
                xTimerStart(xTimer, 0);
            if (game->mode == MULTIPLAYER_MODE)
                ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            for (int i = 0; i < 12; i++)
                valuesToSend[i] = 0;
            valuesToSend[0] = (uint8_t) (game->menuState);
            valuesToSend[1] = game->gameState;
            valuesToSend[2] = (uint8_t) (game->mode);
            valuesToSend[3] = (uint8_t) (game->chosenMap);
            valuesToSend[4] = (uint8_t) (game->controlState);
            sendviaUart(valuesToSend, SIZE_VALUES_TO_SEND);
            game->taktGame++;
            game->firstTimeStartGame = 1;
            game->firstTimeEndGame = 1;
            b_endGame = 0;
            b_endGameBot = 0;

            initializeVehicle(game->ego);
            initializeVehicle(game->bot1);
            initializeVehicle(game->bot2);
            initializeVehicle(game->bot3);
            initializeRoad(game->road[game->chosenMap], game->ego, game->chosenMap);

            if (time_s >= 300 || (time_s >= 250 && game->gameStateOtherPlayer == GAME_PLAYING)) {
                game->gameState = GAME_PLAYING;
                time_s = 0;
            }

            gdispClear(Black);
            font1 = gdispOpenFont("UI2 Double");

            if (time_s <= 100) {
                sprintf(str, "3");
                gdispDrawStringBox(0, 0, displaySizeX, displaySizeY, str, font1, Red, justifyCenter);
            } else if (time_s <= 200) {
                sprintf(str, "2");
                gdispDrawStringBox(0, 0, displaySizeX, displaySizeY, str, font1, Orange, justifyCenter);
            } else {
                sprintf(str, "1");
                gdispDrawStringBox(0, 0, displaySizeX, displaySizeY, str, font1, Green, justifyCenter);
            }

            if (game->mode == MULTIPLAYER_MODE)
                xTaskNotifyGive(receiveHdl);
            // Wait for display to stop writing
            xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
            // swap buffers
            ESPL_DrawLayer();
            vTaskDelayUntil(&xLastWakeTime, tickFramerate);
        }
        else { // If none of the above mentioned game states
            vTaskDelay(20);
        }
    }
}


/************************************************************************************************************************/
/*
 * Calculations
 */
/************************************************************************************************************************/

/*
 * @brief update vehicle y rel position
 */
void calculateVehicleSpeed(Vehicle* vehicle, int fps)
{
    if(vehicle->state == NO_COLLISION) // if collision speed is set to 0.
    {
        if(fps != 0) {
            vehicle->v_y += vehicle->a_y / (double) fps;
        }
        if(vehicle->v_y<=0)
            vehicle->v_y=2;
    }
    if(vehicle->v_y>=V_Y_MAX-15)
        vehicle->v_y=V_Y_MAX-15;
}


/*
 * @brief Main road state machine, detect curves and compute border position accordingly
 */
void updateRoadStatesAndCalculateBorderPosition(Road* road, uint16_t indexCurrentPoint, uint8_t changeCurrentPoint, Vehicle* ego, Border* border)
{
    // Local variables
    // For interpolation
    static double yaw_rad_screen = 0;
    //For input data
    double yaw_rad_joystick = 0;
    uint16_t distanceToNextRoadPoint = 0, offsetY = 0;

    //States: border->yaw_rad[2]
    //Output of state machine: border->sizeHigherBorder

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
            if(ego->v_y > 2)
                ego->v_y -= REDUCED_VEL_COL_BORDER;
        }
        else if (road->side + ROAD_SIZE - displaySizeX/2 - VEHICLE_SIZE_X <= 0) // Collision with right side
        {
            road->side = displaySizeX/2 + VEHICLE_SIZE_X - ROAD_SIZE + 3;
            gdispFillArea(displaySizeX/2 + VEHICLE_SIZE_X, ego->rel.y, 3, VEHICLE_SIZE_Y, Red);
            ego->state = COLLISION_WITH_BORDER;
            if(ego->v_y > 2)
                ego->v_y -= REDUCED_VEL_COL_BORDER;
        }
        else
        {
            ego->state = NO_COLLISION;
        }
    }
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
 * @brief Update vehicle relative positions according to given CP and distance
 */
void updateRelY(Vehicle* bot, Vehicle* ego, Road* road ){
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
    bot->rel.y = displaySizeY/2 - distanceBotToEgo;
}

/*
 * @brief Update vehicle rankings
 */
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

/*
 * @brief update vehicle position (CP and dist) based on given speed
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

/*
 * @brief calculate total distance when given a CP
 */
int calcDist(int roadPoint, Road* road)
{
    if(roadPoint < 0)
        return 0;
    return road->point[roadPoint].distanceToNextRoadPoint + calcDist(roadPoint-1, road);
}

/************************************************************************************************************************/
/*
 * Draw functions
 */
/************************************************************************************************************************/

void drawVehicle(uint16_t x, uint16_t y, int color){
    uint8_t wheelSizeX = 5, wheelSizeY = 10,
            wheelHorizX = 3, wheelHorizY = 2,
            wheelPositionY = y+VEHICLE_SIZE_Y - 8,
            wheelPosUpY = y;
    gdispFillArea(x+wheelSizeX+wheelHorizX, y, VEHICLE_SIZE_X - 16, VEHICLE_SIZE_Y, color);
    // Left wheel
    if(y <= displaySizeY-VEHICLE_SIZE_Y && y>=-VEHICLE_SIZE_Y) {
        gdispFillArea(x, wheelPositionY, wheelSizeX, wheelSizeY, Black); // Vertical
        gdispFillArea(x + wheelSizeX, wheelPositionY + wheelSizeY / 2, wheelHorizX, wheelHorizY, Black); // horizontal
        //Right wheel
        gdispFillArea(x + VEHICLE_SIZE_X - wheelSizeX, wheelPositionY, wheelSizeX, wheelSizeY, Black); // Vertical
        gdispFillArea(x + VEHICLE_SIZE_X - wheelSizeX - wheelHorizX, wheelPositionY + wheelSizeY / 2, wheelHorizX,
                      wheelHorizY, Black); // horizontal
    }
    if(wheelPosUpY <= displaySizeY-wheelSizeY && wheelPosUpY>=-VEHICLE_SIZE_Y) {
        // Left wheel
        gdispFillArea(x, wheelPosUpY, wheelSizeX, wheelSizeY, Black); // Vertical
        gdispFillArea(x + wheelSizeX, wheelPosUpY + wheelSizeY / 2, wheelHorizX, wheelHorizY, Black); // horizontal
        //Right wheel
        gdispFillArea(x + VEHICLE_SIZE_X - wheelSizeX, wheelPosUpY, wheelSizeX, wheelSizeY, Black); // Vertical
        gdispFillArea(x + VEHICLE_SIZE_X - wheelSizeX - wheelHorizX, wheelPosUpY + wheelSizeY / 2, wheelHorizX,
                      wheelHorizY, Black); // horizontal
    }
}

void drawFinishGameAndHighScores(Road* road, Vehicle* ego){
    Box box = {road->side+SIZE_BORDER, displaySizeY / 2 - (road->point[ROAD_POINTS-1].distanceToNextRoadPoint - ego->distanceFromCurrentRoadPoint- 2 * UNIT_ROAD_DISTANCE),20,10};
    for(int i = 0; i<7; i++){
        box.color = White * !(i%2) + Black * ((i%2));
        drawBoxGame(&box, 0);
        box.x += 20;
    }
    gdispDrawLine(road->side, displaySizeY / 2 - (road->point[ROAD_POINTS-1].distanceToNextRoadPoint - ego->distanceFromCurrentRoadPoint- 2 * UNIT_ROAD_DISTANCE),road->side + ROAD_SIZE ,displaySizeY / 2 - (road->point[ROAD_POINTS-1].distanceToNextRoadPoint - ego->distanceFromCurrentRoadPoint- 2 * UNIT_ROAD_DISTANCE), Black );
    sprintf(str, "High Scores:");
    gdispDrawString((int)(road->side+ ROAD_SIZE/2) , 11, str, font1, White);
    for(int i=0; i<3 && road->highScores[i] != INITIAL_HIGH_SCORE; i++)
    {
        sprintf(str, "%d) %d.%d%d",i+1,road->highScores[i]/100, (road->highScores[i]/10) % 10, road->highScores[i] % 10);
        gdispDrawString((int)(road->side+ ROAD_SIZE/2) , 22+i*11, str, font1, White);
    }
}

void drawGrassAndRoad(Road* road, Border* border, Vehicle* ego){
    point grasPoints[5];
    grasPoints[0].x = 0;
    grasPoints[0].y = 0;
    grasPoints[1].x = road->side + border->sizeHigherBorder * tan(border->yaw_rad[HIGHER_BORDER]);
    grasPoints[1].y = 0;
    grasPoints[2].x = road->side;
    grasPoints[2].y = border->sizeHigherBorder;
    grasPoints[3].x = road->side - round((displaySizeY - border->sizeHigherBorder) * tan(border->yaw_rad[LOWER_BORDER]));;
    grasPoints[3].y = displaySizeY;
    grasPoints[4].x = 0;
    grasPoints[4].y = displaySizeY;
    gdispFillConvexPoly(0,0,grasPoints,5,HTML2COLOR(0x458B00));

    point roadPoints[6];
    roadPoints[0] = grasPoints[1];
    roadPoints[0].x += SIZE_BORDER;
    roadPoints[1] = grasPoints[2];
    roadPoints[1].x += SIZE_BORDER;
    roadPoints[2] = grasPoints[3];
    roadPoints[2].x += SIZE_BORDER;

    grasPoints[1].x += ROAD_SIZE + SIZE_BORDER;
    grasPoints[2].x += ROAD_SIZE + SIZE_BORDER;
    grasPoints[3].x += ROAD_SIZE + SIZE_BORDER;
    grasPoints[4].x = displaySizeX;
    grasPoints[0].x = displaySizeX;
    gdispFillConvexPoly(0,0,grasPoints,5,HTML2COLOR(0x458B00));

    roadPoints[3] = grasPoints[3];
    roadPoints[3].x -= SIZE_BORDER;
    roadPoints[4] = grasPoints[2];
    roadPoints[4].x -= SIZE_BORDER;
    roadPoints[5] = grasPoints[1];
    roadPoints[5].x -= SIZE_BORDER;
    gdispFillConvexPoly(0,0,roadPoints,6,HTML2COLOR(0xA2B5CD));

    Box box[4];
    box[0].y = ego->distanceFromCurrentRoadPoint % UNIT_ROAD_DISTANCE;
    box[1].y = (displaySizeY/2 + ego->distanceFromCurrentRoadPoint) % UNIT_ROAD_DISTANCE;
    box[2].y = displaySizeY/2 + (ego->distanceFromCurrentRoadPoint) % UNIT_ROAD_DISTANCE;
    box[3].y = displaySizeY + (ego->distanceFromCurrentRoadPoint) % UNIT_ROAD_DISTANCE;
    for(int i=0; i<4; i++) {
        box[i].color = Black;
        box[i].x = calcX(border, box[i].y, road->side) ;
        box[i].sizeX = SIZE_BORDER ;
        box[i].sizeY = displaySizeY / 4;
        if (box[i].y <= border->sizeHigherBorder)
            drawBoxGame(&box[i], border->yaw_rad[HIGHER_BORDER]);
        else
            drawBoxGame(&box[i], border->yaw_rad[LOWER_BORDER]);
        box[i].x += ROAD_SIZE;
        if (box[i].y <= border->sizeHigherBorder)
            drawBoxGame(&box[i], border->yaw_rad[HIGHER_BORDER]);
        else
            drawBoxGame(&box[i], border->yaw_rad[LOWER_BORDER]);
        box[i].x -= ROAD_SIZE/2;
        box[i].color = White;
        box[i].sizeX = 5;
        if (box[i].y <= border->sizeHigherBorder)
            drawBoxGame(&box[i], border->yaw_rad[HIGHER_BORDER]);
        else
            drawBoxGame(&box[i], border->yaw_rad[LOWER_BORDER]);
    }
}

void drawInfo(Vehicle* ego,Road* road,int fps){
    font1 = gdispOpenFont("UI2 Narrow");
    sprintf(str, "%d.%d%d", time_s/100,(time_s/10) % 10, time_s % 10);
    gdispDrawString(1, 15, str, font1, White);
    sprintf(str, "%d.%d", (int) ego->v_y, (int) (ego->v_y*10 - 10 * (int)ego->v_y));
    gdispDrawString(1, displaySizeY - MAP_SIZE_Y +30, str, font1, White);
    sprintf(str, "Rank: %d", ego->ranking+1);
    gdispDrawString(1, displaySizeY - MAP_SIZE_Y +50, str, font1, White);

    font1 = gdispOpenFont("DejaVuSans24");
    // print Info about buttons
    sprintf(str,"Buttons: ");
    gdispDrawString(0, 35, str, font1, Red);
    sprintf(str,"B: Pause");
    gdispDrawString(0, 56, str, font1,Red);
    sprintf(str,"C: Restart");
    gdispDrawString(0,77, str, font1,Red);
    sprintf(str,"E: Exit");
    gdispDrawString(0,98, str, font1,Red);
    // draw  fps
    sprintf(str, "fps %d", fps);
    gdispDrawString(0, 0, str, font1, White);
}

void drawMap(Road* road, Vehicle* ego, Vehicle* bot[NUM_BOTS], Map* map)
{
    //draw map
    for(int i=0; i<MAP_POINTS; i++)
        gdispDrawThickLine(map->point[i].x+displaySizeX - MAP_SIZE_X + 60,
                           map->point[i].y+displaySizeY - MAP_SIZE_Y + 15,
                           map->point[(i+1)%MAP_POINTS].x+displaySizeX - MAP_SIZE_X + 60,
                           map->point[(i+1)%MAP_POINTS].y+displaySizeY - MAP_SIZE_Y + 15,
                           HTML2COLOR(0xff9f00),3,0);

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

void drawBot(Vehicle* bot, Vehicle*  ego, Border* border, Road* road){
    if(bot->rel.y <= displaySizeY && bot->rel.y>=0 ){
        if(bot->rel.y <= displaySizeY) {
            if (bot->color == GREEN) {
                drawVehicle(bot->rel.x + calcX(border, bot->rel.y, road->side), bot->rel.y, Green);
            } else if (bot->color == YELLOW) {
                drawVehicle(bot->rel.x + calcX(border, bot->rel.y, road->side), bot->rel.y, Yellow);
            } else if (bot->color == BLUE) {
                drawVehicle(bot->rel.x + calcX(border, bot->rel.y, road->side), bot->rel.y, Blue);
            }
        }
    }
}
void drawBoxGame(Box* box, double yaw_rad) {
    uint16_t xend=0, yend=0 ;
    xend =(int) (box->x+ box->sizeX/2 + box->sizeY* sin(yaw_rad))  ;
    yend =(int) (box->y- box->sizeY * cos(yaw_rad)) ;
    gdispDrawThickLine(box->x+ box->sizeX/2 , box->y,xend,yend,box->color,box->sizeX, 0);
}

/************************************************************************************************************************/
/*
 * Artificial intelligence
 */
/************************************************************************************************************************/

/*
 * @brief acceleration is combination of self-computed acceleration based on initial parameters
 * and influence of next vehicle's acceleration
 */
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
                double v_soll = road->point[bot->currentRoadPoint + 1].rel.yaw * 8.0 / 45;
                if (bot->v_y > v_soll)
                    bot->a_y = -bot->absEffect;
                else
                    bot->a_y = 0;
            }
            bot->a_y = 0.7 * bot->a_y + 0.3 * adjustAccToRanking(rankedVehicles,i);
        }
    }
}

/*
 * @brief adjust acceleration to ranking
 */
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

/*
 * @brief Calculate speed in x direction
 */
void calculateLateralSpeed(Vehicle* rankedVehicles[NUM_VEHICLES], Road* road, Border* border){
    double x_soll = 0;
    for(int i=0; i<NUM_VEHICLES; i++) {
        if(rankedVehicles[i]->color != RED) {
            Vehicle *bot = rankedVehicles[i];
            if(bot->ranking == 0) {
                x_soll = ROAD_SIZE / 2;
            }
            else{
                if (rankedVehicles[i]->rel.x <= rankedVehicles[i - 1]->rel.x + VEHICLE_SIZE_X / 2) // überholen in the left direction of next vehicle
                {
                    x_soll = (rankedVehicles[i - 1]->rel.x + (SIZE_BORDER)) / 2 - VEHICLE_SIZE_X/2;
                } else {
                    x_soll = (rankedVehicles[i - 1]->rel.x + VEHICLE_SIZE_X + (ROAD_SIZE)) / 2 - VEHICLE_SIZE_X/2 -SIZE_BORDER;
                }
            }

            rankedVehicles[i]->v_x = round(x_soll - rankedVehicles[i]->rel.x) / 50.0;
        }
    }
}

/*
 * @brief update x position based on computed v_x
 */
void updateXPosition(Vehicle* bot, Border* border, Road* road){
    if(bot->rel.x <= SIZE_BORDER || bot->rel.x + VEHICLE_SIZE_X > ROAD_SIZE)
    {
        bot->state = COLLISION_WITH_BORDER;
        if(bot->v_y > 2)
            bot->v_y -= REDUCED_VEL_COL_BORDER;
    }
    else {
        bot->state = NO_COLLISION;
    }
    bot->rel.x += bot->v_x * (bot->rel.x + bot->v_x >= SIZE_BORDER && bot->rel.x + bot->v_x + VEHICLE_SIZE_X < ROAD_SIZE);
}

/************************************************************************************************************************/
/*
 * Collision
 */
/************************************************************************************************************************/

void checkCarCollision(Vehicle* rankedVehicles[NUM_VEHICLES])
{
    uint8_t offsetY = 8, offsetX = 4;
    for(int i=1; i<NUM_VEHICLES; i++) {
        if(rankedVehicles[i]->state != END_GAME)
            for (int j = 0; j < i; j++) {
                if(fabs(rankedVehicles[i]->currentRoadPoint % LAP_POINTS - rankedVehicles[j]->currentRoadPoint % LAP_POINTS) <= 1) {
                    if (fabs(rankedVehicles[i]->rel.x - rankedVehicles[j]->rel.x) < VEHICLE_SIZE_X
                        && fabs(rankedVehicles[i]->rel.y - rankedVehicles[j]->rel.y) < VEHICLE_SIZE_Y) {
                        uint8_t diffX = rankedVehicles[j]->rel.x - rankedVehicles[i]->rel.x;
                        if (rankedVehicles[j]->color != RED) {
                            // MOve in y direction
                            rankedVehicles[j]->distanceFromCurrentRoadPoint +=VEHICLE_SIZE_Y - fabs(rankedVehicles[i]->rel.y - rankedVehicles[j]->rel.y) + offsetY;
                            // Move in x direction based on sign of diffX
                            if(diffX>0)
                                rankedVehicles[j]->rel.x += offsetX;
                            else
                                rankedVehicles[j]->rel.x -= offsetX;

                             //(rankedVehicles[i]->rel.x - (rankedVehicles[j]->rel.x + VEHICLE_SIZE_X)) + offsetX;

                            if(rankedVehicles[j]->rel.x < SIZE_BORDER)
                                rankedVehicles[j]->rel.x = SIZE_BORDER;
                            else if(rankedVehicles[j]->rel.x > ROAD_SIZE-VEHICLE_SIZE_X)
                                rankedVehicles[j]->rel.x = ROAD_SIZE-VEHICLE_SIZE_X;
                        }
                        else {
                            // Move in x direction
                            if(diffX<0)
                                rankedVehicles[i]->rel.x += offsetX;
                            else
                                rankedVehicles[i]->rel.x -= offsetX;

                            if(rankedVehicles[i]->rel.x < SIZE_BORDER)
                                rankedVehicles[i]->rel.x = SIZE_BORDER;
                            else if(rankedVehicles[i]->rel.x > ROAD_SIZE-VEHICLE_SIZE_X)
                                rankedVehicles[i]->rel.x = ROAD_SIZE-VEHICLE_SIZE_X;
                            // Move in y direction
                            if (rankedVehicles[i]->distanceFromCurrentRoadPoint - (VEHICLE_SIZE_Y - fabs(rankedVehicles[i]->rel.y - rankedVehicles[j]->rel.y) + offsetY) >= 0)
                                rankedVehicles[i]->distanceFromCurrentRoadPoint -= VEHICLE_SIZE_Y - fabs(rankedVehicles[i]->rel.y - rankedVehicles[j]->rel.y) + offsetY;
                            else
                                rankedVehicles[i]->distanceFromCurrentRoadPoint = 0;
                        }
                    }
                }
            }
    }
}

uint8_t checkIfCollisionWithBorder(Border* border, double side)
{
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

