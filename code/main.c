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

uint16_t time_s = 0;
SemaphoreHandle_t game2rcv;
TimerHandle_t xTimer;

void vTimerCallback( TimerHandle_t xTimer );

int main()
{
	// Initialize Board functions and graphics
	ESPL_SystemInit();

    // Create road and vehicles
	Vehicle ego;
	Vehicle bot[NUM_BOTS];
    Road road;
    Map map;

    // initialize road and vehicles
    ego.color = RED;
    bot[0].color = GREEN;
    bot[1].color = YELLOW;
    bot[2].color = BLUE;




    initializeVehicle(&ego);
    for(int i=0; i<NUM_BOTS; i++)
	    initializeVehicle(&bot[i]);
	initializeRoad(&road,&ego);
	fillMap(&road,&map);

    Game game = {NOT_CONNECTED, START_MENU, SINGLE_MODE, NOT_CHOSEN, SPEED_CTRL, &road, &ego, &bot[0], &bot[1], &bot[2], &map, 233, 0, 0};

    vSemaphoreCreateBinary(game2rcv);
	// Initializes Tasks with their respective priority
    xTaskCreate(controlGameState, "controlGameState", STACK_SIZE, &game, 5, NULL);
    xTaskCreate(drawTask, "drawTask", STACK_SIZE, &game, 4, NULL);
    xTaskCreate(startMenu, "startMenu", STACK_SIZE, &game, 4, NULL);
    xTaskCreate(uartReceive, "startMenu", STACK_SIZE, &game, 5, NULL);
    xTimer = xTimerCreate("Timer", configTICK_RATE_HZ, pdTRUE, ( void * ) 0, vTimerCallback); // every second a callback
    xTimerStart( xTimer, 0 );

	// Start FreeRTOS Scheduler
	vTaskStartScheduler();
}

void vTimerCallback( TimerHandle_t xTimer )
{
    time_s++;
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
