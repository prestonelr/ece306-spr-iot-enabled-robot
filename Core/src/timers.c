#include "Core\lib\timers.h"
#include "Core\lib\macros.h"
#include "msp430.h"

// SMCLK = 8MHz
// Divider = /8 (ID__8) * /8 (TBIDEX__8) = /64
// TB0 clock = 8,000,000 / 64 = 125,000 Hz
// 200ms = 0.2 * 125,000 = 25,000
#define TB0CCR0_INTERVAL   (25000u)

// 20ms debounce tick = 0.02 * 125,000 = 2,500
#define TB0CCR1_INTERVAL   (2500u)

void Init_Timer_B0(void)
{
  TB0CTL  = RESET_REGISTER;
  TB0EX0  = RESET_REGISTER;

  TB0CTL |= TBSSEL__SMCLK;       // SMCLK source (8MHz)
  TB0CTL |= MC__CONTINUOUS;      // Continuous mode
  TB0CTL |= ID__8;               // /8
  TB0EX0 |= TBIDEX__8;           // /8  (overall /64)

  TB0CTL |= TBCLR;               // Clear timer

  // CCR0: 200ms tick
  TB0CCR0   = TB0R + TB0CCR0_INTERVAL;
  TB0CCTL0  = CCIE;

  // CCR1: debounce tick (disabled until needed)
  TB0CCR1   = TB0R + TB0CCR1_INTERVAL;
  TB0CCTL1  = 0;

  TB0CTL &= ~TBIE;
  TB0CTL &= ~TBIFG;
}
