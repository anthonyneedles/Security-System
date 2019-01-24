/******************************************************************************
*   Cooperative Multitasking Security System
*   This program uses a timeslice scheduler to create an alarm system. This
*   alarm has an ARMED, DISARMED, and ALARM state (ALARM and TEMP ALARM). If
*   tampering is detected the tampering alarm "TP" will show. If the program
*   hangs for longer than 11ms a watchdog "WD" will show. If the temperature is
*   below 0c or above 40c not in DISARMED mode the alarm will show TEMP ALARM.
*   ALARM mode is also reached if either of the two touch sensors are activated.
*   When in ALARM mode, an alarm noise will be played, via DMA to DAC. A real
*   time clock is displayed.
*
* Created on: 11/27/2017
* Author: Anthony Needles
******************************************************************************/

#include "MCUType.h"
#include "K65TWR_GPIO.h"
#include "LCD.h"
#include "Key.h"
#include "SysTickDelay.h"
#include "TSI.h"
#include "TempADC.h"
#include "MMA8451Q.h"
#include "DMA.h"
#include "WDog.h"

#define SLICE_PERIOD 10
#define RTC_OFFSET 43474    //Must be reset anytime battery is removed
#define A_PRESS 0x11
#define B_PRESS 0x12
#define C_PRESS 0x13
#define D_PRESS 0x14

typedef enum{DISARMED, ARMED, ALARM} ALARMSTATE;

void ControlDisplayTask(void);
void AlarmControlTask(void);
void LEDTask(void);
void TempDisplayTask(void);
void AccelDisplayTask(void);
void RTCDisplayTask(void);
void WDogResetCheck(void);

const INT8C DisarmedPrompt[] = "DISARMED";
const INT8C ArmedPrompt[] = "ARMED";
const INT8C AlarmPrompt[] = "ALARM";
const INT8C TempAlarmPrompt[] = "TEMP ALARM";
const INT8C WDResetPrompt[] = "WD";
const INT8C TamperingPrompt[] = "TP";
const INT8C ClearTwoSpaces[] = "  ";
const INT8C ClearTenSpaces[] = "          ";

static ALARMSTATE AlarmState = ARMED;
static INT8U TempUnitSelect = 0;
static INT8U TempAlarm = 0;

