/**
 * main.c
 *
 *  Created on: 26 de out de 2022
 *      Author: William
 */

#include "Peripheral_Setup.h"

__interrupt void isr_cpu_timer0(void);
__interrupt void isr_adc(void);

int main(void)
{
    InitSysCtrl();                                      // Initialize System Control
    DINT;                                               // Disable CPU interrupts
    InitPieCtrl();                                      // Initialize the PIE control registers to their default state

    IER = 0x0000;                                       // Disable CPU interrupts
    IFR = 0x0000;                                       // Clear all CPU interrupt flags

    InitPieVectTable();                                 // Initialize the PIE vector table

    Setup_GPIO();
    Setup_ePWM();

    EALLOW;
    PieVectTable.TINT0 = &isr_cpu_timer0;               // Pointer to interrupt function
    PieVectTable.ADCINT1 = &isr_adc;                    // Pointer to interrupt function
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;                  // Enable Timer 0 - Line 1 - Column 7
    PieCtrlRegs.PIEIER1.bit.INTx1 = 1;                  // Enable ADC
    EDIS;
    IER |= M_INT1;                                      // Enable interruption line 1

    InitCpuTimers();
    ConfigCpuTimer(&CpuTimer0, 60, 100000);             // Frequency clock: 60 MHz, 100 ms interruption
    StartCpuTimer0();                                   // Enable Timer 0 interruption

    EINT;                                               // Enable Global interrupt INTM
    ERTM;                                               // Enable Global real time interrupt DBGM

    // Configuring LEDs
    GpioDataRegs.GPASET.bit.GPIO2 = 1;
    GpioDataRegs.GPACLEAR.bit.GPIO3 = 1;

    while (1)
    {
    }

    return 0;
}

__interrupt void isr_cpu_timer0(void)
{
    GpioDataRegs.GPATOGGLE.bit.GPIO2 = 1;
    GpioDataRegs.GPATOGGLE.bit.GPIO3 = 1;

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

__interrupt void isr_adc(void)
{
    GpioDataRegs.GPADAT.bit.GPIO2 = 1;

    AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;
    GpioDataRegs.GPADAT.bit.GPIO2 = 0;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}
