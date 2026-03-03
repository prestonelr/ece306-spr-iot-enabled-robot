//===========================================================================
// File Name : line.c
//
// Project-specific:
//   SW1 #1: capture WHITE (motors OFF)
//   SW1 #2: capture BLACK (motors OFF)
//   SW1 #3: START (then motors allowed)
//
// Run:
//   1) Forward until black detected (either sensor)
//   2) Reverse slowly until it leaves line (both white) then hits black again
//   3) Tank-turn FAST until the line is LOST (both sensors white for a few hits)
//   4) Then tank-turn SLOW in the OPPOSITE direction to come back on top
//      of the line (BOTH sensors black)
//   5) Stop
//
// Requirements kept:
//   - Real-time motor control (call Line_Task fast)
//   - Tank turning (one wheel fwd, one wheel rev)
//   - 50ms dead-time anytime a wheel reverses (forward<->reverse)
//   - No new timers: use TB0R as timebase
//   - ADC values displayed at all times
//   - Uses motor.h (no local motor_* defines)
//===========================================================================

#include <msp430.h>

#include "Core\lib\display.h"
#include "Core\lib\adc.h"
#include "Core\lib\ports.h"
#include "Core\lib\motor.h"
#include "Addon\lib\line.h"

// switch debounce globals
extern volatile unsigned char sw1_pressed;
extern volatile unsigned char update_display;

// -------------------- TB0 timing -------------------------------------------
// TB0 tick = 8us => 125 ticks per ms (based on 2500 ticks ~ 20ms)
#define TB0_TICKS_PER_MS   (125u)
#define DEADTIME_MS        (50u)
#define DEADTIME_TICKS     (DEADTIME_MS * TB0_TICKS_PER_MS)

// Software PWM using TB0R phase (no extra timers)
#define PWM_PERIOD_TICKS   (2500u)   // ~20ms
#define REV_SLOW_DUTY_PCT  (30u)

// Turn behavior
#define TURN_FAST_DUTY_PCT (70u)     // spin to lose the line
#define TURN_SLOW_DUTY_PCT (35u)     // creep back onto the line

#define STABLE_HITS        (3u)

// -------------------- Emitter ----------------------------------------------
static void Emitter_On(void)
{
  P2DIR |= IR_LED;
  P2OUT |= IR_LED;
}

static const char* EmitterText(void)
{
  return "EMIT  ON ";
}

// -------------------- Display helpers (always show ADC) --------------------
static char g_lineL[11];
static char g_lineR[11];

static char* MakeLine(const char label, unsigned v, char* buf)
{
  int i;

  for(i=0;i<10;i++) buf[i] = ' ';
  buf[10] = 0;

  buf[0] = label;
  buf[1] = ':';

  i = 9;
  if(v == 0u) { buf[i] = '0'; return buf; }

  while((v > 0u) && (i >= 2))
  {
    buf[i] = (char)('0' + (v % 10u));
    v /= 10u;
    i--;
  }
  return buf;
}

static void DisplayAlways(unsigned L, unsigned R)
{
  ChangeDisplay(EmitterText(),             0);
  ChangeDisplay(MakeLine('L', L, g_lineL), 1);
  ChangeDisplay(MakeLine('R', R, g_lineR), 2);
}

// -------------------- TB0R wait (wrap safe) --------------------------------
static void Wait_TB0Ticks(unsigned ticks)
{
  unsigned start = (unsigned)TB0R;
  while((unsigned)(TB0R - start) < ticks) { ; }
}

// -------------------- Safe motor set w/ dead-time on reversal --------------
static int last_dir_L = motor_off;
static int last_dir_R = motor_off;

static void SafeSetWheel(int wheel, int desired_dir)
{
  int *plast;
  int last;

  plast = (wheel == motor_left) ? &last_dir_L : &last_dir_R;
  last  = *plast;

  if( (last == motor_forward && desired_dir == motor_reverse) ||
      (last == motor_reverse && desired_dir == motor_forward) )
  {
    ChangeWheelDirection(wheel, motor_off);
    Wait_TB0Ticks(DEADTIME_TICKS);
  }

  ChangeWheelDirection(wheel, desired_dir);
  *plast = desired_dir;
}

