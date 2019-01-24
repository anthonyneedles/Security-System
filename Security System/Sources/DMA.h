/*******************************************************************************
* DMA.h -   Project header file for DMA.c
*
* Created on: Dec 7, 2017
* Author: Anthony Needles
*******************************************************************************/
#ifndef SOURCES_DMA_H_
#define SOURCES_DMA_H_

#define PIT0_TIMER_VALUE 3124

/********************************************************************
* DMAInit - Initializes DMA0
*
* Description:  Enables DMA for use with transferring data in dmaWaveTable to
*               DAC0. Uses PIT0 for triggering.
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void DMAInit(void);
/********************************************************************
* DMADAC0Init - Initializes DAC0
*
* Description:  Enables both DAC0 clocks. Enables DAC system, software trigger,
*               and VDDA reference. Also enables DMA and DAC buffer.
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void DMADAC0Init(void);
/********************************************************************
* DMAPIT0Init - Initializes PIT0
*
* Description:  Enables PIT clock. Enables all standard timers. Enables
*               PIT0 timer and PIT0 timer interrupt. Triggers at 300Hz.
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void DMAPIT0Init(void);

#endif /* SOURCES_DMA_H_ */
