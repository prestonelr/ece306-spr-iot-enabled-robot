//===========================================================================
// File Name : display.c
//
// Description: This file handles the display function connections
//
// Author: Preston Elrod
// Date: Feb 2026
// Compiler: Built with Code Composer Version: CCS12.4.0.00007_win64
//===========================================================================

void Init_Display_Conditions(void); //init display to default state
void Display_Process(void); // updating the display
void ChangeDisplay(char *text, int row); // changes the display_changed variable to x.
void ChangeBacklight(char mode);
void UpdateDisplay(void); // sets update_display to 1