static void DriveLR(int left_dir, int right_dir)
{
  SafeSetWheel(motor_left,  left_dir);
  SafeSetWheel(motor_right, right_dir);
}

static void StopAll_Hard(void)
{
  ChangeWheelDirection(motor_left,  motor_off);
  ChangeWheelDirection(motor_right, motor_off);
  last_dir_L = motor_off;
  last_dir_R = motor_off;
}

static void DriveLR_PWM(int left_dir, int right_dir, unsigned duty_pct)
{
  unsigned on_ticks;
  unsigned phase;

  if(duty_pct >= 100u) { DriveLR(left_dir, right_dir); return; }
  if(duty_pct == 0u)   { DriveLR(motor_off, motor_off); return; }

  on_ticks = (unsigned)((PWM_PERIOD_TICKS * duty_pct) / 100u);
  phase = (unsigned)((unsigned)TB0R % PWM_PERIOD_TICKS);

  if(phase < on_ticks) DriveLR(left_dir, right_dir);
  else                 DriveLR(motor_off, motor_off);
}

// -------------------- Calibration ------------------------------------------
static unsigned whiteL = 0, whiteR = 0;
static unsigned blackL = 0, blackR = 0;
static unsigned char have_white = 0;
static unsigned char have_black = 0;

static unsigned MakeMidThresh(unsigned w, unsigned b)
{
  long t = (long)w + ((long)b - (long)w) / 2L;
  if(t < 0) t = 0;
  if(t > 65535) t = 65535;
  return (unsigned)t;
}

static unsigned IsBlack(unsigned now, unsigned w, unsigned b)
{
  unsigned thr = MakeMidThresh(w, b);
  if(b >= w) return (now >= thr);
  else       return (now <= thr);
}

// -------------------- States ------------------------------------------------
typedef enum
{
  ST_CAL_WHITE = 0,
  ST_CAL_BLACK,
  ST_CAL_READY,

  ST_RUN_FWD,
  ST_RUN_REV,

  ST_TURN_LOSE,     // fast tank-turn until line is LOST (both white stable)
  ST_TURN_CORRECT,  // slow tank-turn in OPPOSITE direction until BOTH black

  ST_DONE
} state_t;

static state_t st = ST_CAL_WHITE;

static unsigned stable_count = 0;
static unsigned char rev_seen_white = 0;

// 0=turn left, 1=turn right
static unsigned char turn_dir = 0;

// correction direction is opposite of turn_dir
static unsigned char corr_dir = 0;

// -------------------- Init --------------------------------------------------
void Line_Init(void)
{
  sw1_pressed = 0;

  StopAll_Hard();

  have_white = 0;
  have_black = 0;
  st = ST_CAL_WHITE;

  stable_count = 0;
  rev_seen_white = 0;
  turn_dir = 0;
  corr_dir = 0;

  Emitter_On();

  ChangeDisplay("LINE ROUTE", 0);
  ChangeDisplay(" ",          1);
  ChangeDisplay(" ",          2);
  ChangeDisplay("WHITE->SW1", 3);
}

