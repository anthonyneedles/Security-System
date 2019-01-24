/*******************************************************************************
* TempADC.c - This module initializes ADC0 and PIT1 for use of sampling the
*             temperature via an external analog temperature sensor MCP9701.
*             ADC0 sample triggers are received from PIT1 at 2Hz. TempADCConvert
*             returns converted temperature value.
*
* Created on: Nov 30, 2017
* Author: Anthony Needles
*******************************************************************************/
#include "MCUType.h"
#include "K65TWR_GPIO.h"
#include "TempADC.h"

#define PIT1_TIMER_VALUE 30000000
#define TEMP_CEL_CONV_SCALE_Q15 85
#define TEMP_CEL_CONV_OFFSET_Q15 681235
#define TEMP_CONV_TRUNC_ERROR_FIX 0x4000u
#define TEMP_FAHR_CONV_SCALE_Q15 153
#define TEMP_FAHR_CONV_OFFSET_Q15 177648

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
void TempADCInit(void){
    SIM_SCGC6 |= (SIM_SCGC6_ADC0_MASK);
    SIM_SOPT7 |= (SIM_SOPT7_ADC0ALTTRGEN(1)|SIM_SOPT7_ADC0TRGSEL(5));
    ADC0_CFG1 |= (ADC_CFG1_ADIV(3)|ADC_CFG1_MODE(3)|ADC_CFG1_ADICLK(0));
    ADC0_SC2 |= (ADC_SC2_ADTRG(1));
    ADC0_SC3 |= (ADC_SC3_AVGE(1)|ADC_SC3_AVGS(3));
    ADC0_SC1A = (ADC_SC1_ADCH(3));
}
/********************************************************************
* TempADCPIT1Init - Initializes PIT1
*
* Description:  Enables PIT1 for use of triggering ADC0 at 2Hz.
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void TempADCPIT1Init(void){
    SIM_SCGC6 |= (SIM_SCGC6_PIT(1));
    PIT_MCR = (PIT_MCR & ~PIT_MCR_MDIS_MASK);
    PIT_TCTRL1 |= (PIT_TCTRL_TEN(1));
    PIT_LDVAL1 = PIT1_TIMER_VALUE;
}
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
INT32S LowADCPull(INT8U TempUnitSelect){
    INT32U temp_sample;
    INT32S actual_temp;

    temp_sample = ADC0_RA;
    switch(TempUnitSelect){
        case(0x0):
            if(temp_sample < 4172){
                actual_temp = -10;
            } else if(temp_sample > 56372){
                actual_temp = 125;
            } else{
                actual_temp = (TEMP_CEL_CONV_SCALE_Q15*temp_sample);
                actual_temp = (actual_temp - TEMP_CEL_CONV_OFFSET_Q15);
                actual_temp = (actual_temp + TEMP_CONV_TRUNC_ERROR_FIX);
                actual_temp = (actual_temp >> 15);
            }
            break;
        case(0xFF):
            if(temp_sample < 4172){
                actual_temp = 14;
            } else if(temp_sample > 56372){
                actual_temp = 257;
            } else{
                actual_temp = (TEMP_FAHR_CONV_SCALE_Q15*temp_sample);
                actual_temp = (actual_temp - TEMP_FAHR_CONV_OFFSET_Q15);
                actual_temp = (actual_temp + TEMP_CONV_TRUNC_ERROR_FIX);
                actual_temp = (actual_temp >> 15);
            }
            break;
        default:
            break;
    }
    return (actual_temp);
}
