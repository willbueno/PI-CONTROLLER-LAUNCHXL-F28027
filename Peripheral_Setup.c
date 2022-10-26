/*
 * Peripheral_Setup.c
 *
 *  Created on: 26 de out de 2022
 *      Author: William
 */

#include "Peripheral_Setup.h"

void Setup_GPIO(void)
{
    EALLOW;

    // LED 3
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 0;                 // GPIO2 - LED 3
    GpioCtrlRegs.GPADIR.bit.GPIO2 = 1;                  // I/O DIR - Output = 1

    // LED 4
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 0;                 // GPIO2 - LED 4
    GpioCtrlRegs.GPADIR.bit.GPIO3 = 1;                  // I/O DIR - Output = 1

    // PWM 1
    GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 1;                 // Configure GPIO0 as EPWM1A
    GpioCtrlRegs.GPAPUD.bit.GPIO4 = 1;                  // Disable pull-up on GPIO1

    EDIS;
}

void Setup_ePWM(void)
{
    EALLOW;

    SysCtrlRegs.PCLKCR1.bit.EPWM1ENCLK =1;              // ePWM1

    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;

    // PWM 1A
    EPwm1Regs.TBPRD = 1500;                             // Set period 20 kHz - uC_clock / (2*fs) for UP/DOWN

    EPwm1Regs.CMPA.half.CMPA = 750;                     // Duty Cicle in 50% of PRD

    EPwm1Regs.TBPHS.half.TBPHS = 0;                     // Phase is 0
    EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO;
    EPwm1Regs.TBCTR = 0x0000;                           // Clear counter
    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;      // Count UP/DOWN
    EPwm1Regs.TBCTL.bit.PHSEN = TB_ENABLE;              // Enable phase loading
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;            // Clock ratio to SYSCLKOUT
    EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV1;

    EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;         // Load register every ZERO and PRD
    EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO_PRD;
    EPwm1Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm1Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO_PRD;   // Load register every ZERO and PRD

    EPwm1Regs.AQCTLA.bit.PRD = AQ_NO_ACTION;
    EPwm1Regs.AQCTLA.bit.ZRO = AQ_NO_ACTION;
    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;                // Set actions for EPWM3A
    EPwm1Regs.AQCTLA.bit.CAD = AQ_SET;                  // Set actions for EPWM3A

    // Trigger ADC
    EPwm1Regs.ETSEL.bit.SOCAEN = 1;                     // Enable SOC on A group
    EPwm1Regs.ETSEL.bit.SOCASEL = ET_CTR_PRDZERO;       // Trigger ADC on PRDZERO
    EPwm1Regs.ETPS.bit.SOCAPRD = ET_1ST;                // Trigger on every event

    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;              // Enable TBCLK within the EPWM

    EDIS;
}

void Setup_ADC(void)
{
    EALLOW;

    SysCtrlRegs.PCLKCR0.bit.ADCENCLK = 1;

    AdcRegs.ADCCTL1.bit.ADCBGPWD = 1;                   // Bandgap buffer's circuitry inside core is powered up
    AdcRegs.ADCCTL1.bit.ADCREFPWD = 1;                  // Reference buffers circuitry inside the core is powered up
    AdcRegs.ADCCTL1.bit.ADCPWDN = 1;                    // The analog circuitry inside the core is powered up
    AdcRegs.ADCCTL1.bit.ADCENABLE = 1;                  // ADC Enabled
    AdcRegs.ADCCTL1.bit.ADCREFSEL = 0;                  // Internal Bandgap used for reference generation
    AdcRegs.ADCCTL1.bit.INTPULSEPOS = 1;                // Set pulse one cycle after result
    DELAY_US(1000);                                     // Delay before converting ADC channels

    AdcRegs.ADCSAMPLEMODE.all = 0x0000;                 // Simultaneous sampling enable

    AdcRegs.ADCINTSOCSEL1.all = 0x0000;                 // SOCx ADC Interrupt Trigger Select, 00 = No ADCINTT will trigger SOCx.

    AdcRegs.ADCSOC0CTL.bit.CHSEL = 0x00;                // Selects the channel to be converted when SOCx is received by the ADC, 0 = ADCINA0
    AdcRegs.ADCSOC0CTL.bit.ACQPS = 0x09;                // SOCx Acquisition Prescale. Controls the sample and hold window for SOCx. Minimum 06h.
    AdcRegs.ADCSOC0CTL.bit.TRIGSEL = 0x05;              // SOCx Trigger Source Select, 0x05 = ePWM1, ADCSOCA

    AdcRegs.INTSEL1N2.bit.INT1E = 1;                    // Enable ADCINT1
    AdcRegs.INTSEL1N2.bit.INT1CONT = 1;                 // ADCINTx Continuous Mode Enable. ADCINTx pulses are generated whenever an EOC pulse is generated irrespective if the flag bit is cleared or not.
    AdcRegs.INTSEL1N2.bit.INT1SEL = 0;                  // Connect ADCINT1 to EOC1
    AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;               // Make sure INT1 flag is clear

    EDIS;
}
