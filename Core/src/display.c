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
#include "Core\lib\functions.h"
#include "msp430.h"
#include <string.h>
#include "Core\lib\LCD.h"
#include "Core\lib\ports.h"

extern char display_line[4][11];
extern char *display[4];
extern volatile unsigned char display_changed;
extern volatile unsigned char update_display;

//-----------------------------------------------------------------------------
//  Reset_LCD()
//  Performs hardware reset of LCD using P4.0 (RESET_LCD)
//-----------------------------------------------------------------------------
void Reset_LCD(void)
{
  P4OUT &= ~RESET_LCD;      // Drive RESET low
  __delay_cycles(80000);    // ~10ms @ 8MHz
  P4OUT |= RESET_LCD;       // Drive RESET high
  __delay_cycles(80000);    // allow LCD controller to stabilize
}

void Display_Process(void)
{
  if(update_display)
  {
    update_display = 0;

    if(display_changed)
    {
      display_changed = 0;
      Display_Update(0,0,0,0);
    }
  }
}

void Init_Display_Conditions(void)
{
  int row, i;

  for(row = 0; row < 4; row++)
  {
    for(i = 0; i < 10; i++)
      display_line[row][i] = ' ';   // always pad with spaces

    display_line[row][10] = 0;      // always null terminate
  }

  display[0] = &display_line[0][0];
  display[1] = &display_line[1][0];
  display[2] = &display_line[2][0];
  display[3] = &display_line[3][0];

  update_display = 0;
  display_changed = TRUE;
}

//-----------------------------------------------------------------------------
// ChangeDisplay
// Copies up to 10 chars, pads the rest with spaces, always null-terminates.
// This prevents stale characters / garbage on the LCD.
//-----------------------------------------------------------------------------
void ChangeDisplay(char *text, int row)
{
  int i = 0;

  // copy characters until end or 10 chars
  while(i < 10 && text[i] != 0)
  {
    display_line[row][i] = text[i];
    i++;
  }

  // pad remainder with spaces
  while(i < 10)
  {
    display_line[row][i] = ' ';
    i++;
  }

  display_line[row][10] = 0;
  display_changed = 1;
}

//-----------------------------------------------------------------------------
//  ChangeBacklight()
//-----------------------------------------------------------------------------
void ChangeBacklight(char mode)
{
  if(mode)  P6OUT |=  LCD_BACKLITE;
  else      P6OUT &= ~LCD_BACKLITE;
}

void UpdateDisplay(void)
{
  update_display = 1;
}
