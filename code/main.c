/**
 * This is the main file of the ESPLaboratory project.
 *
 * @author: Ahmed Mzid
 * 			Helmi Abidi
 *
 */

#include "includes.h"

#include "drawTask.h"


QueueHandle_t ESPL_RxQueue; // Already defined in ESPL_Functions.h
SemaphoreHandle_t ESPL_DisplayReady;


int main()
{
	// Initialize Board functions and graphics
	ESPL_SystemInit();

	Road road;

	// Initializes Tasks with their respective priority
	xTaskCreate(drawTask, "drawTask", STACK_SIZE, &road, 4, NULL);

	// Start FreeRTOS Scheduler
	vTaskStartScheduler();

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
