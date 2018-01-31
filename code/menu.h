#ifndef MENU_H
#define MENU_H

#include "includes.h"

#define BOX_SIZE_X     displaySizeX/3
#define BOX_SIZE_Y     displaySizeY/8

#define COLOUR_BUTTON_UNCHOSEN                  HTML2COLOR(0x00E5EE)
#define TEXT_COLOUR_BUTTON_UNCHOSEN             Black

#define COLOUR_BUTTON_CHOSEN                  HTML2COLOR(0x00688B)
#define TEXT_COLOUR_BUTTON_CHOSEN             White

#define COLOUR_BUTTON_MP_OFF                  HTML2COLOR(0xB0B0B0)
#define TEXT_COLOUR_BUTTON_MP_OFF             White

void startMenu(Game* game);
void drawTitel();
void adjustColors(Game* game, Box* speed_ctrl, Box* steering_ctrl);
void joystickToCourseChoice(Game* game);
void drawMapInMenu(Map* map, uint16_t x, uint16_t y);
void joystickToModeChoice(Box* single_mode, Box* multi_mode, Game* game);
void joystickToCtrlChoice(Box* speed_ctrl, Box* steering_ctrl, Game* game);
void drawBox(Box* box);

#endif