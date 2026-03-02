//===========================================================================
// File Name : line.c
//
// Description:
//   Simple LEFT-only line test (right detector broken)
//   - SW1 captures WHITE/BLACK values (OFF then ON)
//   - TA test: SW1 -> 1s delay -> forward until black -> stop -> turn to black
//
// Old-compiler friendly: no floats, no snprintf, no malloc, no uint types
//===========================================================================

#include <msp430.h>

#include "Core\lib\display.h"
#include "Core\lib\adc.h"
#include "Core\lib\ports.h"
#include "Core\lib\motor.h"
#include "Addon\lib\line.h"

// 200ms tick
extern volatile unsigned int Time_Sequence;

// switch debounce globals (provided by you)
extern volatile unsigned char sw1_pressed;
extern volatile unsigned char sw1_debouncing;
extern volatile unsigned int  sw1_db_count;

#define TICKS_1S   (5u)
#define TICKS_4S   (20u)

// -------- emitter control (IR_LED) --------
// If active-low, invert these.
static void Emitter_On(void)
{
  P2DIR |= IR_LED;
  P2OUT |= IR_LED;
}
static void Emitter_Off(void)
{
  P2DIR |= IR_LED;
  P2OUT &= ~IR_LED;
}

static const char* EmitterText(unsigned char on)
{
  return on ? "EMIT  ON " : "EMIT OFF ";
}

// -------- tiny number line "L:   ####" --------
static char g_line[11];

static char* MakeLeftLine(unsigned v)
{
  int i;

  for(i=0;i<10;i++) g_line[i] = ' ';
  g_line[10] = 0;

  g_line[0] = 'L';
  g_line[1] = ':';

  i = 9;
  if(v == 0u)
  {
    g_line[i] = '0';
    return g_line;
  }

  while((v > 0u) && (i >= 2))
  {
    g_line[i] = (char)('0' + (v % 10u));
    v /= 10u;
    i--;
  }

  return g_line;
}

// -------- stored capture values (LEFT only) --------
static unsigned white_off = 0;
static unsigned black_off = 0;
static unsigned white_on  = 0;
static unsigned black_on  = 0;

static unsigned char have_white_off = 0;
static unsigned char have_black_off = 0;
static unsigned char have_white_on  = 0;
static unsigned char have_black_on  = 0;

// threshold between stored white and black
// thr = white + (black-white)*3/5  (60% toward black)
static unsigned MakeThresh(unsigned white, unsigned black)
{
  long dw;
  long t;

  dw = (long)black - (long)white;
  t  = (long)white + (dw * 3L) / 5L;

  if(t < 0) t = 0;
  if(t > 65535) t = 65535;
  return (unsigned)t;
}

static unsigned IsBlack_Left(unsigned left_now)
{
  unsigned thr;

  // Use emitter ON captures for TA run
  if(!have_white_on || !have_black_on) return 0u;

  thr = MakeThresh(white_on, black_on);

  // If black_on < white_on (reversed), the threshold logic still works because dw is negative.
  // In that case, being "black" means <= thr instead of >= thr.
  if(black_on >= white_on)
    return (left_now >= thr);
  else
    return (left_now <= thr);
}

// -------- modes / steps --------
typedef enum
{
  MODE_MANUAL = 0,
  MODE_AUTO_READY,
  MODE_AUTO_DELAY,
  MODE_AUTO_FORWARD,
  MODE_AUTO_STOP,
  MODE_AUTO_TURN,
  MODE_AUTO_DONE
} mode_t;

static mode_t mode = MODE_MANUAL;
static unsigned char emitter_on_state = 0;
static unsigned char step = 0;
static unsigned mode_ticks = 0;

static void SetMode(mode_t m)
{
  mode = m;
  mode_ticks = 0;
}

void Line_Init(void)
{
  have_white_off = have_black_off = 0;
  have_white_on  = have_black_on  = 0;

  emitter_on_state = 0;
  Emitter_Off();

  step = 0;
  SetMode(MODE_MANUAL);

  ChangeDisplay("LINE LEFT", 0);
  ChangeDisplay("SW1 CAP  ", 1);
  ChangeDisplay(" ",        2);
  ChangeDisplay(" ",        3);
}

