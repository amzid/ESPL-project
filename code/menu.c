#include <includes.h>
#include "menu.h"

#include "multiplayer.h"

void startMenu(Game* game){
    Box single_mode = {displaySizeX/8,displaySizeY/3,BOX_SIZE_X,BOX_SIZE_Y,"Single Player Mode",Cyan},
        multi_mode  = {displaySizeX*5/8,displaySizeY/3,BOX_SIZE_X,BOX_SIZE_Y,"Multiplayer Mode",Cyan};
    Box speed_ctrl = {displaySizeX/8,displaySizeY/2,BOX_SIZE_X,BOX_SIZE_Y,"Control speed",Cyan},
        steering_ctrl  = {displaySizeX*5/8,displaySizeY/2,BOX_SIZE_X,BOX_SIZE_Y,"Control steering",Cyan};
    for(int i=0; i<NUM_MAPS;i++)
        game->map[i]->color = White;
    char str[100];
    font_t font1;
    font1 = gdispOpenFont("DejaVuSans24*");
    volatile ConnectionState lastConnectionState = NOT_CONNECTED;
    uint8_t valuesToSend[4];
    while(TRUE){
        if(game->gameState == START_MENU) {
            //clear display
            gdispClear(White);

            sprintf(str, "Uart: %d", (int) game->taktUART);
            gdispDrawString(0, 11, str, font1, Black);

            sprintf(str, "Game: %d ", (int) game->taktGame);
            gdispDrawString(0, 22, str, font1, Black);


            if(lastConnectionState == NOT_CONNECTED && game->connectionState == CONNECTED)
            {
                multi_mode.color = Cyan;
            }
            else if(lastConnectionState == CONNECTED && game->connectionState == NOT_CONNECTED){
                single_mode.color = Lime;
                multi_mode.color = Gray;
                game->mode = SINGLE_MODE;
                game->menuState = NOT_CHOSEN;
            }
            lastConnectionState = game->connectionState;

          //  sprintf(str,"Other mode: %d",game->received_input);//gdispDrawString(80, 11, str, font1, Black);

            //sprintf(str,"Chosen: %d",game->menuState);
            //gdispDrawString(210, 11, str, font1, Black);

            sprintf(str,"Game mode: %d Map: %d Menu State %d Rec %d",game->mode, game->chosenMap, game->menuState,  game->received_buffer);
            gdispDrawString(0, 11, str, font1, Black);


            adjustColors(game,&speed_ctrl, &steering_ctrl);
            if(game->menuState == NOT_CHOSEN)
                joystickToModeChoice(&single_mode, &multi_mode, game);
            if(game->menuState == MODE_CHOSEN)
                joystickToCourseChoice(game);
            if(game->menuState == COURSE_CHOSEN)
                joystickToCtrlChoice(&speed_ctrl, &steering_ctrl, game);

            valuesToSend[0] = (uint8_t) (game->menuState);
            valuesToSend[1] = (uint8_t) (game->gameState);
            valuesToSend[2] = (uint8_t) (game->mode);
            valuesToSend[3] = (uint8_t) (game->chosenMap);
            valuesToSend[4] = (uint8_t) (game->controlState);
            sendviaUart(valuesToSend,5);

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
            vTaskDelay(20);
        }
    }
}

void adjustColors(Game* game, Box* speed_ctrl, Box* steering_ctrl) {

    if(game->menuState >= MODE_CHOSEN) {
        game->map[game->chosenMap]->color = Lime;
        for (int i = 0; i < NUM_MAPS; i++) {
            if (i != game->chosenMap)
                game->map[i]->color = White;
        }
    }

    if(game->menuState >= COURSE_CHOSEN) {
        if (game->controlState == STEERING_CTRL) {
            speed_ctrl->color = Cyan;
            steering_ctrl->color = Lime;
        }
        else if (game->controlState == SPEED_CTRL){
            steering_ctrl->color = Cyan;
            speed_ctrl->color = Lime;
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
    gdispFillConvexPoly(x+60,y+15, map, MAP_POINTS,map->color);
    gdispDrawPoly(x+60,y+15, map, MAP_POINTS, Black);
}


void joystickToModeChoice(Box* single_mode, Box* multi_mode, Game* game){
    int joystick_x = 0,
            threshold = 50;
    joystick_x = (uint8_t) (ADC_GetConversionValue(ESPL_ADC_Joystick_2) >> 4) - 255 / 2;
    if(game->connectionState == CONNECTED) {
        if (joystick_x > threshold) {
            single_mode->color = Cyan;
            multi_mode->color = Lime;
            game->mode = MULTIPLAYER_MODE;
        } else if (joystick_x < -threshold) {
            single_mode->color = Lime;
            multi_mode->color = Cyan;
            game->mode = SINGLE_MODE;
        }
    }
    else {
        single_mode->color = Lime;
        multi_mode->color = Gray;
        game->mode = SINGLE_MODE;
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
    char str[100];
    font_t font1;
    font1 = gdispOpenFont("DejaVuSans24*");

    sprintf(str, box->text);
    gdispFillArea(box->x,box->y,box->sizeX,box->sizeY,box->color);
    gdispDrawStringBox(box->x, box->y, box->sizeX, box->sizeY, str, font1, Black, justifyCenter);

    // gdispFillArea(box->x,box->y,box->sizeX,box->sizeY,Cyan);

    // gdispDrawString(box->x+box->sizeX/2-strlen(box->text)*2-8,box->y+box->sizeY/2-1,str,font1,Black);
}