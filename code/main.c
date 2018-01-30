/**
 * This is the main file of the ESPLaboratory project.
 *
 * @author: Ahmed Mzid
 * 			Helmi Abidi
 *
 */

#include <includes.h>

#include "reset.h"
#include "game.h"
#include "controlGameState.h"
#include "menu.h"
#include "multiplayer.h"


QueueHandle_t ESPL_RxQueue; // Already defined in ESPL_Functions.h
SemaphoreHandle_t ESPL_DisplayReady;

char str[100];
font_t font1;

uint16_t time_s = 0;
TaskHandle_t drawHdl = NULL, receiveHdl  = NULL;
TimerHandle_t xTimer=0;
uint8_t fps = 0, tactDrawTask = 0;

void vTimerCallback( TimerHandle_t xTimer );

int main()
{
	// Initialize Board functions and graphics
	ESPL_SystemInit();

    // Create road and vehicles
	Vehicle ego;
	Vehicle bot[NUM_BOTS];
    Road road[3];
    Map map[3];

    // initialize road and vehicles
    ego.color = RED;
    bot[0].color = GREEN;
    bot[1].color = YELLOW;
    bot[2].color = BLUE;

    initializeVehicle(&ego);
    for(int i=0; i<NUM_BOTS; i++)
	    initializeVehicle(&bot[i]);
    for(int i=0; i<3; i++)
	    initializeRoad(&road[i],&ego,i);

    for(int i=0; i<3; i++)
	    fillMap(&road[i],&map[i]);

    Game game = {NOT_CONNECTED, START_MENU, SINGLE_MODE, NOT_CHOSEN, SPEED_CTRL, INDEX_MAP_2, {&road[0],&road[1],&road[2]}, &ego, &bot[0], &bot[1], &bot[2], {&map[0],&map[1],&map[2]}, START_MENU, 233, 0, 0, 0,0,1,1};

	// Initializes Tasks with their respective priority
    xTaskCreate(controlGameState, "controlGameState", STACK_SIZE, &game, 8, NULL);
    xTaskCreate(drawTask, "drawTask", STACK_SIZE, &game, 7, &drawHdl);
    xTaskCreate(startMenu, "startMenu", STACK_SIZE, &game, 4, NULL);
    xTaskCreate(uartReceive, "startMenu", STACK_SIZE, &game, 6, &receiveHdl);
    xTimer = xTimerCreate("Timer", configTICK_RATE_HZ/100, pdTRUE, ( void * ) 0, vTimerCallback); // every second a callback


	// Start FreeRTOS Scheduler
	vTaskStartScheduler();
}

void vTimerCallback( TimerHandle_t xTimer )
{
    time_s++;
    if (time_s>0 && time_s%100==0){
        fps = tactDrawTask;
        tactDrawTask = 0;
    }
}


/*
 *  Hook definitions needed for FreeRTOS to function.
 */
void vApplicationIdleHook() {
	while (TRUE) {
	};
}

void vApplicationMallocFailedHook() {
	while(TRUE) {
	};
}
