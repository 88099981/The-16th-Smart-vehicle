#ifndef _aicar_motor_h
#define _aicar_motor_h
#include "common.h"

#define DIR_1 D0
#define DIR_2 D1
#define PWM_1 PWM2_MODULE3_CHA_D2
#define PWM_2 PWM2_MODULE3_CHB_D3

#define kp_motor_left 7.40
#define ki_motor_left 1.40
#define kd_motor_left 0.00

#define kp_motor_right 0.00
#define ki_motor_right 0.00
#define kd_motor_right 0.00

extern float kp_l,kp_r,ki_l,ki_r,kd_l,kd_r;
extern float speed_out1,speed_out2;

void aicar_motor_init(void);
void aicar_motor_control(int32 motor1, int32 motor2);
void aicar_motor_pid(int16 aim_left, int16 aim_right);

#endif

