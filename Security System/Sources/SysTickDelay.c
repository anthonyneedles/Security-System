/*****************************************************************************************
* SysTickDelay.c - A delay module based on the system timer
* v1.1
* 10/30/14 Todd Morton
* v2.1 Modify for k65 at 180MHz
* 11/04/2015 Todd Morton
* 11/05/2017 Todd Morton Modify for new header file structure.
******************************************************************************************
* Project master header file
*****************************************************************************************/
#include "MCUType.h"
#include "SysTickDelay.h"
#include "K65TWR_GPIO.h"

/*****************************************************************************************
* Private Resources
*****************************************************************************************/
static volatile INT32U stmsCount;   /* 1ms counter variable */
static INT8U stInitFlag;
static INT32U stLastEvent;

/*****************************************************************************************
* Module Defines
*****************************************************************************************/
#define CLK_PER_MS 180000U          /* Clock cycles per 1ms        */

/*****************************************************************************************
* SysTickDelay Function
*    - Public
*    - Delays 'ms' milliseconds
*    - Accuracy +0/-1 ms
*****************************************************************************************/
void SysTickDelay(const INT32U ms){
    INT32U start_cnt;
    start_cnt = stmsCount;
    while((stmsCount - start_cnt) < ms){} /* wait for ms to pass*/
}

/*****************************************************************************************
* Period Delay Function
*    - Public - NOT reentrant...in fact only one instance.
*    - To next event every 'ms' milliseconds
*    - Accuracy +0/-1 ms
*****************************************************************************************/
void SysTickWaitEvent(const INT32U period){

    DB0_TURN_ON();
    if(stInitFlag == 1){
        while((stmsCount - stLastEvent) < period){}
    }else{
        stInitFlag = 1;
    }
    stLastEvent = stmsCount;
    DB0_TURN_OFF();
}

/*****************************************************************************************
* SysTickDlyInit() - Initialization routine for SysTickDelay()
*****************************************************************************************/
INT32U SysTickDlyInit(void){
    INT32U sterr;
    stInitFlag = 0;
    stmsCount = 0;
    stLastEvent = 0;
    sterr = SysTick_Config(CLK_PER_MS);
    return sterr;
}

/*****************************************************************************************
* SysTick_Handler() - System Tick Interrupt Handler.
*    - setup for a 1ms periodic interrupt.
*****************************************************************************************/
void SysTick_Handler(void){
    stmsCount++;                    /* Increment 1ms counter    */
}
/****************************************************************************************/
