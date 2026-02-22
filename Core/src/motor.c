//===========================================================================
// File Name : motor.c
//
// WheelCommand_Tick: immediate execution of ONE movement command.
// CommandCoordinator: runs a string of (cmd + decimal tickcount) entries.
//===========================================================================

#include "Core\lib\motor.h"
#include "Core\lib\ports.h"
#include "msp430.h"

//===========================================================================
// BASIC MOTOR CONTROL (keep your existing versions)
//===========================================================================

void TurnOffMotors(void)
{
    P6OUT &= ~L_FORWARD;
    P6OUT &= ~R_FORWARD;
    P6OUT &= ~L_REVERSE;
    P6OUT &= ~R_REVERSE;
}

void ChangeFullDirection(int dir)
{
    switch(dir)
    {
        case  1: TurnOffMotors(); P6OUT |= L_FORWARD; P6OUT |= R_FORWARD; break;
        case  0: TurnOffMotors(); break;
        case -1: TurnOffMotors(); P6OUT |= L_REVERSE; P6OUT |= R_REVERSE; break;
        default: TurnOffMotors(); break;
    }
}

void ChangeWheelDirection(int wheel, int dir)
{
    int wheelForward;
    int wheelReverse;

    if (wheel == motor_right)
    {
        wheelForward = R_FORWARD;
        wheelReverse = R_REVERSE;
    }
    else if (wheel == motor_left)
    {
        wheelForward = L_FORWARD;
        wheelReverse = L_REVERSE;
    }
    else
    {
        return;
    }

    switch(dir)
    {
        case motor_forward:
            P6OUT &= ~wheelReverse;
            P6OUT |=  wheelForward;
            break;

        case motor_off:
            P6OUT &= ~wheelReverse;
            P6OUT &= ~wheelForward;
            break;

        case motor_reverse:
            P6OUT &= ~wheelForward;
            P6OUT |=  wheelReverse;
            break;
    }
}

//===========================================================================
// TOOL 1: WheelCommand_Tick (IMMEDIATE, no timing)
// Keep abstraction to movement commands.
// Commands:
//   F  : forward
//   R  : reverse
//   ~  : stop
//   /  : arc right   (simple: left fwd, right off)
//   \  : arc left    (simple: right fwd, left off)
//   >  : spin CW     (left fwd, right rev)
//   <  : spin CCW    (left rev, right fwd)
//===========================================================================

void WheelCommand_Tick(char cmd)
{
    switch(cmd)
    {
        case '~':
            ChangeWheelDirection(motor_left,  motor_off);
            ChangeWheelDirection(motor_right, motor_off);
            break;

        case 'F':
            ChangeWheelDirection(motor_left,  motor_forward);
            ChangeWheelDirection(motor_right, motor_forward);
            break;

        case 'R':
            ChangeWheelDirection(motor_left,  motor_reverse);
            ChangeWheelDirection(motor_right, motor_reverse);
            break;

        case '/':   // arc right (simple)
            ChangeWheelDirection(motor_left,  motor_forward);
            ChangeWheelDirection(motor_right, motor_off);
            break;

        case '\\':  // arc left (simple)
            ChangeWheelDirection(motor_left,  motor_off);
            ChangeWheelDirection(motor_right, motor_forward);
            break;

        case '>':   // spin CW
            ChangeWheelDirection(motor_left,  motor_forward);
            ChangeWheelDirection(motor_right, motor_reverse);
            break;

        case '<':   // spin CCW
            ChangeWheelDirection(motor_left,  motor_reverse);
            ChangeWheelDirection(motor_right, motor_forward);
            break;

        default:
            ChangeWheelDirection(motor_left,  motor_off);
            ChangeWheelDirection(motor_right, motor_off);
            break;
    }
}

//===========================================================================
// TOOL 2: CommandCoordinator (STRING + TICKS)
// - Pass non-NULL to LOAD a new script string.
// - Pass NULL each tick to EXECUTE one tick worth.
// - Returns 1 when script ends, else 0.
//
// Script format:
//    <cmd><decimal_ticks><cmd><decimal_ticks>...
// Example:
//    "F5~5R10~5"
//
// Notes:
// - Whitespace is ignored.
// - If a cmd is missing a number, it defaults to 1 tick.
//===========================================================================

int CommandCoordinator(const char *newCmdString)
{
    static const char *p = 0;
    static char currentCmd = 0;
    static unsigned int ticksLeft = 0;

    // Load new script
    if (newCmdString != 0)
    {
        p = newCmdString;
        currentCmd = 0;
        ticksLeft = 0;
        WheelCommand_Tick('~');   // safe stop immediately on load
        return 0;
    }

    if (p == 0)
        return 0;

    // If we need a new (cmd,ticks) pair, parse it
    if (ticksLeft == 0)
    {
        // skip spaces
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
            p++;

        // end of string
        if (*p == '\0')
        {
            WheelCommand_Tick('~');
            p = 0;
            return 1;
        }

        // read command char
        currentCmd = *p;
        p++;

        // skip spaces between cmd and number
        while (*p == ' ' || *p == '\t')
            p++;

        // parse decimal tick count (optional)
        ticksLeft = 0;
        while (*p >= '0' && *p <= '9')
        {
            ticksLeft = (ticksLeft * 10u) + (unsigned int)(*p - '0');
            p++;
        }

        if (ticksLeft == 0)
            ticksLeft = 1;   // default if no digits present
    }

    // Execute ONE tick immediately
    WheelCommand_Tick(currentCmd);

    // Count down
    ticksLeft--;

    return 0;
}
