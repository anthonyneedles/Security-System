/****************************************************************************************
* SysTickDelay.h - Project Header file for the System Tick - based
*             delay routine. 
*
* Todd Morton 11/04/2015
* 11/05/2017 Todd Morton Modify for new header file structure.
*****************************************************************************************
* Public Function Prototypes 
****************************************************************************************/
#ifndef SYS_TICK_INC
#define SYS_TICK_INC
/****************************************************************************************
 * SysTickDelay()
 * Blocking delay routine. The parameter is the number of ms to delay.
 ***************************************************************************************/
void SysTickDelay(const INT32U ms);
                                
/****************************************************************************************
 * SysTickDlyInit()
 * SysTickDelay Initialization Routine. This function must be called before
 * SysTickDelay() can be called.
 ***************************************************************************************/
INT32U SysTickDlyInit(void);

/****************************************************************************************
 * SysTickWaitEvent()
 * SysTickWaitEvent is a periodic blocking routine. It's more like a task - it should
 * only be called one time in timed event or task loop.
 * THe is: ONLY ONE INSTANCE is allowed.
 ***************************************************************************************/
void SysTickWaitEvent(const INT32U period);

/*****************************************************************************************
* Handler must be public for linker to see it.
*****************************************************************************************/
void SysTick_Handler(void);  /* SysTick interrupt service routine*/

#endif
