//===========================================================================
// File Name : adc.c
//
// Description: Simple blocking ADC driver
//              Uses AVCC as reference
//              Returns raw value or millivolts
//              Battery reading includes divider correction
//
// Author: Preston Elrod
// Date: Feb 2026
//===========================================================================

#include <msp430.h>
#include "Core\lib\ports.h"
#include "Core\lib\adc.h"

//===========================================================================
// Init_ADC
// Configure ADC for 12-bit single channel conversion
//===========================================================================
void Init_ADC(void)
{
    // --- Configure Analog Pins ---
    P1DIR &= ~(V_DETECT_L | V_DETECT_R | V_THUMB);
    P1OUT &= ~(V_DETECT_L | V_DETECT_R | V_THUMB);
    P1SELC |=  (V_DETECT_L | V_DETECT_R | V_THUMB);

    // NOTE: If your battery sense pin is on P5.0 (A8), configure it too.
    // Some projects handle P5 analog select elsewhere; leaving this comment
    // so you can enable if needed:
    //
    // P5DIR &= ~BIT0;
    // P5OUT &= ~BIT0;
    // P5SELC |= BIT0;

    // --- Disable ADC before config ---
    ADCCTL0 &= ~ADCENC;

    // --- ADCCTL0 ---
    ADCCTL0 = 0;
    ADCCTL0 |= ADCSHT_2;     // 16 clock sample time
    ADCCTL0 |= ADCON;        // ADC ON

    // --- ADCCTL1 ---
    ADCCTL1 = 0;
    ADCCTL1 |= ADCSHS_0;     // trigger from ADCSC
    ADCCTL1 |= ADCSHP;       // use sampling timer
    ADCCTL1 |= ADCDIV_0;     // clock divide /1
    ADCCTL1 |= ADCSSEL_0;    // MODCLK
    ADCCTL1 |= ADCCONSEQ_0;  // single-channel, single-conversion

    // --- ADCCTL2 ---
    ADCCTL2 = 0;
    ADCCTL2 |= ADCPDIV0;     // pre-divider /1
    ADCCTL2 |= ADCRES_2;     // 12-bit resolution
    ADCCTL2 &= ~ADCDF;       // unsigned
    ADCCTL2 &= ~ADCSR;       // <=200ksps

    // --- Memory Control ---
    ADCMCTL0 = 0;
    ADCMCTL0 |= ADCSREF_0;   // VR+ = AVCC, VR- = AVSS

    // ADC ready
}

//===========================================================================
// ADC_ReadChannel
// Blocking read of selected channel
// Returns raw 12-bit value (0–4095)
//===========================================================================
unsigned ADC_ReadChannel(unsigned channel)
{
    unsigned value;

    ADCCTL0 &= ~ADCENC;          // allow channel change

    ADCMCTL0 &= ~ADCINCH_15;     // clear channel field
    ADCMCTL0 |= channel;         // set desired channel

    ADCCTL0 |= ADCENC;
    ADCCTL0 |= ADCSC;            // start conversion

    while (ADCCTL1 & ADCBUSY);   // wait until complete

    value = ADCMEM0;

    return value;
}

//===========================================================================
// ADC_ReadMilliVolts
// Returns voltage in millivolts at the ADC pin
// Formula: V = (ADC * Vref) / 4095
//===========================================================================
unsigned ADC_ReadMilliVolts(unsigned channel)
{
    unsigned long temp;

    temp = ADC_ReadChannel(channel);
    temp = temp * ADC_VREF_MV;
    temp = temp / 4095;          // 12-bit full scale

    return (unsigned)temp;
}

//===========================================================================
// ADC_ReadBatteryMilliVolts
// Battery sense is through divider:
//   VBAT -> RTOP -> node -> RBOT -> GND
// node = VBAT * (RBOT / (RTOP + RBOT))
// VBAT = node * ((RTOP + RBOT) / RBOT)
//
// Returns corrected VBAT in millivolts
//===========================================================================
unsigned ADC_ReadBatteryMilliVolts(void)
{
    unsigned long node_mv;
    unsigned long vbat_mv;

    node_mv = ADC_ReadMilliVolts(ADC_CH_BAT);

    // vbat_mv = node_mv * (RTOP + RBOT) / RBOT
    vbat_mv = node_mv * (VBAT_DIV_RTOP_OHMS + VBAT_DIV_RBOT_OHMS);
    vbat_mv = vbat_mv / VBAT_DIV_RBOT_OHMS;

    return (unsigned)vbat_mv;
}
