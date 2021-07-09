/**********************************************************
**0000000000000000000000000000000000000000000000000000000**
**00                                                   00**
**00               东秦ai视觉一队 编写                 00**
**00                  作者：刘坤龙                     00**
**00                                                   00**
**0000000000000000000000000000000000000000000000000000000**
**00            函数作用：  go                          00**
**0000000000000000000000000000000000000000000000000000000**
**********************************************************/
#include "aicar_element.h"
#include "aicar_init.h"
#include "aicar_adc.h"
#include "aicar_servo.h"
#include "fd_GetMid.h"
vuint8 ruhuan=0,chuhuan=0;
vuint8 chuhuan_delay=0;
vuint8 ruhuan_delay=0;
vuint8 ruhuan_turn=20;//打角时间

uint8 hd_in=HD_IN,hd_out=HD_OUT;
uint8 hd_in_delay=HD_IN_DELAY,hd_out_delay=HD_OUT_DELAY;
uint8 hd_turn=0;

uint8 uart_send=0;//1为云台转向完成，2为舵机转向完成, 3为等待完成， 4为打靶完成
int8 lasttime_holder=0;
uint16 holder_duty;

float kp_holder=10.0;
float kd_holder=5.0;

void aicar_huandao()//仅用作判断
{
    if(chuhuan_delay>0)
    {
        chuhuan_delay--;
    }
    if(ruhuan_delay>0)
    {
        ruhuan_delay--;
    }
    if(ruhuan_turn>0)
    {
        ruhuan_turn--;
    }
//    if(ruhuan_wait>0)//不入清零，防止对以后造成影响
//    {
//        ruhuan_wait--;
//        if(ruhuan_wait<=0)
//        {
//            zuo_yuanhuan_flag=0;
//            you_yuanhuan_flag=0;
//        }
//    }
    /*
    if(ad_value_all>400&&chuhuan_delay<=0)
    {
        if(zuo_yuanhuan_flag==0&&you_yuanhuan_flag==0)
        {
            ruhuan++;
            if(flag_Round_ARM_L>0&&ruhuan>=hd_in)//zuo
            {                                
                 zuo_yuanhuan_flag=1;
                 ruhuan=0;
                 ruhuan_delay=hd_in_delay;//防止入不了环
                 ruhuan_turn=hd_turn;
                 bb_time=20;              
            }
            else if(flag_Round_ARM_R>0&&ruhuan>=hd_in)//you
            {
                 you_yuanhuan_flag=1;
                 ruhuan=0;
                 ruhuan_delay=hd_in_delay;//防止入不了环
                 ruhuan_turn=hd_turn;
                 bb_time=20;
            }
        }
        else if(ruhuan_delay<=0)
        {
            chuhuan++;
            if(zuo_yuanhuan_flag==1&&chuhuan>=hd_out)
            {
                zuo_yuanhuan_flag=0;
                //camera_down=100;
                chuhuan=0;
                bb_time=60;
                chuhuan_delay=hd_out_delay;//防止二次入环
            }
            else if(you_yuanhuan_flag==1&&chuhuan>=hd_out)
            {
                you_yuanhuan_flag=0;
                //camera_down=100;
                chuhuan=0;
                bb_time=60;
                chuhuan_delay=hd_out_delay;
            }
        }
    }
    else
    {
        ruhuan=0;
        chuhuan=0;
    }
    */
//    if(x_move_time>0)
//    {
//        x_move_time--;
//        if(camera_down==1&&x_move_time==0)
//        {
//            camera_down==0;
//        }
//        //aim_x=100;
//
//    }

//    sqrt_left=sqrt(ad_left);
//    sqrt_right=sqrt(ad_right);

}


void aicar_right_garage_out()
{
    turn_sum=0;
    while(turn_sum>-18000)
    {
        servo_duty=3550;
        get_icm20602_gyro_spi();
        turn_sum+=icm_gyro_z;
        lcd_showstr(0,3,"turn_sum:");
        lcd_showint32(10*8,3,turn_sum,5);
        lcd_showstr(0,6,"icm_gyro_z:");
        lcd_showint16(10*8,6,icm_gyro_z);
        
        aicar_chasu();
    }
    lcd_clear(BLACK);
    turn_sum=0;
}


void aicar_left_garage_out()
{
    turn_sum=0;
    while(turn_sum<18000)
    {
        servo_duty=4150;
        get_icm20602_gyro_spi();
        turn_sum+=icm_gyro_z;
        lcd_showstr(0,3,"turn_sum:");
        lcd_showint32(10*8,3,turn_sum,5);
        lcd_showstr(0,6,"icm_gyro_z:");
        lcd_showint16(10*8,6,icm_gyro_z);
        
        aicar_chasu();
    }
    lcd_clear(BLACK);
    turn_sum=0;
}


void aicar_right_garage_in()
{
    turn_sum=0;
    while(turn_sum>-20000)
    {
        servo_duty=3650;
        get_icm20602_gyro_spi();
        turn_sum+=icm_gyro_z;
        lcd_showstr(0,3,"turn_sum:");
        lcd_showint32(10*8,3,turn_sum,5);
        lcd_showstr(0,6,"icm_gyro_z:");
        lcd_showint16(10*8,6,icm_gyro_z);
        
        aicar_chasu();
    }
    lcd_clear(BLACK);
    turn_sum=0;
    break_flag=1;
}


