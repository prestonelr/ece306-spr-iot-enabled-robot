//===========================================================================
// File Name : init.c
//
// Description: This file inits all components on the board
//
// Author: Preston Elrod
// Date: Feb 2026
// Compiler: Built with Code Composer Version: CCS12.4.0.00007_win64
//===========================================================================


#include  "Include\macros.h"
#include  "Include\functions.h"
#include  "Include\display.h"
#include "Include\led.h"
#include  "msp430.h"
#include "Include\ports.h"

void Init_Ports(void);
void Init_Clocks(void);
void Init_Display_Conditions(void);
void Init_Timers(void);
void Init_LCD(void);
void Init_LEDs(void);


void Init_All(void) {

    Init_Ports();                        // Initialize Ports
    Init_Clocks();                       // Initialize Clock System
    Init_Display_Conditions();                   // Initialize Variables and Initial Conditions
    Init_Timers();                       // Initialize Timers
    Init_LCD();                         // Initialize LCD
    Init_LEDs(); // turns on both leds
    Init_Display_Conditions(); // initalizes the display
    enable_interrupts(); //enables interrupts that are disabled by default
}
