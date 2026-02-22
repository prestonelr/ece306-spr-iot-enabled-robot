

//===========================================================================
// File Name : motor.h
//
// Description: Contains the driver to alter how motors work
//
// Author: Preston Elrod
// Date: Feb 2026
// Compiler: Built with Code Composer Version: CCS12.4.0.00007_win64
//===========================================================================


// ===== Software tick gating =====
#define TICKS_PER_STEP     (400u)   // tune 200–1000

// ===== Duty settings (5-phase dithering) =====
#define DUTY_PERIOD        (5u)

#define DUTY_FWD_ON        (4u)     // 80%
#define DUTY_ARC_OUT_ON    (4u)     // 80%
#define DUTY_ARC_IN_ON     (1u)     // 20%
#define DUTY_PIVOT_ON      (4u)     // 80%

static unsigned char DutyOn(unsigned char phase, unsigned char onTicks)
{
    return (phase < onTicks);
}



#define motor_left (-1)
#define motor_right (1)
#define motor_forward (1)
#define motor_reverse (-1)
#define motor_off (0)



//1 = forward, 0 = off, -1 = reverse
void ChangeFullDirection(int dir);
int CommandCoordinator(const char *newCmdString);
static void WheelCommand_Tick(char cmd, unsigned char dutyPhase);
void ChangeFullDirection(int dir);

