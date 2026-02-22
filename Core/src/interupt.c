#include "Core\lib\interupt.h"
#include "Core\lib\display.h"
#include "Core\lib\ports.h"
#include "Core\lib\timers.h"
#include "msp430.h"

// ------------------- display timing flags (must exist somewhere) -------------
extern volatile unsigned char update_display;     // set in timer ISR
extern volatile unsigned char display_changed;    // set when text changes

// ------------------- switch debounce globals --------------------------------
volatile unsigned char sw1_pressed     = 0;
volatile unsigned char sw1_debouncing  = 0;
volatile unsigned int  sw1_db_count    = 0;

// ------------------- legacy timing globals ----------------------------------
volatile unsigned int Time_Sequence = 0;
volatile char one_time = 0;

// ------------------- backlight control --------------------------------------
static volatile unsigned char backlight_enabled = 1;
static volatile unsigned char backlight_state   = 0;

// Timer intervals MUST match timers.c (SMCLK/64)
#define TB0CCR0_INTERVAL    (25000u)  // 200ms
#define TB0CCR1_INTERVAL    (2500u)   // 20ms
#define SW1_DEBOUNCE_TICKS  (50u)     // 50 * 20ms = 1000ms

//==============================================================================
// PORT4 ISR (SW1)
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

    // Stop blinking + force backlight OFF during debounce
    backlight_enabled = 0;
    backlight_state   = 0;
    ChangeBacklight(0);

    // IMPORTANT: Do NOT disable CCR0 entirely if you need display timing.
    // We keep CCR0 running for update_display timing; only gate backlight.
    // TB0CCTL0 stays enabled.

    // Enable CCR1 debounce tick
    TB0CCR1   = TB0R + TB0CCR1_INTERVAL;
    TB0CCTL1 &= ~CCIFG;
    TB0CCTL1 |= CCIE;
  }
}

//==============================================================================
// TIMER0_B0_VECTOR (CCR0) - 200ms system tick
// - sets update_display flag (Display_Process uses it)
// - toggles backlight (fast GPIO) only when allowed
// - updates legacy one_time / Time_Sequence
//==============================================================================
#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer0_B0_ISR(void)
{
  // Display scheduling flag (NO SPI HERE)
  update_display = 1;

  // Backlight blinking (fast, safe) unless debouncing
  if (backlight_enabled)
  {
    backlight_state ^= 1;
    ChangeBacklight(backlight_state);
  }

  // Legacy timing flags (if other code depends on them)
  one_time = 1;
  if (Time_Sequence++ > 250) Time_Sequence = 0;

  // schedule next 200ms
  TB0CCR0 += TB0CCR0_INTERVAL;
}

//==============================================================================
// TIMER0_B1_VECTOR (CCR1/CCR2/overflow) - debounce tick
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

          // Resume backlight blinking
          backlight_enabled = 1;
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
