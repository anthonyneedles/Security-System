/*******************************************************************************
* TempADC.h - Project header for TempADC.c
*
* Created on: Nov 30, 2017
* Author: Anthony Needles
*******************************************************************************/
#ifndef SOURCES_TEMPADC_H_
#define SOURCES_TEMPADC_H_
/********************************************************************
* TempADCInit - Initializes ADC0
*
* Description:  Enables ADC0 for triggering from PIT1. Configures ADC0 for
*               16 bit samples at 60MHz/8 = 7.5MHz, with a hardware trigger,
*               hardware averaging 32 samples, with a source of DADP3.
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void TempADCInit(void);
/********************************************************************
* TempADCPIT1Init - Initializes PIT1
*
* Description:  Enables PIT1 for use of triggering ADC0 at 2Hz.
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void TempADCPIT1Init(void);
/********************************************************************
* TempADCPIT1Init - Initializes PIT1
*
* Description:  Samples from the ADC0 and converts the number to either it's
*               correct value in C/F depending on argument. Saturates to -10/125
*               and 14/257 for C/F respectively.
*               Note: TEMP_CONV_TRUNC_ERROR_FIX adds one to the MSB to be
*               truncated, which effectively rounds up.
*
* Return value: Signed value of converted temperature value
*
* Arguments:    The selection for which units to display the temp, 1=F, 0=C
********************************************************************/
INT32S LowADCPull(INT8U TempUnitSelect);

#endif /* SOURCES_TEMPADC_H_ */
