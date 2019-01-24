/*******************************************************************************
* TSI.h - Project header file for TSI.c
*
* Created on: Nov 29, 2017
* Author: Anthony Needles
*******************************************************************************/
#ifndef SOURCES_TSI_H_
#define SOURCES_TSI_H_

#define E1FLAG 0
#define E2FLAG 1

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
void TSIInit(void);
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
void TSITask(void);
/********************************************************************
* TSIGetSensor - Sends status of electrodes (activated/idle)
*
* Description:  With the electrode index as a parameter, returns state of
*               corresponding sensor activation flag.
*
* Return value: None
*
* Arguments:    None
********************************************************************/
INT8U TSIGetSensor(INT8U electrode);

#endif /* SOURCES_TSI_H_ */