void aicar_left_garage_in()
{
    turn_sum=0;
    while(turn_sum<20000)
    {
        servo_duty=4050;
        get_icm20602_gyro_spi();
        turn_sum+=icm_gyro_z;
        lcd_showstr(0,3,"turn_sum:");
        lcd_showint32(10*8,3,turn_sum,5);
        lcd_showstr(0,6,"icm_gyro_z:");
        lcd_showint16(10*8,6,icm_gyro_z);
        
        aicar_chasu();
    }
    lcd_clear(BLACK);
    turn_sum=0;
    break_flag=1;
}


void holder_l_turn()
{
    holder_duty=5450;
    pwm_duty(S_MOTOR2_PIN,holder_duty);
    uart_send = 0x01;
    uart_putchar(USART_1,uart_send);
    use_time=0;
    uart_flag=E_START;
    systick_start();
    while(uart_flag!=E_OK&&use_time<3000)
    {
        use_time = systick_getval_ms();//等待
    }
    if(magic_mode)
    {
        switch (magic_data[2]){
        case ANIMAL: wait_animal();break;
        case FRUIT: shot_fruit();break;
        }
    }
    else if(uart_flag==E_OK) sort_animalorfruit();
    else
    {
        wait_animal();
    } 
        
}


void holder_r_turn()
{
    holder_duty=2250;
    pwm_duty(S_MOTOR2_PIN,holder_duty);
    uart_send = 0x01;
    uart_putchar(USART_1,uart_send);
    use_time=0;
    uart_flag=E_START;
    systick_start();
    while(uart_flag!=E_OK&&use_time<3000)
    {
        use_time = systick_getval_ms();//等待
    }
    if(magic_mode)
    {
        switch (magic_data[2]){
        case ANIMAL: wait_animal();break;
        case FRUIT: shot_fruit();break;
        }
    }
    else if(uart_flag==E_OK) sort_animalorfruit();
    else
    {
        wait_animal();
    } 
}


void servo_l_turn()
{
    turn_sum=0;
    while(turn_sum<10000)
    {
        servo_duty=4050;
        get_icm20602_gyro_spi();
        turn_sum+=icm_gyro_z;
        lcd_showstr(0,3,"turn_sum:");
        lcd_showint32(10*8,3,turn_sum,5);
        lcd_showstr(0,6,"icm_gyro_z:");
        lcd_showint16(10*8,6,icm_gyro_z);
        
        aicar_chasu();
    }
    uart_send = 0x02;
    uart_putchar(USART_1,uart_send);
    lcd_clear(BLACK);
    servo_duty=3850;
}
void servo_r_turn()
{
    break_flag=0;
    turn_sum=0;
    while(turn_sum>-10000)
    {
        servo_duty=3650;
        get_icm20602_gyro_spi();
        turn_sum+=icm_gyro_z;
        lcd_showstr(0,3,"turn_sum:");
        lcd_showint32(10*8,3,turn_sum,5);
        lcd_showstr(0,6,"icm_gyro_z:");
        lcd_showint16(10*8,6,icm_gyro_z);
        
        aicar_chasu();
    }
    uart_send = 0x02;
    uart_putchar(USART_1,uart_send);
    lcd_clear(BLACK);
    servo_duty=3850;
}


void wait_animal()
{
    use_time=0;
    systick_start();
    while(use_time<3000)
    {
        use_time = systick_getval_ms();//等待
    }
    uart_send = 0x03;
    uart_putchar(USART_1,uart_send);
    aicar_holder_control(3850);
    break_flag=0;
}


void shot_fruit()
{
    int8 error=0;
    int16 holder_angle=0;
    error=80-temp_buff[4];
    while(error<-5||error>5)
    {
        error=80-temp_buff[4];//pid
        holder_angle=(int16)(kp_holder*error + kd_holder*(error-lasttime_holder));
        lasttime_holder=error;   
        aicar_holder_control(holder_angle+holder_duty);
    }
    pwm_duty(S_MOTOR3_PIN,25000);
    uart_send = 0x04;
    uart_putchar(USART_1,uart_send);
    aicar_holder_control(3850);
    break_flag=0;
}


void sancha_stop()
{
    break_flag=1;
    uart_flag=E_START;
    use_time=0;
    systick_start();
    while(uart_flag!=E_OK&&use_time<3000)
    {
        use_time = systick_getval_ms();//等待
    }
    bb_time=12;
    if(magic_mode)
    {
        switch (magic_data[1]){
        case SERVO_LEFT: servo_l_turn();break;
        case SERVO_RIGHT: servo_r_turn();break;
        }
    }
    else if(uart_flag==E_OK)    data_analysis(temp_buff);
    else servo_l_turn();
}

void find_apriltag()
{
    break_flag=1;
    uart_flag=E_START;
    use_time=0;
    systick_start();
    while(uart_flag!=E_OK&&use_time<3000)
    {
        use_time = systick_getval_ms();//等待
    }
    bb_time=12;
    if(magic_mode)
    {
        switch (magic_data[0]){
        case HOLDER_LEFT: holder_l_turn();
        case HOLDER_RIGHT: holder_r_turn();
        }
    }
    else if(uart_flag==E_OK)    data_analysis(temp_buff);
    else
    {
       holder_r_turn();
    }
}

void sort_animalorfruit()
{
    if(temp_buff[3] == 0x05)    wait_animal();
    else if(temp_buff[3] == 0x06)   shot_fruit();
}