/**
 * This is the main file of the ESPLaboratory project.
 *
 * @author: Ahmed Mzid
 * 			Helmi Abidi
 *
 */

#include "includes.h"

QueueHandle_t ESPL_RxQueue; // Already defined in ESPL_Functions.h
SemaphoreHandle_t ESPL_DisplayReady;

int main()
{

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
