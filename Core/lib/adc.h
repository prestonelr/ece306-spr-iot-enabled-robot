//===========================================================================
// File Name : adc.h
//
// Description: Simple ADC driver for MSP430FR2355
//              - Blocking single channel read
//              - Direct millivolt output
//              - Includes Battery input (P5.0 / A8) with divider correction
//
// Author: Preston Elrod
// Date: Feb 2026
//===========================================================================


// ---------------- ADC CHANNEL DEFINES ----------------
// P1.2 = A2 = V_DETECT_L
// P1.3 = A3 = V_DETECT_R
// P1.5 = A5 = V_THUMB
// P5.0 = A8 = V_BAT (measured through divider)

#define ADC_CH_LEFT      2
#define ADC_CH_RIGHT     3
#define ADC_CH_THUMB     5
#define ADC_CH_BAT       8

// ---------------- ADC REFERENCE ----------------
// Using AVCC = 3.3V reference
#define ADC_VREF_MV      3300

// ---------------- BATTERY DIVIDER ----------------
// VBAT -> R_TOP (20k) -> SENSE NODE -> R_BOTTOM (10k) -> GND
#define VBAT_DIV_RTOP_OHMS     20000UL
#define VBAT_DIV_RBOT_OHMS     10000UL

// ---------------- FUNCTION PROTOTYPES ----------------
void Init_ADC(void);

unsigned ADC_ReadChannel(unsigned channel);
unsigned ADC_ReadMilliVolts(unsigned channel);

// Returns ACTUAL battery voltage (VBAT) in millivolts, corrected for divider
unsigned ADC_ReadBatteryMilliVolts(void);


