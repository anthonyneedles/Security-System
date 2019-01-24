/*******************************************************************************
* WDog.h - Project header file for WDog.c
*
* Created on: Dec 8, 2017
* Author: Anthony Needles
*******************************************************************************/
#ifndef SOURCES_WDOG_H_
#define SOURCES_WDOG_H_
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
void WDogInit(void);
/********************************************************************
* WDogTask - Refreshes watchdog every 10ms
*
* Description: Sends refresh codes to watchdog 1ms before countdown reaches 0
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void WDogTask(void);

#endif /* SOURCES_WDOG_H_ */
