#include <includes.h>
#include "menu.h"

#include "multiplayer.h"


/*
 * @brief This task is used to draw and update all menu elements on display.
 *
 */
void startMenu(Game* game){
    Box single_mode = {displaySizeX/8,displaySizeY/3+5,BOX_SIZE_X,BOX_SIZE_Y,"Single Player Mode",COLOUR_BUTTON_UNCHOSEN,TEXT_COLOUR_BUTTON_UNCHOSEN},
        multi_mode  = {displaySizeX*5/8,displaySizeY/3+5,BOX_SIZE_X,BOX_SIZE_Y,"Multiplayer Mode",COLOUR_BUTTON_UNCHOSEN,TEXT_COLOUR_BUTTON_UNCHOSEN};
    Box speed_ctrl = {displaySizeX/8,displaySizeY/2+5,BOX_SIZE_X,BOX_SIZE_Y,"Control speed",COLOUR_BUTTON_UNCHOSEN,TEXT_COLOUR_BUTTON_UNCHOSEN},
        steering_ctrl  = {displaySizeX*5/8,displaySizeY/2+5,BOX_SIZE_X,BOX_SIZE_Y,"Control steering",COLOUR_BUTTON_UNCHOSEN,TEXT_COLOUR_BUTTON_UNCHOSEN};
    for(int i=0; i<NUM_MAPS;i++)
        game->map[i]->color = White;
    volatile ConnectionState lastConnectionState = NOT_CONNECTED;
    volatile Mode lastModeOtherPlayer = MULTIPLAYER_MODE;
    uint8_t valuesToSend[12];
    for(int i=0; i<12; i++)
            valuesToSend[i]=0;
    while(TRUE){
        if(game->gameState == START_MENU) {
            //clear display
            gdispClear(White);

            drawTitel();

            // When Connection is turned on
            if((lastConnectionState == NOT_CONNECTED && game->connectionState == CONNECTED) || (lastModeOtherPlayer == SINGLE_MODE && game->modeOtherPlayer == MULTIPLAYER_MODE) )
            {
                multi_mode.color = COLOUR_BUTTON_UNCHOSEN;
                multi_mode.colorText = TEXT_COLOUR_BUTTON_UNCHOSEN;
            }
            // When Connection is turned off
            else if(game->connectionState == NOT_CONNECTED || game->modeOtherPlayer == SINGLE_MODE){
                single_mode.color = COLOUR_BUTTON_CHOSEN;
                single_mode.colorText = TEXT_COLOUR_BUTTON_CHOSEN;
                multi_mode.color = COLOUR_BUTTON_MP_OFF;
                multi_mode.colorText = TEXT_COLOUR_BUTTON_MP_OFF;
                game->mode = SINGLE_MODE;
                if((lastConnectionState == CONNECTED && game->connectionState == NOT_CONNECTED) || ((lastModeOtherPlayer == MULTIPLAYER_MODE && game->modeOtherPlayer == SINGLE_MODE)))
                    game->menuState = NOT_CHOSEN;
            }
            // Store last states
            lastConnectionState = game->connectionState;
            lastModeOtherPlayer = game->modeOtherPlayer;

            adjustColors(game,&speed_ctrl, &steering_ctrl);
            if(game->menuState == NOT_CHOSEN)
                joystickToModeChoice(&single_mode, &multi_mode, game);
            if(game->menuState == MODE_CHOSEN)
                joystickToCourseChoice(game);
            if(game->menuState == COURSE_CHOSEN)
                joystickToCtrlChoice(&speed_ctrl, &steering_ctrl, game);
            for(int i=0; i<12; i++)
                valuesToSend[i]=0;
            valuesToSend[0] = (uint8_t) (game->menuState);
            valuesToSend[1] = (uint8_t) (game->gameState);
            valuesToSend[2] = (uint8_t) (game->mode);
            valuesToSend[3] = (uint8_t) (game->chosenMap);
            valuesToSend[4] = (uint8_t) (game->controlState);
            sendviaUart(valuesToSend,12);

            drawBox(&single_mode);
            drawBox(&multi_mode);

            if(game->menuState >= MODE_CHOSEN)
            {
                for(int i=0; i<NUM_MAPS; i++) {
                    drawMapInMenu(game->map[i], displaySizeX * i / 3, displaySizeY * 2 / 3);
                }
            }

            if(game->menuState >= COURSE_CHOSEN) {
                drawBox(&speed_ctrl);
                drawBox(&steering_ctrl);
            }

            // Wait for display to stop writing
            xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
            // swap buffers
            ESPL_DrawLayer();
        }
        else{
            vTaskDelay(1000);
        }
    }
}

void drawTitel(){
    for(int i=0; i<16; i++) {
        gdispDrawThickLine(20 * i, 5, 20 * i + 10, 5, Black, 10, 0);
        gdispDrawThickLine(20 * i+10, 15, 20 * i + 20, 15, Black, 10, 0);
        gdispDrawThickLine(20 * i, 60, 20 * i + 10, 60, Black, 10, 0);
        gdispDrawThickLine(20 * i+10, 70, 20 * i + 20, 70, Black, 10, 0);
    }
    font1 = gdispOpenFont("UI1 Double");
    sprintf(str,"F1 Game");
    gdispDrawStringBox(displaySizeX/2-60,25,120,30,str,font1,Black,justifyCenter);
}

