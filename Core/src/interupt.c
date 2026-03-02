//==============================================================================
// File Name : interrupt.c
//
// Description:
//  - TB0 CCR0 @ 200ms: sets update_display flag + blinks backlight if latched ON
//  - SW1 (P4.1) PORT4 ISR: toggles (latches) blinking ON/OFF, starts debounce
//  - TB0 CCR1 @ 20ms: debounce timer; re-enables SW1 after ~1s
//
// IMPORTANT:
//  - Do NOT call Display_Update() / SPI / LCD updates inside ISRs.
//  - ISRs only set flags and do fast GPIO (backlight).
//==============================================================================

#include "msp430.h"
#include "Core\lib\interupt.h"
#include "Core\lib\display.h"
#include "Core\lib\ports.h"
#include "Core\lib\timers.h"

// ------------------- display timing flags (must exist somewhere) -------------
extern volatile unsigned char update_display;     // set in CCR0 ISR
extern volatile unsigned char display_changed;    // set when text changes (elsewhere)

// ------------------- switch debounce globals --------------------------------
volatile unsigned char sw1_pressed     = 0;
volatile unsigned char sw1_debouncing  = 0;
volatile unsigned int  sw1_db_count    = 0;

// ------------------- legacy timing globals ----------------------------------
volatile unsigned int Time_Sequence = 0;
volatile char one_time = 0;

// ------------------- backlight control (LATCHED TOGGLE) ---------------------
// blink_latched: user mode toggled by SW1
// backlight_state: actual output state (0/1)
static volatile unsigned char blink_latched   = 1;  // 1=blink enabled, 0=latched OFF
static volatile unsigned char backlight_state = 0;

// Timer intervals MUST match timers.c (SMCLK/64)
#define TB0CCR0_INTERVAL    (25000u)  // 200ms
#define TB0CCR1_INTERVAL    (2500u)   // 20ms
#define SW1_DEBOUNCE_TICKS  (50u)     // 50 * 20ms = 1000ms

//==============================================================================
// PORT4 ISR (SW1) - toggles blinking latch and starts debounce
//==============================================================================
#pragma vector=PORT4_VECTOR
__interrupt void switch_interrupt(void)
{
  if (P4IFG & SW1)
  {
    P4IFG &= ~SW1;                // clear IFG for SW1

    sw1_pressed    = 1;
    sw1_debouncing = 1;
    sw1_db_count   = 0;

    P4IE &= ~SW1;                 // disable SW1 interrupt during debounce

    TB0CCR1   = TB0R + TB0CCR1_INTERVAL;
    TB0CCTL1 &= ~CCIFG;
    TB0CCTL1 |= CCIE;
  }
}

//==============================================================================
// TIMER0_B0_VECTOR (CCR0) - 200ms system tick
// - sets update_display flag (Display_Process uses it in main)
// - toggles backlight only if latched ON and not currently debouncing
//==============================================================================
#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer0_B0_ISR(void)
{
  // Display scheduling flag (NO SPI HERE)
  update_display = 1;

  // Legacy timing flags (if other code depends on them)
  one_time = 1;
  if (Time_Sequence++ > 250) Time_Sequence = 0;

  // schedule next 200ms
  TB0CCR0 += TB0CCR0_INTERVAL;
}

//==============================================================================
// TIMER0_B1_VECTOR (CCR1/CCR2/overflow) - debounce tick
// - runs every 20ms while enabled
// - after ~1s, re-enables SW1 interrupt and stops CCR1
// - IMPORTANT: does NOT force blinking back ON; respects blink_latched
//==============================================================================
#pragma vector = TIMER0_B1_VECTOR
__interrupt void Timer0_B1_ISR(void)
{
  switch(__even_in_range(TB0IV, 14))
  {
    case 2: // CCR1
      if (sw1_debouncing)
      {
        sw1_db_count++;

        if (sw1_db_count >= SW1_DEBOUNCE_TICKS)
        {
          sw1_debouncing = 0;
          sw1_db_count   = 0;

          // Re-enable SW1 interrupt
          P4IFG &= ~SW1;
          P4IE  |= SW1;

          // Stop debounce tick
          TB0CCTL1 &= ~CCIE;

        }
        else
        {
          TB0CCR1 += TB0CCR1_INTERVAL;
        }
      }
      else
      {
        TB0CCTL1 &= ~CCIE;
      }
      break;

    default:
      break;
  }
}