// -------------------- Task (call fast) -------------------------------------
void Line_Task(void)
{
  unsigned L, R;
  unsigned bL, bR;
  unsigned any_black, both_black, both_white;

  L = ADC_ReadChannel(ADC_CH_LEFT);
  R = ADC_ReadChannel(ADC_CH_RIGHT);

  // ADC values MUST be displayed at all times
  DisplayAlways(L, R);

  if(update_display)
  {
    update_display = 0;

    if(st == ST_CAL_WHITE)        ChangeDisplay("WHITE->SW1", 3);
    else if(st == ST_CAL_BLACK)   ChangeDisplay("BLACK->SW1", 3);
    else if(st == ST_CAL_READY)   ChangeDisplay("SW1 START ", 3);
    else if(st == ST_RUN_FWD)     ChangeDisplay("FWD->BLACK", 3);
    else if(st == ST_RUN_REV)     ChangeDisplay("REV->BLACK", 3);
    else if(st == ST_TURN_LOSE)   ChangeDisplay("TURN: LOSE", 3);
    else if(st == ST_TURN_CORRECT)ChangeDisplay("TURN: FIX ", 3);
    else                          ChangeDisplay("DONE      ", 3);
  }

  // -------------------- CALIBRATION GATE: motors NEVER move here ------------
  if(st == ST_CAL_WHITE || st == ST_CAL_BLACK || st == ST_CAL_READY)
  {
    StopAll_Hard();

    if(sw1_pressed)
    {
      sw1_pressed = 0;

      if(st == ST_CAL_WHITE)
      {
        whiteL = L; whiteR = R;
        have_white = 1;
        st = ST_CAL_BLACK;
      }
      else if(st == ST_CAL_BLACK)
      {
        blackL = L; blackR = R;
        have_black = 1;
        st = ST_CAL_READY;
      }
      else
      {
        if(have_white && have_black)
        {
          st = ST_RUN_FWD;
          stable_count = 0;
          rev_seen_white = 0;
          turn_dir = 0;
          corr_dir = 0;
        }
        else
        {
          have_white = 0;
          have_black = 0;
          st = ST_CAL_WHITE;
        }
      }
    }
    return;
  }
  // -------------------------------------------------------------------------

  // Compute black flags (RUN only)
  bL = IsBlack(L, whiteL, blackL);
  bR = IsBlack(R, whiteR, blackR);

  any_black  = (unsigned)(bL || bR);
  both_black = (unsigned)(bL && bR);
  both_white = (unsigned)((!bL) && (!bR));

  // -------------------- Forward until hit black -----------------------------
  if(st == ST_RUN_FWD)
  {
    DriveLR(motor_forward, motor_forward);

    if(any_black)
    {
      stable_count++;
      if(stable_count >= STABLE_HITS)
      {
        st = ST_RUN_REV;
        stable_count = 0;
        rev_seen_white = 0;
      }
    }
    else stable_count = 0;

    return;
  }

  // -------------------- Reverse slowly until hit black again ----------------
  if(st == ST_RUN_REV)
  {
    DriveLR_PWM(motor_reverse, motor_reverse, REV_SLOW_DUTY_PCT);

    // Must leave line first
    if(both_white)
    {
      rev_seen_white = 1;
      stable_count = 0;
    }

    if(rev_seen_white && any_black)
    {
      stable_count++;
      if(stable_count >= STABLE_HITS)
      {
        // Choose turn direction based on which sensor sees black now
        if(bL && !bR) turn_dir = 1;
        else if(bR && !bL) turn_dir = 0;

        // Correction will be opposite direction
        corr_dir = (unsigned char)(turn_dir ? 0 : 1);

        // NEXT: spin until we LOSE the line
        st = ST_TURN_LOSE;
        stable_count = 0;
      }
    }
    else
    {
      if(!any_black) stable_count = 0;
    }

    return;
  }

  // -------------------- Turn FAST until we LOSE the line --------------------
  if(st == ST_TURN_LOSE)
  {
    // Turn in turn_dir until BOTH sensors read white stably
    if(turn_dir)
    {
      // RIGHT
      DriveLR_PWM(motor_forward, motor_reverse, TURN_FAST_DUTY_PCT);
    }
    else
    {
      // LEFT
      DriveLR_PWM(motor_reverse, motor_forward, TURN_FAST_DUTY_PCT);
    }

    if(both_white)
    {
      stable_count++;
      if(stable_count >= STABLE_HITS)
      {
        // Now go BACK slowly in the OPPOSITE direction
        st = ST_TURN_CORRECT;
        stable_count = 0;
      }
    }
    else stable_count = 0;

    return;
  }

  // -------------------- Turn SLOW in OPPOSITE direction until BOTH black ----
  if(st == ST_TURN_CORRECT)
  {
    if(corr_dir)
    {
      // RIGHT
      DriveLR_PWM(motor_forward, motor_reverse, TURN_SLOW_DUTY_PCT);
    }
    else
    {
      // LEFT
      DriveLR_PWM(motor_reverse, motor_forward, TURN_SLOW_DUTY_PCT);
    }

    if(both_black)
    {
      stable_count++;
      if(stable_count >= STABLE_HITS)
      {
        StopAll_Hard();
        st = ST_DONE;
      }
    }
    else stable_count = 0;

    return;
  }

  // -------------------- Done ------------------------------------------------
  StopAll_Hard();
}
