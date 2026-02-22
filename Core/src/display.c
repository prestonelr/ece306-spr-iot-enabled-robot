//===========================================================================
// File Name : display.c
//
// Description: This file handles the display configuration
//
// Author: Preston Elrod
// Date: Feb 2026
// Compiler: Built with Code Composer Version: CCS12.4.0.00007_win64
//===========================================================================

#include <Core\lib\macros.h>
#include  "Core\lib\functions.h"
#include  "msp430.h"
#include <string.h>
#include  "Core\lib\LCD.h"
#include "Core\lib\ports.h"

extern char display_line[4][11];
extern char *display[4];
extern volatile unsigned char display_changed;
extern volatile unsigned char update_display;



void Display_Process(void){
  if(update_display){
    update_display = 0;
    if(display_changed){
      display_changed = 0;
      Display_Update(0,0,0,0);
    }
  }
}

void Init_Display_Conditions(void){
//------------------------------------------------------------------------------

  int i;
  for(i=0;i<11;i++){
    display_line[0][i] = RESET_STATE;
    display_line[1][i] = RESET_STATE;
    display_line[2][i] = RESET_STATE;
    display_line[3][i] = RESET_STATE;
  }
  display_line[0][10] = 0;
  display_line[1][10] = 0;
  display_line[2][10] = 0;
  display_line[3][10] = 0;

  display[0] = &display_line[0][0];
  display[1] = &display_line[1][0];
  display[2] = &display_line[2][0];
  display[3] = &display_line[3][0];
  update_display = 0;

//------------------------------------------------------------------------------

  //P2OUT &= ~RESET_LCD;
    // Place the contents of what you want on the display, in between the quotes
  // Limited to 10 characters per line
  strcpy(display_line[0], "   Custom   ");
  strcpy(display_line[1], " Ports.c ");
  strcpy(display_line[2], "  Ports.h  ");
  strcpy(display_line[3], "  Display.c  ");
  display_changed = TRUE;
}

void ChangeDisplay(char *text, int row)
{
    strcpy(display_line[row], text);
    display_changed = 1;
}



//-----------------------------------------------------------------------------
//  ChangeBacklight()
//  mode: 0 = OFF
//        1 = ON
//-----------------------------------------------------------------------------
void ChangeBacklight(char mode)
{
  if(mode){
      P6OUT |= LCD_BACKLITE;   // Turn ON
  }
  else{
      P6OUT &= ~LCD_BACKLITE;  // Turn OFF
  }
}

void UpdateDisplay(void)
{
    update_display = 1;
}

//-----------------------------------------------------------------------------
//  Reset_LCD()
//  Performs hardware reset of LCD using P4.0 (RESET_LCD)
//-----------------------------------------------------------------------------
void Reset_LCD(void)
{
  // Assert reset (active low on most LCDs)
  P4OUT &= ~RESET_LCD;      // Drive RESET low
  __delay_cycles(80000);    // ~10ms @ 8MHz

  // Release reset
  P4OUT |= RESET_LCD;       // Drive RESET high
  __delay_cycles(80000);    // allow LCD controller to stabilize
}