void main(void){
    GpioDBugBitsInit();
    GpioLED8Init();
    GpioLED9Init();
    LcdInit();
    KeyInit();
    SysTickDlyInit();
    DMAPIT0Init();
    DMADAC0Init();
    TSIInit();
    TempADCInit();
    TempADCPIT1Init();
    I2CInit();
    MMA8451PLInit();
    DMAInit();
    WDogResetCheck();
    WDogInit();

    while(1){
        SysTickWaitEvent(SLICE_PERIOD);
        WDogTask();
        ControlDisplayTask();
        TempDisplayTask();
        KeyTask();
        TSITask();
        LEDTask();
        AccelDisplayTask();
        RTCDisplayTask();
    }
}
/********************************************************************
* ControlDisplayTask - Handles alarm on/off key press and will display
*                      current alarm state on LCD display
*
* Description:  This task will read the current key press and the current state
*               of the electrodes, and displays the current state of the program.
*               If a B is pressed, the temperature select is changed. If a C is
*               displayed, the tampering alarm is cleared. When in DISARMED mode,
*               if an A is pressed the alarm goes to ARMED. When in ARMED mode,
*               if the touch sensors are active or the temperature went out of
*               bounds, the program will enter ALARM state. If the temperature
*               alarm was triggered, TEMP ALARM will be displayed, else the
*               standard ALARM will be displayed. A D press will exit ALARM to
*               DISARMED state.
*               This task runs once every [2*SLICE_PERIOD] = 20ms.
*
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void ControlDisplayTask(void){
    static INT8U cdt_slice_counter = 1;
    static ALARMSTATE last_state = DISARMED;
    INT8C button_press;
    ALARMSTATE cur_state;
    INT8U electrode1_flag;
    INT8U electrode2_flag;

    DB1_TURN_ON();
    if(cdt_slice_counter > 0){
        cdt_slice_counter = 0;
        button_press = GetKey();
        electrode1_flag = TSIGetSensor(E1FLAG);
        electrode2_flag = TSIGetSensor(E2FLAG);
        cur_state = AlarmState;
        switch(button_press){
            case(B_PRESS):
                TempUnitSelect = ~TempUnitSelect;
                break;
            case(C_PRESS):
                LcdMoveCursor(2,12);
                LcdDispStrg(ClearTwoSpaces);
                break;
            default:
                break;
        }
        switch (cur_state){
            case(DISARMED):
                PIT_TCTRL0 &= PIT_TCTRL_TEN(0);
                if(last_state != cur_state){
                    LcdMoveCursor(2,1);
                    LcdDispStrg(ClearTenSpaces);
                    LcdMoveCursor(2,1);
                    LcdDispStrg(DisarmedPrompt);
                } else{
                }
                switch(button_press){
                    case(A_PRESS):
                        AlarmState = ARMED;
                        break;
                    default:
                        break;
                }
                break;
            case(ARMED):
                PIT_TCTRL0 &= PIT_TCTRL_TEN(0);
                if(last_state != cur_state){
                    LcdMoveCursor(2,1);
                    LcdDispStrg(ClearTenSpaces);
                    LcdMoveCursor(2,1);
                    LcdDispStrg(ArmedPrompt);
                } else{
                }
                if((electrode1_flag == 0x1)||(electrode2_flag == 0x1)||(TempAlarm == 1)){
                    AlarmState = ALARM;
                } else{
                }
                switch(button_press){
                    case(D_PRESS):
                        AlarmState = DISARMED;
                        break;
                    default:
                        break;
                }
                break;
            case(ALARM):
                PIT_TCTRL0 |= PIT_TCTRL_TEN(1);
                if(last_state!= cur_state){
                    LcdMoveCursor(2,1);
                    LcdDispStrg(ClearTenSpaces);
                    LcdMoveCursor(2,1);
                    if(TempAlarm == 1){
                        LcdDispStrg(TempAlarmPrompt);
                    } else{
                        LcdDispStrg(AlarmPrompt);
                    }
                } else{
                }
                switch(button_press){
                    case(D_PRESS):
                        AlarmState = DISARMED;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
        last_state = cur_state;
    } else {
        cdt_slice_counter++;
    }
    DB1_TURN_OFF();
}
/********************************************************************
* LEDTask - Displays on board LED pattern based on current alarm state
*
* Description:  A routine that will feed necessary values to turn on LEDs
*               in touch sensor pads on board (D8 and D9) based on AlarmState.
*               In DISARMED currently activated LEDs will flash at 2Hz. In ARMED
*               LEDs will alternate at 2Hz. In ALARM LEDs that were pressed or
*               will be pressed before disarming will be flashing at 10Hz.
*               Runs once every (5*SLICE_PERIOD): [50ms].
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void LEDTask(void){
    static INT16U ledt_slice_counter = 4;
    static INT8U ledt_enter_counter = 5;
    static INT8U ledt_toggle_counter = 0;
    static INT8U latched_sensor_states;
    INT8U electrode1_flag;
    INT8U electrode2_flag;

    DB4_TURN_ON();
    if(ledt_slice_counter > 3){
        ledt_slice_counter = 0;
        electrode1_flag = TSIGetSensor(E1FLAG);
        electrode2_flag = TSIGetSensor(E2FLAG);
        switch(AlarmState){
            case(ALARM):
                if(electrode1_flag == 1){
                    latched_sensor_states |= 2; //These if/else blocks latch
                } else{                         //which sensors have been activated
                    LED9_TURN_OFF();
                }
                if(electrode2_flag == 1){
                    latched_sensor_states |= 1;
                } else{
                    LED8_TURN_OFF();
                }
                switch(latched_sensor_states){ //These are different LED configurations
                   case(1):                    //based on which sensors have been activated
                        if(ledt_toggle_counter > 0){
                            ledt_toggle_counter = 0;
                            DB7_TURN_OFF();
                            LED9_TURN_OFF();
                        } else{
                            ledt_toggle_counter++;
                            DB7_TURN_ON();
                            LED9_TURN_ON();
                        }
                        break;
                    case(2):
                        if(ledt_toggle_counter > 0){
                            ledt_toggle_counter = 0;
                            LED8_TURN_OFF();
                        } else{
                            ledt_toggle_counter++;
                            LED8_TURN_ON();
                        }
                        break;
                    case(3):
                        if(ledt_toggle_counter > 0){
                            ledt_toggle_counter = 0;
                            LED9_TURN_OFF();
                            LED8_TURN_OFF();
                        } else{
                            ledt_toggle_counter++;
                            LED9_TURN_ON();
                            LED8_TURN_ON();
                        }
                        break;
                    default:
                        break;
                }
                break;
            case(DISARMED):
                latched_sensor_states = 0;
                if(ledt_enter_counter > 4){ //Required to multiply 50ms/slice
                    ledt_enter_counter = 0; //to 250ms/slice for 500ms period
                    switch(electrode1_flag){
                        case(1):
                            LED8_TOGGLE();
                            break;
                        default:
                            LED8_TURN_OFF();
                            break;
                    }
                    switch(electrode2_flag){
                        case(1):
                            LED9_TOGGLE();
                            break;
                        default:
                            LED9_TURN_OFF();
                            break;
                    }
                } else{
                }
                break;
            case(ARMED):
                latched_sensor_states = 0;
                if(ledt_enter_counter > 4){
                    ledt_enter_counter = 0;
                    if(ledt_toggle_counter > 0){
                        ledt_toggle_counter = 0;
                        LED8_TURN_OFF();
                        LED9_TURN_ON();
                    } else{
                        ledt_toggle_counter++;
                        LED8_TURN_ON();
                        LED9_TURN_OFF();
                    }
                } else{
                }
            default:
                break;
        }
        ledt_enter_counter++;
    } else{
        ledt_slice_counter++;
    }
    DB4_TURN_OFF();
}
/********************************************************************
* TempDisplayTask - Handles display of current detected temperature
*
* Description:  A task that will show the current temperature to the LCD.
*               The value will be fetched from "GetTemp". Supports negative
*               values. Sets TempAlarm flag if the ALARM should be set based
*               off of the current temperature (<0c or >40c). This task
*               runs once every time ADC_SC1_COCO_MASK is set, which is
*               the set period of PIT1.
*               This task runs once every 500ms.
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void TempDisplayTask(void){
    INT32S temperature;
    INT8U negative_temp_flag = 0;

    DB6_TURN_ON();
    if((ADC0_SC1A & ADC_SC1_COCO_MASK) != 0){        //Via configuration of PIT1
        temperature = LowADCPull(TempUnitSelect);//this will enter every 500ms
        if(temperature < 0){
            LcdMoveCursor(1,1);
            LcdDispChar('-');
            temperature = (~temperature + 1);
            negative_temp_flag = 1;
        } else{
            LcdMoveCursor(1,1);
            LcdDispChar(' ');
        }
        switch(TempUnitSelect){
            case(0x0):
                LcdMoveCursor(1,2);
                LcdDispDecByte((INT8U)temperature, 0);
                LcdMoveCursor(1,5);
                LcdDispChar(0xDF);
                LcdMoveCursor(1,6);
                LcdDispChar('C');
                if((negative_temp_flag == 1)||(temperature > 40)){
                    TempAlarm = 1;
                } else{
                    TempAlarm = 0;
                }
                break;
            case(0xFF):
                LcdMoveCursor(1,2);
                LcdDispDecByte((INT8U)temperature, 0);
                LcdMoveCursor(1,5);
                LcdDispChar(0xDF);
                LcdMoveCursor(1,6);
                LcdDispChar('F');
                if((temperature < 32)||(temperature > 104)){
                    TempAlarm = 1;
                } else{
                    TempAlarm = 0;
                }
                break;
            default:
                break;
        }
    } else {
    }
    DB6_TURN_OFF();
}
/********************************************************************
* AccelDisplayTask - Handles display of tampering display
*
* Description:  A task that checks whether the status flag for a change in
*               orientation from the accelerometer was set. A tampering display
*               will be shown if the flag was set. Note an extra volatile
*               variable who's purpose is only to ignore the first status flag
*               read. This results from an unknown bug where the status flag
*               is not cleared following the first write (as it should).
*               This task runs once every [5*SLICE_PERIOD] = 50ms.
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void AccelDisplayTask(void){
    static INT16U at_slice_counter = 4;
    static INT8U first_time_run = 1; //This variable is needed for a bug, see above
    INT8U lp_check;

    DB7_TURN_ON();
    if(at_slice_counter > 3){
        at_slice_counter = 0;
        lp_check = (MMA8451RegRd(MMA8451_PL_STATUS)&0x80); //Bit 7 corresponds
        lp_check = lp_check >> 7;                          //to status change
        if((lp_check == 1)&&(first_time_run == 0)){
            LcdMoveCursor(2,12);
            LcdDispStrg(TamperingPrompt);
        } else{
            first_time_run = 0;
        }
    } else{
        at_slice_counter++;
    }
    DB7_TURN_OFF();
}
/********************************************************************
* RTCDisplayTask - Displays current time 24h clock based off of on board RTC.
*
* Description:  This task will pull the current RTC count value (counting at
*               1Hz). The current time is then calculated from this value, with
*               a special offset RTC_OFFSET which is needed for tuning the time.
*               This task runs once every [100*SLICE_PERIOD] = 1000ms.
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void RTCDisplayTask(void){
    static INT8U rtcdt_slice_counter = 99;
    INT32U time;
    INT32U sec;
    INT32U min;
    INT32U hour;

    //DB7_TURN_ON(); //Un comment and comment same macro above for use
    if(rtcdt_slice_counter > 98){
        rtcdt_slice_counter = 0;
        time = ((RTC_TSR + RTC_OFFSET) % 86400); //Creates 0-86400 periodic counter
        sec = (time % 60);                       //from non periodic counter
        min = ((time / 60) % 60);                //(# seconds in a day)
        hour = (((time / 60) / 60) % 24);
        LcdMoveCursor(1,8);
        LcdDispDecByte(hour,0);
        LcdMoveCursor(1,11);
        LcdDispDecByte(min,1);
        LcdMoveCursor(1,11);
        LcdDispChar(':');
        LcdMoveCursor(1,14);
        LcdDispDecByte(sec,1);
        LcdMoveCursor(1,14);
        LcdDispChar(':');
    } else{
        rtcdt_slice_counter++;
    }
    //DB7_TURN_OFF();
}
/********************************************************************
* WDogResetCheck - Displays whether or not the watchdog caused a reset
*
* Description:  Checks current status of System Reset Status Register
*               for set flag for reset caused by watchdog timeout
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void WDogResetCheck(void){
    if((RCM_SRS0&0x20) != 0){
        LcdMoveCursor(2,15);
        LcdDispStrg(WDResetPrompt);
    } else{
    }
}
