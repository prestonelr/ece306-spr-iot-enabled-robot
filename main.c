//===========================================================================
// File Name : main.c
//
// Description: Contains the main while loop and functions of the board
//
// Author: Preston Elrod
// Date: Feb 2026
// Compiler: Built with Code Composer Version: CCS12.4.0.00007_win64
//===========================================================================

#include "Core\core.h"
#include "Addon\lib\led.h"
#include "msp430.h"

static char line[11];

// Make "line" = 10 chars wide, right-aligned decimal number, padded with spaces
static void PutNumberLine10(unsigned v)
{
  int i;

  // fill with spaces
  for(i = 0; i < 10; i++) line[i] = ' ';
  line[10] = 0;

  // write digits from right to left
  i = 9;
  if(v == 0)
  {
    line[i] = '0';
    return;
  }

  while(v > 0 && i >= 0)
  {
    line[i] = (char)('0' + (v % 10));
    v /= 10;
    i--;
  }
}

void main(void)
{
  PM5CTL0 &= ~LOCKLPM5;

  SMCLK_CONFIG = USE_SMCLK; // USE_GPIO or USE_SMCLK
  Init_All();

  while(ALWAYS)
  {
    Carlson_StateMachine();

    // Build all 4 lines first
    PutNumberLine10(ADC_ReadMilliVolts(ADC_CH_LEFT));
    ChangeDisplay(line, 0);

    PutNumberLine10(ADC_ReadMilliVolts(ADC_CH_RIGHT));
    ChangeDisplay(line, 1);

    PutNumberLine10(ADC_ReadMilliVolts(ADC_CH_THUMB));
    ChangeDisplay(line, 2);

    PutNumberLine10(ADC_ReadBatteryMilliVolts());
    ChangeDisplay(line, 3);

    // Request + push update
    UpdateDisplay();
    Display_Process();

  }
}
