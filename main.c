//===========================================================================
// File Name : main.c
//
// Description: Contains the main while loop and functions of the board
//
// Author: Preston Elrod
// Date: Feb 2026
// Compiler: Built with Code Composer Version: CCS12.4.0.00007_win64
//===========================================================================

//------------------------------------------------------------------------------
#include  "Core\core.h"
#include "Addon\lib\led.h"
#include "Addon\lib\shape.h"



void main(void){

  PM5CTL0 &= ~LOCKLPM5; // Disable the GPIO power-on default high-impedance mode to activate previously configured port settings

  SMCLK_CONFIG = USE_SMCLK; // USE_GPIO or USE_SMCLK
  Init_All();
  Shape_Init();

//------------------------------------------------------------------------------
  while(ALWAYS) {                      // Can the Operating system run
    Carlson_StateMachine();            // Run a Time Based State Machine
    Shape_Task();
    Display_Process();                 // Update Display
    //P3OUT ^= TEST_PROBE;               // Change State of TEST_PROBE OFF
  }
//------------------------------------------------------------------------------

}




