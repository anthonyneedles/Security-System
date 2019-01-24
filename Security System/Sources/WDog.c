/*******************************************************************************
* WDog.c - A module for initializing and maintaining a watchdog timer for
*              blocking routines preventing a timelsice cycle from completing.
*
* Created on: Dec 8, 2017
* Author: Anthony Needles
*******************************************************************************/
#include "MCUType.h"
#include "K65TWR_GPIO.h"
#include "WDog.h"

#define WDOG_TOTAL_VAL 0x000A1220

/********************************************************************
* WDogInit - Unlocks and updates watchdog timer
*
* Description: Unlock codes are sent to watchdog timer, then a count value
*              that will be completed in 11ms (just after the expected 10ms
*              timeslice period). Therefore watchdog will only trigger if
*              program loop hangs.
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void WDogInit(void){
    WDOG_UNLOCK = 0xC520;
    WDOG_UNLOCK = 0xD928;
    WDOG_TOVALH = (WDOG_TOTAL_VAL >> 16);
    WDOG_TOVALL = (WDOG_TOTAL_VAL&0xFFFFu);
    WDOG_STCTRLH |= WDOG_STCTRLH_WDOGEN(1);
}
/********************************************************************
* WDogTask - Refreshes watchdog every 10ms
*
* Description: Sends refresh codes to watchdog 1ms before countdown reaches 0
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void WDogTask(void){
    DB5_TURN_ON();
    WDOG_REFRESH = 0xA602;
    WDOG_REFRESH = 0xB480;
    DB5_TURN_OFF();
}
