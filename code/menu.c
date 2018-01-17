#include <includes.h>
#include "menu.h"

void startMenu(Game* game){
    Box single_mode = {displaySizeX/8,displaySizeY/2,displaySizeX/3,displaySizeY/8,"Single Player Mode",Cyan},
        multi_mode  = {displaySizeX*5/8,displaySizeY/2,displaySizeX/3,displaySizeY/8,"Multiplayer Mode",Cyan};
    int joystick_x = 0, threshold = 50;
    char str[100];
    font_t font1;
    font1 = gdispOpenFont("DejaVuSans24*");
    while(TRUE){
        if(game->gameState == START_MENU) {
            //clear display
            gdispClear(White);

            sprintf(str,"Ego mode: %d",game->mode);
            gdispDrawString(0, 11, str, font1, Black);

          //  sprintf(str,"Other mode: %d",game->received_input);//gdispDrawString(80, 11, str, font1, Black);

            sprintf(str,"Chosen: %d",game->menuState);
            gdispDrawString(210, 11, str, font1, Black);

            joystick_x = (uint8_t) (ADC_GetConversionValue(ESPL_ADC_Joystick_2) >> 4) - 255 / 2;
            if(joystick_x > threshold)
            {
                single_mode.color = Cyan;
                multi_mode.color = Lime;
                game->mode = MULTIPLAYER_MODE;
            }
            else if(joystick_x < -threshold)
            {
                single_mode.color = Lime;
                multi_mode.color = Cyan;
                game->mode = SINGLE_MODE;
            }

            drawBox(&single_mode);
            drawBox(&multi_mode);

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