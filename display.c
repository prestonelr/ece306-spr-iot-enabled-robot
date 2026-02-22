//===========================================================================
// File Name : display.c
//
// Description: This file handles the display configuration
//
// Author: Preston Elrod
// Date: Feb 2026
// Compiler: Built with Code Composer Version: CCS12.4.0.00007_win64
//===========================================================================

#include <Include/macros.h>
#include  "Include\functions.h"
#include  "msp430.h"
#include <string.h>
#include  "Include\LCD.h"


extern char display_line[4][11];
extern char *display[4];
extern volatile unsigned char display_changed;
extern volatile unsigned char update_display;
extern volatile unsigned int update_display_count;


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




