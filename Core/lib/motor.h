#ifndef __MOTOR_H__
#define __MOTOR_H__

#define motor_left   (-1)
#define motor_right  (1)

#define motor_forward (1)
#define motor_reverse (-1)
#define motor_off     (0)

void TurnOffMotors(void);
void ChangeFullDirection(int dir);
void ChangeWheelDirection(int wheel, int dir);

// tooling
void WheelCommand_Tick(char cmd);
int  CommandCoordinator(const char *newCmdString);

#endif
