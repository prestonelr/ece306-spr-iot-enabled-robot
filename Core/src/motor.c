//===========================================================================
// File Name : motor.c
//
// Description: Contains the driver to alter how motors work
//
// Author: Preston Elrod
// Date: Feb 2026
// Compiler: Built with Code Composer Version: CCS12.4.0.00007_win64
//===========================================================================



#include "Core\lib\motor.h"
#include "Core\lib\ports.h"
#include  "msp430.h"





void TurnOffMotors(void) // disable all motors
{
P6OUT &= ~L_FORWARD;
P6OUT &= ~R_FORWARD;
P6OUT &= ~L_REVERSE;
P6OUT &= ~R_REVERSE;
}

// safe direction changer to prevent altering the state of the motors without going to an intermediate off state.
void ChangeFullDirection(int dir)
{
    switch(dir)
    {
    case 1: //forward
        TurnOffMotors();
        P6OUT |= L_FORWARD;
        P6OUT |= R_FORWARD;
    break;
    case 0: // off
        TurnOffMotors();
    break;
    case -1: //reverse
        TurnOffMotors();
        P6OUT |= L_REVERSE;
        P6OUT |= R_REVERSE;
    break;
    default:
        TurnOffMotors();
    break;
    }
}

void ChangeWheelDirection(int wheel, int dir)
{
    //dynamic wheel variables
    int wheelForward;
    int wheelReverse;


    if (wheel == 1)  //right
    {
        wheelForward = R_FORWARD;
        wheelReverse = R_REVERSE;

    } else if (wheel == -1) //left
    {
        wheelForward = L_FORWARD;
        wheelReverse = L_REVERSE;
    } else
        return; //invalid input


    switch (dir)
    {
    case motor_forward:
        P6OUT &= ~wheelReverse; //disable reverse for safety
        P6OUT |= wheelForward; // turn on forward
        break;
    case motor_off:
        P6OUT &= ~wheelReverse; //disable reverse
        P6OUT &= ~wheelForward; // disable forward
        break;
    case motor_reverse:
        P6OUT &= ~wheelForward; //disable forward for safety
        P6OUT |= wheelReverse; // turn on forward
        break;
    }
}


/*
void Physics_Tick(char cmd)
{
    // ===== Calibration constants =====
    static const float acceleration   = 7.62f;   // m/s²
    static const float speedIncrement = 0.254f;  // m/s
    static const float mass           = 0.1f;    // kg
    static const float dt             = 1.0f;    // seconds per tick
    static const float shiftStep      = 0.05f;

    // ===== Persistent state =====
    static float speed       = 0.0f;   // current velocity (m/s)
    static float maxSpeed    = 0.254f; // speed cap
    static float shift       = 0.0f;   // -1 .. 1
    static float momentum    = 0.0f;

    // ===== Command processing =====
    switch (cmd)
    {
        // Steering
        case '/':  shift += shiftStep; break;
        case '\\': shift -= shiftStep; break;
        case '>':  shift = 1.0f; break;
        case '<':  shift = -1.0f; break;

        // Motion
        case 'F':
            speed += acceleration * dt;
            break;

        case 'R':
            speed -= acceleration * dt;
            break;

        case '~':
            speed = 0.0f;
            break;

        // Speed control
        case '+':
            maxSpeed += speedIncrement;
            break;

        case '-':
            maxSpeed -= speedIncrement;
            if (maxSpeed < 0.0f) maxSpeed = 0.0f;
            break;

        // Hold / idle — intentionally do nothing
        case 'H':
            break;

        default:
            break;
    }

    // ===== Clamp steering =====
    if (shift > 1.0f)  shift = 1.0f;
    if (shift < -1.0f) shift = -1.0f;

    // ===== Clamp speed to cap =====
    if (speed >  maxSpeed) speed =  maxSpeed;
    if (speed < -maxSpeed) speed = -maxSpeed;

    // ===== Physics =====
    momentum = mass * speed;
}*/


// Rough, non-physics character controls.
// Uses ONLY:
//   - ChangeFullDirection(dir) where dir:  1=fwd, 0=off, -1=rev
//   - ChangeWheelDirection(wheel, dir) where wheel: 1=right, -1=left
// and dir is one of: motor_forward, motor_off, motor_reverse
//
// Mapping:
//   F : full forward
//   R : full reverse
//   ~ : stop
//   / : arc right (left forward, right off)
//   \ : arc left  (right forward, left off)
//   > : pivot right (left forward, right reverse)
//   < : pivot left  (left reverse, right forward)
//   H : hold last state (do nothing)

