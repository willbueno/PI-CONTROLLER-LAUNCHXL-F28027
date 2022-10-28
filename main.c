/**
 * main.c
 *
 *  Created on: 26 de out de 2022
 *      Author: William
 */
#include "Peripheral_Setup.h"
#include "math.h"

#define PI 3.14159265359

const float Kp = 1.109656605978368;
const float Ki = 0.666049185980663;
const float Ts = 25E-6;

float setpoint = 1.5;
float v_out = 0;

float u[2] = {0, 0};
float error[2] = {0, 0};

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
    v_out = AdcResult.ADCRESULT0 * 8.0586E-04;

    // Error
    error[0] = setpoint - v_out;

    // PI CONTROLLER
    u[0] = u[1] + Kp * (error[0] - error[1]) + Ki * Ts * 0.5 * (error[0] + error[1]);

    // Anti wind-up
    if (u[0] > 3.3)
    {
        u[0] = 3.3;
    }
    else if (u[0] < 0)
    {
        u[0] = 0.0;
    }

    // Update variables
    u[1] = u[0];
    error[1] = error[0];

    // Update duty cycle
    EPwm1Regs.CMPA.half.CMPA = (Uint16) (u[0] * 4.545454545454546E02);

    AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}
