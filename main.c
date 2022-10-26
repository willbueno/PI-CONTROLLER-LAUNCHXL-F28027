/**
 * main.c
 *
 *  Created on: 26 de out de 2022
 *      Author: William
 */
#include "Peripheral_Setup.h"
#include "math.h"

#define PI 3.14159265359
#define Kp 1560                                         // Kp = Kp*PRD <- Kp = 1.04  PRD = 1500
#define Ki 0.626
#define Ts 0.00005

//uint32_t index = 0;
//uint16_t sinetable[400];

//uint16_t plot[400];

uint16_t adc1 = 0;
uint16_t adc2 = 0;
uint16_t *adc = &adc1;

float setpoint = 1.5;
float out = 0;
float e = 10;
float e_1 = 10;
float u = 0;
float u_1 = 0;

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
    InitGpio();                                         // Initialize default GPIO

    Setup_GPIO();                                       // Initialize my configs in GPIO
    Setup_ePWM();                                       // Initialize my configs in ePWM
    Setup_ADC();                                        // Initialize my configs in ADC

    EALLOW;
    PieVectTable.TINT0 = &isr_cpu_timer0;               // Pointer to interrupt function
    PieVectTable.ADCINT1 = &isr_adc;                    // Pointer to interrupt function
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;                  // Enable Timer 0 - Line 1 - Column 7
    PieCtrlRegs.PIEIER1.bit.INTx1 = 1;                  // Enable ADC
    EDIS;
    IER |= M_INT1;                                      // Enable register interruption line 1

    InitCpuTimers();                                    // Initialize Timer 0
    ConfigCpuTimer(&CpuTimer0, 60, 100000);             // Config Timer 0 (Freq - MHz, T - us) - Frequency clock: 60 MHz and 100 ms interruption
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
    // ADC
    adc1 = AdcResult.ADCRESULT0;
    out = 3.3 * adc1 / 4095.0;

    // PI CONTROLLER
    e = setpoint - out;
    u = u_1 + Kp * (e - e_1) + 0.5 * Ki * Ts * (e + e_1);

    if (u > 1500)
    {
        u = 1500;
    }
    else if (u < 0)
    {
        u = 0;
    }

    u_1 = u;
    e_1 = e;

    EPwm1Regs.CMPA.half.CMPA = (uint16_t) u;

    // GRAPHS
//    plot[index] = *adc;

    AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}