// Tick-based wheel control (on/off only).
// No reverse support; anything needing reverse is approximated.
void WheelCommand_Tick(char cmd, unsigned char dutyPhase)
{
    unsigned char onL = 0;
    unsigned char onR = 0;

    switch(cmd)
    {
        case '~':   // HARD STOP
            ChangeWheelDirection(motor_left,  motor_off);
            ChangeWheelDirection(motor_right, motor_off);
            return;

        case 'F':   // forward
            onL = DutyOn(dutyPhase, DUTY_FWD_ON);
            onR = DutyOn(dutyPhase, DUTY_FWD_ON);

            ChangeWheelDirection(motor_left,  onL ? motor_forward : motor_off);
            ChangeWheelDirection(motor_right, onR ? motor_forward : motor_off);
            return;

        case '/':   // arc right
            // left = outer (fast), right = inner (slow)
            onL = DutyOn(dutyPhase, DUTY_ARC_OUT_ON);
            onR = DutyOn(dutyPhase, DUTY_ARC_IN_ON);

            ChangeWheelDirection(motor_left,  onL ? motor_forward : motor_off);
            ChangeWheelDirection(motor_right, onR ? motor_forward : motor_off);
            return;

        case '\\':  // arc left
            onL = DutyOn(dutyPhase, DUTY_ARC_IN_ON);
            onR = DutyOn(dutyPhase, DUTY_ARC_OUT_ON);

            ChangeWheelDirection(motor_left,  onL ? motor_forward : motor_off);
            ChangeWheelDirection(motor_right, onR ? motor_forward : motor_off);
            return;

        case '>':   // hard right pivot
            onL = DutyOn(dutyPhase, DUTY_PIVOT_ON);

            ChangeWheelDirection(motor_left,  onL ? motor_forward : motor_off);
            ChangeWheelDirection(motor_right, motor_off);
            return;

        case '<':   // hard left pivot
            onR = DutyOn(dutyPhase, DUTY_PIVOT_ON);

            ChangeWheelDirection(motor_left,  motor_off);
            ChangeWheelDirection(motor_right, onR ? motor_forward : motor_off);
            return;

        default:
            ChangeWheelDirection(motor_left,  motor_off);
            ChangeWheelDirection(motor_right, motor_off);
            return;
    }
}




int CommandCoordinator(const char *newCmdString)
{
    static const char *cmdPtr = 0;
    static unsigned int index = 0;

    static char currentCmd = 0;
    static unsigned int stepsLeft = 0;

    static unsigned char dutyPhase = 0;

    static unsigned int tickDiv = 0;

    if (newCmdString != 0)
    {
        cmdPtr = newCmdString;
        index = 0;
        currentCmd = 0;
        stepsLeft = 0;
        dutyPhase = 0;
        tickDiv = 0;
        return 0;
    }

    if (cmdPtr == 0) return 0;

    // tick gate
    tickDiv++;
    if (tickDiv < TICKS_PER_STEP) return 0;
    tickDiv = 0;

    // fetch next command
    if (currentCmd == 0)
    {
        char c = cmdPtr[index];
        if (c == '\0')
        {
            ChangeWheelDirection(motor_left,  motor_off);
            ChangeWheelDirection(motor_right, motor_off);
            return 1;
        }

        currentCmd = c;
        dutyPhase = 0;

        switch(currentCmd)
        {
            case 'F':  stepsLeft = 8;  break;   // ~1 inch target
            case '/':  stepsLeft = 6;  break;   // ~10° arc chunk
            case '\\': stepsLeft = 6;  break;
            case '>':  stepsLeft = 5;  break;   // ~30° pivot chunk
            case '<':  stepsLeft = 5;  break;
            case '~':  stepsLeft = 1;  break;
            default:   stepsLeft = 1;  break;
        }
    }

    WheelCommand_Tick(currentCmd, dutyPhase);

    dutyPhase++;
    if (dutyPhase >= DUTY_PERIOD) dutyPhase = 0;

    if (--stepsLeft == 0)
    {
        currentCmd = 0;
        index++;
    }

    return 0;
}




