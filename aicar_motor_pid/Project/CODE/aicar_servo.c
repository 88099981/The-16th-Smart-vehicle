/**********************************************************
**0000000000000000000000000000000000000000000000000000000**
**00                                                   00**
**00               ����ai�Ӿ�һ�� ��д                 00**
**00                  ���ߣ�������                     00**
**00                                                   00**
**0000000000000000000000000000000000000000000000000000000**
**00            �������ã� servo                        00**
**0000000000000000000000000000000000000000000000000000000**
**********************************************************/
#include "aicar_init.h"
#include "aicar_servo.h"
#include "zf_pwm.h"

void aicar_servo_init(void)
{
    pwm_init(S_MOTOR1_PIN,50,1.5*50000/20);
}
void aicar_servo_control(uint32 duty)//ǰ�ֶ����3450-4250����ֵ3850
{
    if(duty<=0)
    {
        duty=0;
    }
    else if(duty>PWM_DUTY_MAX)
    {
        duty=PWM_DUTY_MAX;
    }
    pwm_duty(S_MOTOR1_PIN,duty);
}
void aicar_servo_holder(void)//��̨��1150-6350����ֵ3850
{
    
}