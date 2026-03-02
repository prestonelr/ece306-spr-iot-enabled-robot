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
#include "Addon\lib\line.h"
#include "Addon\lib\led.h"

void main(void)
{
  PM5CTL0 &= ~LOCKLPM5;

  SMCLK_CONFIG = USE_SMCLK; // USE_GPIO or USE_SMCLK
  Init_All();
  Line_Init();


  while(ALWAYS)
  {
    Carlson_StateMachine();   // if you still use it
    Line_Task();

    UpdateDisplay();
    Display_Process();
  }
}
