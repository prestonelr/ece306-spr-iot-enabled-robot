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




void main(void){

  PM5CTL0 &= ~LOCKLPM5; // Disable the GPIO power-on default high-impedance mode to activate previously configured port settings

  SMCLK_CONFIG = USE_SMCLK; // USE_GPIO or USE_SMCLK
  Init_All();

  ChangeBacklight(1);
  __delay_cycles(8000000);   // ~1s at 8MHz
  ChangeBacklight(0);
  __delay_cycles(8000000);


//------------------------------------------------------------------------------
  while(ALWAYS) {                      // Can the Operating system run
    Carlson_StateMachine();            // Run a Time Based State Machine
    Display_Process();                 // Update Display
    //P3OUT ^= TEST_PROBE;               // Change State of TEST_PROBE OFF
  }
//------------------------------------------------------------------------------

}




