/**********************************************************
**0000000000000000000000000000000000000000000000000000000**
**00                                                   00**
**00               ����ai�Ӿ�һ�� ��д                 00**
**00                  ���ߣ�������                     00**
**00                                                   00**
**0000000000000000000000000000000000000000000000000000000**
**00            �������ã�  go                          00**
**0000000000000000000000000000000000000000000000000000000**
**********************************************************/
#include "aicar_gogogo.h"
#include "headfile.h"

void aicar_gogogo()
{
    DisableGlobalIRQ();
    board_init();           //��ر��������������ڳ�ʼ��MPU ʱ�� ���Դ���
    systick_delay_ms(1000);

    aicar_init();
    aicar_switch_get();
    pit_init();                     //��ʼ��pit����
    pit_interrupt_ms(PIT_CH0,10);  //��ʼ��pitͨ��0 ����
    NVIC_SetPriority(PIT_IRQn,5);  //�����ж����ȼ� ��Χ0-15 ԽС���ȼ�Խ�� ��·PIT����һ��PIT�жϺ���
    servo_duty=3850;
    kp_l=kp_motor_left;
    ki_l=ki_motor_left;
    kp_r=kp_motor_right;
    ki_r=ki_motor_right;
    EnableGlobalIRQ(0); //���ж������
    while(1)
    {
//        if(mt9v03x_csi_finish_flag)
//        {      
//            mt9v03x_csi_finish_flag = 0;
//            //ʹ��������ʾ����������ԭʼͼ���С �Լ�������Ҫ��ʾ�Ĵ�С�Զ��������Ż��߷Ŵ���ʾ
//            lcd_displayimage032_zoom(mt9v03x_csi_image[0], MT9V03X_CSI_W, MT9V03X_CSI_H, 160, 128);
//      
//        }
        aicar_key_get();
        lcd_showstr(0,0,"encoder1:");  
        lcd_showint16(12*8,0,encoder1);
        lcd_showstr(0,1,"encoder2:");  
        lcd_showint16(12*8,1,encoder2);

        lcd_showstr(0,2,"aim_speed:");  
        lcd_showint32(12*8,2,aim_speed, 5);
        
        lcd_showstr(0,3,"sw1_status:");  
        lcd_showuint8(12*8,3,sw1_status);
        lcd_showstr(0,4,"sw2_status:");  
        lcd_showuint8(12*8,4,sw2_status);
        
        lcd_showstr(0,5,"kp_l:");  
        lcd_showfloat(12*8,5,kp_l, 3,2);
        lcd_showstr(0,6,"ki_l:");  
        lcd_showfloat(12*8,6,ki_l, 3,2);
        lcd_showstr(0,7,"speed1:");  
        lcd_showfloat(12*8,7,speed_out1, 5,0);
    }
}