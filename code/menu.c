#include <includes.h>
#include "menu.h"

#include "multiplayer.h"

void startMenu(Game* game){
    Box single_mode = {displaySizeX/8,displaySizeY/3,BOX_SIZE_X,BOX_SIZE_Y,"Single Player Mode",Cyan},
        multi_mode  = {displaySizeX*5/8,displaySizeY/3,BOX_SIZE_X,BOX_SIZE_Y,"Multiplayer Mode",Cyan};
    Box speed_ctrl = {displaySizeX/8,displaySizeY/2,BOX_SIZE_X,BOX_SIZE_Y,"Control speed",Cyan},
        steering_ctrl  = {displaySizeX*5/8,displaySizeY/2,BOX_SIZE_X,BOX_SIZE_Y,"Control steering",Cyan};
    char str[100];
    font_t font1;
    font1 = gdispOpenFont("DejaVuSans24*");
    int a = 0;
    while(TRUE){
        if(game->gameState == START_MENU) {
            sendviaUart( (uint8_t) &(game->menuState), 1);

            //clear display
            gdispClear(White);

            sprintf(str,"Game mode: %d Game State: %d Menu State %d Rec %d",game->mode, game->gameState, game->menuState,  game->received_buffer);
            gdispDrawString(0, 11, str, font1, Black);

          //  sprintf(str,"Other mode: %d",game->received_input);//gdispDrawString(80, 11, str, font1, Black);

            //sprintf(str,"Chosen: %d",game->menuState);
            //gdispDrawString(210, 11, str, font1, Black);

            if(game->menuState == NOT_CHOSEN)
                joystickToModeChoice(&single_mode, &multi_mode, game);
            if(game->menuState == MODE_CHOSEN)
                joystickToCtrlChoice(&speed_ctrl, &steering_ctrl, game);

            drawBox(&single_mode);
            drawBox(&multi_mode);

            if(game->menuState >= MODE_CHOSEN) {
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

void joystickToModeChoice(Box* single_mode, Box* multi_mode, Game* game){
    int joystick_x = 0,
            threshold = 50;
    joystick_x = (uint8_t) (ADC_GetConversionValue(ESPL_ADC_Joystick_2) >> 4) - 255 / 2;
    if(joystick_x > threshold)
    {
        single_mode->color = Cyan;
        multi_mode->color = Lime;
        game->mode = MULTIPLAYER_MODE;
    }
    else if(joystick_x < -threshold)
    {
        single_mode->color = Lime;
        multi_mode->color = Cyan;
        game->mode = SINGLE_MODE;
    }
}

void joystickToCtrlChoice(Box* speed_ctrl, Box* steering_ctrl, Game* game){
    int joystick_x = (uint8_t) (ADC_GetConversionValue(ESPL_ADC_Joystick_2) >> 4) - 255 / 2,
            threshold = 50;
    if(joystick_x > threshold)
    {
        speed_ctrl->color = Cyan;
        steering_ctrl->color = Lime;
        game->controlState = STEERING_CTRL;
    }
    else if(joystick_x < -threshold)
    {
        steering_ctrl->color = Cyan;
        speed_ctrl->color = Lime;
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