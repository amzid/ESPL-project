/**
 * Main include file.
 *
 * @author: Jonathan Müller-Boruttau, Nadja Peters nadja.peters@tum.de (RCS, TUM)
 *
 **/
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

/* Board includes */
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f429i_discovery_lcd.h"

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "timers.h"
#include "task.h"
#include "queue.h"

/* uGFX inclues. */
#include "gfx.h"

/* convenience functions and init includes */

#include "ESPL_functions.h"
#include "math.h"
#include "definitions.h"