void adjustColors(Game* game, Box* speed_ctrl, Box* steering_ctrl) {

    if(game->menuState >= MODE_CHOSEN) {
        game->map[game->chosenMap]->color = COLOUR_BUTTON_CHOSEN;
        for (int i = 0; i < NUM_MAPS; i++) {
            if (i != game->chosenMap)
                game->map[i]->color = White;
        }
    }

    if(game->menuState >= COURSE_CHOSEN) {
        if (game->controlState == STEERING_CTRL) {
            speed_ctrl->color = COLOUR_BUTTON_UNCHOSEN;
            steering_ctrl->color = COLOUR_BUTTON_CHOSEN;
            steering_ctrl->colorText = TEXT_COLOUR_BUTTON_CHOSEN;
        }
        else if (game->controlState == SPEED_CTRL){
            steering_ctrl->color = COLOUR_BUTTON_UNCHOSEN;
            speed_ctrl->color = COLOUR_BUTTON_CHOSEN;
            speed_ctrl->colorText = TEXT_COLOUR_BUTTON_CHOSEN;
        }
    }

}

void joystickToCourseChoice(Game* game)
{
    int joystick_x = 0,
            threshold = 30;
    joystick_x = (uint8_t) (ADC_GetConversionValue(ESPL_ADC_Joystick_2) >> 4) - 255 / 2;
    if(joystick_x > threshold){
        if(game->chosenMap<NUM_MAPS-1){
            game->chosenMap++;
            vTaskDelay(750);
        }
    }
    else if(joystick_x < -threshold){
        if(game->chosenMap>0){
            game->chosenMap--;
            vTaskDelay(750);
        }
    }
}

void drawMapInMenu(Map* map, uint16_t x, uint16_t y){
    //draw map
    static int posInMap = 0;
    gdispFillConvexPoly(x+60,y+15, map, MAP_POINTS,map->color);
    gdispDrawPoly(x+60,y+15, map, MAP_POINTS, Black);
    if(map->color == COLOUR_BUTTON_CHOSEN){
        point middlePoint = {(map->point[posInMap/4].x + map->point[(posInMap/4+1)%MAP_POINTS].x) / 2,(map->point[posInMap/4].y + map->point[(posInMap/4+1)%MAP_POINTS].y) / 2};
        point pointToDraw;
        switch (posInMap%4){
            case 0:
                pointToDraw = map->point[posInMap/4];
                break;
            case 1:
                pointToDraw.x = (map->point[posInMap/4].x + middlePoint.x)/2;
                pointToDraw.y = (map->point[posInMap/4].y + middlePoint.y)/2;
                break;
            case 2:
                pointToDraw = middlePoint;
                break;
            case 3:
                pointToDraw.x = (map->point[posInMap/4+1].x + middlePoint.x)/2;
                pointToDraw.y = (map->point[posInMap/4+1].y + middlePoint.y)/2;
                break;
        }
        gdispFillCircle(pointToDraw.x+x+60, pointToDraw.y+y+15, 2, Red);
        posInMap++;
        vTaskDelay(10);
        if(posInMap>=4*MAP_POINTS-1)
            posInMap=0;
    }
}


void joystickToModeChoice(Box* single_mode, Box* multi_mode, Game* game){
    int joystick_x = 0,
            threshold = 50;
    joystick_x = (uint8_t) (ADC_GetConversionValue(ESPL_ADC_Joystick_2) >> 4) - 255 / 2;
    if(game->connectionState == CONNECTED && game->modeOtherPlayer == MULTIPLAYER_MODE) {
        if (joystick_x > threshold) {
            single_mode->color = COLOUR_BUTTON_UNCHOSEN;
            single_mode->colorText = TEXT_COLOUR_BUTTON_UNCHOSEN;
            multi_mode->color = COLOUR_BUTTON_CHOSEN;
            multi_mode->colorText = TEXT_COLOUR_BUTTON_CHOSEN;
            game->mode = MULTIPLAYER_MODE;
        } else if (joystick_x < -threshold) {
            single_mode->color = COLOUR_BUTTON_CHOSEN;
            single_mode->colorText = TEXT_COLOUR_BUTTON_CHOSEN;
            multi_mode->color = COLOUR_BUTTON_UNCHOSEN;
            multi_mode->colorText = TEXT_COLOUR_BUTTON_UNCHOSEN;
            game->mode = SINGLE_MODE;
        }
    }
}

void joystickToCtrlChoice(Box* speed_ctrl, Box* steering_ctrl, Game* game){
    int joystick_x = (uint8_t) (ADC_GetConversionValue(ESPL_ADC_Joystick_2) >> 4) - 255 / 2,
            threshold = 50;
    if(joystick_x > threshold)
    {
        game->controlState = STEERING_CTRL;
    }
    else if(joystick_x < -threshold)
    {
        game->controlState = SPEED_CTRL;
    }

}


void drawBox(Box* box){
    font1 = gdispOpenFont("UI1");

    sprintf(str, box->text);
    gdispFillArea(box->x,box->y,box->sizeX,box->sizeY,box->color);
    gdispDrawStringBox(box->x, box->y, box->sizeX, box->sizeY, str, font1, box->colorText, justifyCenter);
}