static void ManualPrompt(void)
{
  switch(step)
  {
    case 0: ChangeDisplay("OFF->WHITE", 3); break;
    case 1: ChangeDisplay("CAP WHITE ", 3); break;
    case 2: ChangeDisplay("OFF->BLACK", 3); break;
    case 3: ChangeDisplay("CAP BLACK ", 3); break;
    case 4: ChangeDisplay("EMIT ON   ", 3); break;
    case 5: ChangeDisplay("ON->WHITE ", 3); break;
    case 6: ChangeDisplay("CAP WHITE ", 3); break;
    case 7: ChangeDisplay("ON->BLACK ", 3); break;
    case 8: ChangeDisplay("CAP BLACK ", 3); break;
    case 9: ChangeDisplay("DONE      ", 3); break;
    default: ChangeDisplay("DONE      ", 3); break;
  }
}

void Line_Task(void)
{
  static unsigned int last_ts = 0;

  unsigned L;

  // 200ms tick gating
  if(Time_Sequence == last_ts) return;
  last_ts = Time_Sequence;

  // LEFT detector only
  L = ADC_ReadChannel(ADC_CH_LEFT);

  // Display emitter + left value always
  ChangeDisplay(EmitterText(emitter_on_state), 0);
  ChangeDisplay(MakeLeftLine(L),              1);
  ChangeDisplay("R: BROKEN",                  2);

  // Consume SW1
  if(sw1_pressed)
  {
    sw1_pressed = 0;

    if(mode == MODE_MANUAL)
    {
      // capture steps
      if(step == 1u) { white_off = L; have_white_off = 1; }
      if(step == 3u) { black_off = L; have_black_off = 1; }
      if(step == 4u) { emitter_on_state = 1; Emitter_On(); }
      if(step == 6u) { white_on  = L; have_white_on  = 1; }
      if(step == 8u) { black_on  = L; have_black_on  = 1; }

      step++;

      // after finishing captures, go to TA ready on next press
      if(step >= 10u)
      {
        SetMode(MODE_AUTO_READY);
      }
    }
    else if(mode == MODE_AUTO_READY)
    {
      // TA presses SW1 to start
      SetMode(MODE_AUTO_DELAY);
    }
  }

  // -------- MANUAL mode --------
  if(mode == MODE_MANUAL)
  {
    ManualPrompt();
    return;
  }

  // -------- TA modes --------
  switch(mode)
  {
    case MODE_AUTO_READY:
      WheelCommand_Tick('~');

      // ensure emitter ON for run
      if(!emitter_on_state) { emitter_on_state = 1; Emitter_On(); }

      if(!have_white_on || !have_black_on)
        ChangeDisplay("NEED CAP ", 3);
      else
        ChangeDisplay("SW1 START", 3);
      break;

    case MODE_AUTO_DELAY:
      WheelCommand_Tick('~');
      ChangeDisplay("DELAY 1s ", 3);
      mode_ticks++;
      if(mode_ticks >= TICKS_1S)
        SetMode(MODE_AUTO_FORWARD);
      break;

    case MODE_AUTO_FORWARD:
      ChangeDisplay("FWD->BLK ", 3);
      WheelCommand_Tick('F');

      if(IsBlack_Left(L))
      {
        WheelCommand_Tick('~');
        ChangeDisplay("BLACKLINE", 3);
        SetMode(MODE_AUTO_STOP);
      }
      break;

    case MODE_AUTO_STOP:
      WheelCommand_Tick('~');
      ChangeDisplay("BLACKLINE", 3);
      mode_ticks++;
      if(mode_ticks >= TICKS_4S)
        SetMode(MODE_AUTO_TURN);
      break;

    case MODE_AUTO_TURN:
      ChangeDisplay("TURN->BLK", 3);

      // Turn one direction only (simple, since right sensor is dead).
      // Swap '<' and '>' if your turn direction is opposite.
      WheelCommand_Tick('>');

      if(IsBlack_Left(L))
      {
        WheelCommand_Tick('~');
        SetMode(MODE_AUTO_DONE);
      }
      break;

    default:
      WheelCommand_Tick('~');
      ChangeDisplay("BLK VALUE", 3);
      // Line 1 already shows current left reading
      break;
  }
}
