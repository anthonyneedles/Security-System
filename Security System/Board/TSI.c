/*******************************************************************************
* TSI.c - A module for initializing the touch sensors on board, polling the
*            current state of the sensors, and sending the current state of the
*            sensors.
*
* Created on: Nov 29, 2017
* Author: Anthony Needles
*******************************************************************************/
#include "MCUType.h"
#include "TSI.h"
#include "K65TWR_GPIO.h"

#define E1_TOUCH_OFFSET 2000
#define E2_TOUCH_OFFSET 2000
#define E1 12
#define E2 11

typedef enum{E2SCAN_E1READ, E1SCAN_E2READ} SENSORSTATE;

static INT16U tsiTouchLevelE1;
static INT16U tsiTouchLevelE2;
static SENSORSTATE tsiSensorState;
static INT8U tsiSensorFlags[2];

/********************************************************************
* TSIInit - Initializes  and calibrates TSI
*
* Description:  Enables electrodes 0 and 1 for use within program. Calibrates
*               the sensors based off of a control baseline and a small offset
*               to ensure noise does not trigger the sensors yet light presses
*               still get sensed.
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void TSIInit(void){
    INT16U baseline_e1;
    INT16U baseline_e2;

    SIM_SCGC5 |= (SIM_SCGC5_PORTB_MASK|SIM_SCGC5_TSI_MASK);
    PORTA_PCR18 = PORT_PCR_MUX(0);
    PORTA_PCR19 = PORT_PCR_MUX(0);
    TSI0_GENCS = (TSI_GENCS_MODE(0)|TSI_GENCS_REFCHRG(5)|TSI_GENCS_DVOLT(0)|TSI_GENCS_EXTCHRG(5)|TSI_GENCS_PS(5)|TSI_GENCS_NSCN(15));
    TSI0_GENCS |= TSI_GENCS_TSIEN(1);

    TSI0_DATA = TSI_DATA_TSICH(E1); //Calibration for E1
    TSI0_DATA |= TSI_DATA_SWTS(1);
    while(!(TSI0_GENCS & TSI_GENCS_EOSF_MASK)){} //Wait for scan to finish
    TSI0_GENCS |= TSI_GENCS_EOSF(1);
    baseline_e1 = (INT16U)(TSI0_DATA & TSI_DATA_TSICNT_MASK);
    tsiTouchLevelE1 = (baseline_e1 + E1_TOUCH_OFFSET);

    TSI0_DATA = TSI_DATA_TSICH(E2); //Calibration for E2
    TSI0_DATA |= TSI_DATA_SWTS(1);
    while(!(TSI0_GENCS & TSI_GENCS_EOSF_MASK)){} //Is not blocking
    TSI0_GENCS |= TSI_GENCS_EOSF(1);
    baseline_e2 = (INT16U)(TSI0_DATA & TSI_DATA_TSICNT_MASK);
    tsiTouchLevelE2 = (baseline_e2 + E2_TOUCH_OFFSET);
}
/********************************************************************
* TSITask - Poll both TSI counters for values over calibrated TouchLevel
*
* Description:  Uses a state machine design that starts a scan for second electrode,
*               while reading the current value for the first, then switches to
*               reading the second electrode and starting scan for the first to
*               ensure scanning is completed by the time the value is read.
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void TSITask(void){
    static INT8U st_slice_counter = 1;

    DB3_TURN_ON();
    if(st_slice_counter > 0){
        st_slice_counter = 0;
        switch(tsiSensorState){
            case(E2SCAN_E1READ):
                TSI0_DATA = TSI_DATA_TSICH(E2); //Start E2 scan
                TSI0_DATA |= TSI_DATA_SWTS(1);
                if((INT16U)(TSI0_DATA & TSI_DATA_TSICNT_MASK) > tsiTouchLevelE1){
                    tsiSensorFlags[E1FLAG] = 1; //Read E1
                } else{
                    tsiSensorFlags[E1FLAG] = 0;
                }
                tsiSensorState = E1SCAN_E2READ;
                break;
            case(E1SCAN_E2READ):
                TSI0_DATA = TSI_DATA_TSICH(E1); //Start E1 scan
                TSI0_DATA |= TSI_DATA_SWTS(1);
                if((INT16U)(TSI0_DATA & TSI_DATA_TSICNT_MASK) > tsiTouchLevelE2){
                    tsiSensorFlags[E2FLAG] = 1; //Read E2
                } else{
                    tsiSensorFlags[E2FLAG] = 0;
                }
                tsiSensorState = E2SCAN_E1READ;
                break;
            default:
                break;
        }
    } else{
        st_slice_counter++;
    }
    DB3_TURN_OFF();
}
/********************************************************************
* TSIGetSensor - Sends status of electrodes (activated/idle)
*
* Description:  With the electrode index as a parameter, returns state of
*               corresponding sensor activation flag.
*
* Return value: Current state of requested electrode
*
* Arguments:    Requested electrode to have it's state returned
********************************************************************/
INT8U TSIGetSensor(INT8U electrode){
    return(tsiSensorFlags[electrode]);
}



