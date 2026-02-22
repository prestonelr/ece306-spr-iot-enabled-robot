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
#include  "msp430.h"
#include  "Include\functions.h"
#include  "Include\display.h"
#include  "Include\ports.h"
#include  "Include\macros.h"
#include  "Include\led.h"
#include  "Include\init.h"
#include  "Include\LCD.h"
#include  "Include\motor.h"
#include  "Include\shape.h"



// Function Prototypes
void main(void);


  // Global Variables
volatile char slow_input_down;
unsigned char display_mode;

unsigned int test_value;
char chosen_direction;
char change;

unsigned int wheel_move;
char forward;



//void main(void){
void main(void){
//    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

//------------------------------------------------------------------------------
// Main Program
// This is the main routine for the program. Execution of code starts here.
// The operating system is Back Ground Fore Ground.
//
//------------------------------------------------------------------------------
  PM5CTL0 &= ~LOCKLPM5;
// Disable the GPIO power-on default high-impedance mode to activate
// previously configured port settings

  //ShapeUI_Init();
  wheel_move = 0;
  forward = TRUE;


  SMCLK_CONFIG = USE_SMCLK;
  Init_All();


//------------------------------------------------------------------------------
// Begining of the "While" Operating System
//------------------------------------------------------------------------------
  while(ALWAYS) {                      // Can the Operating system run
    Carlson_StateMachine();            // Run a Time Based State Machine
    //Switches_Process();                // Check for switch state change
    //ShapeUI_Task();
    Display_Process();                 // Update Display
    P3OUT ^= TEST_PROBE;               // Change State of TEST_PROBE OFF


    if ((P4IN & SW1) == 0)   // button pressed (active low)
    {
        SMCLK_CONFIG ^= 1;   // toggle between 0x00 and 0x01
        Init_All();          // reinitialize with new mode
    }
  }
//------------------------------------------------------------------------------

}




