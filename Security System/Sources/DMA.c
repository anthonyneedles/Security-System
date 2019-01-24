/*******************************************************************************
* DMA.c -   A module of initializing DMA0, DAC0, and PIT0 to feed 12bit values
*           for a sinusoidal wave of 300Hz with 4 harmonies at 600Hz, 900Hz,
*           1200Hz, and 1500Hz. These values are taken by the DMA to the DAC
*           with triggers from the PIT.
*
* Created on: Dec 7, 2017
* Author: Anthony Needles
*******************************************************************************/
#include "MCUType.h"
#include "K65TWR_GPIO.h"
#include "DMA.h"

const INT16U dmaWaveTable[] = {0x800, 0xA4C, 0xC46, 0xDA9, 0xE4C, 0xE2B, 0xD61,
                            0xC24, 0xABB, 0x96A, 0x86B, 0x7DE, 0x7C8, 0x815,
                            0x89B, 0x92C, 0x999, 0x9C5, 0x9A5, 0x943, 0x8B8,
                            0x82A, 0x7B9, 0x780, 0x788, 0x7C7, 0x829, 0x88D,
                            0x8D6, 0x8ED, 0x8C9, 0x873, 0x7FF, 0x78C, 0x736,
                            0x712, 0x729, 0x772, 0x7D6, 0x838, 0x877, 0x87F,
                            0x846, 0x7D5, 0x747, 0x6BC, 0x65A, 0x63A, 0x666,
                            0x6D3, 0x764, 0x7EA, 0x837, 0x821, 0x794, 0x695,
                            0x544, 0x3DB, 0x29E, 0x1D4, 0x1B3, 0x256, 0x3B9,
                            0x5B3};

/********************************************************************
* DMAInit - Initializes DMA0
*
* Description:  Enables DMA for use with transferring data in dmaWaveTable to
*               DAC0. Uses PIT0 for triggering. Disables the DMAMUX, the gives
*               the source table address, 16bit data size, 2byte increments, -128
*               end address change (return to original address). For destination
*               the address is the DAC0 data register, 0 byte offset, 16 bit size.
*               Minor loop size increment of 2 byes, with 64 samples total.
*               Scatter/gather processing turned on. The DMAMUX is then reenabled
*               with DMAMUX 0 selected.
*
*
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void DMAInit(void){
    SIM_SCGC6 |= SIM_SCGC6_DMAMUX_MASK;
    SIM_SCGC7 |= SIM_SCGC7_DMA_MASK;

    DMAMUX_CHCFG(0) |= (DMAMUX_CHCFG_ENBL(0)|DMAMUX_CHCFG_TRIG(0));
    DMA_SADDR(0) = DMA_SADDR_SADDR(dmaWaveTable);
    DMA_ATTR(0) = (DMA_ATTR_SSIZE(001) | DMA_ATTR_SMOD(0) | DMA_ATTR_DMOD(0) | DMA_ATTR_DSIZE(001));
    DMA_SOFF(0) = 2;
    DMA_SLAST(0) = DMA_SLAST_SLAST(-128);
    DMA_DADDR(0) = DMA_DADDR_DADDR(&DAC0_DAT0L);
    DMA_DOFF(0) = DMA_DOFF_DOFF(0);
    DMA_TCD0_NBYTES_MLNO = DMA_NBYTES_MLNO_NBYTES(2);
    DMA_CITER_ELINKNO(0) = DMA_CITER_ELINKNO_CITER(64);
    DMA_BITER_ELINKNO(0) = DMA_BITER_ELINKNO_BITER(64);
    DMA_TCD0_CSR = 0x5300u;
    DMA_DLAST_SGA(0) = DMA_DLAST_SGA_DLASTSGA(0);
    DMAMUX_CHCFG(0) = DMAMUX_CHCFG_ENBL(1)|DMAMUX_CHCFG_TRIG(1)|DMAMUX_CHCFG_SOURCE(60);
    DMA_SERQ = DMA_SERQ_SERQ(0);

}
/********************************************************************
* DMADAC0Init - Initializes DAC0
*
* Description:  Enables DAC system, software trigger, and VDDA reference. Also
*               enables DMA and DAC buffer.
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void DMADAC0Init(void){
    SIM_SCGC2 = (SIM_SCGC2 | SIM_SCGC2_DAC0(1));
    DAC0_C0 |= DAC_C0_DACEN(1) | DAC_C0_DACRFS(1) | DAC_C0_DACTRGSEL(1);
    DAC0_C1 |= (DAC_C1_DMAEN(1) | DAC_C1_DACBFEN(1));
}
/********************************************************************
* DMAPIT0Init - Initializes PIT0
*
* Description:  Enables PIT clock 0. Enables all standard timers. Enables
*               PIT0 timer and PIT0 timer interrupt. Triggers at 300Hz.
*
* Return value: None
*
* Arguments:    None
********************************************************************/
void DMAPIT0Init(void){
    SIM_SCGC6 = (SIM_SCGC6 | SIM_SCGC6_PIT(1));
    PIT_MCR = PIT_MCR_MDIS(0);
    PIT_TCTRL0 = (PIT_TCTRL0 | PIT_TCTRL_TIE(1) | PIT_TCTRL_TEN(1));
    PIT_LDVAL0 = PIT0_TIMER_VALUE;
}



