//===========================================================================
// File Name : init.c
//
// Description: This file inits all components on the board
//
// Author: Preston Elrod
// Date: Feb 2026
// Compiler: Built with Code Composer Version: CCS12.4.0.00007_win64
//===========================================================================

#include  "Core\core.h"

void Init_All(void) {

  Init_Ports();                 // GPIO + peripheral pin mux first
  Init_Clocks();                // ACLK/SMCLK/MCLK stable

  Init_LCD();                   // LCD + SPI must be ready BEFORE any display updates
  Init_Display_Conditions();    // Initialize display_line[] contents + display_changed

  Init_Timer_B0();              // Now start 200ms update_display timer
  enable_interrupts();          // Enable port interrupts (SW1/SW2 etc.)

  Init_Ports();
  Init_Clocks();
  Init_LCD();
  Init_Display_Conditions();
  Init_Timer_B0();
  enable_interrupts();     // enables P4IE/P2IE etc.
  __enable_interrupt();    // GIE last
}
